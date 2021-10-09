#pragma once
#include <Preferences.h>

struct wifiCreds {
    char ssid[20];
    char pass[20];
    int mbtaStop;
};

class WifiCredentialStore {
private:
    const char* storageNamespace = "wifiCreds";
    const char* ssidKey = "ssid";
    const char* passwdKey = "passwd";
    const char* mbtaStopKey = "mbtaStop";
    Preferences flashStorage;

public:
    void storeCredentials(const char* ssid, const char* password) {
        Serial.printf("storing wifi credentials: %s, %s \n", ssid, password);
        flashStorage.begin(storageNamespace);
        flashStorage.putString(ssidKey, ssid);
        flashStorage.putString(passwdKey, password);
        flashStorage.end();
    }

    void storeSSID(const char* ssid)
    {
        flashStorage.begin(storageNamespace);
        flashStorage.putString(ssidKey, ssid);
        flashStorage.end();
    }

    void storePass(const char* password)
    {
        flashStorage.begin(storageNamespace);
        flashStorage.putString(passwdKey, password);
        flashStorage.end();
    }

    void storeStop(int stopNumber)
    {
        flashStorage.begin(storageNamespace);
        flashStorage.putInt(mbtaStopKey, stopNumber);
        flashStorage.end();
    }

    wifiCreds getCredentials() {
        wifiCreds credentials;

        flashStorage.begin(storageNamespace);
        auto ssidLength = flashStorage.getString(ssidKey, credentials.ssid, 20);
        auto passLength = flashStorage.getString(passwdKey, credentials.pass, 20);
        credentials.mbtaStop = flashStorage.getInt(mbtaStopKey);

        Serial.printf("Got creds: %s, %s, lens: %d, %d  \n", credentials.ssid, credentials.pass, ssidLength, passLength);
        flashStorage.end();

        return credentials;
    }


};