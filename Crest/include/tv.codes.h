#pragma once
#include <Arduino.h>

struct IRCode {
    const char* brand;
    uint16_t khz;
    const uint16_t* rawData;
    uint16_t length;
};

// Raw structural timing data for common brands
const uint16_t sony_pwr[] PROGMEM = {2400, 600, 1200, 600, 600, 600, 1200, 600, 600, 600};
const uint16_t sam_pwr[] PROGMEM = {4500, 4500, 560, 1690, 560, 1690, 560, 560, 560, 560};
const uint16_t lg_pwr[] PROGMEM = {9000, 4500, 560, 560, 560, 1690, 560, 560, 560, 1690};

const IRCode tvDatabase[] = {
    {"Sony", 40, sony_pwr, sizeof(sony_pwr) / sizeof(uint16_t)},
    {"Samsung", 38, sam_pwr, sizeof(sam_pwr) / sizeof(uint16_t)},
    {"LG", 38, lg_pwr, sizeof(lg_pwr) / sizeof(uint16_t)}
};

const uint16_t TOTAL_TV_CODES = sizeof(tvDatabase) / sizeof(IRCode);
