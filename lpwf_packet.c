#include "lpwf_packet.h"
#include "lpwf_packet_private.h"
#include "assert.h"
#include "string.h"

#define PKT_BLK_ID 170 //10101010

crc_t lp_crc(packet_t *data){
  return data->id ^ data->data;
}

bool lp_valid_packet(packet_t *data){
  return lp_crc(data) == data->crc;
}

void lp_to_buff(packet_t *data, lb_chunk *dest, size_t buff_size){
  assert(sizeof *data <= buff_size);
  memcpy(dest, data, sizeof *data);
}

void lp_build(packet_t *pkt, data_t data){
  pkt->data = data;
  pkt->id = PKT_BLK_ID;
  pkt->crc = lp_crc(pkt);
}

bool lp_from_buff(packet_t *data, lb_chunk *buff, size_t buff_size){
  assert(buff_size <= sizeof *data);
  memcpy(data, buff, buff_size);
  return lp_valid_packet(data);
}