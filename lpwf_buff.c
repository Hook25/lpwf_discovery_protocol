#include "lpwf_buff.h"

lb_chunk lb_shift_l(lb_chunk value, lb_chunk offset){
  return (value << offset) & (255 << offset);
}

lb_chunk lb_shift_r(lb_chunk value, lb_chunk offset){
  return (value >> offset) & (255 >> offset);
}

lb_chunk lb_get_value(lb_chunk *arr, lb_chunk offset){
  lb_chunk to_r = lb_shift_l(*arr, offset);
  to_r += lb_shift_r(*(arr+1), 8-offset);
  return to_r;
}

int lb_search(lb_chunk value, lb_chunk *arr, size_t a_size){
  for(int i = 0; i<a_size-1; i++){
    for(int offset = 0; offset<8; offset++){
      lb_chunk test = lb_get_value(arr + i, offset);
      if(test == value){
        return i*8 + offset;
      }
    }
  }
}