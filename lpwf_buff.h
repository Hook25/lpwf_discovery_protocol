#ifndef LPWF_BUFF
#define LPWF_BUFF

#include<stdio.h>

typedef unsigned char lb_chunk;

lb_chunk lb_shift_l(lb_chunk value, lb_chunk offset);
lb_chunk lb_shift_r(lb_chunk value, lb_chunk offset);
lb_chunk lb_get_value(lb_chunk *arr, lb_chunk offset);
int lb_search(lb_chunk value, lb_chunk *arr, size_t a_size);
#endif