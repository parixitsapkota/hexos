#include <stdint.h>

uint8_t g_M;
uint64_t __attribute__((aligned(4096))) g_PML4[512] = {0};
uint64_t __attribute__((aligned(4096))) g_PDPT[512] = {0};
uint64_t __attribute__((aligned(4096))) g_PD[512] = {0};
uint64_t __attribute__((aligned(4096))) g_PT[512] = {0};
void calc_M(void);

void init_paging(void) {
  calc_M();
  // Zero out the tables to clear any random garbage data.
  for (uint32_t i = 0; i < 512; i++) {
    g_PML4[i] = 0;
    g_PDPT[i] = 0;
    g_PD[i] = 0;
    g_PT[i] = 0;
  }
}

void set_page_entry(uint8_t write, uint8_t cache, uint64_t addr, uint64_t *entry) {
  uint64_t max_phys_mask = ((1ULL << g_M) - 1);
  uint64_t page_aligned_mask = max_phys_mask & ~0xFFFULL;
  uint64_t clean_addr = addr & page_aligned_mask;
  *entry = clean_addr | ((uint64_t)(write & 1) << 1) | ((uint64_t)(cache & 1) << 3) | 0x01ULL;
}

void setup_paging(void) {
  set_page_entry(1, 0, (uint64_t)g_PDPT, &g_PML4[0]);
  set_page_entry(1, 0, (uint64_t)g_PDPT, &g_PML4[511]);

  set_page_entry(1, 0, (uint64_t)g_PD, &g_PDPT[0]);
  set_page_entry(1, 0, (uint64_t)g_PD, &g_PDPT[510]);

  uint64_t pd_entry = 0;
  for (uint32_t i = 0; i < 512; i++) {
    set_page_entry(1, 0, (4096 * i), &pd_entry);
    g_PD[i] = pd_entry | 0x80ULL;
  }
}
