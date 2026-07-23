#include <uefi.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  
  printf("Hello, UEFI!\n");
  printf("\nPress any key to exit...\n");
  getchar();

  return 0;
}