#ifndef settings_hpp__included
#define settings_hpp__included

#include <functional>
#include "internal.hpp"

class Settings;

class SettingsCallback {
    protected:
        Settings *config;
        void register_callback(const char *key);

    public:    
        SettingsCallback(Settings *conf) { config = conf; };

        virtual void settings_changed(const char *key, String value);
        virtual void settings_changed(const char *key, char *value);
        virtual void settings_changed(const char *key, int value);
        virtual void settings_changed(const char *key, Color value);        

};

typedef enum _DataType {
    DataTypeString,
    DataTypeChar,
    DataTypeInt,
    DataTypeColor
} DataType;

typedef struct _CallbackListItem CallbackListItem;
typedef struct _Cache Cache;

class Settings {
    private:
        CallbackListItem *cb;
        Cache *cache;
        
        void runCallback(const char *key, String value);
        void runCallback(const char *key, char *value);
        void runCallback(const char *key, int value);
        void runCallback(const char *key, Color value);

        Cache *getCache(const char *key, DataType typ, bool *created = NULL);

    public:    
        Settings();
        ~Settings();

        void set(const char *key, String value, bool volatile_value=false);
        void set(const char *key, char *value, bool volatile_value=false);
        void set(const char *key, int value, bool volatile_value=false);
        void set(const char *key, Color value, bool volatile_value=false);

        void setDefault(const char *key, String value, bool volatile_value=false);
        void setDefault(const char *key, const char *value, bool volatile_value=false);
        void setDefault(const char *key, int value, bool volatile_value=false);
        void setDefault(const char *key, Color value, bool volatile_value=false);
        
        void get(const char *key, String *result);
        void get(const char *key, char **result);
        void get(const char *key, int *result);
        void get(const char *key, Color *result);
        
        void save();
        
        CallbackListItem *register_callback(const char *key, SettingsCallback *cb);
        void unregister_callback(CallbackListItem *callback);

};

#endif /* settings_hpp__included */