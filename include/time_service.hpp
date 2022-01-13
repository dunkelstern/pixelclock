#ifndef time_service_hpp__included
#define time_service_hpp__included

#include "settings.hpp"

#include <WiFiUdp.h>
#include <NTPClient.h>


class TimeService: public SettingsCallback {
    private:        
        // NTP time
        WiFiUDP ntpUDP;
        NTPClient *timeClient;

        // Timezone offsets for ntp
        int tz_offset_hours = 0;
        int tz_offset_minutes = 0;

        // If no NTP time is available user may set the time manually via
        // mqtt or web interface
        int tune_hour = 0;
        int tune_minute = 0;
        int tune_second = 0;

        bool ntp_enabled = true;
        char *ntp_server;
        
        void init_ntp();
        void shutdown_ntp();
        
    public:
        TimeService(Settings *conf);
        void loop();
        
        void tune(int hour, int minute, int second);
        
        int getHours();
        int getMinutes();
        int getSeconds();
        int getMilliSeconds();
        
        void settings_changed(const char *key, const char *value);
        void settings_changed(const char *key, int value);
};

#endif /* time_service_hpp__included */