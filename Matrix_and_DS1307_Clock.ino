// Use the DS1307 clock module
#define USE_DS1307 0
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"
#include <MD_DS1307.h>
#include <Wire.h>

// Use the DHT11 temp and humidity sensor
#include "dht.h"
dht DHT;
#define DHT11_PIN 2
  float h = 0;
  float t = 0;
  float f = 0;

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, 16);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define SPEED_TIME  80
#define PAUSE_TIME  10
#define MAX_MESG  20

// Global variables
char szTime[9];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";
char szsecond[4];    // ss

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F

char *mon2str(uint8_t mon, char *psz, uint8_t len)
// Get a label from PROGMEM into a char array
{
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  *psz = '\0';
  mon--;
  if (mon < 12)
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return(psz);
}

char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const char str[][10] PROGMEM =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };

  *psz = '\0';
  code--;
  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }

  return(psz);
}
void getTime(char *psz, bool f = true)
// Code for reading clock time
{
  sprintf(psz, "%02d%c%02d", RTC.h, (f ? ':' : ' '), RTC.m);
}
void getsecond(char *psz)
// Code for reading clock date
{
  char  szBuf[10];
  sprintf(psz, "%02d", RTC.s);
}
void getDate(char *psz)
// Code for reading clock date
{
  char  szBuf[10];
  sprintf(psz, "%d %s %04d", RTC.dd, mon2str(RTC.mm, szBuf, sizeof(szBuf)-1), RTC.yyyy);
}
void getTemperature()
{
DHT.read11(DHT11_PIN);
  h = DHT.humidity;
  t = DHT.temperature;
  // Read temperature as Fahrenheit
  f = (1.8 * DHT.temperature)+32;
}
void setup(void)
{
// initialise the LED display
  P.begin(4);
  // Set up zones for 4 halves of the display
  // Each zone gets a different font, making up the top
  // and bottom half of each letter
  P.setZone(0, 0, 1);
  P.setZone(1, 2, 7);
  P.setZone(2, 10, 15);
  P.setZone(3, 8, 9);
  
      P.setFont(0, NULL);
      P.setFont(1, BigFontLower);
      P.setFont(2, BigFontUpper);
      P.setFont(3, NULL);
  P.displayZoneText(0, szsecond, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(1, szTime, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(2, szTime, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(3, szMesg, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.addChar('$', degC);
  P.addChar('&', degF);
  RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
  RTC.control(DS1307_12H, DS1307_OFF);
    RTC.readTime();
}

void loop(void)
{
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher
  P.displayAnimate();
  if (P.getZoneStatus(3))
  {
    switch (display)
    {

      case 0: // day of week
        P.setTextEffect(3, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        dow2str(RTC.dow, szMesg, MAX_MESG);
        break;
        
      case 1:  // Calendar
        P.setTextEffect(3, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        getDate(szMesg);
        break;

      case 2: // Temperature deg F
        P.setTextEffect(3, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        getTemperature();
          dtostrf(t, 3, 1, szMesg);
          strcat(szMesg, "$");
        break;

      case 3: // Temperature deg F
        P.setTextEffect(3, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
          dtostrf(f, 3, 1, szMesg);
          strcat(szMesg, "&");
        break;

      case 4: // Relative Humidity
        P.setTextEffect(3, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display = 0;
          dtostrf(h, 3, 0, szMesg);
          strcat(szMesg, "% RH");
        break;
    }

    P.displayReset(3);
  }
  // Finally, adjust the time string if we have to
  if (millis() - lastTime >= 1000)
  {
    RTC.readTime();
    lastTime = millis();
    getsecond(szsecond);
    getTime(szTime, flasher);
    flasher = !flasher;
    P.displayReset(0);
    P.displayReset(1);
    P.displayReset(2);
  }
  }
