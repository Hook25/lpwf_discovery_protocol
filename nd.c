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
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
static struct nd_callbacks app_cb = {
  .nd_new_nbr = NULL,
  .nd_epoch_end = NULL
  };
/*---------------------------------------------------------------------------*/
void
nd_recv(void)
{
  int len = packetbuf_datalen();
  void *data = packetbuf_dataptr();
  lpwf_id id;
  if(lpwf_get_id(data, len, &id)){
    printf("%d was discovered\n", id);
    assert(app_cb.nd_new_nbr);
    app_cb.nd_new_nbr(0, id);
  }else{
    printf("received corrupted packet\n");
  }
  /* New packet received
   * 1. Read packet from packetbuf---packetbuf_dataptr()
   * 2. If a new neighbor is discovered within the epoch, notify the application
   */
}
/*---------------------------------------------------------------------------*/
void
nd_start(uint8_t mode, const struct nd_callbacks *cb)
{
  app_cb.nd_new_nbr = cb->nd_new_nbr;
  app_cb.nd_epoch_end = cb->nd_epoch_end;
  
}
/*---------------------------------------------------------------------------*/
