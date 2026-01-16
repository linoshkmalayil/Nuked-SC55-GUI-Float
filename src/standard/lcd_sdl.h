#pragma once

#include "audio.h"
#include "lcd.h"
#include <SDL.h>

#define SIN45 0.7071067811865476f
#define DEG2RAD(d) ((d) * (float) M_PI / 180.0f)
#define RAD2DEG(r) ((r) * 180.0f / (float) M_PI)

typedef struct {
	int   x, y, w, h;
	int   dragging, changed, absolute;
	float angle, step, min, max, def, delta, initial;
} LCD_Knob;

class LCD_SDL_Backend : public LCD_Backend
{
public:
    LCD_SDL_Backend() = default;
    ~LCD_SDL_Backend();

    bool Start(lcd_t& lcd);
    void Stop();

    void HandleEvent(const SDL_Event& ev);
    void Render();

    bool IsQuitRequested() const;

    AudioVolume GetSDLVolume() { return lcd_sdl_volume; }

private:
    lcd_t*        m_lcd        = nullptr;
    SDL_Window*   m_window     = nullptr;
    SDL_Renderer* m_renderer   = nullptr;
    SDL_Texture*  m_texture    = nullptr;
    SDL_Surface*  m_image      = nullptr;
    SDL_Texture*  m_background = nullptr;
    AudioVolume   lcd_sdl_volume{};

    uint32_t drag_volume_knob  = 0;
    bool background_enabled    = false;

    LCD_Knob sc_volume = {
        .x = 153,
        .y = 42,
        .w = 59,
        .h = 59,
        .dragging = 0,
        .changed = 0,
        .absolute = 1,
        .angle = DEG2RAD(270.0f),
        .step = 0.0f,
        .min = DEG2RAD(30.0f),
        .max = DEG2RAD(330.0f),
        .def = DEG2RAD(270.0f),
        .delta = 0.0f,
        .initial = 0.0f,
    };

    LCD_Knob jv_volume = {
        .x = 23,
        .y = 86,
        .w = 59,
        .h = 59,
        .dragging = 0,
        .changed = 0,
        .absolute = 1,
        .angle = DEG2RAD(270.0f),
        .step = 0.0f,
        .min = DEG2RAD(30.0f),
        .max = DEG2RAD(330.0f),
        .def = DEG2RAD(270.0f),
        .delta = 0.0f,
        .initial = 0.0f,
    };

    LCD_Knob jv_encoder = {
        .x = 706,
        .y = 39,
        .w = 64,
        .h = 64,
        .dragging = 0,
        .changed = 0,
        .absolute = 0,
        .angle = DEG2RAD(180.0f),
        .step = DEG2RAD(15.0f),
        .min = 0.0f,
        .max = DEG2RAD(360.0f),
        .def = DEG2RAD(180.0f),
        .delta = 0.0f,
        .initial = 0.0f,
    };

    // When the user closes the window this becomes true
    bool m_quit_requested = false;
};
