//
// Created by Ico on 03/12/2021.
//

#ifndef SCD30_MAIN_H
#define SCD0_MAIN_H

#include <Arduino.h>

#ifdef USE_WIFI
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else //ESP32
#include <WiFi.h>
#endif
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#endif
#include <Adafruit_SCD30.h>
#include <Wire.h>
#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

#define SENSOR_NAME                 "Sensirion SCD30"
#define SENSOR_CO2_LEVEL_DANGER     2000
#define SENSOR_ALARM_DURATION       200

#define CHAR_DEGREE         (char)248

#define BUZZER              LED_BUILTIN_AUX

#define BUZZER_ON           LOW
#define BUZZER_OFF          HIGH

#define LED_ON              LOW
#define LED_OFF             HIGH

#define outSetup(x)         pinMode(x,OUTPUT)
#define ledBuiltinSetup()   outSetup(LED_BUILTIN)
#define buzzerSetup()       outSetup(BUZZER)
#define out(x,y)            digitalWrite(x,y)
#define ledBuiltin(x)       out(LED_BUILTIN,x)
#define buzzer(x)           out(BUZZER,x)

#ifdef USE_SERIAL
#define printSerial(x)          Serial.print(x)
#define printfSerial(x,y...)    Serial.printf(x,y)
#define printlnSerial(x)        Serial.println(x)
#endif

#ifdef USE_DISPLAY
#define printDisplay(x)         display.print(x)
#define printfDisplay(x,y...)   display.printf(x,y)
#define printlnDisplay(x)       display.println(x)
#define clearDisplay()          display.clearDisplay(); display.setCursor(0,0)
#define showDisplay()           display.display()
#define setDisplaySize(x)       display.setTextSize(x)
#endif

#if defined(USE_SERIAL) && defined(USE_DISPLAY)
#define print(x)                printDisplay(x); printSerial(x)
#define printf(x,y...)          printfDisplay(x,y); printfSerial(x,y)
#define println(x)              printlnDisplay(x); printlnSerial(x)
#define clear()                 clearDisplay()
#define show()                  showDisplay()
#define setSize(x)              setDisplaySize(x)
#elif defined(USE_SERIAL)
#define print(x)                printSerial(x)
#define printf(x,y...)          printfSerial(x,y)
#define println(x)              printlnSerial(x)
#define clear()
#define show()
#define setSize(x)
#elif defined(USE_DISPLAY)
#define print(x)                printDisplay(x)
#define printf(x,y...)          printfDisplay(x,y)
#define println(x)              printlnDisplay(x)
#define clear()                 clearDisplay()
#define show()                  showDisplay()
#define setSize(x)              setDisplaySize(x)
#endif

#endif //SCD30_MAIN_H
