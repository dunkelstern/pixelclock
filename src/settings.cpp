#include "settings.hpp"
#include "ArduinoNvs.hpp"

struct _CallbackListItem {
    DataType typ;
    SettingsCallback *cb;
    char *key;
    struct _CallbackListItem *next;
};

struct _Cache {
    DataType typ;
    union {
        String string_value;
        char *char_value;
        int int_value;
        Color color_value;
    } data;
    char *key;
    struct _Cache *next;
};

/*
 * Callback helper superclass
 */
void SettingsCallback::register_callback(const char *key) {
    config->register_callback(key, this);
    // FIXME: Unregister callbacks
}

void SettingsCallback::settings_changed(const char *key, String value) {};
void SettingsCallback::settings_changed(const char *key, char *value) {};
void SettingsCallback::settings_changed(const char *key, int value) {};
void SettingsCallback::settings_changed(const char *key, Color value) {};

/*
 * Settings Class
 */
Settings::Settings() {
    NVS.begin();
    cb = NULL;
    cache = NULL;
}

Settings::~Settings() {
    CallbackListItem *item = cb, *nextItem;
    while (item != NULL) {
        free(item->key);
        nextItem = item->next;
        free(item);
        item = nextItem;
    }
    Cache *cacheItem = cache, *nextCacheItem;
    while(cacheItem != NULL) {
        free(cacheItem->key);
        nextCacheItem = cacheItem->next;
        if (cacheItem->typ == DataTypeChar) {
            free(cacheItem->data.char_value);
        }
        free(cacheItem);
        cacheItem = nextCacheItem;
    }
}

/*
 * Setters
 */

void Settings::set(const char *key, String value, bool volatile_value) {
    if (!volatile_value) {
        NVS.setString(key, value);
    }
    Cache *cached = getCache(key, DataTypeString);
    cached->data.string_value = value;

    runCallback(key, value);
    runCallback(key, value.c_str());
}

void Settings::set(const char *key, char *value, bool volatile_value) {
    if (!volatile_value) {
        NVS.setString(key, value);
    }
    Cache *cached = getCache(key, DataTypeChar);
    cached->data.char_value = strdup(value);
    
    runCallback(key, value);
    runCallback(key, String(value));
}

void Settings::set(const char *key, int value, bool volatile_value) {
    if (!volatile_value) {
        NVS.setInt(key, value);
    }
    Cache *cached = getCache(key, DataTypeInt);
    cached->data.int_value = value;

    runCallback(key, value);
}

void Settings::set(const char *key, Color value, bool volatile_value) {
    if (!volatile_value) {
        NVS.setInt(String(key) + "_r", value.R);
        NVS.setInt(String(key) + "_g", value.G);
        NVS.setInt(String(key) + "_b", value.B);    
    }
    Cache *cached = getCache(key, DataTypeColor);
    cached->data.color_value = value;

    runCallback(key, value);
}

/*
 * Defaults
 */

void Settings::setDefault(const char *key, String value, bool volatile_value) {
    Cache *cached = getCache(key, DataTypeString);
    if (!volatile_value) {
        String def = NVS.getString(key);
        if (def == String("")) {
            NVS.setString(key, value);        
            cached->data.string_value = value;
        } else {
            cached->data.string_value = def;
        }
    } else {
        cached->data.string_value = value;
    }
}

void Settings::setDefault(const char *key, const char *value, bool volatile_value) {
    Cache *cached = getCache(key, DataTypeChar);
    if (!volatile_value) {
        String def = NVS.getString(key);
        if (def == String("")) {
            NVS.setString(key, value);
            cached->data.char_value = strdup(value);
        } else {
            cached->data.char_value = strdup(def.c_str());
        }
    } else {
        cached->data.char_value = strdup(value);
    }
}

void Settings::setDefault(const char *key, int value, bool volatile_value) {
    Cache *cached = getCache(key, DataTypeInt);
    if (!volatile_value) {
        int64_t def = NVS.getInt(key, INT64_MIN);
        if (def == INT64_MIN) {
            NVS.setInt(key, value);
            cached->data.int_value = value;
        } else {
            cached->data.int_value = def;
        }
    } else {
        cached->data.int_value = value;
    }
}

void Settings::setDefault(const char *key, Color value, bool volatile_value) {
    Cache *cached = getCache(key, DataTypeColor);
    if (!volatile_value) {
        int64_t def = NVS.getInt(String(key) + "_r", INT64_MIN);
        if (def == INT64_MIN) {
            NVS.setInt(String(key) + "_r", value.R);
            NVS.setInt(String(key) + "_g", value.G);
            NVS.setInt(String(key) + "_b", value.B);
            cached->data.color_value = value;
        } else {
            cached->data.color_value.R = NVS.getInt(String(key) + "_r");
            cached->data.color_value.G = NVS.getInt(String(key) + "_g");
            cached->data.color_value.B = NVS.getInt(String(key) + "_b");
        }
    } else {
        cached->data.color_value = value;
    }
}

/*
 * Getters
 */

void Settings::get(const char *key, String *result) {
    bool created;
    Cache *cached = getCache(key, DataTypeString, &created);
    if (created) {
        *result = NVS.getString(key);
        cached->data.string_value = *result;
    } else {
        *result = cached->data.string_value;
    }
}

void Settings::get(const char *key, char **result) {
    bool created;
    Cache *cached = getCache(key, DataTypeChar, &created);
    if (created) {
        *result = strdup(NVS.getString(key).c_str());
        cached->data.char_value = strdup(*result);
    } else {
        *result = strdup(cached->data.char_value);
    }
}

void Settings::get(const char *key, int *result) {
    bool created;
    Cache *cached = getCache(key, DataTypeInt, &created);
    if (created) {
        *result = NVS.getInt(key);
        cached->data.int_value = *result;
    } else {
        *result = cached->data.int_value;
    }
}

void Settings::get(const char *key, Color *result) {
    bool created;
    Cache *cached = getCache(key, DataTypeColor, &created);
    if (created) {
        result->R = NVS.getInt(String(key) + "_r");
        result->G = NVS.getInt(String(key) + "_g");
        result->B = NVS.getInt(String(key) + "_b");
        cached->data.color_value = *result;
    } else {
        *result = cached->data.color_value;
    }
}

/*
 * Saving
 */

void Settings::save() {
    NVS.commit();
}

/*
 * Callbacks
 */

CallbackListItem *Settings::register_callback(const char *key, SettingsCallback *callback) {
    CallbackListItem *item = (CallbackListItem *)calloc(sizeof(CallbackListItem), 1);
    item->cb = callback;
    item->key = strdup(key);
    item->next = cb;
    cb = item;
    return item;
}

void Settings::unregister_callback(CallbackListItem *callback) {
    CallbackListItem *item = cb, *prev = NULL;
    
    if (item == callback) {
        cb = item->next;
    } else {
        while (item != NULL) {
            if (item == callback) {
                prev->next = item->next;
            }
            prev = item;
            item = item->next;
        }
    }
    free(callback->key);
    free(callback);
}

/*
 * Callback runners
 */

void Settings::runCallback(const char *key, String value) {
    CallbackListItem *item = cb;
    while (item != NULL) {
        item->cb->settings_changed(key, value);
        item = item->next;
    }
}

void Settings::runCallback(const char *key, char* value) {
    CallbackListItem *item = cb;
    while (item != NULL) {
        item->cb->settings_changed(key, value);
        item = item->next;
    }
}

void Settings::runCallback(const char *key, int value) {
    CallbackListItem *item = cb;
    while (item != NULL) {
        item->cb->settings_changed(key, value);
        item = item->next;
    }
}

void Settings::runCallback(const char *key, Color value) {
    CallbackListItem *item = cb;
    while (item != NULL) {
        item->cb->settings_changed(key, value);
        item = item->next;
    }
}

/*
 * Internal
 */

Cache *Settings::getCache(const char *key, DataType typ, bool *created) {
    Cache *item = cache;
    while (item != NULL) {
        if ((strcmp(key, item->key) == 0) && (typ == item->typ)) {
            break;
        }
        item = item->next;
    }
    
    if (item) {
        if (created != NULL) {
            *created = false;
        }
        return item;
    }
    
    item = (Cache *)calloc(sizeof(Cache), 1);
    item->typ = typ;
    item->key = strdup(key);
    item->next = cache;
    cache = item;
    
    if (created != NULL) {
        *created = true;
    }
    
    return item;
}
