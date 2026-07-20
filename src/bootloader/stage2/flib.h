#ifndef _FLIB_H_
#define _FLIB_H_

#include <stdint.h>

#define TTY_WIDTH 80
#define TTY_HEIGHT 25

#undef NULL
#define NULL ((void *)0)

#define true 1
#define false 0

extern uint8_t *g_mem;

void bputc(char c);

void *fmemset(void *s, int c, uint64_t n);
void wordcpy32(void *dest, volatile const void *src, uint32_t words);
int fmemcmp(const void *s1, const void *s2, uint64_t n);

char *strchr(const char *s, int c);
int toupper(uint8_t c);

void *falloc(uint64_t bytes);
void ffree(void *memory);

#endif // _FLIB_H_
