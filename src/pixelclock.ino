#include "internal.hpp"

#include "settings.hpp"
#include "wifi_service.hpp"
#include "mqtt_service.hpp"
#include "time_service.hpp"
#include "pixelclock.hpp"

#include "esp_wifi.h"

#if LED_STRIP_ENABLED
#include "rgbwled_service.hpp"
#endif

/*********************************************************
 * To change basic HW configuration look into config.hpp *
 *********************************************************/

Settings *conf;
WifiService *wifi;
MQTTService *mqtt;
TimeService *timeService;
PixelClock *pixelClock;

#if LED_STRIP_ENABLED
RGBWLedService *ledService;
#endif

void setup() {
    setCpuFrequencyMhz(240);
    esp_wifi_set_ps(WIFI_PS_NONE);
    
    Serial.begin(115200);
    conf = new Settings();
    
    conf->setDefault("color_h", Color(0, 0, 255));
    conf->setDefault("color_m", Color(0, 255, 0));
    conf->setDefault("color_s", Color(255, 0, 0));
    conf->setDefault("brightness", 255, true);
    conf->setDefault("enabled", 1, true);
    conf->setDefault("tz_offset_h", 0);
    conf->setDefault("tz_offset_m", 0);
    conf->setDefault("topology", 0);
    
    conf->setDefault("wifi_ssid", "");
    conf->setDefault("wifi_pass", "");
    
    conf->setDefault("ntp_server", "time.nist.gov");
    conf->setDefault("ntp_enabled", 1);
    
    conf->setDefault("mqtt_server", "10.0.0.2");
    conf->setDefault("mqtt_port", 1883);
    conf->setDefault("mqtt_user", "pixelclock");
    conf->setDefault("mqtt_pass", "pixelclock");
    conf->setDefault("mqtt_hass", 1);
    conf->setDefault("mqtt_enabled", 1);
    
    
    wifi = new WifiService(conf, MY_NAME);
    mqtt = new MQTTService(conf, MY_NAME);
    timeService = new TimeService(conf);
    pixelClock = new PixelClock(conf, timeService);
#if LED_STRIP_ENABLED
    conf->setDefault("s_color", Color(32, 32, 32));
    conf->setDefault("s_brightness", 255, true);
    conf->setDefault("s_enabled", 1, true);

    ledService = new RGBWLedService(conf);
#endif
}

void loop() {
    timeService->loop();
    mqtt->loop();
    pixelClock->loop();
#if LED_STRIP_ENABLED 
    ledService->loop();
#endif
    delay(33);
}
