#include "lpwf_packet.h"
#include<stdbool.h>
#include<assert.h>
#include<stddef.h>
#include<stdio.h>
#include<string.h>

bool lpwf_valid(lpwf_packet *p){
  return p;
}

void lpwf_build_from_data(lpwf_packet *p, void *data, size_t data_l){
  assert(p && data && data_l <= sizeof (*p));
  memcpy(p, data, data_l); 
}

void lpwf_calc_check(lpwf_packet *p){
  
}

void lpwf_build_from_id(lpwf_packet *p, lpwf_id *id){
  memcpy(&p->id, id, sizeof *id);
  lpwf_calc_check(p);
}

void lpwf_dbg_print(lpwf_packet *p){
  printf("{\n  id = %d;\n  check = %d;\n}\n", p->id, p->check);
}

bool lpwf_get_id(void *data, size_t data_l, lpwf_id *id){
  lpwf_packet tmp;
  lpwf_build_from_data(&tmp, data, data_l);
  if(lpwf_valid(&tmp)){
    memcpy(id, &tmp.id, sizeof tmp.id);
    return true;
  }
  return false;
}


