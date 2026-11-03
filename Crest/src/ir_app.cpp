#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "ir_app.h"
#include "hal.h"
#include "menu.h"

// --- EMBEDDED DATABASE STRUCT ---
struct IRCode {
    const char* brand;
    uint16_t khz;
    const uint16_t* rawData;
    uint16_t length;
};

// Raw structural timing data for common brands stored directly in flash memory
const uint16_t sony_pwr[] PROGMEM = {2400, 600, 1200, 600, 600, 600, 1200, 600, 600, 600};
const uint16_t sam_pwr[] PROGMEM = {4500, 4500, 560, 1690, 560, 1690, 560, 560, 560, 560};
const uint16_t lg_pwr[] PROGMEM = {9000, 4500, 560, 560, 560, 1690, 560, 560, 560, 1690};

const IRCode tvDatabase[] = {
    {"Sony", 40, sony_pwr, sizeof(sony_pwr) / sizeof(uint16_t)},
    {"Samsung", 38, sam_pwr, sizeof(sam_pwr) / sizeof(uint16_t)},
    {"LG", 38, lg_pwr, sizeof(lg_pwr) / sizeof(uint16_t)}
};

const uint16_t TOTAL_TV_CODES = sizeof(tvDatabase) / sizeof(IRCode);

// --- END DATABASE ---

// Fixed: Changed from 'SystemState' to map correctly to main.cpp's initialization
extern SystemState currentState; 
extern void drawTextLine(int row, const char* text, bool inverted);
extern void clearCanvas();
extern void renderCanvas();

IRsend irTransmitter(IR_TX_PIN);

void loopIrApp() {
    clearCanvas();
    drawTextLine(0, "[ IR TV-B-Gone ]", false);
    drawTextLine(1, "Blasting database...", false);
    renderCanvas();
    
    irTransmitter.begin();

    for (uint16_t i = 0; i < TOTAL_TV_CODES; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Sending: %s", tvDatabase[i].brand);
        drawTextLine(2, buffer, false);
        renderCanvas();

        uint16_t codeLength = tvDatabase[i].length;
        uint16_t localRawData[codeLength];
        memcpy_P(localRawData, tvDatabase[i].rawData, codeLength * sizeof(uint16_t));

        irTransmitter.sendRaw(localRawData, codeLength, tvDatabase[i].khz);
        
        if (digitalRead(BTN_SEL) == LOW) break;
        delay(300);
    }

    delay(200);
    currentState = STATE_MENU;
    initMenu();
}
