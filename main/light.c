#include "main/light.h"

// See https://en.wikipedia.org/wiki/Mired

void LIGHT_rgb_from_mired(
    uint16_t mired,
    LIGHT_rgb_t *rgb)
{
    LIGHT_rgb_from_kelvin(1000000 / mired, rgb);
}

// See http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/

void LIGHT_rgb_from_kelvin(
    uint16_t kelvin,
    LIGHT_rgb_t *rgb)
{
    uint8_t temp = kelvin / 100;
    int16_t r, g, b;

    // Calculate red
    if (temp <= 66) {
        r = 255;
    } else {
        r = temp - 60;
        r = 329.698727446 * pow(r, -0.1332047592);
        if (r < 0) r = 0;
        if (r > 255) r = 255;
    }

    // Calculate green
    if (temp  <= 66) {
        g = temp;
        g = 99.4708025861 * log(g) - 161.1195681661;
        if (g < 0) g = 0;
        if (g > 255) g = 255;
    } else {
        g = temp - 60;
        g = 288.1221695283 * pow(g, -0.0755148492);
        if (g < 0) g = 0;
        if (g > 255) g = 255;
    }

    // Calculate blue
    if (temp  >= 66) {
        b = 255;
    } else if (temp <= 19) {
        b = 0;
    } else {
        b = temp - 10;
        b = 138.5177312231 * log(b) - 305.0447927307;
        if (b < 0) b = 0;
        if (b > 255) b = 255;
    }

    rgb->r = r;
    rgb->g = g;
    rgb->b = b;
}

// See http://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/

static void LIGHT_rgb_from_HS()
{

}

