#include "gfx.h"
#include <avr/interrupt.h>
#include <util/delay.h>

#define PIN_BIT 1
#define NUM_LEDS 64

uint8_t frame_buffer[16];

uint8_t pallete[4][3];

uint8_t set_pallete(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    pallete[index][0] = r;
    pallete[index][1] = g;
    pallete[index][2] = b;
}

uint8_t get_led_index(uint8_t x, uint8_t y)
{
    uint8_t matrix = 0;
    uint8_t localX = x % 4;
    uint8_t localY = y % 4;

    if (x < 4 && y < 4)
    {
        uint8_t tmp = localX;
        localX = 3 - localY;
        localY = tmp;
    }
    else if (x >= 4 && y < 4)
    {
        matrix = 1;
    }
    else if (x >= 4 && y >= 4)
    {
        uint8_t tmp = localX;
        localX = localY;
        localY = 3 - tmp;
        matrix = 2;
    }
    else
    {
        localX = 3 - localX;
        localY = 3 - localY;
        matrix = 3;
    }

    uint8_t localIndex = (localY * 4) + ((localY % 2 == 0) ? localX : (3 - localX));

    return (matrix * 16) + localIndex;
}

void draw_pixel(uint8_t x, uint8_t y, uint8_t palette_index)
{
    if (x > 7 || y > 7)
    {
        return;
    }

    uint8_t ledIndex = get_led_index(x, y);

    uint8_t byte = ledIndex >> 2;
    uint8_t bit_pair_shift = (ledIndex % 4) * 2;

    uint8_t mask = ~(3 << bit_pair_shift);
    uint8_t value = (mask & frame_buffer[byte]) | (palette_index << bit_pair_shift);

    frame_buffer[byte] = value;
}

void bit_bang_byte(uint8_t b)
{
    for (int8_t bit = 7; bit >= 0; bit--)
    {
        if (b & (1 << bit))
        {
            VPORTA.OUT |= (1 << PIN_BIT);
            __asm__("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
            VPORTA.OUT &= ~(1 << PIN_BIT);
            __asm__("nop\n nop\n nop\n");
        }
        else
        {
            VPORTA.OUT |= (1 << PIN_BIT);
            __asm__("nop\n nop\n nop\n nop\n");
            VPORTA.OUT &= ~(1 << PIN_BIT);
            __asm__("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
        }
    }
}

void draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t colorIndex)
{
    int8_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int8_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int8_t err = dx + dy, e2;

    for (;;)
    {
        draw_pixel(x0, y0, colorIndex);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colour)
{
    for (uint8_t j = x; j < x + w; j++)
    {
        draw_pixel(j, y, colour);
        draw_pixel(j, y + h - 1, colour);
    }
    for (uint8_t i = y; i < y + h; i++)
    {
        draw_pixel(x, i, colour);
        draw_pixel(x + w - 1, i, colour);
    }
}

void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colour)
{
    for (uint8_t i = y; i < y + h; i++)
    {
        for (uint8_t j = x; j < x + w; j++)
        {
            drawPixel(j, i, colour);
        }
    }
}

void commit_frame()
{
    cli();

    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t pixel = frame_buffer[i];

        for (int j = 0; j < 4; j++)
        {
            uint8_t c = (pixel >> (j * 2)) & 0x3;

            uint8_t r = pallete[c][0];
            uint8_t g = pallete[c][1];
            uint8_t b = pallete[c][2];

            bit_bang_byte(g); // Green
            bit_bang_byte(r); // Red
            bit_bang_byte(b); // Blue
        }
    }

    sei();
    _delay_us(300);
}

void setup_gfx()
{
    // VPORTA.DIR uses PA1
    VPORTA.DIR |= (1 << PIN_BIT);
    memset(frame_buffer, 0, sizeof(frame_buffer));
    memset(pallete, 0, sizeof(pallete));
}
