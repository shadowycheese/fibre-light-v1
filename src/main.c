#include "gfx.h"
#include <Arduino.h>

inline uint8_t rgb332(uint8_t r, uint8_t g, uint8_t b)
{
    return (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
}

byte _colours[4];
byte _pos = 0;

void setup()
{
    setup_gfx();

    _colours[0] = rgb332(0x7F, 0, 0);
    _colours[1] = rgb332(0, 0x7F, 0);
    _colours[2] = rgb332(0, 0, 0x7F);
    _colours[3] = rgb332(0x7F, 0x7F, 0);
}

void loop()
{
    for (int i = 0; i < 4; i++)
    {
        int ci = (_pos + i) % 4;
        int size = 8 - (i * 2);

        draw_rect(i, i, size, size, _colours[ci]);
    }

    _colours[_pos] = random() % 0xFF;

    _pos = (_pos + 1) & 0x03;

    commit_frame();
    delay(500);
}
