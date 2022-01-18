#ifndef mqtt_service_hpp__included
#define mqtt_service_hpp__included

#include "internal.hpp"
#include "settings.hpp"
#include "time_service.hpp"

#include <PubSubClient.h>
#include <WiFi.h>

class MQTTService : SettingsCallback {
    private:
        bool enabled = false;
        bool homeassistant_autodetect = true;
        
        char *name;
        
        // TODO: Load these from config always
        char *server;
        int port;
        char *user;
        char *password;

        WiFiClient *espClient;
        PubSubClient *mqtt;

        void event(char* top, byte* message, unsigned int length);
        void init_mqtt();
        void shutdown_mqtt();
        void restart(std::function<void (void)>block);
        void reconnect();
        void publish_state();
        char *autodetect_json(String part, bool independent_brightness = false);
        void homeassistant();

    public:
        MQTTService(Settings *conf, const char *name);
        ~MQTTService();
        
        void loop();
        
        void settings_changed(const char *key, const char *value);
        void settings_changed(const char *key, int value);
};

#endif /* mqtt_service_hpp__included */