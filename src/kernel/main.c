#include <kernel.h>

#include "printf.h"

// Setup the kernel.
void __attribute__((section(".entry"))) kmain(BootInfo *b_info) {
  (void)b_info;

  printf("\n\nKernel loaded successfully!\n");
  printf("Info address: %p\n", (void*)b_info->g_framebuffer);

  for (;;) {
    asm volatile("\
      cli;\
      hlt\
      ");
  }
}
