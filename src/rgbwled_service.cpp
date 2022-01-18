#include "rgbwled_service.hpp"
#include "config.hpp"

void RGBWLedService::settings_changed(const char *key, Color value) {
    if (strcmp(key, "s_color") == 0) {
        requested_color.R = (uint8_t)((float)value.R * RED_CORRECTION);
        requested_color.G = (uint8_t)((float)value.G * GREEN_CORRECTION);
        requested_color.B = (uint8_t)((float)value.B * BLUE_CORRECTION);
        requested_color.W = (uint8_t)((float)value.W * WHITE_CORRECTION);
        
        fade_ms = millis();
    }
}

void RGBWLedService::settings_changed(const char *key, int value) {
    if (strcmp(key, "s_brightness") == 0) {
        requested_brightness = value;
        fade_ms = millis();
    }
    if (strcmp(key, "s_enabled") == 0) {
        enabled = value;
    }
}

RGBWLedService::RGBWLedService(Settings *conf): SettingsCallback(conf) {
    current_brightness = 0;
    fade_ms = millis();

    config->get("s_color", &requested_color);
    requested_color.R = (uint8_t)((float)requested_color.R * RED_CORRECTION);
    requested_color.G = (uint8_t)((float)requested_color.G * GREEN_CORRECTION);
    requested_color.B = (uint8_t)((float)requested_color.B * BLUE_CORRECTION);
    requested_color.W = (uint8_t)((float)requested_color.W * WHITE_CORRECTION);
    
    config->get("s_brightness", &requested_brightness);
    int strip_enabled = 1;
    config->get("s_enabled", &strip_enabled);
    enabled = strip_enabled;

    current_color = requested_color;

    register_callback("s_color");
    
    register_callback("s_brightness");
    register_callback("s_enabled");

    ledcSetup(0, 1000.0, 8);
    ledcAttachPin(RED_PIN, 0);
    ledcSetup(1, 1000.0, 8);
    ledcAttachPin(GREEN_PIN, 1);
    ledcSetup(2, 1000.0, 8);
    ledcAttachPin(BLUE_PIN, 2);
    ledcSetup(3, 1000.0, 8);
    ledcAttachPin(WHITE_PIN, 3);
}

RgbwColor RGBWLedService::blend(RgbwColor c1, RgbwColor c2, int b1, int b2, float percentage) {
    RgbwColor result;

    float dist = b2 - b1;
    float b = (b1 + (percentage * dist)) / 255.0;
    float c;
    
    dist = c2.R - c1.R;
    c = (c1.R + (percentage * dist)) * b;
    result.R = (uint8_t)(c > 255.0f ? 255.0 : c);
    
    dist = c2.G - c1.G;
    c = (c1.G + (percentage * dist)) * b;
    result.G = (uint8_t)(c > 255.0f ? 255.0 : c);
    
    dist = c2.B - c1.B;
    c = (c1.B + (percentage * dist)) * b;
    result.B = (uint8_t)(c > 255.0f ? 255.0 : c);

    dist = c2.W - c1.W;
    c = (c1.W + (percentage * dist)) * b;
    result.W = (uint8_t)(c > 255.0f ? 255.0 : c);

    return result;
}

void RGBWLedService::loop() {
    int milliseconds = millis();
    
    if (!enabled) {
        ledcWrite(0, 0);
        ledcWrite(1, 0);
        ledcWrite(2, 0);
        ledcWrite(3, 0);
        return;
    }
    
    float percentage = 1.0;
    if (fade_ms > 0) {
        percentage = (float)(milliseconds - fade_ms) / FADE_LENGTH;
        if (percentage > 1.0) {
            percentage = 1.0;
            fade_ms = 0;
            current_color = requested_color;
            current_brightness = requested_brightness;
        }
        RgbwColor color = blend(current_color, requested_color, current_brightness, requested_brightness, percentage);
        ledcWrite(0, color.R);
        ledcWrite(1, color.G);
        ledcWrite(2, color.B);
        ledcWrite(3, color.W);
    } else {
        float b = (float)current_brightness / 255.0f;
        if (b > 1.0) b = 1.0;
        ledcWrite(0, current_color.R * b);
        ledcWrite(1, current_color.G * b);
        ledcWrite(2, current_color.B * b);
        ledcWrite(3, current_color.W * b);        
    }
    
    // TODO: move white part of the color into W channel if enabled
}
