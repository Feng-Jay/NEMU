#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <string.h>
#include "nemu.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	//return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	int l1_1st_line = read_cache1(addr);
	uint32_t offset = addr & (L1cache_block_size - 1);
	uint8_t ret[BURST_LEN << 1];
	if (offset + len > L1cache_block_size){
		int l1_2nd_line = read_cache1(addr + L1cache_block_size - offset);
		memcpy(ret,cache1[l1_1st_line].block + offset,L1cache_block_size - offset);
		memcpy(ret + L1cache_block_size - offset,cache1[l1_2nd_line].block,len - (L1cache_block_size - offset));
	}else {
		memcpy(ret,cache1[l1_1st_line].block + offset,len);
	}

	int tmp = 0;
	uint32_t ans = unalign_rw(ret + tmp, 4) & (~0u >> ((4 - len) << 3));
	return ans;

}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	//dram_write(addr, len, data);
	write_cache1(addr,len,data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

