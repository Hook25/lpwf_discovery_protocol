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
#include <assert.h>
/*---------------------------------------------------------------------------*/
#include "nd.h"
#include "lpwf_packet.h"
#include "etimer.h"
/*---------------------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 1
#endif
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define T_WINDOW_L (30 * (CLOCK_SECOND / 100))
#define T_SPACING (1 * (CLOCK_SECOND / 100))
#define T_BEACON_COUNT (T_WINDOW_L / T_SPACING)
#define R_WINDOW_L (2*T_WINDOW_L)
#define R_WINDOW_D = (R_WINDOW_L / 2)
#define EPOCH_DURATION_N 2

static struct nd_callbacks app_cb = {
  .nd_new_nbr = NULL,
  .nd_epoch_end = NULL
}; //consider moving this

void nd_recv(void) {
  int len = packetbuf_datalen();
  void *data = packetbuf_dataptr();
  lpwf_id id;
  if(lpwf_get_id(data, len, &id)){
    assert(app_cb.nd_new_nbr);
    app_cb.nd_new_nbr(0, id);
  }else{
    printf("received corrupted packet\n");
  }
}

static void beacon(void){
  lpwf_packet to_send;
  memset(&to_send, 0, sizeof to_send);
  lpwf_build_from_id(&to_send, &node_id);
  packetbuf_clear();
  packetbuf_set_datalen(sizeof(lpwf_packet));
  memcpy(packetbuf_dataptr(), &to_send, sizeof to_send);
  NETSTACK_RADIO.send(packetbuf_dataptr(), packetbuf_datalen());
}

static int send_p(int mode, int *phase){
  static int beacon_count = -1;
  //static rtimer burst_timer; //TODO: work with this after
  if(beacon_count == -1){
    beacon_count = T_BEACON_COUNT + 1;
  }else if(beacon_count == 0){
    beacon_count = T_BEACON_COUNT;
    (*phase)++;
    return 0;
  }
  beacon_count--;
  beacon();
  if(mode == ND_BURST){
    return T_SPACING;
  }
  //else mode is ND_SCATTER
  return T_WINDOW_L;
}

static int recv_p(int mode, int *phase){
  static uint8_t on;  //leads to bug on non even cycles, fix
  (*phase)++;
  if(on){
    NETSTACK_RADIO.off();
  }else{
    NETSTACK_RADIO.on();
  }
  on = !on;
  if(mode == ND_BURST){
    if(on){
      return R_WINDOW_D;
    }
    return R_WINDOW_L - R_WINDOW_D;
  } 
  //else mode is ND_SCATTER
  return R_WINDOW_L;
}

static void init_p_arr(const uint8_t mode, int (*phase[])(int, int*)){
  if(mode == ND_BURST){
    phase[0] = send_p;
    phase[1] = recv_p;
  }else{
    phase[0] = recv_p;
    phase[1] = send_p;
  } 
}

static void epoch_end(uint8_t *epoch, uint8_t *phase_i){
  NETSTACK_RADIO.off();
  app_cb.nd_epoch_end(*epoch, 0);
  (*epoch) ++;
  (*phase_i) = 0; 
}

/*---------------------------------------------------------------------------*/
PROCESS(nd_process, "ND algorithm process");
PROCESS_THREAD(nd_process, ev, data)
{
  static struct etimer backoff_timer;
  static uint8_t epoch, mode;
  static int phase_i;
  static int (*phase[2])(int, int*);
  PROCESS_BEGIN();
  mode = *((uint8_t *)data);
  init_p_arr(mode, phase);
  for(;;){
    if(phase_i == EPOCH_DURATION_N){ //epoch is done
      epoch_end(&epoch, &phase_i);
      //cancel backoff timer?
    }
    int backoff = phase[phase_i == 0 ? 0 : 1](mode, &phase_i);
    etimer_set(&backoff_timer, backoff);
    PROCESS_WAIT_UNTIL(etimer_expired(&backoff_timer));
  }
  PROCESS_END();
}

void nd_start(uint8_t mode, const struct nd_callbacks *cb) {
  app_cb.nd_new_nbr = cb->nd_new_nbr;
  app_cb.nd_epoch_end = cb->nd_epoch_end;
  /* wont support threads */
  assert(mode == ND_BURST || mode == ND_SCATTER);
  process_start(&nd_process, &mode);
}
/*---------------------------------------------------------------------------*/
