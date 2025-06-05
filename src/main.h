//
// Created by Ico on 03/12/2021.
//

#ifndef SCD30_MAIN_H
#define SCD30_MAIN_H

#include <passwords.h>
#include <feeds.h>
#include <utilities.h>

#ifndef ESP8266
    #error "Source files targetting ESP8266 in a non ESP8266 project."
#endif

#define ON                  1
#define OFF                 0

#include <Arduino.h>
#include <Adafruit_SCD30.h>
#include <Wire.h>

#if USE_WIFI
    #define CONNECTION_RETRY_INTERVAL       5000
    #define CONNECTION_SCAN_INTERVAL_MIN    500
    #define CONNECTION_SCAN_INTERVAL_MAX    1500
    #include <ESP8266WiFi.h>

    #if USE_OTA
        #include <ArduinoOTA.h>
    #endif

    #if USE_MQTT
        #include <Adafruit_MQTT.h>
        #include <Adafruit_MQTT_Client.h>
    #endif

    #if USE_MODBUS
        #include <modbus.h>
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

    #if USE_DISPLAY_DELAY
        #define REFRESH_DISPLAY 3000
        #define REFRESH_RESTART 5000

        #define setupDisplayDelay()     unsigned long __display_delay = 0

        
        #define showDisplay()          do{unsigned long ms = millis(); \
            if(ms - __display_delay < REFRESH_DISPLAY) oled.display(); \
            else if(ms - __display_delay < REFRESH_RESTART) {clearDisplay(); oled.display(); }\
            else __display_delay = ms; }while(0)

        #define forceShowDisplay()      oled.display(); __display_delay = millis();
        
    #else
        #define showDisplay()           oled.display()
        #define forceShowDisplay()      showDisplay()
    #endif


    #define printDisplay(x)             oled.print(x)
    #define printfDisplay(x,y...)       oled.printf(x,y)
    #define printlnDisplay(x)           oled.println(x)

    #define rtlprintDisplay(x)          rtlprint(x)
    #define rtlprintfDisplay(x,y...)    rtlprintf(x,y)
    #define rtlprintlnDisplay(x)        rtlprintln(x)
    #define setDisplaySize(x)           oled.setTextSize(x)
    #define getDisplayCursorX()         oled.getCursorX()
    #define getDisplayCursorY()         oled.getCursorY()
    #define setDisplayCursor(x,y)       oled.setCursor(x,y)
    #define setDisplayCursorX(x)        oled.setCursor(x,getDisplayCursorY())
    #define setDisplayCursorY(y)        oled.setCursor(getDisplayCursorX(),y)
    #define clearDisplay()              oled.clearDisplay(); setDisplayCursor(0,0)

#endif

#if USE_SERIAL && USE_DISPLAY
    #define _print(x)                   printDisplay(x); printSerial(x)
    #define _printf(x,y...)             printfDisplay(x,y); printfSerial(x,y)
    #define _println(x)                 printlnDisplay(x); printlnSerial(x)
    #define _clear()                    clearDisplay()
    #define _show()                     showDisplay()
    #define _forceShow()                forceShowDisplay()
    #define _textSize(x)                setDisplaySize(x)
    #define _cursorX()                  getDisplayCursorX()
    #define _cursorY()                  getDisplayCursorY()
    #define _setCursor(x,y)             setDisplayCursor(x,y)
    #define _setCursorX(x)              setDisplayCursorX(x)
    #define _setCursorY(y)              setDisplayCursorY(y)
    #define _rtlprint(x)                rtlprintDisplay(x); printSerial(x)
    #define _rtlprintf(x,y...)          rtlprintfDisplay(x,y); printfSerial(x,y)
    #define _rtlprintln(x)              rtlprintlnDisplay(x); printlnSerial(x)
#elif USE_SERIAL
    #define _print(x)                   printSerial(x)
    #define _printf(x,y...)             printfSerial(x,y)
    #define _println(x)                 printlnSerial(x)
    #define _clear()
    #define _show()
    #define _forceShow()
    #define _textSize(x)
    #define _cursorX()
    #define _cursorY()
    #define _setCursor(x,y)
    #define _setCursorX(x)
    #define _setCursorY(y)
    #define _rtlprint(x)                printSerial(x)
    #define _rtlprintf(x,y...)          printfSerial(x,y)
    #define _rtlprintln(x)              printlnSerial(x)
#elif USE_DISPLAY
    #define _print(x)                   printDisplay(x)
    #define _printf(x,y...)             printfDisplay(x,y)
    #define _println(x)                 printlnDisplay(x)
    #define _clear()                    clearDisplay()
    #define _show()                     showDisplay()
    #define _forceShow()                forceShowDisplay()
    #define _textSize(x)                setDisplaySize(x)
    #define _cursorX()                  getDisplayCursorX()
    #define _cursorY()                  getDisplayCursorY()
    #define _setCursor(x,y)             setDisplayCursor(x,y)
    #define _setCursorX(x)              setDisplayCursorX(x)
    #define _setCursorY(y)              setDisplayCursorY(y)
    #define _rtlprint(x)                rtlprintDisplay(x)
    #define _rtlprintf(x,y...)          rtlprintfDisplay(x,y)
    #define _rtlprintln(x)              rtlprintlnDisplay(x)
#endif

#define SENSOR_NAME                 "Sensirion SCD30"
#define SENSOR_CO2_LEVEL_DANGER     2000
#define SENSOR_ALARM_DURATION       100

#define CHAR_DEGREE         (char)248
#define TEXT_ON             "ON"
#define TEXT_OFF            "OFF"
#define VERSION             "3.3"
#define VERSION_NAME        "v" VERSION

#define BUZZER              LED_BUILTIN_AUX

#define BUZZER_ON           LOW
#define BUZZER_OFF          HIGH

#define LED_ON              LOW
#define LED_OFF             HIGH

#define outSetup(x)         pinMode(x,OUTPUT)
#define ledBuiltinSetup()   outSetup(LED_BUILTIN)
#define out(x,y)            digitalWrite(x,y)
#define ledBuiltin(x)       out(LED_BUILTIN,x)

#if USE_ALARM
#define buzzerSetup()       outSetup(BUZZER)
#define buzzer(x)           out(BUZZER,x)
#endif

#define _restart()           delay(500); ESP.restart();

#if USE_WIFI

    #define WIFI_SSID_MAX       32
    #define WIFI_PASSWORD_MAX   64

    const char MEM_DEFAULT_SSID[] PROGMEM =     DEFAULT_SSID;
    const char MEM_WIFI01_SSID[] PROGMEM =      WIFI01_SSID;
    const char MEM_WIFI02_SSID[] PROGMEM =      WIFI02_SSID;

    const char MEM_DEFAULT_PASSWORD[] PROGMEM = DEFAULT_PASSWORD;
    const char MEM_WIFI01_PASSWORD[] PROGMEM =  WIFI01_PASSWORD;
    const char MEM_WIFI02_PASSWORD[] PROGMEM =  WIFI02_PASSWORD;

    const char* const WIFI_SSIDS[] PROGMEM =        {MEM_DEFAULT_SSID,MEM_WIFI01_SSID,MEM_WIFI02_SSID};
    const char* const WIFI_PASSWORDS[] PROGMEM =    {MEM_DEFAULT_PASSWORD,MEM_WIFI01_PASSWORD,MEM_WIFI02_PASSWORD};

    #define WIFI_COUNT  3

    #if USE_MQTT

        #define AIO_SERVER      "homeassistant.local"

        #if USE_SSL
            #define AIO_SERVERPORT  8883
        #else
            #define AIO_SERVERPORT  1883
        #endif


    #endif

#endif

#if USE_SERIAL || USE_DISPLAY
    void rtlprintf(const char* format, ...);
    void rtlprint(const char* string);
    void rtlprintln(const char* string);
#endif

#if USE_WIFI
    void scanResult(int foundCount);
#endif

#endif //SCD30_MAIN_H