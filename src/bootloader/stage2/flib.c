#include <int.h>
#include <stdarg.h>

#include "flib.h"

u8 g_tty_color = 0x0F;
volatile u16 *g_textbuffer = (volatile u16 *)0xB8000;
static volatile u8 *g_momory = (volatile u8 *)0x00F00000; // Random memory adr for fake allocator.

void bputc(u8 c) {
  static u16 c_col = 0;
  static u16 c_line = 0;

  if (c == '\n') {
    c_col = 0;
    c_line++;
    goto check_scroll;
  }

  if (c == '\r') {
    c_col = 0;
    return;
  }

  u16 c_index = (c_line * TTY_WIDTH) + c_col;
  g_textbuffer[c_index] = (u16)c | ((u16)g_tty_color << 8);

  c_col++;
  if (c_col >= TTY_WIDTH) {
    c_col = 0;
    c_line++;
  }

check_scroll:
  if (c_line >= TTY_HEIGHT) {
    u32 total_cells_to_move = (TTY_HEIGHT - 1) * TTY_WIDTH;
    wordcpy32((void *)g_textbuffer, (volatile const void *)(g_textbuffer + TTY_WIDTH),
              total_cells_to_move / 2);

    u16 blank_char = (u16)' ' | ((u16)g_tty_color << 8);

    u32 blank_doubleword = ((u32)blank_char << 16) | blank_char;
    u16 last_line_start = (TTY_HEIGHT - 1) * TTY_WIDTH;

    volatile u32 *last_line_ptr = (volatile u32 *)(g_textbuffer + last_line_start);
    for (u16 x = 0; x < TTY_WIDTH / 2; x++) {
      last_line_ptr[x] = blank_doubleword;
    }

    c_line = TTY_HEIGHT - 1;
  }
}

void bprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  for (const char *p = format; *p != '\0'; p++) {
    if (*p != '%') {
      bputc(*p);
      continue;
    }

    p++; // Skip '%'

    switch (*p) {
    case 's': {
      char *s = va_arg(args, char *);
      if (!s) {
        s = "(null)";
      }
      while (*s) {
        bputc(*s++);
      }
      break;
    }

    case '%': {
      bputc('%');
      break;
    }

    default: {
      bputc('%');
      bputc(*p);
      break;
    }
    }
  }

  va_end(args);
}

void *fmemset(void *s, int c, u64 n) {
  unsigned char *p = s;
  while (n--) {
    *p++ = (unsigned char)c;
  }
  return s;
}

int fmemcmp(const void *s1, const void *s2, u64 n) {
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

int toupper(u8 c) {
  if (c >= 'a' && c <= 'z') {
    return c - ('a' - 'A');
  }
  return c;
}

void wordcpy32(void *dest, volatile const void *src, u32 words) {
  asm volatile("cld;\n"
               "rep movsd;\n"
               : "+D"(dest), "+S"(src), "+c"(words)
               :
               : "memory");
}

// fake allocator.
void *falloc(u64 bytes) {
  u8 *mem = (u8 *)g_momory;
  g_momory = g_momory + bytes;
  return (void *)mem;
}

// fake free.
void ffree(void *memory) {
  (void)memory;
  return;
}
