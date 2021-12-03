//#define USE_SERIAL
#define USE_DISPLAY
//#define USE_WIFI

#include <main.h>
#include <passwords.h>

Adafruit_SCD30  scd30;
#ifdef USE_DISPLAY
Adafruit_SSD1306 display;
#endif

uint16_t refreshTime;


void setup(void)
{
    Wire.begin(4,5);
#ifdef USE_WIFI
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
#endif
#ifdef USE_SERIAL
    Serial.begin(115200);
    while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
#endif
#ifdef USE_DISPLAY
    display = Adafruit_SSD1306(128, 64, &Wire);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
#endif

    ledBuiltinSetup();
    buzzer(BUZZER_OFF);
    buzzerSetup();

    show();
    delay(1000);
    clear();
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);

    //Serial.println("Adafruit SCD30 test!");

    // Try to initialize!
    if (!scd30.begin())
    {
        println("Failed to find SCD30 chip.");
        show();
        //Serial.println("Failed to find SCD30 chip");
        while (1) { delay(10); }
    }
    //Serial.println("SCD30 Found!");


    // if (!scd30.setMeasurementInterval(10)){
    //   Serial.println("Failed to set measurement interval");
    //   while(1){ delay(10);}
    // }
    refreshTime = scd30.getMeasurementInterval() * 1000;
}

int16_t x,y;
bool first = true;
bool alarm = false;

void loop() {
    if (scd30.dataReady())
    {
        if (!scd30.read())
        {
            println("Error reading sensor data.");
            return;
        }
        ledBuiltin(LED_ON);
        clear();
        printf("%s\n\n",SENSOR_NAME);
        if(scd30.temperature != 0)
        {
            printf("Temperature: %.1f%c C\n",scd30.temperature,CHAR_DEGREE);
            display.setCursor(display.getCursorX(),display.getCursorY() + 3);
        }
        if(scd30.relative_humidity != 0)
            printf("Humidity: %.1f %%\n\n",scd30.relative_humidity);

        if(first)
        {
            first = false;
        }
        else if(scd30.CO2 != 0)
        {
            setSize(2);
            print("CO");
            x = display.getCursorX();
            y = display.getCursorY();
            setSize(1);
            print("\n");
            display.setCursor(x,display.getCursorY());
            print("2 ");
            setSize(2);
            display.setCursor(display.getCursorX(),y);
            printf("%.0f",scd30.CO2);
            x = display.getCursorX();
            setSize(1);
            alarm = scd30.CO2 > SENSOR_CO2_LEVEL_DANGER;
            if(alarm)
            {
                println(" ALARM!");
                buzzer(BUZZER_ON);
            }
            else
                print("\n");
            display.setCursor(x,display.getCursorY());
            //display.setCursor(display.getCursorX(),display.getCursorY() + 1);
            print(" ppm");
        }
        show();
        ledBuiltin(LED_OFF);
    }

    if(alarm)
    {
        delay(SENSOR_ALARM_DURATION);
        buzzer(BUZZER_OFF);
        delay(refreshTime - SENSOR_ALARM_DURATION);
    }
    else
        delay(refreshTime);
}