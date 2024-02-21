#include <string.h>
#include "runtime_types.h"
#include "macros.h"

extern "C" {

__declspec(section ".init")
void *memcpy(void *RESTRICT dest, const void *RESTRICT src, size_t count);

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
