#include <kernel.h>
#include <stdarg.h>


// Setup the kernel.
void __attribute__((section(".entry"))) kmain(BootInfo *b_info) {
  (void)b_info;

  *((volatile char *)0xB8000) = 'H';
  *((volatile char *)0xB8002) = 'e';
  *((volatile char *)0xB8004) = 'l';
  *((volatile char *)0xB8006) = 'l';
  *((volatile char *)0xB8008) = 'o';
  *((volatile char *)0xB800A) = ' ';
  *((volatile char *)0xB800C) = ' ';
  *((volatile char *)0xB800E) = ' ';
  *((volatile char *)0xB8010) = ' ';

  for (;;) {
    asm volatile("\
      cli;\
      hlt\
      ");
  }
}
