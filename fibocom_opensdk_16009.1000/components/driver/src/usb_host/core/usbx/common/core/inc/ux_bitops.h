
#ifndef UX_BITOPS_H
#define UX_BITOPS_H

#define BIT(nr) (1UL << (nr))
#define BIT_ULL(nr) (1ULL << (nr))
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr) (1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr) ((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE 8

#define BITS_PER_LONG 32
#define BITS_PER_LONG_LONG 64
/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l)            \
    (((~0ULL) - (1ULL << (l)) + 1) & \
     (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

#define BITOP_WORD(nr) ((nr) / BITS_PER_LONG)
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)

/** 
 * BITS_TO_LONGS: caculate the number of long that one value
 * for example: 
 * 32 need 1 long
 * but, 33 need 2 long
 */
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define BITS_PER_BYTE 8
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define for_each_set_bit(bit, addr, size)        \
    for ((bit) = find_first_bit((addr), (size)); \
         (bit) < (size);                         \
         (bit) = find_next_bit((addr), (size), (bit) + 1))

/* same as for_each_set_bit() but use bit as value to start with */
#define for_each_set_bit_from(bit, addr, size)         \
    for ((bit) = find_next_bit((addr), (size), (bit)); \
         (bit) < (size);                               \
         (bit) = find_next_bit((addr), (size), (bit) + 1))

unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
                            unsigned long offset);

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                 unsigned long offset);

unsigned long find_first_bit(const unsigned long *addr, unsigned long size);

unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size);

#endif
