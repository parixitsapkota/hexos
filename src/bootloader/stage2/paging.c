#include <int.h>

extern u8 g_M;

void calc_M(void);

u64 __attribute__((aligned(4096))) g_PML4[512] = {0};
u64 __attribute__((aligned(4096))) g_PDPT[512] = {0};
u64 __attribute__((aligned(4096))) g_PD[512] = {0};
u64 __attribute__((aligned(4096))) g_PT[512] = {0};

void init_paging(void) {
  calc_M();
  // Zero out the tables to clear any random garbage data.
  for (u32 i = 0; i < 512; i++) {
    g_PML4[i] = 0;
    g_PDPT[i] = 0;
    g_PD[i] = 0;
    g_PT[i] = 0;
  }
}

void set_page_entry(u8 write, u8 cache, u64 addr, u64 *entry) {
  u64 max_phys_mask = ((1ULL << g_M) - 1);
  u64 page_aligned_mask = max_phys_mask & ~0xFFFULL;
  u64 clean_addr = addr & page_aligned_mask;
  *entry = clean_addr | ((u64)(write & 1) << 1) | ((u64)(cache & 1) << 3) | 0x01ULL;
}

void setup_paging(void) {
  set_page_entry(1, 0, (u64)g_PDPT, &g_PML4[0]);
  set_page_entry(1, 0, (u64)g_PDPT, &g_PML4[511]);

  set_page_entry(1, 0, (u64)g_PD, &g_PDPT[0]);
  set_page_entry(1, 0, (u64)g_PD, &g_PDPT[510]);

  u64 pd_entry = 0;
  for (u32 i = 0; i < 1; i++) {
    set_page_entry(1, 0, (4096 * i), &pd_entry);
    g_PD[i] = pd_entry | 0x80ULL;
  }
}
