#include "lpwf_packet.h"
#include "lpwf_buff.h"
#include<stdio.h>

int main(void){
  lb_chunk k [] = {42,42,191};
  lb_chunk result = lb_get_value(k+1, 2);
  printf("%ud\n", result);
  int whr = lb_search(170, k, 3);
  printf("Found at: %d\n", whr);
  packet_t test;
  lp_build(&test, 120);

  lb_chunk buff[sizeof test];
  lp_to_buff(&test, buff, sizeof buff);
  printf("Packet of size %d\n  id: %ud\n  data: %d\n  crc: %d\n", sizeof buff, buff[0], buff[1], buff[2]);

  return 0;
}
