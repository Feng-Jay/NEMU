#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void cache_ddr3_read(hwaddr_t addr, void* data);
/*while in the dram.c, these two part is static. But I am afraid to remove the "static"
key word. So I define two new int dram.c and state here to use them.*/
void cache_ddr3_write(hwaddr_t addr, void* data, uint8_t *mask);

void dram_write(hwaddr_t addr, size_t len, uint32_t data);
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
    used_time=0;
}

int read_cache2(hwaddr_t address)
{
    uint32_t group_id=(address>>L2cache_bit_blockoffset)&(L2cache_group_number-1);
    uint32_t tag_id=(address >> (L2cache_bit_blockoffset+L2cache_bit_group));

    int group_address=group_id*L2cache_way_number;

    int i;
    for(i=group_address;i<group_address+L2cache_way_number;i++){
        if(cache2[i].valid==1&&cache2[i].tag==tag_id)
        {
            /*hit cache2!*/
            #ifndef TEST
            used_time += 20;
            #endif   
            return i;
        }
    }

    /*not hit read date from RAM*/
    #ifndef TEST
    used_time += 200;
    #endif 
    srand(time(0));
    int victimblock=(rand()%L2cache_way_number)+group_address;

    /*if the victimblock is dirty, we must write back*/
    if(cache2[victimblock].valid==1&&cache2[victimblock].dirty==1){
        uint32_t block2add=(group_id<<L2cache_bit_blockoffset)|(cache2[victimblock].tag<<(L2cache_bit_blockoffset+L2cache_bit_group));
        uint8_t append[BURST_LEN << 1];
        memset(append,1,sizeof(append));
        for(i=0;i<(L2cache_Size/BURST_LEN);i++){
        cache_ddr3_write(block2add+ BURST_LEN * i, cache2[i].block + BURST_LEN * i,append);
        }
    }
    /*read from RAM*/
    for(i=0;i<(L2cache_Size/BURST_LEN);i++){
        cache_ddr3_read(group_address + BURST_LEN * i, cache2[i].block + BURST_LEN * i);
    }

    cache2[victimblock].dirty=0;
    cache2[victimblock].valid=1;
    cache2[victimblock].tag=tag_id;
    return victimblock;
}

int read_cache1(hwaddr_t address)
{
    uint32_t group_id=(address>>L1cache_bit_blockoffset)&(L1cache_group_number-1);
    uint32_t tag_id=(address >> (L1cache_bit_blockoffset+L1cache_bit_group));

    int group_address=group_id* L1cache_way_number;

    int i;
    for(i=group_address;i<group_address+L1cache_way_number;i++){
        if(cache1[i].valid==1&&cache1[i].tag==tag_id)
        {
            /*hit cache1!*/
            #ifndef TEST
            used_time += 2;
            #endif 
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

void write_cache2(hwaddr_t addr, size_t len, uint32_t data)/*write back starget!!!!*/
{
    uint32_t group=(addr>>L2cache_bit_blockoffset)&(L2cache_group_number-1);
    uint32_t tag=(addr>>(L2cache_bit_blockoffset+L2cache_bit_group));
    uint32_t offset=(addr&(L2cache_block_size-1));

    int start_add=group* L2cache_way_number;
    int i;
    for(i=start_add+0;i<start_add+L2cache_way_number;i++){
        /*write hit*/
        if(cache2[i].valid==1&&cache2[i].tag==tag)
        {
            cache2[i].dirty=1;
            if(offset+len>L2cache_block_size)
            {
                memcpy(cache2[i].block+offset,&data,L2cache_block_size-offset);
                write_cache2(addr+L2cache_block_size-offset, len-L2cache_block_size+offset,data>>(L2cache_block_size-offset));
            }
            else
            {
                memcpy(cache2[i].block+offset,&data,len);
            }
            return ;
        }
    }
     /*if not hit, you must read it from RAM then rewrite it.*/
     i=read_cache2(addr);
     cache2[i].dirty=1;
     memcpy(cache2[i].block+offset,&data,len);
}

/*easy code with write through*/
void write_cache1(hwaddr_t addr, size_t len, uint32_t data)
{
    uint32_t group=(addr>>L1cache_bit_blockoffset)&(L1cache_group_number-1);
    uint32_t tag=(addr>>(L1cache_bit_blockoffset+L1cache_bit_group));
    uint32_t offset=(addr&(L1cache_block_size-1));

    int start_add=group* L1cache_way_number;
    int i;
    for(i=start_add+0;i<start_add+L1cache_way_number;i++){
        /*hit*/
        if(cache1[i].valid==1&&cache1[i].tag==tag)
        {
            
            if(offset+len>L1cache_block_size)
            {
                /*write the ram*/
                dram_write(addr,L1cache_block_size - offset,data);
                memcpy(cache1[i].block+offset,&data,L1cache_block_size-offset);
                /*and L2 need refresh*/
                write_cache2(addr, L1cache_block_size-offset,data);
                /*write the left things*/
                write_cache1(addr+L1cache_block_size-offset, len-L1cache_block_size+offset,data>>(L1cache_block_size-offset));
            }
            else
            {
                dram_write(addr,len,data);
                memcpy(cache1[i].block+offset,&data,len);
                write_cache2(addr,len,data);
            }
            return ;
        }
    }
    /*not hit, then go to L2cache to look for*/
    write_cache2(addr,len,data);
}