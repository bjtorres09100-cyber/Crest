#pragma once

enum SystemState {
    STATE_MENU,
    STATE_WIFI,
    STATE_SUBGHZ,
    STATE_NRF,
    STATE_IR,
    STATE_BLE // Fixed: Adding the missing BLE state definition here
};

void initMenu();
void drawMenu();
void loopMenu();
