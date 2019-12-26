#ifndef LPWF_PACKET
#define LPWF_PACKET
#include "lpwf_buff.h"
#include "stdbool.h"

typedef int data_t;
typedef int crc_t;
typedef unsigned char id_t;

#pragma pack(1)

typedef struct packet_tag{
  id_t id;
  data_t data;
  crc_t crc;
} packet_t;

#pragma pack()

void lp_build(packet_t *pkt, data_t data);
void lp_to_buff(packet_t *pkt, lb_chunk *dest, size_t buff_size);
bool lp_from_buff(packet_t *pkt, lb_chunk *buff, size_t buff_size);

#endif