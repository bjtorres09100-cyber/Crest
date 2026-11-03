#include <Arduino.h>
#include <esp_wifi.h>
#include "wifi_app.h"
#include "hal.h"
#include "menu.h"

extern SystemState currentState;
extern void drawTextLine(int row, const char* text, bool inverted);
extern void clearCanvas();
extern void renderCanvas();

bool wifiRunning = false;

void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t* payload = pkt->payload;
    
    // Check if it's a beacon frame (0x80) safely using array notation
    if (payload[0] == 0x80) { 
        char macStr[32];
        // Safely parse out portions of the transmitter MAC address from the header
        snprintf(macStr, sizeof(macStr), "MAC: %02X:%02X:%02X:%02X", payload[10], payload[11], payload[12], payload[13]);
        drawTextLine(2, macStr, false);
        renderCanvas();
    }
}

void loopWifiApp() {
    if (!wifiRunning) {
        clearCanvas();
        drawTextLine(0, "[ Wi-Fi Sniffer ]", false);
        drawTextLine(1, "Scanning...", false);
        drawTextLine(4, "Press SEL to exit", false);
        renderCanvas();
        
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);
        esp_wifi_set_storage(WIFI_STORAGE_RAM);
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_start();
        esp_wifi_set_promiscuous(true);
        esp_wifi_set_promiscuous_rx_cb(&snifferCallback);
        
        // Fixed: Added WIFI_SECOND_CHAN_NONE as the required second argument
        esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
        
        wifiRunning = true;
    }

    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        esp_wifi_set_promiscuous(false);
        esp_wifi_stop();
        wifiRunning = false;
        currentState = STATE_MENU;
        initMenu();
    }
}
