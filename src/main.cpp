#define USE_SERIAL      OFF
#define USE_DISPLAY     ON
#define USE_WIFI        ON
#define USE_PEAP        OFF
#define USE_OTA         ON
#define USE_MQTT        ON
#define USE_SSL         ON
#define USE_ALARM       ON

#include <main.h>
#include <passwords.h>
#include <feeds.h>

Adafruit_SCD30  scd30;
#if USE_SERIAL || USE_DISPLAY || USE_ALARM
    unsigned long msAlarm = 0;
    #if USE_SERIAL || USE_DISPLAY
        int16_t x,y;
        #if USE_DISPLAY
            Adafruit_SSD1306 oled;
        #endif
    #endif
#endif
#if USE_WIFI && USE_MQTT
    unsigned long msPublish = 0;
    #if USE_SSL
        WiFiClientSecure client;
    #else
        WiFiClient client;
    #endif
    Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
    Adafruit_MQTT_Publish co2(&mqtt, FEED_CO2);
    Adafruit_MQTT_Publish temp(&mqtt, FEED_TEMPERATURE);
    Adafruit_MQTT_Publish hum(&mqtt, FEED_HUMIDITY);
#endif
bool first = true;


void setup(void)
{
    Wire.begin(4,5);

    #if USE_SERIAL
        Serial.begin(115200);
        _restart();
    #endif

    #if USE_DISPLAY
        oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
        oled.begin(SSD1306_SWITCHCAPVCC, 0x3c);
        oled.setTextColor(SSD1306_WHITE);
        oled.cp437(true);
    #endif

    #if USE_SERIAL || USE_DISPLAY
        _printf("WiFi: %s",USE_WIFI ? USE_PEAP ? TEXT_PEAP : TEXT_ON : TEXT_OFF);
        _rtlprint(VERSION_NAME);
        _setCursorY(_cursorY() + 1);
        _printf("\nOTA: %s\n",USE_OTA ? TEXT_ON : TEXT_OFF);
        _setCursorY(_cursorY() + 1);
        _printf("MQTT: %s\n",USE_MQTT ? TEXT_ON : TEXT_OFF);
        _setCursorY(_cursorY() + 1);
        _printf("SSL: %s\n",USE_SSL ? TEXT_ON : TEXT_OFF);

        _show();
        _clear();
    #endif

    #if USE_WIFI
        WiFi.mode(WIFI_STA);
        #if USE_PEAP
            struct station_config wifi_config{};
            strcpy((char*)wifi_config.ssid,WIFI_SSID);
            strcpy((char*)wifi_config.password,WIFI_USER_PASSWORD);
            wifi_station_set_config(&wifi_config);

            wifi_station_set_wpa2_enterprise_auth(true);

            wifi_station_clear_cert_key();
            wifi_station_clear_enterprise_ca_cert();
            wifi_station_clear_enterprise_identity();
            wifi_station_clear_enterprise_username();
            wifi_station_clear_enterprise_password();
            wifi_station_clear_enterprise_new_password();

            wifi_station_set_enterprise_identity((uint8*)WIFI_IDENTITY, strlen(WIFI_IDENTITY));
            wifi_station_set_enterprise_username((uint8*)WIFI_USER_NAME, strlen(WIFI_USER_NAME));
            wifi_station_set_enterprise_password((uint8*)WIFI_USER_PASSWORD, strlen((char*)WIFI_USER_PASSWORD));

            wifi_station_connect();
        #else
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        #endif
        uint8 attempts;
        for(attempts = 0; attempts < CONNECTION_RETRY_COUNT; attempts++)
        {
            delay(CONNECTION_RETRY_INTERVAL);
            if(WiFi.isConnected())
                break;
            else
            {
                WiFi.reconnect();
                #if USE_SERIAL || USE_DISPLAY
                    if(attempts == 0)
                        _printf("Connecting to %s\n", WIFI_SSID);
                    _printf("Attempt %i of %i...\n",attempts + 1,CONNECTION_RETRY_COUNT);
                    _show();
                #endif
            }
        }
        if(!WiFi.isConnected())
        {
            #if USE_SERIAL || USE_DISPLAY
                _print("Connection failed.\nRebooting...");
                _show();
            #endif
            _restart();
        }

        #if USE_OTA
            #if USE_SERIAL || USE_DISPLAY
                ArduinoOTA.onStart([]()
                {
                    _clear();
                    _printf("OTA update utility\n\nCurrent version: %s\n",VERSION);
                    _setCursorY(_cursorY() + 1);
                    oled.drawRect(_cursorX(),_cursorY(),SCREEN_WIDTH,10,SSD1306_WHITE);
                    _show();
                });
                ArduinoOTA.onEnd([]()
                {
                    _print("\n\nOTA update ended");
                    _show();
                });
                #if USE_DISPLAY
                    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                    {
                        oled.fillRect(_cursorX(), _cursorY(), progress * SCREEN_WIDTH / total, 10, SSD1306_WHITE);
                        _show();
                    });
                #endif
                ArduinoOTA.onError([](ota_error_t error)
                {
                    _printf("\n\nError %u: ",error);
                    switch (error)
                    {
                        case OTA_AUTH_ERROR:
                        {
                            _println("auth failed");
                            break;
                        }
                        case OTA_BEGIN_ERROR:
                        {
                            _println("begin failed");
                            break;
                        }
                        case OTA_CONNECT_ERROR:
                        {
                            _println("connect failed");
                            break;
                        }
                        case OTA_RECEIVE_ERROR:
                        {
                            _println("receive failed");
                            break;
                        }
                        case OTA_END_ERROR:
                        {
                            _println("end failed");
                            break;
                        }
                        default:
                        {
                            _println("unknown");
                        }
                    }
                    _show();
                });
            #endif
            ArduinoOTA.begin();
        #endif
        #if USE_SSL
            client.setFingerprint(AIO_FINGERPRINT);
        #endif
    #endif

    ledBuiltinSetup();
    #if USE_ALARM
        buzzer(BUZZER_OFF);
        buzzerSetup();
    #endif
    if (!scd30.begin())
    {
        #if USE_SERIAL || USE_DISPLAY
            _println("Failed to find SCD30 chip.");
            _show();
        #endif
        _restart();
    }
}

void loop()
{
    #if USE_WIFI && USE_OTA
        ArduinoOTA.handle();
    #endif
    if(scd30.dataReady())
    {
        if(!scd30.read())
        {
            #if USE_SERIAL || USE_DISPLAY
                _println("Error reading sensor data.");
            #endif
            return;
        }
        ledBuiltin(LED_ON);

        #if USE_SERIAL || USE_DISPLAY
            _clear();
        #endif

        #if USE_WIFI && USE_MQTT
            if(!mqtt.connected())
            {
                #if USE_SERIAL || USE_DISPLAY
                    _println("MQTT not connected,\nreconnecting...\n");
                    _show();
                    int8_t result = mqtt.connect();
                    if(result)
                    {
                        _printf("Not connected:\n%s",(char*)mqtt.connectErrorString(result));
                    }
                    else
                        _print("Connected!");
                    _show();
                    _clear();
                    delay(500);
                #else
                    mqtt.connect();
                #endif

            }
        #endif


        #if USE_SERIAL || USE_DISPLAY
            _print(SENSOR_NAME);
            #if USE_WIFI
                _rtlprintf("%i.%i",WiFi.localIP()[2],WiFi.localIP()[3]);
            #endif
            _print("\n\n");
            _setCursor(_cursorX(), _cursorY() + 1);
        #endif
        #if USE_WIFI && USE_MQTT
            msPublish = millis() - msPublish > REFRESH_FEED ? 0 : msPublish;
        #endif
        #if USE_SERIAL || USE_DISPLAY || (USE_WIFI && USE_MQTT)
            if(scd30.temperature != 0)
            {
                #if USE_SERIAL || USE_DISPLAY
                    _printf("Temperature: %.1f%c C\n",scd30.temperature,CHAR_DEGREE);
                    _setCursor(_cursorX(), _cursorY() + 3);
                #endif
                #if USE_WIFI && USE_MQTT
                    if(msPublish == 0)
                        temp.publish(scd30.temperature,1);
                #endif
            }
            if(scd30.relative_humidity != 0)
            {
                #if USE_SERIAL || USE_DISPLAY
                    _printf("Humidity: %.1f %%\n\n",scd30.relative_humidity);
                #endif
                #if USE_WIFI && USE_MQTT
                    if(msPublish == 0)
                        hum.publish(scd30.relative_humidity,1);
                #endif
            }
        #endif
        if(first)
        {
            first = false;
        }
        else if(scd30.CO2 != 0)
        {
            #if USE_SERIAL || USE_DISPLAY
                _textSize(2);
                _print("CO");
                x = _cursorX();
                y = _cursorY();
                _textSize(1);
                _print("\n");
                _setCursorX(x);
                _print("2 ");
                _textSize(2);
                _setCursor(_cursorX(), y);
                _printf("%.0f",scd30.CO2);
                x = _cursorX();
                _textSize(1);
            #endif

            #if USE_WIFI && USE_MQTT
                if(msPublish == 0)
                    co2.publish(scd30.CO2,0);
            #endif
            #if USE_SERIAL || USE_DISPLAY || USE_ALARM
                if(msAlarm == 0)
                    msAlarm = scd30.CO2 > SENSOR_CO2_LEVEL_DANGER ? millis() : 0;
                if(msAlarm != 0)
                {
                    #if USE_SERIAL || USE_DISPLAY
                        _println(" ALARM!");
                    #endif
                }
                #if USE_SERIAL || USE_DISPLAY
                    else
                        _print("\n");
                    _setCursorX(x);
                    _print(" ppm");
                #endif
            #endif
        }
        #if USE_WIFI && USE_MQTT
            if(msPublish == 0)
                msPublish = millis();
        #endif
        ledBuiltin(LED_OFF);
    }

    #if USE_ALARM
        if(msAlarm != 0)
        {
            if(millis() > msAlarm + SENSOR_ALARM_DURATION)
            {
                buzzer(BUZZER_OFF);
                msAlarm = 0;
            }
            else
                buzzer(BUZZER_ON);
        }
    #endif

    #if USE_SERIAL || USE_DISPLAY
        #if USE_DISPLAY && USE_WIFI && USE_MQTT
            if(mqtt.connected())
                oled.drawFastHLine(0,SCREEN_HEIGHT - 1,(millis() - msPublish) * SCREEN_WIDTH / REFRESH_FEED,SSD1306_WHITE);
        #endif
        _show();
    #endif
}

#if USE_SERIAL || USE_DISPLAY

    void rtlprintf(const char* format, ...)
    {
        va_list arg;
        va_start(arg, format);
        char temp[64];
        char* buffer = temp;
        size_t len = vsnprintf(temp, sizeof(temp), format, arg);
        va_end(arg);
        if (len > sizeof(temp) - 1) {
            buffer = new (std::nothrow) char[len + 1];
            if (!buffer) {
                return;
            }
            va_start(arg, format);
            vsnprintf(buffer, len + 1, format, arg);
            va_end(arg);
        }
        rtlprint(buffer);
        if (buffer != temp) {
            delete[] buffer;
        }
    }

    void rtlprint(const char* string)
    {
        uint16_t w = 0;
        int16_t tmp = 0;
        oled.getTextBounds(string, 0, 0, &tmp, &tmp, &w, (uint16_t*)&tmp);
        _setCursor(SCREEN_WIDTH - w,_cursorY());
        _print(string);
    }

    void rtlprintln(const char* string)
    {
        rtlprint(string);
        _print("\n");
    }

#endif