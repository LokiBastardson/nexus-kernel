#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char      int8_t;
typedef signed short     int16_t;
typedef signed int       int32_t;
typedef signed long long int64_t;

typedef uint32_t size_t;
typedef int32_t  ssize_t;
typedef int32_t  pid_t;
typedef uint32_t mode_t;
typedef uint32_t off_t;
typedef uint32_t ino_t;
typedef uint32_t dev_t;

typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;

#define NULL ((void *)0)

#define true  1
#define false 0
typedef int bool;

#define PACKED      __attribute__((packed))
#define ALIGNED(x)  __attribute__((aligned(x)))
#define NORETURN    __attribute__((noreturn))
#define UNUSED      __attribute__((unused))

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x18
#define USER_DS   0x20

#define PAGE_SIZE 4096

#endif
