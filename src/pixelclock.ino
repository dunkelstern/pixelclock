#include "internal.hpp"

#include "settings.hpp"
#include "wifi_service.hpp"
#include "mqtt_service.hpp"
#include "time_service.hpp"
#include "pixelclock.hpp"

/*********************************************************
 * To change basic HW configuration look into config.hpp *
 *********************************************************/

Settings *conf;
WifiService *wifi;
MQTTService *mqtt;
TimeService *timeService;
PixelClock *pixelClock;

void setup() {
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
}

void loop() {
    timeService->loop();
    mqtt->loop();
    pixelClock->loop();
    delay(33);
}
