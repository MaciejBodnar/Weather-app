
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <math.h>

#include "config.h"

// =====================================
// EKRAN OLED
// =====================================

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1);

// =====================================
// DANE POGODOWE
// =====================================

struct WeatherData
{
  float temperature = 0.0f;
  int humidity = 0;
  float windSpeed = 0.0f;
  int weatherCode = -1;
  bool valid = false;
};

WeatherData weather;

unsigned long lastWeatherAttempt = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastWifiAttempt = 0;

bool weatherError = false;
bool firstWeatherAttempt = true;

// =====================================
// KOMUNIKATY OLED
// =====================================

void showMessage(
    const String &line1,
    const String &line2 = "")
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  display.setCursor(0, 15);
  display.println(line1);

  display.setCursor(0, 30);
  display.println(line2);

  display.display();
}

// =====================================
// WIFI
// =====================================

void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  showMessage(
      "Laczenie z WiFi...",
      WIFI_SSID);

  Serial.print("Laczenie z WiFi");

  unsigned long start = millis();

  while (
      WiFi.status() != WL_CONNECTED &&
      millis() - start < 20000UL)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("WiFi polaczone!");
    Serial.println(WiFi.localIP());

    showMessage(
        "WiFi polaczone!",
        WiFi.localIP().toString());

    delay(1000);
  }
  else
  {
    Serial.println();
    Serial.println("Brak WiFi");

    showMessage(
        "Blad polaczenia",
        "Ponawiam...");
  }
}

// =====================================
// POBIERANIE DANYCH OPEN-METEO
// =====================================

bool fetchWeather()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Brak WiFi");
    return false;
  }

  Serial.println("Pobieranie pogody...");

  BearSSL::WiFiClientSecure client;

  // Prototyp: certyfikat TLS nie jest
  // weryfikowany.
  client.setInsecure();

  HTTPClient http;

  String url =
      "https://api.open-meteo.com/v1/forecast";

  url += "?latitude=";
  url += String(LATITUDE, 4);

  url += "&longitude=";
  url += String(LONGITUDE, 4);

  url += "&current=";
  url += "temperature_2m,";
  url += "relative_humidity_2m,";
  url += "wind_speed_10m,";
  url += "weather_code";

  url += "&wind_speed_unit=kmh";
  url += "&timezone=auto";

  http.setTimeout(12000);
  http.useHTTP10(true);

  if (!http.begin(client, url))
  {
    Serial.println("Blad HTTP begin");
    return false;
  }

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK)
  {
    Serial.print("Blad HTTP: ");
    Serial.println(httpCode);

    http.end();
    return false;
  }

  JsonDocument doc;

  DeserializationError error =
      deserializeJson(doc, http.getStream());

  http.end();

  if (error)
  {
    Serial.print("Blad JSON: ");
    Serial.println(error.c_str());
    return false;
  }

  JsonVariant current = doc["current"];

  if (
      current.isNull() ||
      current["temperature_2m"].isNull() ||
      current["relative_humidity_2m"].isNull() ||
      current["wind_speed_10m"].isNull() ||
      current["weather_code"].isNull())
  {
    Serial.println("Brak danych pogody");
    return false;
  }

  WeatherData newWeather;

  newWeather.temperature =
      current["temperature_2m"].as<float>();

  newWeather.humidity =
      current["relative_humidity_2m"].as<int>();

  newWeather.windSpeed =
      current["wind_speed_10m"].as<float>();

  newWeather.weatherCode =
      current["weather_code"].as<int>();

  newWeather.valid = true;

  // Aktualizacja dopiero po poprawnym
  // odczytaniu wszystkich danych.
  weather = newWeather;

  Serial.println("Pogoda pobrana!");

  Serial.print("Temperatura: ");
  Serial.println(weather.temperature);

  Serial.print("Wilgotnosc: ");
  Serial.println(weather.humidity);

  Serial.print("Wiatr km/h: ");
  Serial.println(weather.windSpeed);

  Serial.print("Kod pogody: ");
  Serial.println(weather.weatherCode);

  return true;
}

// =====================================
// OPIS POGODY
// =====================================

const char *getWeatherDescription(int code)
{
  if (code == 0)
    return "Slonecznie";

  if (code == 1 || code == 2)
    return "Lekkie chmury";

  if (code == 3)
    return "Pochmurno";

  if (code == 45 || code == 48)
    return "Mgla";

  if (code >= 51 && code <= 57)
    return "Mzawka";

  if (code >= 61 && code <= 67)
    return "Deszcz";

  if (code >= 71 && code <= 77)
    return "Snieg";

  if (code >= 80 && code <= 82)
    return "Opady deszczu";

  if (code == 85 || code == 86)
    return "Opady sniegu";

  if (code >= 95 && code <= 99)
    return "Burza";

  return "Inna pogoda";
}

// =====================================
// IKONA SLONCA
// =====================================

void drawSun(int x, int y)
{
  display.drawCircle(
      x, y, 6, SSD1306_WHITE);

  for (int i = 0; i < 8; i++)
  {
    float angle = i * PI / 4.0f;

    int x1 = x + cos(angle) * 9;
    int y1 = y + sin(angle) * 9;

    int x2 = x + cos(angle) * 12;
    int y2 = y + sin(angle) * 12;

    display.drawLine(
        x1, y1, x2, y2,
        SSD1306_WHITE);
  }
}

// =====================================
// IKONA CHMURY
// =====================================

void drawCloud(int x, int y)
{
  display.fillCircle(
      x + 6, y + 10, 5,
      SSD1306_WHITE);

  display.fillCircle(
      x + 14, y + 7, 7,
      SSD1306_WHITE);

  display.fillCircle(
      x + 23, y + 11, 5,
      SSD1306_WHITE);

  display.fillRoundRect(
      x, y + 10, 29, 8, 3,
      SSD1306_WHITE);
}

// =====================================
// IKONA DESZCZU
// =====================================

void drawRain(int x, int y)
{
  drawCloud(x, y);

  for (int i = 0; i < 3; i++)
  {
    int px = x + 6 + i * 9;

    display.drawLine(
        px, y + 21,
        px - 2, y + 25,
        SSD1306_WHITE);
  }
}

// =====================================
// IKONA SNIEGU
// =====================================

void drawSnow(int x, int y)
{
  drawCloud(x, y);

  for (int i = 0; i < 3; i++)
  {
    int px = x + 6 + i * 9;
    int py = y + 23;

    display.drawLine(
        px - 2, py,
        px + 2, py,
        SSD1306_WHITE);

    display.drawLine(
        px, py - 2,
        px, py + 2,
        SSD1306_WHITE);
  }
}

// =====================================
// IKONA BURZY
// =====================================

void drawThunder(int x, int y)
{
  drawCloud(x, y);

  display.drawLine(
      x + 17, y + 18,
      x + 12, y + 24,
      SSD1306_WHITE);

  display.drawLine(
      x + 12, y + 24,
      x + 18, y + 24,
      SSD1306_WHITE);

  display.drawLine(
      x + 18, y + 24,
      x + 13, y + 29,
      SSD1306_WHITE);
}

// =====================================
// WYBOR IKONY POGODY
// =====================================

void drawWeatherIcon(int code)
{
  const int x = 98;
  const int y = 15;

  if (code == 0)
  {
    drawSun(112, 28);
  }
  else if (code == 1 || code == 2)
  {
    drawSun(112, 25);
    drawCloud(x, y + 7);
  }
  else if (code == 3)
  {
    drawCloud(x, y + 5);
  }
  else if (code == 45 || code == 48)
  {
    for (int i = 0; i < 4; i++)
    {
      display.drawLine(
          99, 19 + i * 6,
          125, 19 + i * 6,
          SSD1306_WHITE);
    }
  }
  else if (
      (code >= 71 && code <= 77) ||
      code == 85 || code == 86)
  {
    drawSnow(x, y);
  }
  else if (code >= 95 && code <= 99)
  {
    drawThunder(x, y);
  }
  else if (
      (code >= 51 && code <= 67) ||
      (code >= 80 && code <= 82))
  {
    drawRain(x, y);
  }
  else
  {
    drawCloud(x, y + 5);
  }
}

// =====================================
// ZEGAR NTP
// =====================================

String getClock()
{
  time_t now = time(nullptr);

  // Brak synchronizacji czasu
  if (now < 1700000000)
  {
    return "--:--";
  }

  struct tm timeInfo;
  localtime_r(&now, &timeInfo);

  char buffer[6];

  strftime(
      buffer,
      sizeof(buffer),
      "%H:%M",
      &timeInfo);

  return String(buffer);
}

// =====================================
// GLOWNY EKRAN OLED
// =====================================

void drawWeatherScreen()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.setTextSize(1);

  // Miasto
  display.setCursor(0, 0);
  display.print(CITY_NAME);

  // Godzina
  display.setCursor(98, 0);
  display.print(getClock());

  // Linia
  display.drawLine(
      0, 10, 127, 10,
      SSD1306_WHITE);

  // Brak danych pogodowych
  if (!weather.valid)
  {
    display.setCursor(0, 20);
    display.println("Brak danych pogody");

    display.setCursor(0, 35);
    display.println("Ponawiam pobieranie");

    display.setCursor(0, 55);

    if (WiFi.status() == WL_CONNECTED)
    {
      display.print("WiFi OK");
    }
    else
    {
      display.print("Brak WiFi");
    }

    display.display();
    return;
  }

  // Temperatura
  display.setTextSize(2);
  display.setCursor(0, 16);

  display.print(weather.temperature, 1);

  // Symbol stopnia
  int cursorX = display.getCursorX();

  display.drawCircle(
      cursorX + 3, 18, 2,
      SSD1306_WHITE);

  display.setCursor(cursorX + 8, 16);
  display.print("C");

  // Ikona pogody
  drawWeatherIcon(weather.weatherCode);

  // Opis pogody
  display.setTextSize(1);
  display.setCursor(0, 42);

  display.print(
      getWeatherDescription(
          weather.weatherCode));

  // Oddzielenie dolnego paska
  display.drawLine(
      0, 52, 127, 52,
      SSD1306_WHITE);

  // Wilgotnosc
  display.setCursor(0, 56);
  display.print("H:");
  display.print(weather.humidity);
  display.print("%");

  // Predkosc wiatru w km/h
  display.setCursor(55, 56);
  display.print("W:");
  display.print(weather.windSpeed, 0);

  // Stan danych / polaczenia
  display.setCursor(113, 56);

  if (
      WiFi.status() != WL_CONNECTED ||
      weatherError)
  {
    display.print("!");
  }
  else
  {
    display.print("*");
  }

  display.display();
}

// =====================================
// SETUP
// =====================================

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("ESP8266 Weather Station");

  // I2C: SDA GPIO4, SCL GPIO5
  Wire.begin(SDA_PIN, SCL_PIN);

  // Start ekranu
  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          OLED_ADDRESS))
  {
    Serial.println("Blad OLED!");

    while (true)
    {
      delay(1000);
    }
  }

  showMessage(
      "Weather Station",
      "Uruchamianie...");

  delay(1000);

  // Laczenie z siecia
  connectWiFi();

  // Czas polski przez NTP
  configTime(
      TIME_ZONE,
      "pool.ntp.org",
      "time.nist.gov");

  showMessage(
      "Weather Station",
      "Pobieram pogode...");

  // Pobranie wykona sie w loop()
  lastWifiAttempt = millis();

  drawWeatherScreen();
}

// =====================================
// LOOP
// =====================================

void loop()
{
  unsigned long now = millis();

  // Ponowne laczenie WiFi
  if (
      WiFi.status() != WL_CONNECTED &&
      now - lastWifiAttempt >=
          WIFI_RETRY_INTERVAL)
  {
    lastWifiAttempt = now;

    Serial.println(
        "Ponowne laczenie WiFi...");

    WiFi.reconnect();
  }

  // Aktualizacja danych pogodowych
  unsigned long interval =
      weatherError
          ? RETRY_INTERVAL
          : UPDATE_INTERVAL;

  if (
      WiFi.status() == WL_CONNECTED &&
      (firstWeatherAttempt ||
       now - lastWeatherAttempt >= interval))
  {
    firstWeatherAttempt = false;

    weatherError = !fetchWeather();

    lastWeatherAttempt = millis();

    // Odswiez natychmiast po pobraniu
    drawWeatherScreen();
  }

  // Aktualizacja zegara na ekranie
  if (
      millis() - lastDisplayUpdate >=
      DISPLAY_INTERVAL)
  {
    lastDisplayUpdate = millis();
    drawWeatherScreen();
  }

  delay(10);
}
