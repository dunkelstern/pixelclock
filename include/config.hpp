#ifndef config_hpp__included
#define config_hpp__included

/*
 * Basic configuration
 */

// this is the hostname as well as the MQTT topic (if enabled)
#define MY_NAME "pixelclock1"

// define this to 1 if you're using RGBW LED strips, or to 0 if not
#define NEOPIXEL_IS_RGBW 1

// define the type of LEDs you use for the pixel ring 
#define NEOPIXEL_TYPE NeoSk6812Method

// the GPIO pin where you connected the LEDs
#define NEOPIXEL_PIN 33


#endif /* config_hpp__included */