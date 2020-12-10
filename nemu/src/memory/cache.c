#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


void ini_cache()
{
    /*set all the valid bit=0*/
    /*for cache1*/
    int i;
    for(i=0;i<(L1cache_Size/L1cache_block_size);i++)
    cache1[i].valid=0;

    /*cache2*/
    for(i=0;i<(L2cache_Size/L2cache_block_size);i++){
        cache2[i].valid=0;
        cache2[i].dirty=0;
    }

}

int read_cache2(hwaddr_t address)
{
    uint32_t group_id=(address>>L2cache_bit_blockoffset)&(L2cache_group_number-1);
    uint32_t tag_id=(address >> (L2cache_bit_blockoffset+L2cache_bit_group));

    int group_address=group_id*L2cache_way_number;

    int i;
    for(i=0;i<group_address+L2cache_way_number;i++){
        if(cache2[i].valid==1&&cache2[i].tag==tag_id)
        {
            /*hit cache2!*/
            return i;
        }
    }

    /*not hit read date from RAM*/
    srand(time(0));
    int victimblock=(rand()%L2cache_way_number)+group_address;

    /*if the victimblock is dirty, we must write back*/
    if(cache2[victimblock].valid==1&&cache2[victimblock].dirty==1){
        //uint32_t block2add=(group_id<<L2cache_bit_blockoffset)|(cache2[victimblock].tag<<(L2cache_bit_blockoffset+L2cache_bit_group));
        uint8_t append[BURST_LEN << 1];
        memset(append,1,sizeof(append));
        for(i=0;i<L2cache_Size/BURST_LEN;i++){
           // public_ddr3_write(block2add+ BURST_LEN * i, cache2[i].block + BURST_LEN * i,append);
        }
    }
return 0;
    /*read from memory*/
}

int read_cache1(hwaddr_t address)
{
    uint32_t group_id=(address>>L1cache_bit_blockoffset)&(L1cache_group_number-1);
    uint32_t tag_id=(address >> (L1cache_bit_blockoffset+L1cache_bit_group));

    int group_address=group_id* L1cache_way_number;

    int i;
    for(i=0;i<group_address+L1cache_way_number;i++){
        if(cache1[i].valid==1&&cache1[i].tag==tag_id)
        {
            /*hit cache1!*/
            return i;
        }
    }
    /*Not hit, we must replace one block */
    int newblock=read_cache2(address);
    srand(time(0));
    int victimblock=(rand()%L1cache_way_number)+group_address;
    /*cache1[victimblock].block=cache2[newblock].block;*/
    memcpy(cache1[victimblock].block,cache2[newblock].block,L1cache_block_size);
    cache1[victimblock].valid=1;
    cache1[victimblock].tag=tag_id;
    return victimblock;

}