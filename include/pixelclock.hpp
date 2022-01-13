#ifndef pixelclock_hpp__included
#define pixelclock_hpp__included

#include "internal.hpp"
#include "settings.hpp"
#include "time_service.hpp"

class PixelClock: public SettingsCallback {
    private:
        TimeService *time;
        
        NeoPixelBrightnessBus<NEOPIXEL_FEATURE, NEOPIXEL_TYPE> *pixels;

        Color black;
        Color second_color;
        Color minute_color;
        Color hour_color;

        int brightness = 255;
        bool enabled = true;
        int topology_offset = 0;

        int correct_topology(int index);
        Color add_colors(Color c1, Color c2);
        
    public:
        PixelClock(Settings *conf, TimeService *time);
        void loop();
        
        void settings_changed(const char *key, int value);
        void settings_changed(const char *key, Color value);        

};

#endif /* pixelclock_hpp__included */