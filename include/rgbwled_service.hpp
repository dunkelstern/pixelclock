#ifndef rgbwled_service_hpp__included
#define rgbwled_service_hpp__included

#include "internal.hpp"
#include "settings.hpp"

#define FADE_LENGTH 2000.0

class RGBWLedService: public SettingsCallback {
    private:
        RgbwColor current_color;
        RgbwColor requested_color;
        int64_t fade_ms;
        
        int current_brightness = 255;
        int requested_brightness = 255;
        
        bool enabled = true;

        RgbwColor blend(RgbwColor c1, RgbwColor c2, int b1, int b2, float percentage);
        
    public:
        RGBWLedService(Settings *conf);
        void loop();
        
        void settings_changed(const char *key, int value);
        void settings_changed(const char *key, Color value);        

};

#endif /* rgbwled_service_hpp__included */