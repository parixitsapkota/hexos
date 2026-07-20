#include <stdint.h>
#include <stdarg.h>

#include "flib.h"

uint8_t g_tty_color = 0x0F;
volatile uint16_t *g_textbuffer = (volatile uint16_t *)0xB8000;
static volatile uint8_t *g_momory = (volatile uint8_t *)0x00F00000; // Random memory adr for fake allocator.

void bputc(char c) {
  static uint16_t c_col = 0;
  static uint16_t c_line = 0;

  if (c == '\n') {
    c_col = 0;
    c_line++;
    goto check_scroll;
  }

  if (c == '\r') {
    c_col = 0;
    return;
  }

  uint16_t c_index = (c_line * TTY_WIDTH) + c_col;
  g_textbuffer[c_index] = (uint16_t)c | ((uint16_t)g_tty_color << 8);

  c_col++;
  if (c_col >= TTY_WIDTH) {
    c_col = 0;
    c_line++;
  }

check_scroll:
  if (c_line >= TTY_HEIGHT) {
    uint32_t total_cells_to_move = (TTY_HEIGHT - 1) * TTY_WIDTH;
    wordcpy32((void *)g_textbuffer, (volatile const void *)(g_textbuffer + TTY_WIDTH),
              total_cells_to_move / 2);

    uint16_t blank_char = (uint16_t)' ' | ((uint16_t)g_tty_color << 8);

    uint32_t blank_doubleword = ((uint32_t)blank_char << 16) | blank_char;
    uint16_t last_line_start = (TTY_HEIGHT - 1) * TTY_WIDTH;

    volatile uint32_t *last_line_ptr = (volatile uint32_t *)(g_textbuffer + last_line_start);
    for (uint16_t x = 0; x < TTY_WIDTH / 2; x++) {
      last_line_ptr[x] = blank_doubleword;
    }

    c_line = TTY_HEIGHT - 1;
  }
}

void *fmemset(void *s, int c, uint64_t n) {
  unsigned char *p = s;
  while (n--) {
    *p++ = (unsigned char)c;
  }
  return s;
}

int fmemcmp(const void *s1, const void *s2, uint64_t n) {
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;
  while (n--) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

char *strchr(const char *s, int c) {
  while (*s != '\0') {
    if (*s == (char)c) {
      return (char *)s;
    }
    s++;
  }
  if (c == '\0') {
    return (char *)s;
  }
  return NULL;
}

int toupper(uint8_t c) {
  if (c >= 'a' && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

void wordcpy32(void *dest, volatile const void *src, uint32_t words) {
  asm volatile("cld;\n"
               "rep movsd;\n"
               : "+D"(dest), "+S"(src), "+c"(words)
               :
               : "memory");
}

// fake allocator.
void *falloc(uint64_t bytes) {
  uint8_t *mem = (uint8_t *)g_momory;
  g_momory = g_momory + bytes;
  return (void *)mem;
}

// fake free.
void ffree(void *memory) {
  (void)memory;
  return;
}
