#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


void ini_cache
{
    /*set all the valid bit=0*/
    for(int i=0;i<(L1cache_Size/L1cache_block_size);i++)
    cache1[i].valid=0;
}

int read_cache(hwaddr_t address)
{
    uint32_t group_id=(address>>L1cache_bit_blockoffset)&(L1cache_group_number-1);
    uint32_t tag_id=(address >> (L1cache_bit_blockoffset+L1cache_bit_group));

    int group_address=group_id* L1cache_way_number;

    int i=0;
    for(i;i<group_address+L1cache_way_number;i++){
        if(cache1[i].valid==1&&cache1[i].tag==tag_id)
        {
            /*hit cache!*/
            return i;
        }
    }
    /*Not hit, we must replace one block*/
    

}