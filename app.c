#include "contiki.h"
#include "node-id.h"
#include "dev/radio.h"
#include "lib/random.h"
#include "net/netstack.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#include "nd.h"
/*---------------------------------------------------------------------------*/
#define ACTION_RANGE 4

#ifdef TX_BURST
#define BURST tx_burst
#define SPARSE rx_sparse
#else
#define BURST rx_burst
#define SPARSE tx_sparse
#endif

typedef char action_t;

static void
nd_new_nbr_cb(uint16_t epoch, uint8_t nbr_id)
{
  printf("App: Epoch %u New NBR %u\n",
    epoch, nbr_id);
}
/*---------------------------------------------------------------------------*/
static void
nd_epoch_end_cb(uint16_t epoch, uint8_t num_nbr)
{
  printf("App: Epoch %u finished Num NBR %u\n",
    epoch, num_nbr);
}
/*---------------------------------------------------------------------------*/
struct nd_callbacks rcb = {
  .nd_new_nbr = nd_new_nbr_cb,
  .nd_epoch_end = nd_epoch_end_cb};
/*---------------------------------------------------------------------------*/
#if TX_BURST
int rx_sparse(void){
  return 0;
}
int tx_burst(void){
  return 0;
}
#else
int tx_sparse(void){
  return 0;
}
int rx_burst(void){
  return 0;
}
#endif

PROCESS(app_process, "Application process");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(app_process, ev, data)
{
  static struct etimer et;
  static action_t _sparse;

  PROCESS_BEGIN();

  /* Initialization */
  printf("Node ID: %u\n", node_id);
  printf("RTIMER_SECOND: %u\n", RTIMER_SECOND);

  /* Begin with radio off */
  NETSTACK_RADIO.off();

  /* Configure radio filtering */
  NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, 0);

  /* Wait at the beginning a random time to de-synchronize node start */
  etimer_set(&et, random_rand() % CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  /* Start ND Primitive */
  //nd_start(ND_BURST, &rcb);
  /* nd_start(ND_SCATTER, &rcb); */

  /* Do nothing else */
  while (1) {
    //PROCESS_WAIT_EVENT();
    _sparse = (_sparse + 1) % ACTION_RANGE;
    int err = 0;
    if(_sparse){
      err = SPARSE();
    }else{
      printf("Epoch\n");
      err = BURST();
    }
    if(err){
      printf("Failed on action %d, err: %d\n", _sparse, err);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
