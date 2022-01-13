#include <WiFi.h>
#include <ESPmDNS.h>

#include "wifi_service.hpp"

void WifiService::settings_changed(const char *key, const char *value) {
    if (strcmp(key, "wifi_ssid") == 0) {
        if (ssid) free(ssid);
        ssid = (value == NULL) ? NULL : strdup(value);
    }
    if (strcmp(key, "wifi_pass") == 0) {
        if (password) free(password);
        password = (value == NULL) ? NULL : strdup(value);        
    }
    
    if (ssid != NULL) {
        connect_sta();
    } else {
        connect_ap();
    }
}

WifiService::WifiService(Settings *conf, const char *n): SettingsCallback(conf) {
    name = strdup(n);
    
    // load settings
    config->get("wifi_ssid", &ssid);
    config->get("wifi_pass", &password);
    
    // register callbacks to get notified if settings change
    register_callback("wifi_ssid");
    register_callback("wifi_pass");

    // connect to AP or open up own AP
    if (ssid != NULL) {
        connect_sta();
    } else {
        connect_ap();
    }
}

WifiService::~WifiService() {
    free(name);
    free(ssid);
    free(password);   
}

void WifiService::connect_sta() {
    // Run in station mode
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to network...");
    }
    if(!MDNS.begin(name)) {
        Serial.println("Error starting mDNS");
        return;
    }
    Serial.println(WiFi.localIP()); 
}

void WifiService::connect_ap() {
    // Run in AP mode
    // FIXME: AP mode
}

