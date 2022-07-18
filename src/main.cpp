#define USE_SERIAL      OFF
#define USE_DISPLAY     ON
#define USE_WIFI        ON
#define USE_OTA         ON
#define USE_MQTT        ON
#define USE_MODBUS      ON
#define USE_SSL         ON
#define USE_ALARM       OFF

#include <main.h>

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
#if USE_WIFI
    bool scanning = false;
    bool scanFinished = false;
    int found1[WIFI_COUNT];
    short wifi,f1;
    unsigned long msConnect = 0;
    #if USE_MODBUS
        uint16_t WriteHreg[4] = {0};
    #endif
    #if USE_MQTT
        unsigned long msPublish = 0;
        bool firstConnection = true;
        #if USE_SSL
            WiFiClientSecure client;
        #else
            WiFiClient client;
        #endif
        Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);
        Adafruit_MQTT_Publish co2(&mqtt, FEED_CO2);
        Adafruit_MQTT_Publish temperature(&mqtt, FEED_TEMPERATURE);
        Adafruit_MQTT_Publish humidity(&mqtt, FEED_HUMIDITY);
        Adafruit_MQTT_Publish logs(&mqtt, FEED_LOG);
    #endif
#endif
char first = -2;
char highCO2 = 0;

void setup(void)
{
    Wire.begin(4,5);

    #if USE_SERIAL
        Serial.begin(115200);
    #endif

    #if USE_DISPLAY
        oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
        oled.begin(SSD1306_SWITCHCAPVCC, 0x3c);
        oled.setTextColor(SSD1306_WHITE);
        oled.cp437(true);
    #endif

    #if USE_SERIAL || USE_DISPLAY
        _printf("WiFi: %s",USE_WIFI ? TEXT_ON : TEXT_OFF);
        _rtlprint(VERSION_NAME);
        _setCursorY(_cursorY() + 1);
        _printf("\nOTA: %s\n",USE_OTA ? TEXT_ON : TEXT_OFF);
        _setCursorY(_cursorY() + 1);
        _printf("MQTT: %s\n",USE_MQTT ? TEXT_ON : TEXT_OFF);
        _setCursorY(_cursorY() + 1);
        _printf("SSL: %s\n",USE_SSL ? TEXT_ON : TEXT_OFF);

        _show();
        _clear();
        delay(2000);
    #endif

    #if USE_WIFI
        WiFi.mode(WIFI_STA);
        #if USE_MQTT && USE_CALIBRATION
            //mqtt.subscribe(&calibrationValue);
            //mqtt.subscribe(&calibrationConfirm);
            //calibrationValue.setCallback(&calibrationValueReceived);
            //calibrationConfirm.setCallback(&calibrationConfirmReceived);
        #endif
        #if USE_OTA
            #if USE_SERIAL || USE_DISPLAY || USE_MQTT
                ArduinoOTA.onStart([]()
                {
                    #if USE_SERIAL || USE_DISPLAY
                        _clear();
                        _printf("OTA update incoming\n\nCurrent version: %s\n",VERSION);
                        _setCursorY(_cursorY() + 1);
                        oled.drawRect(_cursorX(),_cursorY(),SCREEN_WIDTH,10,SSD1306_WHITE);
                        _show();
                    #endif
                    #if USE_MQTT
                        if(mqtt.connected())
                        {
                            #define LENGTH_LOGS_OTA 64
                            char* log = (char*)malloc(LENGTH_LOGS_OTA);
                            snprintf(log,LENGTH_LOGS_OTA,"OTA update incoming, current version is %s",VERSION);
                            logs.publish(log);
                            free(log);
                        }
                    #endif
                });
            #endif
            #if USE_SERIAL || USE_DISPLAY
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
        #if USE_MODBUS
            ModbusTaskSetup();

            ModbusSetWHreg(WriteHreg, 4, 4);
        #endif
    #endif

    ledBuiltinSetup();
    ledBuiltin(LED_OFF);
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
    #if USE_SERIAL || USE_DISPLAY
        _clear();
    #endif
    #if USE_WIFI
        if(WiFi.isConnected())
        {
            #if USE_SERIAL || USE_DISPLAY
                _print(WiFi.SSID());
            #endif
            #if USE_OTA
                ArduinoOTA.handle();
            #endif
            #if USE_MQTT
                #if USE_SERIAL || USE_DISPLAY
                    _print(" MQTT");
                #endif
                if(mqtt.connected())
                {
                    if((millis() - msPublish) > REFRESH_FEED)
                    {
                        if(first == 1)
                        {
                            ledBuiltin(LED_ON);
                            temperature.publish(scd30.temperature, 1);
                            humidity.publish(scd30.relative_humidity, 1);
                            co2.publish(scd30.CO2,0);
                            if(scd30.CO2 > SENSOR_CO2_LEVEL_DANGER && !highCO2)
                            {
                                highCO2 = 1;
                                char log[100];
                                snprintf(log,sizeof(log)/sizeof(log[0]),"CO2 level has exceeded danger level of "
                                    STRINGIFY(SENSOR_CO2_LEVEL_DANGER) " ppm and now is %.0f ppm",scd30.CO2);
                                logs.publish(log);
                            }
                            else if(scd30.CO2 < SENSOR_CO2_LEVEL_DANGER && highCO2)
                            {
                                highCO2 = 0;
                                char log[100];
                                snprintf(log,sizeof(log)/sizeof(log[0]),"CO2 level has lowered below danger level of "
                                    STRINGIFY(SENSOR_CO2_LEVEL_DANGER) " ppm and now is %.0f ppm",scd30.CO2);
                                logs.publish(log);
                            }
                            ledBuiltin(LED_OFF);
                        }
                        if(firstConnection)
                        {
                            firstConnection = false;
                            #define LENGTH_LOGS_CONNECTION (100 + WIFI_SSID_MAX)
                            char log[LENGTH_LOGS_CONNECTION];
                            snprintf(log,LENGTH_LOGS_CONNECTION,"Connection established through network %s,"
                                " now sending updates every %.1fs",WiFi.SSID().c_str(),REFRESH_FEED / 1000.0);
                            logs.publish(log);
                        }
                        msPublish = millis();
                    }
                    #if USE_DISPLAY
                        oled.drawFastHLine(0,SCREEN_HEIGHT - 1,(int16_t)((millis() - msPublish) * SCREEN_WIDTH / REFRESH_FEED),SSD1306_WHITE);
                    #endif
                }
                else
                {
                    firstConnection = true;
                    #if USE_SERIAL || USE_DISPLAY
                        _println('!');
                        _print(mqtt.connectErrorString(mqtt.connect()));
                    #else
                        mqtt.connect();
                    #endif
                }
            #endif
            #if USE_SERIAL || USE_DISPLAY
                _rtlprintf("%i.%i",WiFi.localIP()[2],WiFi.localIP()[3]);
            #endif

        }
        else
        {
            if(scanning)
            {
                if(f1 == -1)
                {
                    _print("Scanning...");
                }
                else if(f1 == 0)
                {
                    _print("No wifi available");
                }
                else
                {
                    int count = f1;
                    _print(count);
                    _print(" network");
                    if(count > 1)
                        _print("s");
                    _print(" found");
                }

            }
            else if(scanFinished)
            {
                if(millis() - msConnect > CONNECTION_RETRY_INTERVAL)
                {
                    if(f1 != 0 && ++wifi < f1)
                    {
                        char ssid[WIFI_SSID_MAX + 1];
                        char password[WIFI_PASSWORD_MAX + 1];
                        strncpy_P(ssid, (char*) pgm_read_dword(&(WIFI_SSIDS[found1[wifi]])), WIFI_SSID_MAX + 1);
                        strncpy_P(password, (char*) pgm_read_dword(&(WIFI_PASSWORDS[found1[wifi]])), WIFI_PASSWORD_MAX + 1);
                        WiFi.begin(ssid,password);
                        #if USE_SERIAL || USE_DISPLAY
                            _print(ssid);
                            _print('!');
                        #endif
                        msConnect = millis();
                    }
                    else
                        scanFinished = false;
                }
            }
            else
            {
                WiFi.disconnect(false);
                wifi = -1;
                scanFinished = false;
                scanning = true;
                WiFi.scanNetworksAsync(&scanResult, true);
            }
        }
        #if USE_MODBUS
            ModbusTaskLoop();
            static int16_t a;
            if(a > 100) a = 0;
            WriteHreg[3] = a++;
        #endif
    #endif

    #if USE_SERIAL || USE_DISPLAY || USE_ALARM || (USE_WIFI && USE_MQTT)
        if(scd30.dataReady())
        {
            if (scd30.read())
            {
                if(first != 1)
                    first++;
            }
        }
    #endif

    #if USE_SERIAL || USE_DISPLAY || USE_ALARM
        #if USE_SERIAL || USE_DISPLAY
            #if !USE_WIFI
                _print(SENSOR_NAME);
            #endif
            _print("\n\n");
            _setCursor(_cursorX(), _cursorY() + 1);
        #endif
        if(first == 1)
        {
            #if USE_SERIAL || USE_DISPLAY
                if(scd30.temperature != 0)
                {
                    _printf("Temperature: %.1f%cC\n",scd30.temperature,CHAR_DEGREE);
                    _setCursor(_cursorX(), _cursorY() + 3);
                    #if USE_WIFI && USE_MODBUS
                        WriteHreg[1] = (uint16_t)(scd30.temperature * 10);
                    #endif
                }
                if(scd30.relative_humidity != 0)
                {
                    _printf("Humidity: %.1f%%\n\n",scd30.relative_humidity);

                    #if USE_WIFI && USE_MODBUS
                        WriteHreg[2] = (uint16_t)(scd30.relative_humidity * 10);
                    #endif
                }
            #endif
            if(scd30.CO2 != 0)
            {
                if(msAlarm == 0)
                    msAlarm = scd30.CO2 > SENSOR_CO2_LEVEL_DANGER ? millis() : 0;
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
                    if(msAlarm != 0)
                    {
                        _println(" ALARM!");
                    }
                    else
                        _print("\n");
                    _setCursorX(x);
                    _print("ppm");
                #endif
                #if USE_WIFI && USE_MODBUS
                    WriteHreg[0] = (uint16_t)scd30.CO2;
                #endif
            }
        }
        if(msAlarm != 0)
        {
            if(millis() > msAlarm + SENSOR_ALARM_DURATION)
            {
                #if USE_ALARM
                    buzzer(BUZZER_OFF);
                #endif
                msAlarm = 0;
            }
            #if USE_ALARM
                else
                    buzzer(BUZZER_ON);
            #endif
        }
    #endif

    #if USE_SERIAL || USE_DISPLAY
        _show();
    #endif
}

#if USE_WIFI
    #if USE_MODBUS
        bool IsWifiConnected()
        {
            return WiFi.isConnected();
        }

        unsigned long SetTimer(unsigned long mS)
        {
            return (millis() + mS);
        }

        bool IsTimerExpired(unsigned long Timer)
        {
            return millis() > Timer;
        }

        void ModbusTaskSetup()
        {
            mb.client();
            ModbusStart();
        }

        void ModbusTaskLoop()
        {
            //digitalWrite(LED_BUILTIN, mb.isConnected(MbServer) ? LOW : HIGH);

            switch(ModbusControl.TaskState)
            {
                case MODBUS_START:
                {
                    if(IsWifiConnected())
                    {
                        if(mb.isConnected(MbServer))
                        {
                            ModbusControl.TaskState = MODBUS_CONNECTED;
                            Serial.printf("MODBUS: Connected.\n");
                        }
                        else
                        {
                            Serial.printf("MODBUS: Connecting...\n");
                            mb.connect(MbServer,MODBUS_PORT);           // Try to connect if no connection (connect is synchronous: returns after connection is established)
                        }
                    }
                    break;
                }
                case MODBUS_CONNECTED:
                {
                    if(mb.isConnected(MbServer))
                    {  // If still connected
                        if(ModbusControl.prHreg && ModbusControl.nrHreg)
                        {  // If any read to do...
        //            Serial.printf("MODBUS: Starting read transaction\n");

                            ModbusControl.Trans = mb.readHreg(MbServer, ModbusControl.OfsrHreg, ModbusControl.prHreg, ModbusControl.nrHreg);  // Initiate Read HR from Modbus Slave
                            ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                            ModbusControl.TaskState = MODBUS_READ_TRANS;
                        }
                        else if(ModbusControl.pwHreg && ModbusControl.nwHreg)
                        {  // else if any write to do...
        //            Serial.printf("MODBUS: Starting write transaction -> [%d]\n",ModbusControl.pwHreg[0]);

                            ModbusControl.Trans = mb.writeHreg(MbServer, ModbusControl.OfswHreg, ModbusControl.pwHreg, ModbusControl.nwHreg);  // Initiate Write HR to Modbus Slave
                            ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                            ModbusControl.TaskState = MODBUS_WRITE_TRANS;
                        }
        // Send and receive
                    }
                    else
                    {  // Reconnect
                        Serial.printf("MODBUS: Connection lost. Restart connection\n");
                        ModbusControl.TaskState = MODBUS_START;
                    }
                    break;
                }
                case MODBUS_READ_TRANS:
                {
                    if(mb.isTransaction(ModbusControl.Trans))
                    { // If read still pending
                        if(IsTimerExpired(ModbusControl.Timer))
                        {  // If timeout occurred, restart connection
                            Serial.printf("MODBUS: Read transaction timeout...\n");
                            ModbusControl.TaskState = MODBUS_RESTART;
                        }
                    }
                    else if(ModbusControl.pwHreg && ModbusControl.nwHreg)
                    {  // Start pending write if any
        //         Serial.printf("MODBUS: Starting write transaction -> [%d]\n",ModbusControl.pwHreg[0]);

                        ModbusControl.Trans = mb.writeHreg(MbServer, ModbusControl.OfswHreg, ModbusControl.pwHreg, ModbusControl.nwHreg);  // Initiate Write HR to Modbus Slave
                        ModbusControl.Timer = SetTimer(MODBUS_TRANSACTION_TIME);
                        ModbusControl.TaskState = MODBUS_WRITE_TRANS;
                    }
                    else
                    {  // Restart communication cycle
                        ModbusControl.TaskState = MODBUS_CONNECTED;
                    }
                    break;
                }
                case MODBUS_WRITE_TRANS:
                {
                    if(mb.isTransaction(ModbusControl.Trans))
                    {  // If write still pending
                        if(IsTimerExpired(ModbusControl.Timer))
                        { // If timeout occurred, restart connection
                            Serial.printf("MODBUS: Write transaction timeout...\n");
                            ModbusControl.TaskState = MODBUS_RESTART;
                        }
                    }
                    else
                    {  // Restart communication cycle
                        ModbusControl.TaskState = MODBUS_CONNECTED;
                    }
                    break;
                }
                case MODBUS_RESTART:
                case MODBUS_END:
                {
                    mb.disconnect(MbServer);
                    mb.dropTransactions();              // Cancel all waiting transactions

                    Serial.printf("MODBUS: Disconnect and %s\n",MODBUS_END == ModbusControl.TaskState ? "Go to IDLE" : "RESTART");

                    ModbusControl.TaskState = MODBUS_END == ModbusControl.TaskState ? MODBUS_IDLE : MODBUS_START;
                    break;
                }
                /*case MODBUS_IDLE:
                {
                    break;
                }
                default:
                {
                    break;
                }*/
            }
            mb.task();                      // Common local Modbus task
        }

        void ModbusStart()
        {
            memset(&ModbusControl, 0, sizeof(ModbusControl));
            ModbusControl.TaskState = MODBUS_START;
            Serial.printf("ModBusStart.\n");
        }

        void ModbusStop(bool fRestart)
        {
            ModbusControl.TaskState = fRestart ? MODBUS_RESTART : MODBUS_END;
            Serial.printf("ModBusStop [%s].\n",fRestart ? "RESTART" : "END");
        }

        void ModbusSetWHreg(uint16_t* p, uint16_t ofs, uint16_t n)
        {
            ModbusControl.pwHreg = p;
            ModbusControl.nwHreg = n;
            ModbusControl.OfswHreg = ofs;
        }

        void ModbusSetRHreg(uint16_t* p, uint16_t ofs, uint16_t n)
        {
            ModbusControl.prHreg = p;
            ModbusControl.nrHreg = n;
            ModbusControl.OfsrHreg = ofs;
        }
    #endif

    void scanResult(int foundCount)
    {
        int i1,i2;
        char name[WIFI_SSID_MAX + 1];
        char * buffer1[WIFI_COUNT];
        memset(found1,0,sizeof(int) * WIFI_COUNT);
        f1 = 0;
        for(i1 = 0; i1 < WIFI_COUNT; i1++)
        {
            buffer1[i1] = (char*)malloc(WIFI_SSID_MAX + 1);
            strncpy_P(buffer1[i1],(char*) pgm_read_dword(&(WIFI_SSIDS[i1])),WIFI_SSID_MAX + 1);
        }
        for(i1 = 0; i1 < foundCount; i1++)
        {
            strncpy(name,WiFi.SSID(i1).c_str(),WIFI_SSID_MAX + 1);
            for(i2 = 0; i2 < WIFI_COUNT; i2++)
            {
                if(buffer1[i2] != nullptr)
                {
                    if(strcmp(name,buffer1[i2]) == 0)
                    {
                        buffer1[i2] = nullptr;
                        found1[f1] = i2;
                        f1++;
                        break;
                    }
                }
            }
        }
        scanning = false;
        scanFinished = true;
        for(i1 = 0; i1 < WIFI_COUNT; i1++)
        {
            free(buffer1[i1]);
        }
    }
#endif

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
        if (buffer != temp)
            delete[] buffer;
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
//-----------------------------------------------