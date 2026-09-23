#include "gfx.h"
#include <avr/interrupt.h>
#include <util/delay.h>

#define PIN_BIT 1 // Using PA1 (Bit 1 of PORTA)
#define NUM_LEDS 64
#define BRIGHTNESS 32 // 0-255 color intensity

uint8_t frameBuffer[64];

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

void draw_pixel(uint8_t x, uint8_t y, uint8_t colorIndex)
{
    if (x > 7 || y > 7)
    {
        return;
    }

    uint8_t ledIndex = get_led_index(x, y);

    frameBuffer[ledIndex] = colorIndex;
}

// Highly precise inline bit-streamer tuned for 16MHz on modern tinyAVR hardware
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

void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colorIndex)
{
    for (uint8_t j = x; j < x + w; j++)
    {
        draw_pixel(j, y, colorIndex);         // Top edge
        draw_pixel(j, y + h - 1, colorIndex); // Bottom edge
    }
    for (uint8_t i = y; i < y + h; i++)
    {
        draw_pixel(x, i, colorIndex);         // Left edge
        draw_pixel(x + w - 1, i, colorIndex); // Right edge
    }
}

void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colorIndex)
{
    for (uint8_t i = y; i < y + h; i++)
    {
        for (uint8_t j = x; j < x + w; j++)
        {
            drawPixel(j, i, colorIndex);
        }
    }
}

void commit_frame()
{
    cli(); // Lock interrupts tightly for flawless NeoPixel rendering

    for (uint8_t i = 0; i < 64; i++)
    {
        uint8_t pixel = frameBuffer[i];
        uint8_t r = frameBuffer[i] & 0xE0;
        uint8_t g = (frameBuffer[i] << 3) & 0xE0;
        uint8_t b = (frameBuffer[i] << 6) & 0xE0;

        bit_bang_byte(g >> 1); // Green
        bit_bang_byte(r >> 1); // Red
        bit_bang_byte(b >> 1); // Blue
    }

    sei();          // Restore interrupts
    _delay_us(300); // Latch command
}

void setup_gfx()
{
    // Use VPORTA.DIR to toggle PA1 as an output pin safely on the 412
    VPORTA.DIR |= (1 << PIN_BIT);
    memset(frameBuffer, 0, sizeof(frameBuffer));
}
