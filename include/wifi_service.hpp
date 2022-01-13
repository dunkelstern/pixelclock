#ifndef wifi_servide_hpp__included
#define wifi_service_hpp__included

#include "settings.hpp"

class WifiService: public SettingsCallback {
    private:
        // TODO: load these from config always
        char *ssid;
        char *password;
        char *name;
        
        void connect_sta();
        void connect_ap();

    public:
        WifiService(Settings *conf, const char *hostname);
        ~WifiService();
        
        void settings_changed(const char *key, const char *value);
};

#endif /* wifi_service_hpp__included */