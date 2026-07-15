#ifndef _FLIB_H_
#define _FLIB_H_

#include <int.h>

#define TTY_WIDTH 80
#define TTY_HEIGHT 25

extern u8 *g_mem;

void bputc(u8 c);
void bprintf(const char *format, ...);

void *fmemset(void *s, int c, u64 n);
void wordcpy32(void *dest, volatile const void *src, u32 words);
int fmemcmp(const void *s1, const void *s2, u64 n);

char *strchr(const char *s, int c);
int toupper(u8 c);

void *falloc(u64 bytes);
void ffree(void *memory);

#endif // _FLIB_H_
