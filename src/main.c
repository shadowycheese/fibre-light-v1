#include "gfx.h"
#include <Arduino.h>

typedef struct
{
        uint8_t colour1;
        uint8_t colour2;
        uint8_t phase;
        uint8_t brightness;
        int8_t direction;
} palette_t;

static uint8_t _colours[15][3] = {
    {0xFF, 0x00, 0x00}, //
    {0x00, 0xFF, 0x00}, //
    {0x00, 0x00, 0xFF}, //
    {0xFF, 0xFF, 0x00}, //
    {0xFF, 0x00, 0xFF}, //
    {0x00, 0xFF, 0xFF}, //
    {0x80, 0x00, 0x80}, //
    {0xFF, 0x00, 0x80}, //
    {0x80, 0x00, 0xFF}, //
    {0x00, 0x80, 0x80}, //
    {0x00, 0xFF, 0x80}, //
    {0x00, 0x00, 0xFF}, //
    {0x80, 0x80, 0x00}, //
    {0xFF, 0x80, 0x00}, //
    {0x80, 0xFF, 0x00},
};

palette_t _palette[4];

void setup()
{
    for (int i = 0; i < 4; i++)
    {
        _palette[i].colour1 = random() % 15;
        _palette[i].colour2 = random() % 15;
        _palette[i].brightness = 0x80 + (0x20 * i);
        _palette[i].phase = 0x00;
        _palette[i].direction = 1;
    }

    setup_gfx();

    for (uint8_t i = 0; i < 64; i++)
    {
        draw_pixel(i % 8, i / 8, i & 0x3);
    }
}

uint8_t update_palette(uint8_t palette)
{
    if (_palette[palette].direction == 1)
    {
        _palette[palette].brightness++;

        if (_palette[palette].brightness == 0xFF)
        {
            _palette[palette].direction = -1;
        }
    }
    else if (_palette[palette].direction == -1)
    {
        _palette[palette].brightness--;

        if (_palette[palette].brightness <= 0x40)
        {
            _palette[palette].direction = 0;
            _palette[palette].brightness = 0x40;
            _palette[palette].phase = 0;
        }
    }

    if (_palette[palette].direction == 0)
    {
        _palette[palette].phase += 2;

        uint32_t ph = (uint32_t)_palette[palette].phase;

        uint8_t c1 = _palette[palette].colour1;
        uint8_t c2 = _palette[palette].colour2;
        uint32_t br32 = _palette[palette].brightness;

        int32_t r = _colours[c1][0] + ((int32_t)(_colours[c2][0] - _colours[c1][0]) * ph) / 128;
        int32_t g = _colours[c1][1] + ((int32_t)(_colours[c2][1] - _colours[c1][1]) * ph) / 128;
        int32_t b = _colours[c1][2] + ((int32_t)(_colours[c2][2] - _colours[c1][2]) * ph) / 128;

        uint8_t r8 = (uint8_t)((r * br32) >> 8);
        uint8_t g8 = (uint8_t)((g * br32) >> 8);
        uint8_t b8 = (uint8_t)((b * br32) >> 8);

        set_pallete(palette, r8, g8, b8);

        if (_palette[palette].phase >= 0x80)
        {
            _palette[palette].direction = 1;
            _palette[palette].colour1 = _palette[palette].colour2;
            _palette[palette].colour2 = random() % 15;
        }
    }
    else
    {
        uint8_t c = _palette[palette].colour1;
        uint32_t br32 = (uint32_t)_palette[palette].brightness & 0xFF;

        uint32_t r32 = (uint32_t)_colours[c][0] & 0xFF;
        uint32_t g32 = (uint32_t)_colours[c][1] & 0xFF;
        uint32_t b32 = (uint32_t)_colours[c][2] & 0xFF;

        uint8_t r8 = (uint8_t)((r32 * br32) >> 8);
        uint8_t g8 = (uint8_t)((g32 * br32) >> 8);
        uint8_t b8 = (uint8_t)((b32 * br32) >> 8);

        set_pallete(palette, r8, g8, b8);
    }
}

void loop()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        update_palette(i);
    }

    /*
for (uint8_t i = 0; i < 4; i++)
{
    update_palette(i);

    uint8_t size = 8 - (i * 2);

    draw_rect(i, i, size, size, i);
}
*/

    commit_frame();

    delay(10);
}
