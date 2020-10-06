/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/random.h"
#include "sys/rtimer.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "node-id.h"
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#include "nd.h"
#include "lpwf_packet.h"
#include "etimer.h"
/*---------------------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 0
#define VERBOSE 0
#define STDOUT_ASSERT 1
#endif

#include "lpwf_params.h"

#define T_WINDOW_L (LPWF_T_W_MS * (RTIMER_ARCH_SECOND / 1000))
#define T_SPACING (LPWF_T_S_MS * (RTIMER_ARCH_SECOND / 1000))
#define T_BEACON_COUNT (T_WINDOW_L / T_SPACING)
#define R_WINDOW_L (LPWF_R_W_PROP*T_WINDOW_L)
#define R_WINDOW_D (R_WINDOW_L / R_RADIO_ON)

PROCESS(nd_process, "ND algorithm process");

typedef struct {
  struct nd_callbacks app_cb;
  int beacon_count;
  int mode;
  int phase;
  int phase_count;
  int discovered;
  int discovered_ids[MAX_NBR];
  int epoch;
} env_t;

#if DEBUG
  #define PRINTF(...) printf(__VA_ARGS__)
  #define dbg_pc(x) printf(#x ": %d\n", x);
  #if VERBOSE
  void dbg_print_env_t(const env_t *e){
    PRINTF("{beacon_count : %d,mode : %d,phase : %d,"
          "phase_count : %d,discovered : %d,epoch : %d}\n",
          e->beacon_count, e->mode, e->phase, e->phase_count,
          e->discovered, e->epoch);
  }
  #else
  void dbg_print_env_t(const env_t *e){}
  #endif
#else
  #define dbg_pc(x) 
  #define PRINTF(...)
  void dbg_print_env_t(const env_t *e){}
#endif
#if STDOUT_ASSERT
  #warning assert is not asserting but printing fails!
  #define assert(x) if(!(x)){printf("[%s:%d]: " #x "\n", __FILE__, __LINE__);}
#else
  #include <assert.h> //not supported on all boards, still
#endif
#define POKE_ID 200
#define RECV_PENDING_ID 201

static env_t e;

bool ins_disc(int id){
  int i;
  assert(e.discovered < MAX_NBR);
  for(i = 0; i < e.discovered; i++){
    if(e.discovered_ids[i] == id){
      return false;
    }
  }
  e.discovered_ids[e.discovered] = id;
  e.discovered++;
  return true;
}

void act_recv(void) {
  int len = packetbuf_datalen();
  if(len < sizeof(lpwf_packet)){
    return;
  }
  void *data = packetbuf_dataptr();
  lpwf_id id;
  if(lpwf_get_id(data, sizeof(lpwf_packet), &id)){
    assert(e.app_cb.nd_new_nbr);
    if(ins_disc(id)){
      e.app_cb.nd_new_nbr(e.epoch, id);
    }
  }else{
    PRINTF("received corrupted packet\n");
  }
}

void nd_recv(void){
  assert(process_post(&nd_process, RECV_PENDING_ID, 0) == PROCESS_ERR_OK);
}

static void beacon(void){
  lpwf_packet to_send;
  memset(&to_send, 0, sizeof to_send);
  lpwf_build_from_id(&to_send, &node_id);
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(to_send));
  memcpy(packetbuf_dataptr(), &to_send, sizeof to_send);
  NETSTACK_RADIO.send(packetbuf_dataptr(), packetbuf_datalen());
}

static void reset_t_beacon_count(void){
  if(e.mode == ND_BURST){
    e.beacon_count = T_BEACON_COUNT;
  }else{
    e.beacon_count = 1;
  }
}

static int send_phase(void){
  if(e.beacon_count == 0){
    //send_phase is over
    reset_t_beacon_count();
    if(e.mode == ND_BURST){
      e.phase++;
    }
    e.phase_count++;
    return 0;
  }
  e.beacon_count--;
  beacon();
  if(e.mode == ND_BURST){
    return T_SPACING;
  }
  //else mode is ND_SCATTER
  return T_WINDOW_L;
}

static int recv_start(void){
  NETSTACK_RADIO.on();
  e.phase++;
  if(e.mode == ND_BURST){
    return R_WINDOW_D;
  } 
  //else mode is ND_SCATTER
  return R_WINDOW_L;
}

static int recv_end(void){
  NETSTACK_RADIO.off();
  e.phase_count++;
  if(e.mode == ND_BURST){
    e.phase --;
    return (R_WINDOW_L - R_WINDOW_D);
  }
  e.phase++;
  return 0;
}

static void init_p_arr(const uint8_t mode, int (*phase[])(void)){
  if(mode == ND_BURST){
    phase[0] = send_phase;
    phase[1] = recv_start;
    phase[2] = recv_end;
  }else{
    phase[0] = recv_start;
    phase[1] = recv_end;
    phase[2] = send_phase;
  } 
}

static void reset_nbr_ids(void){
  memset(&e.discovered_ids, 0, sizeof(e.discovered_ids));
}

static void epoch_end(void){
  e.app_cb.nd_epoch_end(e.epoch, e.discovered);
  e.epoch ++;
  e.discovered = 0;
  e.phase_count = 0;
  e.phase = 0;
  reset_t_beacon_count();
  reset_nbr_ids();
}

static void poke(struct rtimer *t, void *unused){
  PRINTF("NEXT POKE\n");
  if(process_post(&nd_process, POKE_ID, 0)){
    PRINTF("PROCESS_ERR_FULL\n"); 
  }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(nd_process, ev, data)
{
  static struct rtimer backoff_timer;
  static int (*phases[3])(void);
  static rtimer_clock_t eow;
  PROCESS_BEGIN();
  e.mode = *((uint8_t *)data);
  init_p_arr(e.mode, phases);
  reset_t_beacon_count();
  reset_nbr_ids();
  for(;;){
    dbg_print_env_t(&e);
    if(e.phase_count == EPOCH_DURATION_N){ //epoch is done
      epoch_end();
    }
    assert(e.phase < 3);
    rtimer_clock_t backoff = phases[e.phase]();
    if(backoff > 0){
      eow = RTIMER_NOW() + backoff;
      assert(
        rtimer_set(&backoff_timer, eow, 0, poke, 0) == RTIMER_OK
      );
      PROCESS_YIELD();
      while(ev != POKE_ID && eow < RTIMER_NOW()){ //recv_pending or other
        if(RTIMER_NOW() > RTIMER_TIME(&backoff_timer)){
          printf("%u > %u\n", RTIMER_NOW(), RTIMER_TIME(&backoff_timer));
        }
        if(ev == RECV_PENDING_ID){
          act_recv();
        }else{
          printf("EV: %d\n", ev);
        }
        PROCESS_YIELD(); 
      }
    }else{
      printf("SKIPPED TIMER\n");
    }
  }
  PROCESS_END();
}

void nd_start(uint8_t mode, const struct nd_callbacks *cb) {
  e.app_cb.nd_new_nbr = cb->nd_new_nbr;
  e.app_cb.nd_epoch_end = cb->nd_epoch_end;
  /* wont support threads */
  dbg_pc(EPOCH_DURATION_N);
  dbg_pc(R_WINDOW_L);
  dbg_pc(R_WINDOW_D);
  dbg_pc(T_WINDOW_L);
  dbg_pc(T_SPACING);
  assert(EPOCH_DURATION_N > 0 && R_WINDOW_L > 0 && R_WINDOW_D > 0
      && T_WINDOW_L > 0 && T_SPACING > 0);
  assert(mode == ND_BURST || mode == ND_SCATTER);
  process_start(&nd_process, &mode);
  assert(0);
}
/*---------------------------------------------------------------------------*/
