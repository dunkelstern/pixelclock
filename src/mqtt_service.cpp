#include "mqtt_service.hpp"
#include "internal.hpp"

#include <ArduinoJson.h>


static Color parse_rgb(String data) {
    Color result(0);
    result.R = data.toInt();
    int comma = data.indexOf(',') + 1;
    result.G = data.substring(comma).toInt();
    comma = data.indexOf(',', comma) + 1;
    result.B = data.substring(comma).toInt();
    return result;
}


static char *format_rgb(Color *color) {
    char *result = (char *)malloc(12);
    snprintf(result, 12, "%d,%d,%d", color->R, color->G, color->B);
    return result;
}

void MQTTService::init_mqtt() {
    espClient = new WiFiClient();
    mqtt = new PubSubClient(*espClient);
    mqtt->setServer(server, port);
    mqtt->setKeepAlive(60);
    mqtt->setCallback([&](char* topic, byte* message, unsigned int length) {
        event(topic, message, length);
    });
}

void MQTTService::shutdown_mqtt() {
    mqtt->disconnect();
    delete(mqtt);
    delete(espClient);
    mqtt = NULL;
    espClient = NULL;
}

void MQTTService::restart(std::function<void (void)>block) {
    if (enabled) {
        shutdown_mqtt();
    }
    block();
    if (enabled) {
        init_mqtt();
    } 
}

void MQTTService::settings_changed(const char *key, int value) {
    if (strcmp(key, "mqtt_enabled") == 0) {
        restart([&]{
            enabled = value;
        });
    }
    if (strcmp(key, "mqtt_hass") == 0) {
        homeassistant_autodetect = value;
        homeassistant();
    }
    if (strcmp(key, "mqtt_port") == 0) {
        restart([&]{
            port = value;
        });
    }
}

void MQTTService::settings_changed(const char *key, const char *value) {
    if (strcmp(key, "mqtt_server") == 0) {
        restart([&]{
            if (server) free(server);
            server = strdup(value);
        }); 
    }
    if (strcmp(key, "mqtt_user") == 0) {
        restart([&]{
            if (user) free(user);
            user = strdup(value);
        }); 
    }
    if (strcmp(key, "mqtt_pass") == 0) {
        restart([&]{
            if (password) free(password);
            password = strdup(value);
        }); 
    }
}

MQTTService::MQTTService(Settings *conf, const char *n): SettingsCallback(conf) {
    name = strdup(n);
    espClient = NULL;
    mqtt = NULL;
    
    int ena;
    config->get("mqtt_enabled", &ena);
    enabled = ena;
    config->get("mqtt_hass", &ena);
    homeassistant_autodetect = ena;
    
    config->get("mqtt_server", &server);
    config->get("mqtt_port", &port);
    config->get("mqtt_user", &user);
    config->get("mqtt_pass", &password);
    
    // Subscribe to updates
    register_callback("mqtt_enabled");
    register_callback("mqtt_hass");
    register_callback("mqtt_server");
    register_callback("mqtt_port");
    register_callback("mqtt_user");
    register_callback("mqtt_pass");
    
    init_mqtt();
}

MQTTService::~MQTTService() {
    shutdown_mqtt();
    free(name);
    free(server);
    free(user);
    free(password);
}

char *MQTTService::autodetect_json(String part) {
    StaticJsonDocument <1024>json;
    JsonArray identifiers = json["device"].createNestedArray("identifiers");
    identifiers.add(MY_NAME);
    
    String my_name = String(name);
    
    json["device"]["manufacturer"] = "Dunkelstern";
    json["device"]["model"] = "Pixel Clock";
    json["device"]["name"] = name;

    json["device_class"] = "light";
    json["name"] = my_name + "_" + part;
    json["unique_id"] = my_name + "_" + part;

    json["command_topic"] = my_name + "/switch";
    json["state_topic"] = my_name + "/switch_state";

    json["rgb_command_topic"] = my_name + "/" + part + "/rgb";
    json["rgb_state_topic"] = my_name + "/" + part + "/rgb_state";
    //  json["rgb_value_template"] = "{{ red }},{{ green }},{{ blue }}";
    
    json["brightness_command_topic"] = my_name + "/brightness";
    json["brightness_state_topic"] = my_name + "/brightness_state";
    json["brightness_value_template"] = "{{ value }}";

    char *result = (char *)calloc(1024, 1);    
    serializeJson(json, result, 1024);
    return result;  
}


void MQTTService::publish_state() {
    char tz_string[6];
    int tz_offset_hours = 0, tz_offset_minutes = 0, brightness, topology_offset, clock_enabled;
    Color hour_color, minute_color, second_color;
    
    config->get("tz_offset_h", &tz_offset_hours);
    config->get("tz_offset_m", &tz_offset_minutes);
    config->get("topology", &topology_offset);
    
    config->get("color_h", &hour_color);
    config->get("color_m", &minute_color);
    config->get("color_s", &second_color);
    
    config->get("brightness", &brightness);
    config->get("enabled", &clock_enabled);
    
    if (tz_offset_hours < 0) {
        snprintf(tz_string, 6, "-%02d:%02d", abs(tz_offset_hours), abs(tz_offset_minutes));
    } else {
        snprintf(tz_string, 6, "%02d:%02d", abs(tz_offset_hours), abs(tz_offset_minutes));    
    }
    char *h, *m, *s;
    h = format_rgb(&hour_color);
    m = format_rgb(&minute_color);
    s = format_rgb(&second_color);
    
    String my_name = String(name);
    mqtt->publish((my_name + "/hours/rgb_state").c_str(), h, true);
    mqtt->publish((my_name + "/minutes/rgb_state").c_str(), m, true);
    mqtt->publish((my_name + "/seconds/rgb_state").c_str(), s, true);
    mqtt->publish((my_name + "/brightness_state").c_str(), String(brightness).c_str(), true);
    mqtt->publish((my_name + "/switch_state").c_str(), clock_enabled ? "ON" : "OFF", true);
    mqtt->publish((my_name + "/tz_offset_state").c_str(), tz_string, true);
    mqtt->publish((my_name + "/topology_state").c_str(), String(topology_offset).c_str(), true);

    free(h);
    free(m);
    free(s);
}



void MQTTService::event(char* top, byte* message, unsigned int length) {
    String msg;
    String topic;
    for (int i = strlen(name) + 1; i < strlen(top); i++) {
        topic += (char)top[i];
    }
    for (int i = 0; i < length; i++) {
        msg += (char)message[i];
    }

    if (topic == String("switch")) {
        int clock_enabled = (msg == String("ON"));
        config->set("enabled", clock_enabled, true);
        goto finished;
    }
    if (topic == String("brightness")) {
        int brightness = msg.toInt();
        config->set("brightness", brightness, true);
        goto finished;
    }
    if (topic == String("topology")) {
        int topology_offset = msg.toInt();
        config->set("topology", topology_offset);
        goto finished;
    }
    if (topic == String("tz_offset")) {
        int tz_offset_hours = msg.substring(0,2).toInt();
        int tz_offset_minutes = msg.substring(3).toInt();
        if (tz_offset_hours < 0) {
            tz_offset_minutes *= -1;
        }
        config->set("tz_offset_h", tz_offset_hours);
        config->set("tz_offset_m", tz_offset_minutes);
        goto finished;
    }
    if (topic == String("hours/rgb")) {
        Color hour_color = parse_rgb(msg);
        config->set("color_h", hour_color);
        goto finished;
    }
    if (topic == String("minutes/rgb")) {
        Color minute_color = parse_rgb(msg);
        config->set("color_m", minute_color);
        goto finished;
    }
    if (topic == String("seconds/rgb")) {
        Color second_color = parse_rgb(msg);
        config->set("color_s", second_color);
        goto finished;
    }
    return;
    
finished:
    config->save();
    publish_state();
    return;
}

void MQTTService::homeassistant() {
    if ((!homeassistant_autodetect) || (mqtt == NULL)) {
        return;
    }
    
    char *json;
    String prefix = String("homeassistant/light/") + name;

    json = autodetect_json(String("hours"));
    mqtt->beginPublish((prefix + "/hours/config").c_str(), strlen(json), true);
    mqtt->print(json);
    mqtt->endPublish();
    free(json);

    json = autodetect_json(String("minutes"));
    mqtt->beginPublish((prefix + "/minutes/config").c_str(), strlen(json), true);
    mqtt->print(json);
    mqtt->endPublish();
    free(json);

    json = autodetect_json(String("seconds"));
    mqtt->beginPublish((prefix + "/seconds/config").c_str(), strlen(json), true);
    mqtt->print(json);
    mqtt->endPublish();
    free(json);
}

void MQTTService::reconnect() {
    while (!mqtt->connected()) {
        Serial.println("Connecting to MQTT broker...");
        if (mqtt->connect(name, user, password)) {
            Serial.println("MQTT connected!");

            // subscribe to settings
            String my_name = String(name);
            mqtt->subscribe((my_name + "/hours/rgb").c_str());
            mqtt->subscribe((my_name + "/minutes/rgb").c_str());
            mqtt->subscribe((my_name + "/seconds/rgb").c_str());
            mqtt->subscribe((my_name + "/brightness").c_str());
            mqtt->subscribe((my_name + "/switch").c_str());
            mqtt->subscribe((my_name + "/tz_offset").c_str());
            mqtt->subscribe((my_name + "/topology").c_str());

            // publish auto detect
            homeassistant();
        } else {
            Serial.print("MQTT connection failed: ");
            Serial.print(mqtt->state());
            Serial.println();
            delay(5000);
        }
    }
}

void MQTTService::loop() {
    if (enabled) {
        if (!mqtt->connected()) {
            reconnect();
            publish_state();
        }
        mqtt->loop();
    } else {
        if (mqtt->connected()) {
            mqtt->disconnect();
        }
    }
}