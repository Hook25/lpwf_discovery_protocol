#include "lpwf_packet.h"
#include "lpwf_packet_private.h"
#include "assert.h"
#include "string.h"

#define PKT_BLK_ID 170 //10101010

crc_t lp_crc(packet_t *pkt){
  return pkt->id ^ pkt->data;
}

bool lp_valid_packet(packet_t *pkt){
  return lp_crc(pkt) == pkt->crc;
}

void lp_to_buff(packet_t *pkt, lb_chunk *dest, size_t buff_size){
  assert(sizeof *pkt <= buff_size);
  memcpy(dest, pkt, sizeof *pkt);
}

void lp_build(packet_t *pkt, data_t data){
  pkt->data = data;
  pkt->id = PKT_BLK_ID;
  pkt->crc = lp_crc(pkt);
}

bool lp_from_buff(packet_t *pkt, lb_chunk *buff, size_t buff_size){
  assert(buff_size <= sizeof *pkt);
  memcpy(pkt, buff, buff_size);
  return lp_valid_packet(pkt);
}