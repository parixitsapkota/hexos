#ifndef _INT_H_
#define _INT_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

#define true 1
#define false 0

#define NULL ((void *)0)

#define PACKED __attribute__((packed))

#endif // _INT_H_
