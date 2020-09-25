#include "contiki.h"
#include "node-id.h"
#include "core/sys/rtimer.h"
#include "dev/radio.h"
#include "lib/random.h"
#include "net/netstack.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#include "nd.h"
/*---------------------------------------------------------------------------*/
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

PROCESS(app_process, "Application process");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(app_process, ev, data)
{
  static struct etimer et;
  static struct rtimer rt;

  PROCESS_BEGIN();
  rtimer_init();
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
  nd_start(ND_BURST, &rcb);
  /* Do nothing else */
    //PROCESS_WAIT_EVENT();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
