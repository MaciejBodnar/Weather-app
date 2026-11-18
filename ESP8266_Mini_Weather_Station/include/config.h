
#pragma once

// =====================================
// WIFI - WPISZ SWOJE DANE
// =====================================

const char *WIFI_SSID = "PLAYD54A6E5";
const char *WIFI_PASSWORD = "yJ6jzfjkrfAt";

// =====================================
// LOKALIZACJA
// Przyklad: Warszawa
// =====================================

const char *CITY_NAME = "WARSZAWA";

const float LATITUDE = 52.2297;
const float LONGITUDE = 21.0122;

// =====================================
// EKRAN OLED SSD1306
// =====================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

// ESP8266 NodeMCU
// SDA = D2 = GPIO4
// SCL = D1 = GPIO5

#define SDA_PIN 4
#define SCL_PIN 5

// =====================================
// STREFA CZASOWA - POLSKA
// Automatyczna zmiana czasu
// letniego i zimowego
// =====================================

const char *TIME_ZONE =
    "CET-1CEST,M3.5.0,M10.5.0/3";

// =====================================
// AKTUALIZACJA DANYCH
// =====================================

// Pogoda co 10 minut
const unsigned long UPDATE_INTERVAL = 600000UL;

// Ponowna proba po bledzie: 30 sekund
const unsigned long RETRY_INTERVAL = 30000UL;

// Ponowne laczenie WiFi: 15 sekund
const unsigned long WIFI_RETRY_INTERVAL = 15000UL;

// Odswiezanie OLED: 1 sekunda
const unsigned long DISPLAY_INTERVAL = 1000UL;
