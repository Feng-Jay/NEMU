#ifndef __CACHE_H_
#define __CACHE_H_
#include "common.h"

#define L1cache_Size 65536
#define L1cache_block_size 64   /*one line's size*/

#define L1cache_way_number 8    /*one group have eight lines*/
#define L1cache_group_number 128

#define L1cache_bit_blockoffset 6/*64=2^6*/
#define L1cache_bit_group 7      /*128=2^7*/
#define L1cache_bit_way 3        /*8=2^3*/

typedef struct{
    uint8_t block[L1cache_block_size];
    uint32_t tag;
    bool valid;
}L1cache;

L1cache cache1[L1cache_Size/L1cache_block_size];


/*---------next is L2 cache-----------*/
#define L2cache_Size 4*1024*1024
#define L2cache_block_size 64   /*one line's size*/

#define L2cache_way_number 16   /*1 group have 16 lines*/
#define L2cache_group_number 4096

#define L2cache_bit_blockoffset 6/*64=2^6*/
#define L2cache_bit_group 12      /*4096=2^12*/
#define L2cache_bit_way 4        /*16=2^4*/

typedef struct {
    uint8_t block[L2cache_block_size];
    uint32_t tag;
    bool valid;
    bool dirty;
}L2cache;

L2cache cache2[L2cache_Size/L2cache_block_size];

void ini_cache();
int read_cache2(hwaddr_t address);
int read_cache1(hwaddr_t address);
void write_cache2(hwaddr_t addr, size_t len, uint32_t data);
void write_cache1(hwaddr_t addr, size_t len, uint32_t data);
#endif