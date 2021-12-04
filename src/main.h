//
// Created by Ico on 03/12/2021.
//

#ifndef SCD30_MAIN_H
#define SCD30_MAIN_H

#define ON                  1
#define OFF                 0

#include <Arduino.h>
#include <Adafruit_SCD30.h>
#include <Wire.h>

#if USE_WIFI
    #define CONNECTION_RETRY_COUNT      5
    #define CONNECTION_RETRY_INTERVAL   5000

    #if ESP8266
        #include <ESP8266WiFi.h>
    #else
        #include <WiFi.h>
    #endif

    #if USE_OTA
        #include <ArduinoOTA.h>
    #endif

    #if USE_MQTT
        #include <Adafruit_MQTT.h>
        #include <Adafruit_MQTT_Client.h>
    #endif

#endif

#if USE_SERIAL
    #define printSerial(x)          Serial.print(x)
    #define printfSerial(x,y...)    Serial.printf(x,y)
    #define printlnSerial(x)        Serial.println(x)
#endif

#if USE_DISPLAY
    #include <Adafruit_SSD1306.h>
    #define SCREEN_WIDTH            128
    #define SCREEN_HEIGHT           64

    #define printDisplay(x)                 oled.print(x)
    #define printfDisplay(format, args...)  oled.printf(format, args)
    #define printlnDisplay(x)               oled.println(x)
    #define showDisplay()                   oled.display()
    #define setDisplaySize(x)               oled.setTextSize(x)
    #define getDisplayCursorX()             oled.getCursorX()
    #define getDisplayCursorY()             oled.getCursorY()
    #define setDisplayCursor(x,y)           oled.setCursor(x,y)
    #define setDisplayCursorX(x)            oled.setCursor(x,getDisplayCursorY())
    #define setDisplayCursorY(y)            oled.setCursor(getDisplayCursorX(),y)
    #define clearDisplay()                  oled.clearDisplay(); setDisplayCursor(0,0)

#endif

#if USE_SERIAL && USE_DISPLAY
    #define _print(x)               printDisplay(x); printSerial(x)
    #define _printf(x,y...)         printfDisplay(x,y); printfSerial(x,y)
    #define _println(x)             printlnDisplay(x); printlnSerial(x)
    #define _clear()                clearDisplay()
    #define _show()                 showDisplay()
    #define textSize(x)             setDisplaySize(x)
    #define getCursorX()            getDisplayCursorX()
    #define getCursorY()            getDisplayCursorY()
    #define cursor(x,y)             setDisplayCursor(x,y)
    #define setCursorX(x)           setDisplayCursorX(x)
    #define setCursorY(y)           setDisplayCursorY(y)
#elif USE_SERIAL
    #define _print(x)               printSerial(x)
    #define _printf(x,y...)         printfSerial(x,y)
    #define _println(x)             printlnSerial(x)
    #define _clear()
    #define _show()
    #define textSize(x)
    #define getCursorX()
    #define getCursorY()
    #define cursor(x,y)
    #define setCursorX(x)
    #define setCursorY(y)
#elif USE_DISPLAY
    #define _print(x)               printDisplay(x)
    #define _printf(x,y...)         printfDisplay(x,y)
    #define _println(x)             printlnDisplay(x)
    #define _clear()                clearDisplay()
    #define _show()                 showDisplay()
    #define textSize(x)             setDisplaySize(x)
    #define getCursorX()            getDisplayCursorX()
    #define getCursorY()            getDisplayCursorY()
    #define cursorX(x)              setDisplayCursorX(x)
    #define cursorY(y)              setDisplayCursorY(y)
    #define cursor(x,y)             setDisplayCursor(x,y)
#endif

#define SENSOR_NAME                 "Sensirion SCD30"
#define SENSOR_CO2_LEVEL_DANGER     2000
#define SENSOR_ALARM_DURATION       200

#define CHAR_DEGREE         (char)248
#define TEXT_ON             "ON"
#define TEXT_OFF            "OFF"

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

#endif //SCD30_MAIN_H