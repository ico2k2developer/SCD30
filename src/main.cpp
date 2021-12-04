#define USE_SERIAL      OFF
#define USE_DISPLAY     ON
#define USE_WIFI        ON
#define USE_OTA         ON
#define USE_MQTT        ON
#define USE_SSL         ON

#include <main.h>
#include <passwords.h>

Adafruit_SCD30  scd30;
#if USE_DISPLAY
Adafruit_SSD1306 oled;
#endif
#if USE_WIFI && USE_MQTT
#if USE_SSL
WiFiClientSecure client;
static const char *fingerprint PROGMEM = "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF";
#else
WiFiClient client;
#endif
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish co2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/CO2");
#endif


void setup(void)
{
    Wire.begin(4,5);

#if USE_SERIAL
    Serial.begin(115200);
    while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
#endif

#if USE_DISPLAY
    oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3c);
    oled.setTextColor(SSD1306_WHITE);
    oled.cp437(true);
#endif

    _printf("WiFi: %s\n",USE_WIFI ? TEXT_ON : TEXT_OFF);
    cursor(getCursorX(), getCursorY() + 1);
    _printf("OTA: %s\n",USE_OTA ? TEXT_ON : TEXT_OFF);
    cursor(getCursorX(), getCursorY() + 1);
    _printf("MQTT: %s\n",USE_MQTT ? TEXT_ON : TEXT_OFF);
    cursor(getCursorX(), getCursorY() + 1);
    _printf("SSL: %s\n",USE_SSL ? TEXT_ON : TEXT_OFF);

    _show();
    _clear();

#if USE_WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
    uint8 attempts;
    for(attempts = 0; attempts < CONNECTION_RETRY_COUNT; attempts++)
    {
        delay(CONNECTION_RETRY_INTERVAL);
        if(WiFi.isConnected())
            break;
        else
        {
            WiFi.reconnect();
            if(attempts == 0)
                _printf("Connecting to %s\n",WLAN_SSID);
            _printf("Attempt %i of %i...\n",attempts + 1,CONNECTION_RETRY_COUNT);
            _show();
        }
    }
    if(!WiFi.isConnected())
    {
        _print("Connection failed.\nRebooting...");
        _show();
        delay(500);
        ESP.restart();
    }

#if USE_OTA
    ArduinoOTA.onStart([]()
                       {
                           _clear();
                           _println("OTA update utility\n");
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
        oled.fillRect(getCursorX(), getCursorY(), progress * SCREEN_WIDTH / total, 10, SSD1306_WHITE);
        _show();
    });
#endif
    ArduinoOTA.onError([](ota_error_t error) {
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
    ArduinoOTA.begin();
#endif
#ifdef USE_SSL
    client.setFingerprint(fingerprint);
#endif
#endif

    ledBuiltinSetup();
    buzzer(BUZZER_OFF);
    buzzerSetup();

    //Serial.println("Adafruit SCD30 test!");

    // Try to initialize!
    if (!scd30.begin())
    {
        _println("Failed to find SCD30 chip.");
        _show();
        //Serial.println("Failed to find SCD30 chip");
        while (1) { delay(10); }
    }
    //Serial.println("SCD30 Found!");


    // if (!scd30.setMeasurementInterval(10)){
    //   Serial.println("Failed to set measurement interval");
    //   while(1){ delay(10);}
    // }
}

int16_t x,y;
bool first = true;
bool alarm = false;

void loop() {
#if USE_WIFI && USE_OTA
    ArduinoOTA.handle();
#endif
    if (scd30.dataReady())
    {
        if (!scd30.read())
        {
            _println("Error reading sensor data.");
            return;
        }
        ledBuiltin(LED_ON);
        _clear();

#if USE_WIFI && USE_MQTT
        if(!mqtt.connected())
        {
            _println("MQTT not connected,\nreconnecting...:\n");
            _show();
            int8_t result = mqtt.connect();
            if(result)
            {
                _printf("Not connected:\n%s",(char*)mqtt.connectErrorString(result));
            }
            else
                _print("Connected!");
            _show();
            delay(500);
            _clear();
        }
#endif
        _printf("%s\n\n",SENSOR_NAME);
        if(scd30.temperature != 0)
        {
            _printf("Temperature: %.1f%c C\n",scd30.temperature,CHAR_DEGREE);
            cursor(getCursorX(), getCursorY() + 3);
        }
        if(scd30.relative_humidity != 0)
            _printf("Humidity: %.1f %%\n\n",scd30.relative_humidity);

        if(first)
        {
            first = false;
        }
        else if(scd30.CO2 != 0)
        {
            textSize(2);
            _print("CO");
            x = getCursorX();
            y = getCursorY();
            textSize(1);
            _print("\n");
            cursor(x, getCursorY());
            _print("2 ");
            textSize(2);
            cursor(getCursorX(), y);
            _printf("%.0f",scd30.CO2);
            x = getCursorX();
            textSize(1);
            alarm = scd30.CO2 > SENSOR_CO2_LEVEL_DANGER;
            if(alarm)
            {
                _println(" ALARM!");
                buzzer(BUZZER_ON);
            }
            else
                _print("\n");
            cursor(x, getCursorY());
            //display.cursor(display.getCursorX(),display.getCursorY() + 1);
            _print(" ppm");

            co2.publish(scd30.CO2,0);
        }
        _show();
        ledBuiltin(LED_OFF);
    }

    if(alarm)
    {
        delay(SENSOR_ALARM_DURATION);
        buzzer(BUZZER_OFF);
    }
    else
        delay(100);
}