#ifndef LPWF_PACKET
#define LPWF_PACKET

#include<stdbool.h>
#include<stddef.h>

typedef char lpwf_id;
typedef char lpwf_check;

typedef struct {
  lpwf_id id;
  lpwf_check check;
} lpwf_packet;

bool lpwf_get_id(void *data, size_t data_l, lpwf_id *id);
void lpwf_build_from_id(lpwf_packet *p, lpwf_id *id);

#endif
