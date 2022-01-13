# Pixel clock

This is an ornamental LED clock based on the standard SK6212 programmable
LED-Strips.

You will find a KiCAD schematic for an ESP32 module based PCB in the `/kicad`
folder.

To compile you will need Platform.io.

## Basic settings

For Hardware setup look into `/include/config.hpp` you can set up the following
things there:

1. `MY_NAME` is used as a MDNS Hostname as well as for MQTT topics if MQTT is enabled
2. `NEOPIXEL_IS_RGBW`, set this to 1 if you use Neopixel strips that have 4 color components (the 4. is usually White)
3. `NEOPIXEL_TYPE` see the documentation of [Neopixelbus]() for values you can use here...
4. `NEOPIXEL_PIN` the GPIO number where you connected the neopixel ring.

You have to configure the device by connecting to the AP it will open (`Pixelclock`, password is `pixelclock`) and open the website it provides at `<MY_NAME>.local`.

## TODO

- Standalone AP when no Wifi is configured
- Website that allows configuration
- JSON based API that will be used by the website
- Webserver to serve Website
- Add RTC support to PCB
- Add RTC support to code
- Add STL files for 3D printed parts
- Documentation and pictures of finished project

## Done

- MQTT connection
- NTP update to automatically fetch time
- Saving of values to internal flash to persist settings across reboots
- Actual display of clock
- Home Assistant autoconfiguration
- Basic PCB layout
- 3D Printed parts to build clock
- 3D Printed electronics enclosure
- Topology configuration (if you have to hang the thing upside down, etc.)
