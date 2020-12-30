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
	uint32_t offset = addr & (Cache_L1_Block_Size - 1);
	uint8_t temp[BURST_LEN << 1];

	int start_address = read_cache1(addr);
	

	if(offset + len > Cache_L1_Block_Size){
		memcpy(temp, cache1[start_address].data + offset, Cache_L1_Block_Size - offset);
		int next_address = read_cache1(addr + Cache_L1_Block_Size - offset);
		memcpy(temp + (Cache_L1_Block_Size - offset), cache1[next_address].data, len - (Cache_L1_Block_Size - offset));
	}else{
		memcpy(temp, cache1[start_address].data + offset, len);
	}

	int zero = 0;
	uint32_t ans = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3));
	return ans;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	//dram_write(addr, len, data);
	write_cache1(addr,len,data);
}
hwaddr_t page_translate(lnaddr_t addr, size_t len) {
	if(cpu.CR0.protect_enable && cpu.CR0.paging) {
	//	printf("%x\n",addr);
		hwaddr_t tmpad;
		if((tmpad = readTLB(addr & 0xfffff000)) != -1) return (tmpad << 12) + (addr & 0xfff);
		PageDescriptor dir, page;
		uint32_t dir_offset = addr >> 22;
		uint32_t page_offset = ((addr >> 12) & 0x3ff);
		uint32_t offset = addr & 0xfff;
		dir.val = hwaddr_read((cpu.CR3.page_directory_base << 12) + (dir_offset << 2), 4);
		Assert(dir.p, "Invalid page. %x", addr);
		page.val = hwaddr_read((dir.addr << 12) + (page_offset << 2), 4);
		Assert(page.p, "Invalid page. %x", addr);
	//	hwaddr_t hwaddr = (page.base << 12) + offset;
		//Assert((hwaddr & 0xfff) + len == ((hwaddr + len) & 0xfff), "Fatal Error!!");
		writeTLB(addr & 0xfffff000, page.addr);
		return (page.addr << 12) + offset;
	} else {
		return addr;
	}
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	/*To Avoid Potential Errors (len = 1 + 3)*/
	assert(len == 1 || len == 2 || len == 4);
	hwaddr_t hwaddr = page_translate(addr, len); 
	return hwaddr_read(hwaddr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len == 4);
	hwaddr_t hwaddr = page_translate(addr, len); 
	hwaddr_write(hwaddr, len, data);
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg){
	if(cpu.CR0.protect_enable == 0){
		return addr;
	}else{
		return cpu.segment_reg[sreg].base+addr;
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr=seg_translate(addr,len,current_sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr=seg_translate(addr,len,current_sreg);
	lnaddr_write(lnaddr, len, data);
}

