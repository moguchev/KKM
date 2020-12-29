// Leonid Moguchev (c) 2020
#pragma once
# include "pch.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

static unsigned char reverse_table[16] = {
  0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
  0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

uint8_t reverse_bits(uint8_t byte) {
    // Reverse the top and bottom nibble then swap them.
    return (reverse_table[byte & 0b1111] << 4) | reverse_table[byte >> 4];
}

uint16_t reverse_word(uint16_t word) {
    return ((reverse_bits(word & 0xFF) << 8) | reverse_bits(word >> 8));
}

uint16_t crc16_common(uint8_t* data, uint8_t len, uint16_t poly, uint16_t init,
    uint16_t doXor, bool refIn, bool refOut) {
    uint8_t y;
    uint16_t crc;

    crc = init;
    while (len--) {
        if (refIn)
            crc = ((uint16_t)reverse_bits(*data++) << 8) ^ crc;
        else
            crc = ((uint16_t)*data++ << 8) ^ crc;
        for (y = 0; y < 8; y++)  {
            if (crc & 0x8000)
                crc = (crc << 1) ^ poly;
            else
                crc = crc << 1;
        }
    }

    if (refOut)
        crc = reverse_word(crc);
    return (crc ^ doXor);
}

uint16_t crc16_ccitt(uint8_t* data, uint8_t len) {
    return crc16_common(data, len, 0x1021, 0xFFFF, 0x0000, false, false);
}