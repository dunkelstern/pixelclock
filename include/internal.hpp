#ifndef internal_hpp__included
#define internal_hpp__included

#include "config.hpp"

#include <NeoPixelBrightnessBus.h>

#if NEOPIXEL_IS_RGBW
    #define NEOPIXEL_FEATURE NeoGrbwFeature
    typedef RgbwColor Color;
#else
    #define NEOPIXEL_FEATURE NeoGrbFeature
    typedef RgbColor Color;
#endif

#endif /* internal_hpp__included */
