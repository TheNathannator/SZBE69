#include <string.h>
#include "runtime_types.h"
#include "macros.h"

extern "C" {

__declspec(section ".init")
void *memcpy(void *RESTRICT dest, const void *RESTRICT src, size_t count) {
    if (count == 0) {
        return dest;
    }

    if (src >= dest) {
        if (src == dest) {
            return dest;
        }

        if (count >= 128) {
            size_t srcAlign = (size_t)src & 7;
            size_t destAlign = (size_t)dest & 7;

            byte *byteDest = (byte *)dest - destAlign;

            __dcbt((void *)src, 0);

            if (!(destAlign ^ srcAlign)) {
                const byte *byteSrc = (const byte *)src;

                if (destAlign & 7) {
                    // Align to 8-byte boundary
                    size_t alignDist = 8 - (size_t)src & 7;
                    count -= alignDist;
                    if (alignDist != 0) {
                        do {
                            *++byteDest = *++byteSrc;
                        } while (--alignDist);
                    }
                }

                // Copy 8 bytes at a time, first in chunks of 32 bytes
                double* alignSrc = (double*)byteSrc;
                double* alignDest = (double*)byteDest;
                for (size_t alignCount = count / 32; alignCount != 0; alignCount--) {
                    double d0 = alignSrc[0];
                    double d1 = alignSrc[1];
                    double d2 = alignSrc[2];
                    double d3 = alignSrc[3];
                    alignSrc += 4;
                    alignDest[0] = d0;
                    alignDest[1] = d1;
                    alignDest[2] = d2;
                    alignDest[3] = d3;
                    alignDest += 4;
                }

                if ((count & 0x1f) == 0) {
                    return dest;
                }
            }
        } else {
        }
    }

    if (count > 20 && ((size_t)dest & 3) == ((size_t)src & 3)) {
        const byte *byteSrc = (const byte *)src;
        byte *byteDest = (byte *)dest;

        // Align to 4-byte boundary
        size_t bytesTilAligned = 4 - ((size_t)src & 3);
        size_t alignedCount = count - bytesTilAligned;
        for (count++; --bytesTilAligned;) {
            *byteDest++ = *byteSrc++;
        }

        const uint *alignSrc = (const uint *)byteSrc;
        uint *alignDest = (uint *)byteDest;
        size_t alignCount = count / 16;
        if (alignCount != 0) {
            do {
                alignDest[1] = alignSrc[1];
                alignDest[2] = alignSrc[2];
                alignDest[3] = alignSrc[3];
                alignDest[4] = alignSrc[4];
                alignDest[5] = alignSrc[5];
                alignDest[6] = alignSrc[6];
                alignDest[7] = alignSrc[7];
                alignDest[8] = alignSrc[8];
                alignDest += 8;
                alignSrc += 8;
            } while (--alignCount != 0);
        }
    }

    // const byte *p = (const byte*)src - count;
    // byte *q = (byte *)dest - count;
    // for (count++; --count;) {
    //     *--q = *--p;
    // }

    // const byte *p = (const byte*)src - 1;
    // byte *q = (byte *)dest - 1;
    // for (count++; --count;) {
    //     *++q = *++p;
    // }

    // const byte *p = (const byte *)src - 1;
    // byte *q = (byte *)dest - 1;

    // if (count < 128) {
    // } else {
    // }

    // for (count++; --count;) {
    //     *++q = *++p;
    // }

    return dest;
}

__declspec(section ".init")
static void __fill_mem(void *dest, int ch, size_t count) {
    ch &= 0xFF;
    byte *byteDest = (byte *)dest - 1;

    if (count >= 32) {
        // Align to 4-byte boundary
        size_t alignDist = ~(size_t)byteDest & 3;
        if (alignDist != 0) {
            count -= alignDist;
            while (alignDist--, *++byteDest = ch, alignDist)
                ;
        }

        // Copy value to upper bytes
        if (ch != 0) {
            ch |= (ch << 0x18) | (ch << 0x10) | (ch << 8);
        }

        // Copy 4 bytes at a time, first in chunks of 32 bytes
        uint *alignDest = (uint *)(byteDest - 3);
        size_t alignCount = count / 32;
        if (alignCount != 0) {
            do {
                alignDest[1] = ch;
                alignDest[2] = ch;
                alignDest[3] = ch;
                alignDest[4] = ch;
                alignDest[5] = ch;
                alignDest[6] = ch;
                alignDest[7] = ch;
                alignDest[8] = ch;
                alignDest += 8;
            } while (--alignCount != 0);
        }

        // Copy remaining 4-byte chunks
        alignCount = (count / 4) & 7;
        if (alignCount != 0) {
            do {
                *++alignDest = ch;
            } while (--alignCount != 0);
        }

        // Prepare for final byte-wise copy
        byteDest = (byte *)((size_t)alignDest + 3);
        count &= 3;
    }

    // Copy one byte at a time
    if (count != 0) {
        do {
            *++byteDest = ch;
        } while (--count);
    }
}

__declspec(section ".init")
void *memset(void *dest, int ch, size_t count) {
    __fill_mem(dest, ch, count);
    return dest;
}

size_t strlen(const char *str) {
    size_t len = -1;
    byte *p = (byte *)(str - 1);

    do {
        len++;
    } while (*++p);

    return len;
}

} // extern "C"
