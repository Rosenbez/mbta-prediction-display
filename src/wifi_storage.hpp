#include <Preferences.h>

struct wifiCreds {
    char ssid[20];
    char pass[20];
};

class WifiCredentialStore {
private:
    const char* storageNamespace = "wifiCreds";
    const char* ssidKey = "ssid";
    const char* passwdKey = "passwd";
    Preferences flashStorage;
public:
    void storeCredentials(const char* ssid, const char* password) {
        Serial.printf("storing wifi credentials: %s, %s \n", ssid, password);
        flashStorage.begin(storageNamespace );
        flashStorage.putString(ssidKey, ssid);
        flashStorage.putString(passwdKey, password);
        flashStorage.end();
    }

    wifiCreds getCredentials() {
        wifiCreds credentials;

        flashStorage.begin(storageNamespace);
        auto ssidLength = flashStorage.getString(ssidKey, credentials.ssid, 20);
        auto passLength = flashStorage.getString(passwdKey, credentials.pass, 20);

        Serial.printf("Got creds: %s, %s, lens: %d, %d  \n", credentials.ssid, credentials.pass, ssidLength, passLength);
        flashStorage.end();

        return credentials;
    }


};