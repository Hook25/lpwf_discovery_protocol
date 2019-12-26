#include "lpwf_packet.h"
#include "lpwf_buff.h"
#include<stdio.h>
#include<stdbool.h>
#define expect(x) do{if(!(x)){return false;}}while(0)

bool test_valid_packet(){
  packet_t pkt;
  lp_build(&pkt, 130);
  lb_chunk buff[sizeof pkt];
  lp_to_buff(&pkt, buff, sizeof pkt);
  expect(lp_from_buff(&pkt, buff, sizeof buff));
  buff[0] = !buff[0];
  expect(!lp_from_buff(&pkt, buff, sizeof buff));
  return true;
}


int main(void){
  if(!test_valid_packet()){
    printf("Test valid packet failed\n");
  }

  return 0;
}
