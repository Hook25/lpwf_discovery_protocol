#ifndef LPWF_PACKET_PRIVATE
#define LPWF_PACKET_PRIVATE
#include "lpwf_packet.h"
crc_t lp_crc(packet_t *data);
bool lp_valid_packet(packet_t *data);
#endif