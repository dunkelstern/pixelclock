#include "pixelclock.hpp"


int PixelClock::correct_topology(int index) {
    int result = index + topology_offset;
    if (result < 0) {
        result += 60;
    }
    return result % 60;
}

Color PixelClock::add_colors(Color c1, Color c2) {
    int r,g,b;

    r = c1.R + c2.R;
    g = c1.G + c2.G;
    b = c1.B + c2.B;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    return Color(r,g,b);
}

void PixelClock::settings_changed(const char *key, Color value) {
    if (strcmp(key, "color_h") == 0) {
        hour_color = value;
    }
    if (strcmp(key, "color_m") == 0) {
        minute_color = value;
    }
    if (strcmp(key, "color_s") == 0) {
        second_color = value;
    }
}

void PixelClock::settings_changed(const char *key, int value) {
    if (strcmp(key, "brightness") == 0) {
        brightness = value;
    }
    if (strcmp(key, "enabled") == 0) {
        enabled = value;
    }
}

PixelClock::PixelClock(Settings *conf, TimeService *time_client): SettingsCallback(conf) {
    time = time_client;
    black = Color(0);

    config->get("color_h", &hour_color);
    config->get("color_m", &minute_color);
    config->get("color_s", &second_color);
    config->get("brightness", &brightness);
    int clock_enabled = 1;
    config->get("enabled", &clock_enabled);
    enabled = clock_enabled;

    register_callback("color_h");
    register_callback("color_m");
    register_callback("color_s");
    
    register_callback("brightness");
    register_callback("enabled");

    pixels = new NeoPixelBrightnessBus<NEOPIXEL_FEATURE, NEOPIXEL_TYPE>(60, NEOPIXEL_PIN);
    pixels->Begin();
}

void PixelClock::loop() {
    int second = time->getSeconds();
    int minute = time->getMinutes();
    int hour = time->getHours();
    int milliseconds = time->getMilliSeconds();
    

    float percentage_of_second = (float)(milliseconds) / 1000.0;
    float percentage_of_minute = (float)second / 60.0;
    
    // clear
    pixels->ClearTo(black);
    if (!enabled) {
        pixels->Show();
        return;
    }

    // second hand
    Color s1 = second_color, s2 = second_color;
    s1.Darken(percentage_of_second * 255.0);
    s2.Darken((1.0 - percentage_of_second) * 255.0);
    pixels->SetPixelColor(correct_topology(second), s1);
    pixels->SetPixelColor(correct_topology(second + 1), s2);
    
    // minute hand
    Color m1 = minute_color, m2 = minute_color;
    m1.Darken(percentage_of_minute * 255.0);
    m2.Darken((1.0 - percentage_of_minute) * 255.0);
    Color cm1 = pixels->GetPixelColor(correct_topology(minute));
    Color cm2 = pixels->GetPixelColor(correct_topology(minute + 1));
    m1 = add_colors(m1,cm1);
    m2 = add_colors(m2,cm2);
        
    pixels->SetPixelColor(correct_topology(minute), m1);
    pixels->SetPixelColor(correct_topology(minute + 1), m2);

    // hour hand
    pixels->SetPixelColor(correct_topology((hour % 12) * 5), hour_color);

    // display
    pixels->Show();
}
