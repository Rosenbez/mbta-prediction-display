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

#define EPD_DC      33 // A9can be any pin, but required!
#define EPD_CS      15  // A8 can be any pin, but required!
#define EPD_BUSY    -1  // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS     32  // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_RESET   -1  // can set to -1 and share with chip Reset (can't deep sleep)

// 2.9" Grayscale Featherwing or Breakout:
ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#define COLOR1 EPD_BLACK
#define COLOR2 EPD_LIGHT
#define COLOR3 EPD_DARK
// WiFi network name and password:
const char * networkName = "Holodeck3-2.4";
const char * networkPswd = "rikerprogram1";

const char* mbta_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEdTCCA12gAwIBAgIJAKcOSkw0grd/MA0GCSqGSIb3DQEBCwUAMGgxCzAJBgNV\n" \
"BAYTAlVTMSUwIwYDVQQKExxTdGFyZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTIw\n" \
"MAYDVQQLEylTdGFyZmllbGQgQ2xhc3MgMiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\n" \
"eTAeFw0wOTA5MDIwMDAwMDBaFw0zNDA2MjgxNzM5MTZaMIGYMQswCQYDVQQGEwJV\n" \
"UzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UE\n" \
"ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjE7MDkGA1UEAxMyU3RhcmZp\n" \
"ZWxkIFNlcnZpY2VzIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n" \
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVDDrEKvlO4vW+GZdfjohTsR8/\n" \
"y8+fIBNtKTrID30892t2OGPZNmCom15cAICyL1l/9of5JUOG52kbUpqQ4XHj2C0N\n" \
"Tm/2yEnZtvMaVq4rtnQU68/7JuMauh2WLmo7WJSJR1b/JaCTcFOD2oR0FMNnngRo\n" \
"Ot+OQFodSk7PQ5E751bWAHDLUu57fa4657wx+UX2wmDPE1kCK4DMNEffud6QZW0C\n" \
"zyyRpqbn3oUYSXxmTqM6bam17jQuug0DuDPfR+uxa40l2ZvOgdFFRjKWcIfeAg5J\n" \
"Q4W2bHO7ZOphQazJ1FTfhy/HIrImzJ9ZVGif/L4qL8RVHHVAYBeFAlU5i38FAgMB\n" \
"AAGjgfAwge0wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0O\n" \
"BBYEFJxfAN+qAdcwKziIorhtSpzyEZGDMB8GA1UdIwQYMBaAFL9ft9HO3R+G9FtV\n" \
"rNzXEMIOqYjnME8GCCsGAQUFBwEBBEMwQTAcBggrBgEFBQcwAYYQaHR0cDovL28u\n" \
"c3MyLnVzLzAhBggrBgEFBQcwAoYVaHR0cDovL3guc3MyLnVzL3guY2VyMCYGA1Ud\n" \
"HwQfMB0wG6AZoBeGFWh0dHA6Ly9zLnNzMi51cy9yLmNybDARBgNVHSAECjAIMAYG\n" \
"BFUdIAAwDQYJKoZIhvcNAQELBQADggEBACMd44pXyn3pF3lM8R5V/cxTbj5HD9/G\n" \
"VfKyBDbtgB9TxF00KGu+x1X8Z+rLP3+QsjPNG1gQggL4+C/1E2DUBc7xgQjB3ad1\n" \
"l08YuW3e95ORCLp+QCztweq7dp4zBncdDQh/U90bZKuCJ/Fp1U1ervShw3WnWEQt\n" \
"8jxwmKy6abaVd38PMV4s/KCHOkdp8Hlf9BRUpJVeEXgSYCfOn8J3/yNTd126/+pZ\n" \
"59vPr5KW7ySaNRB6nJHGDn2Z9j8Z3/VyVOEVqQdZe4O/Ui5GjLIAZHYcSNPYeehu\n" \
"VsyuLAOQ1xk4meTKCRlb/weWsKh/NEnfVqn3sF/tM+2MR7cwA130A4w=\n" \
"-----END CERTIFICATE-----\n";


bool connected = false;

//Are we currently connected?
float voltage;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
long StartTime;

void print_heap() {
  Serial.printf("Free heap: %u  \n", esp_get_free_heap_size()/1000);
}

void ScreenInit() {
  display.clearBuffer();
  display.setTextSize(2);
  display.setCursor(5, 50);
  display.setTextColor(COLOR1);
  display.setTextWrap(true);
  display.print("System Booting");
  display.display();
}

void write_predictions(StopPrediction predictions[], int num_predictions) {
  // Write an array of predictions to the screen
  int line_number = 0;
  for (int s = 0; s <= num_predictions; s++) {
    auto pred = predictions[s];
    Serial.print("Writing prediction to screen");
    int cursor_pos_y = 16 * (line_number + 1);
    display.setCursor(1, cursor_pos_y);
    //Serial.printf("set curser to y: %d \n", cursor_pos_y);
    //Serial.println("write string to screen buff");
    int arriving_in = pred.get_countdown_min(timeinfo);
    //Serial.printf("On print object %d \n", s);
    if (arriving_in < 0) {
      continue;
    }
    if (line_number > 6) {
      break;}
    if (strcmp(pred.via(),"0") == 0) {
      display.printf("Bus: %s %d Min", pred.route(), arriving_in);
    }
    else {
      display.printf("Bus: %s %d Min v %s", pred.route(), arriving_in, pred.via());
    }
    line_number++;
  }
}

void parse_and_display(String &payload) {
  print_heap();
  DynamicJsonDocument doc(6144);
  Serial.println("allocated json");
  print_heap();

  DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
  Serial.println();
  //serializeJsonPretty(doc, Serial);
  int i = 0;
  StopPrediction predictions[15];
  int num_predictions = 0;
  display.setTextSize(2);


  for (JsonObject elem : doc["data"].as<JsonArray>()) {

    JsonObject attributes = elem["attributes"];
    const char* arrival_time = attributes["arrival_time"]; // "2021-03-07T09:30:51-05:00",
    JsonObject relationships = elem["relationships"];

    const char* route_num = relationships["route"]["data"]["id"]; // "87", "88"
    const char* stop_id = relationships["stop"]["data"]["id"]; // "2579",
    predictions[i] = StopPrediction(route_num, stop_id, arrival_time);
    if (strcmp(route_num, "88") == 0) {
      predictions[i].set_via("Highlnd");
    }
    else if (strcmp(route_num, "87") == 0) {
      predictions[i].set_via("Elm");
    }
    i++;
    Serial.printf("Adding prediction %d \n", i);
    num_predictions++;
  };
  i = 0;
  write_predictions(predictions, num_predictions);
}


String get_mbta_prediction_json(int stop) {
  while (!connected) {
    sleep(1);
  }
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  String mbta_query_url = "https://api-v3.mbta.com/predictions?filter[stop]=";
  mbta_query_url += stop;
  http.begin(mbta_query_url, mbta_cert); //HTTP

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  String payload;

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          payload = http.getString();
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      Serial.println(http.getString());
      payload = "HTTP Error";
  }
  http.end();
  return payload;
}

void printLocalTime()
{
   while (!connected) {
    sleep(1);
  }
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    sleep(1);
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
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
      default: break;
    }
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
  while (!connected) {
    sleep(1);
  }
}

void read_batt(char * fill_batt) {
  float voltage = analogRead(A13)*2*(3.3/4096);
  float pct = map(voltage * 1000, 2900, 4000, 0.0, 100.0);
  Serial.printf("Bat voltage: %4.2f pct", voltage);
  sprintf(fill_batt, "Batt Voltage: %2.0f %%", pct);
  Serial.println();
}

void wifi_off() {
  connected = false;
  WiFi.disconnect(true);
}

void BeginSleep() {
  // Function for deep sleep power savings - not implimented yet.
  long SleepDuration = 60; // Sleep time in seconds, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
 // int  WakeupTime    = 7;  // Don't wakeup until after 07:00 to save battery power
 // int  SleepTime     = 23; // Sleep after (23+1) 00:00 to save battery power
  long SleepTimer = (SleepDuration ); //Some ESP32 are too fast to maintain accurate time
  esp_sleep_enable_timer_wakeup((SleepTimer) * 1000000LL); // Added +20 seconnds to cover ESP32 RTC timer source inaccuracies

  Serial.println("Entering " + String(SleepTimer) + "-secs of sleep time");
  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}

void WriteBanner() {
  char batt_str[30];
  read_batt(batt_str);
  display.print(batt_str);
  printLocalTime();
  display.setCursor(150, 1);
  display.print(&timeinfo, "%B %d %Y %H:%M");
  display.drawFastHLine(0, 14, display.width(), COLOR1);
}

void PrepDisplay() {
  display.clearBuffer();
  display.setTextSize(1);
  display.setCursor(0, 1);
  display.setTextColor(COLOR1);
  display.setTextWrap(true);

}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  StartTime = millis();
  Serial.println("Begin program");

  auto wifistore = WifiCredentialStore();
  //wifistore.storeCredentials(networkName, networkPswd);
  auto creds = wifistore.getCredentials();


  connectToWiFi(creds.ssid, creds.pass);
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  //display.begin(THINKINK_GRAYSCALE4);
  display.begin(THINKINK_MONO);
  
  // put your main code here, to run repeatedly:

  PrepDisplay();
  WriteBanner();
  String mbta_data = get_mbta_prediction_json(2579);

  parse_and_display(mbta_data);
  display.display();
  wifi_off();
  BeginSleep();
  sleep(60);

}

void loop() {

}

