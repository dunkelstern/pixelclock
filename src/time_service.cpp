#include "time_service.hpp"

void TimeService::settings_changed(const char *key, int value) {
    if (strcmp(key, "ntp_enabled") == 0) {
        if (ntp_enabled) {
            shutdown_ntp();
        }
        ntp_enabled = value;
        if (ntp_enabled) {
            init_ntp();
        } 
    }
    if (strcmp(key, "tz_offset_h") == 0) {
        tz_offset_hours = value;
    }
    if (strcmp(key, "tz_offset_m") == 0) {
        tz_offset_minutes = value;
    }
}

void TimeService::settings_changed(const char *key, const char *value) {
    if (strcmp(key, "ntp_server") == 0) {
        if (ntp_enabled) {
            shutdown_ntp();
        }
        if (ntp_server) free(ntp_server);
        ntp_server = strdup(value);
        if (ntp_enabled) {
            init_ntp();
        } 
    }
}

TimeService::TimeService(Settings *conf): SettingsCallback(conf) {
    // load settings from config
    ntp_server = NULL;
    config->get("ntp_server", &ntp_server);
    if (ntp_server == NULL) {
        ntp_server = strdup("time.nist.gov");
    }
    
    int enabled;
    config->get("ntp_enabled", &enabled);
    ntp_enabled = enabled;
    
    config->get("tz_offset_h", &tz_offset_hours);
    config->get("tz_offset_m", &tz_offset_minutes);

    // subscribe to settings updates
    register_callback("ntp_enabled");
    register_callback("ntp_server");
    register_callback("tz_offset_h");
    register_callback("tz_offset_m");
    
    // initialize NTP if enabled
    if (ntp_enabled) {
        init_ntp();
    }
}

void TimeService::init_ntp() {
    timeClient = new NTPClient(ntpUDP, ntp_server);
    timeClient->begin();
}

void TimeService::shutdown_ntp() {
    if (timeClient) {
        timeClient->end();
        delete(timeClient);
        timeClient = NULL;
    }
}

void TimeService::loop() {
    if (ntp_enabled) {
        timeClient->update();
    }
}

void TimeService::tune(int hour, int minute, int second) {
    // reset offsets and calculate new offsets
    tune_hour = 0;
    tune_hour = hour - getHours();
    tune_minute = 0;
    tune_minute = minute - getMinutes();
    tune_second = 0;
    tune_second = second - getSeconds();
}

int TimeService::getHours() {
    if (ntp_enabled) {
        return (timeClient->getHours() + tz_offset_hours) % 24;
    }
    int hour = (int)(((millis() / 1000.0) - getSeconds() - (getMinutes() * 60)) / 3600 + tune_hour) % 24;
    hour += tz_offset_hours;
    if (hour < 0) hour += 24;
    if (hour > 23) hour %= 24;
    return hour;
}

int TimeService::getMinutes() {
    if (ntp_enabled) {
        return (timeClient->getMinutes() + tz_offset_minutes) % 60;
    }

    int minute = (int)(((millis() / 1000.0) - getSeconds()) / 60 + tune_minute) % 60;
    minute += tz_offset_minutes;
    if (minute < 0) minute += 60;
    if (minute > 59) minute %= 60;
    return minute;
}

int TimeService::getSeconds() {
    if (ntp_enabled) {
        return timeClient->getSeconds();
    }
    
    return (int)((millis() / 1000.0) + tune_second) % 60;
}

int TimeService::getMilliSeconds() {
    static int lastMillis = 0;
    static int lastSecond = -1;

    if (getSeconds() != lastSecond) {
        lastMillis = millis();
        lastSecond = getSeconds();
    }
    
    return millis() - lastMillis;
}