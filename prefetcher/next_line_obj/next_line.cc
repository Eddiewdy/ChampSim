#include "cache.h"
#include <iostream>
void CACHE::prefetcher_initialize() {}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint64_t objectID, uint8_t cache_hit, bool useful_prefetch, uint8_t type, uint32_t metadata_in)
{
  // std::cout << "prefetcher_cache_operate IP: " << std::hex << ip << std::dec << " v_address: " << std::hex << addr << " SIZE: " << std::dec << size << std::endl;
  uint64_t pf_addr = addr + (1 << LOG2_BLOCK_SIZE);
  prefetch_line(pf_addr, true, metadata_in);
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_cycle_operate() {}

void CACHE::prefetcher_final_stats() {}
