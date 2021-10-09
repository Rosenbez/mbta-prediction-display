#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include <ArduinoJson.h>

#include "prediction.hpp"

const char *mbta_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEdTCCA12gAwIBAgIJAKcOSkw0grd/MA0GCSqGSIb3DQEBCwUAMGgxCzAJBgNV\n"
    "BAYTAlVTMSUwIwYDVQQKExxTdGFyZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTIw\n"
    "MAYDVQQLEylTdGFyZmllbGQgQ2xhc3MgMiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0\n"
    "eTAeFw0wOTA5MDIwMDAwMDBaFw0zNDA2MjgxNzM5MTZaMIGYMQswCQYDVQQGEwJV\n"
    "UzEQMA4GA1UECBMHQXJpem9uYTETMBEGA1UEBxMKU2NvdHRzZGFsZTElMCMGA1UE\n"
    "ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjE7MDkGA1UEAxMyU3RhcmZp\n"
    "ZWxkIFNlcnZpY2VzIFJvb3QgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n"
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVDDrEKvlO4vW+GZdfjohTsR8/\n"
    "y8+fIBNtKTrID30892t2OGPZNmCom15cAICyL1l/9of5JUOG52kbUpqQ4XHj2C0N\n"
    "Tm/2yEnZtvMaVq4rtnQU68/7JuMauh2WLmo7WJSJR1b/JaCTcFOD2oR0FMNnngRo\n"
    "Ot+OQFodSk7PQ5E751bWAHDLUu57fa4657wx+UX2wmDPE1kCK4DMNEffud6QZW0C\n"
    "zyyRpqbn3oUYSXxmTqM6bam17jQuug0DuDPfR+uxa40l2ZvOgdFFRjKWcIfeAg5J\n"
    "Q4W2bHO7ZOphQazJ1FTfhy/HIrImzJ9ZVGif/L4qL8RVHHVAYBeFAlU5i38FAgMB\n"
    "AAGjgfAwge0wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0O\n"
    "BBYEFJxfAN+qAdcwKziIorhtSpzyEZGDMB8GA1UdIwQYMBaAFL9ft9HO3R+G9FtV\n"
    "rNzXEMIOqYjnME8GCCsGAQUFBwEBBEMwQTAcBggrBgEFBQcwAYYQaHR0cDovL28u\n"
    "c3MyLnVzLzAhBggrBgEFBQcwAoYVaHR0cDovL3guc3MyLnVzL3guY2VyMCYGA1Ud\n"
    "HwQfMB0wG6AZoBeGFWh0dHA6Ly9zLnNzMi51cy9yLmNybDARBgNVHSAECjAIMAYG\n"
    "BFUdIAAwDQYJKoZIhvcNAQELBQADggEBACMd44pXyn3pF3lM8R5V/cxTbj5HD9/G\n"
    "VfKyBDbtgB9TxF00KGu+x1X8Z+rLP3+QsjPNG1gQggL4+C/1E2DUBc7xgQjB3ad1\n"
    "l08YuW3e95ORCLp+QCztweq7dp4zBncdDQh/U90bZKuCJ/Fp1U1ervShw3WnWEQt\n"
    "8jxwmKy6abaVd38PMV4s/KCHOkdp8Hlf9BRUpJVeEXgSYCfOn8J3/yNTd126/+pZ\n"
    "59vPr5KW7ySaNRB6nJHGDn2Z9j8Z3/VyVOEVqQdZe4O/Ui5GjLIAZHYcSNPYeehu\n"
    "VsyuLAOQ1xk4meTKCRlb/weWsKh/NEnfVqn3sF/tM+2MR7cwA130A4w=\n"
    "-----END CERTIFICATE-----\n";


class MbtaApi {
public:

    MbtaApi() {}

    PredictionPack getPredictions(int mbta_stop_number)
    {
        String prediction_json = getMbtaPredictionJson(mbta_stop_number);

        PredictionPack predictions = parsePredictionString(prediction_json);
        return predictions;
    }

private:
    String getMbtaPredictionJson(int stop)
    {
        Serial.printf("Getting predictions for stop: %d \n", stop);
        String mbta_query_url = getQueryUrl(stop);
       HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        http.begin(mbta_query_url, mbta_cert); //HTTP
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        String payload;

        // httpCode will be negative on error
        if (httpSuccess(httpCode))
        {
            payload = http.getString();
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            Serial.println(http.getString());
            payload = "HTTP Error";
        }
        http.end();
        return payload;
    }

    String getQueryUrl(int stop)
    {
        String mbta_query_url = "https://api-v3.mbta.com/predictions?filter[stop]=";
        mbta_query_url += stop;
        return mbta_query_url;
    }

    bool httpSuccess(int http_code)
    {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", http_code);
        return http_code == HTTP_CODE_OK;
    } 

    PredictionPack parsePredictionString(String &payload)
    {
        DynamicJsonDocument doc(12288);
        Serial.println("allocated json");

        DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        Serial.println();
        //serializeJsonPretty(doc, Serial);

        int num_predictions = 0;
        
        PredictionPack prediction_pack;

        for (JsonObject elem : doc["data"].as<JsonArray>())
        {
            prediction_pack.predictions[num_predictions] = parsePredictionFromJson(elem);

            num_predictions++;
            Serial.printf("Adding prediction %d \n", num_predictions);
        };
        prediction_pack.num_predictions = num_predictions;
        return prediction_pack;
    }

    StopPrediction parsePredictionFromJson(JsonObject elem)
    {
        JsonObject attributes = elem["attributes"];
        const char *arrival_time = attributes["arrival_time"]; // "2021-03-07T09:30:51-05:00",
        JsonObject relationships = elem["relationships"];

        const char *route_num = relationships["route"]["data"]["id"]; // "87", "88"
        const char *stop_id = relationships["stop"]["data"]["id"];    // "2579",
        // const char *trip_id = relationships["trip"]["data"]["id"];
        auto prediction = StopPrediction(route_num, stop_id, arrival_time);
        if (strcmp(route_num, "88") == 0)
        {
            prediction.set_via("Highlnd");
        }
        else if (strcmp(route_num, "87") == 0)
        {
            prediction.set_via("Elm");
        }
        return prediction;
    }
};
