#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_ThinkInk.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include "prediction.hpp"
#include "wifi_storage.hpp"
#include "mbta_api.hpp"

bool wifiChanged = false;
#include "mbta_bluetooth.hpp"
#include "display_handle.hpp"

#define EPD_DC 33    // A9can be any pin, but required!
#define EPD_CS 15    // A8 can be any pin, but required!
#define EPD_BUSY -1  // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS 32   // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_RESET -1 // can set to -1 and share with chip Reset (can't deep sleep)

// 2.9" Grayscale Featherwing or Breakout:
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

//Are we currently connected?
bool connected = false;

float voltage;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;
struct tm timeinfo;
long StartTime;

void print_heap()
{
    Serial.printf("Free heap: %u  \n", esp_get_free_heap_size() / 1000);
}

void printLocalTime()
{
    while (!connected)
    {
        sleep(1);
    }
    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        sleep(1);
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        //When connected set
        Serial.print("WiFi connected! IP address: ");
        Serial.println(WiFi.localIP());

        connected = true;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        connected = false;
        break;
    default:
        break;
    }
}

void connectToWiFi(const char *ssid, const char *pwd)
{
    Serial.print("Connecting to WiFi network: ");
    Serial.println(ssid);

    // delete old config
    WiFi.disconnect(true);
    //register event handler
    WiFi.onEvent(WiFiEvent);

    //Initiate connection
    WiFi.begin(ssid, pwd);

    Serial.println("Waiting for WIFI connection...");
    while (!connected)
    {
        sleep(1);
    }
}

float get_battery_percentage()
{
    float voltage = analogRead(A13) * 2 * (3.3 / 4096);
    float pct = map(voltage * 1000, 2900, 4000, 0.0, 100.0);
    Serial.printf("Bat voltage: %4.2f pct", pct);
    return pct;
}

void wifi_off()
{
    connected = false;
    WiFi.disconnect(true);
}

bool get_button_c_pressed(int button=13)
{
    pinMode(button, PULLUP);
    delay(50);

    int pressed = digitalRead(button);
    
    Serial.println("Button pressed!!!");
    Serial.printf("Button int: %d \n", pressed);
    return (pressed == 0);

}

void BeginSleep()
{
    // Function for deep sleep power savings - not implimented yet.
    long SleepDuration = 60;                               // Sleep time in seconds, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
                                                           // int  WakeupTime    = 7;  // Don't wakeup until after 07:00 to save battery power
                                                           // int  SleepTime     = 23; // Sleep after (23+1) 00:00 to save battery power
    long SleepTimer = (SleepDuration);                     //Some ESP32 are too fast to maintain accurate time
    esp_sleep_enable_timer_wakeup((SleepTimer)*1000000LL); // Added +20 seconnds to cover ESP32 RTC timer source inaccuracies

    Serial.println("Entering " + String(SleepTimer) + "-secs of sleep time");
    Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
    Serial.println("Starting deep-sleep period...");
    esp_deep_sleep_start(); // Sleep for e.g. 30 minutes
}

void testBLE()
{
    Serial.println("testing ble system");
    BLEDevice::init("esp32");
    auto ble = BleSystem();
    ble.startServer();
    delay(200);
    
    while(!get_button_c_pressed()){
        delay(40);
    }
}

// Create display handle and display the predictions
void write_predictions_to_display(PredictionPack prediction_pack)
{
    DisplayHandle mbta_display = DisplayHandle(&display, &timeinfo);

    mbta_display.PrepDisplay();

    float batt = get_battery_percentage();
    mbta_display.WriteBanner(batt);

    mbta_display.writePredictions(prediction_pack.predictions, prediction_pack.num_predictions);
    mbta_display.displayData();
}

void run_bluetooth_mode()
{
    Serial.println("engaging BT display");
    auto disp = DisplayHandle(&display, &timeinfo);
    disp.PrepDisplay();
    float batt = get_battery_percentage();
    disp.WriteBanner(batt);
    disp.writeBleScreen();
    disp.displayData();
    delay(2000);
    testBLE();

}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    StartTime = millis();
    Serial.println("Begin program");

    auto wifistore = WifiCredentialStore();
    //wifistore.storeCredentials(networkName, networkPswd);
    //wifistore.storeStop(2579);

    bool button_c = get_button_c_pressed();
    if (button_c)
    {
        run_bluetooth_mode();
    }

    auto creds = wifistore.getCredentials();
    connectToWiFi(creds.ssid, creds.pass);


    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    MbtaApi mbta = MbtaApi();
    
    auto prediction_pack = mbta.getPredictions(creds.mbtaStop);

    write_predictions_to_display(prediction_pack);

    wifi_off();
    BeginSleep();
}

void loop()
{
}
