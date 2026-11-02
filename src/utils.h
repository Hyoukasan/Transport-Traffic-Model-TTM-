#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

static inline uint64_t rdtsc (void) 
{
    uint32_t hi, lo;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));

    return (uint64_t)hi << 32 | lo;
}

#endif