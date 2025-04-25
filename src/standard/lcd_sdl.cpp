#include "lcd_sdl.h"
#include "path_util.h"
#include "emu.h"
#include "mcu.h"
#include <string>

const SDL_Rect lcd_button_regions_sc55[32] = {
    {  38,  36, 67, 19}, // Power
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    { 968,  38, 53, 18}, // Instrument
    {1024,  38, 53, 18},
    { 754,  82, 26, 26}, // Mute
    { 754,  35, 26, 26}, // All
    {   0,   0,  0,  0},
    { 968, 178, 53, 18}, // MIDI ch
    {1024, 178, 53, 18},
    { 968, 132, 53, 18}, // Chorus
    {1024, 132, 53, 18},
    { 968,  85, 53, 18}, // Pan
    {1024,  85, 53, 18},
    { 903,  37, 53, 18}, // Part R
    {   0,   0,  0,  0},
    { 831, 178, 53, 18}, // Key shift
    { 887, 178, 53, 18},
    { 831, 132, 53, 18}, // Reverb
    { 887, 132, 53, 18},
    { 831,  85, 53, 18}, // Level
    { 887,  85, 53, 18},
    { 849,  37, 53, 18}, // Part L
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0},
    {   0,   0,  0,  0}
};

const int button_map_sc55[][2] = {
    {SDL_SCANCODE_Q,           MCU_BUTTON_POWER},
    {SDL_SCANCODE_W,           MCU_BUTTON_INST_ALL},
    {SDL_SCANCODE_E,           MCU_BUTTON_INST_MUTE},
    {SDL_SCANCODE_R,           MCU_BUTTON_PART_L},
    {SDL_SCANCODE_T,           MCU_BUTTON_PART_R},
    {SDL_SCANCODE_Y,           MCU_BUTTON_INST_L},
    {SDL_SCANCODE_U,           MCU_BUTTON_INST_R},
    {SDL_SCANCODE_I,           MCU_BUTTON_KEY_SHIFT_L},
    {SDL_SCANCODE_O,           MCU_BUTTON_KEY_SHIFT_R},
    {SDL_SCANCODE_P,           MCU_BUTTON_LEVEL_L},
    {SDL_SCANCODE_LEFTBRACKET, MCU_BUTTON_LEVEL_R},
    {SDL_SCANCODE_A,           MCU_BUTTON_MIDI_CH_L},
    {SDL_SCANCODE_S,           MCU_BUTTON_MIDI_CH_R},
    {SDL_SCANCODE_D,           MCU_BUTTON_PAN_L},
    {SDL_SCANCODE_F,           MCU_BUTTON_PAN_R},
    {SDL_SCANCODE_G,           MCU_BUTTON_REVERB_L},
    {SDL_SCANCODE_H,           MCU_BUTTON_REVERB_R},
    {SDL_SCANCODE_J,           MCU_BUTTON_CHORUS_L},
    {SDL_SCANCODE_K,           MCU_BUTTON_CHORUS_R},
    {SDL_SCANCODE_LEFT,        MCU_BUTTON_PART_L},
    {SDL_SCANCODE_RIGHT,       MCU_BUTTON_PART_R},
};

const int button_map_jv880[][2] = {
    {SDL_SCANCODE_P,     MCU_BUTTON_PREVIEW},
    {SDL_SCANCODE_LEFT,  MCU_BUTTON_CURSOR_L},
    {SDL_SCANCODE_RIGHT, MCU_BUTTON_CURSOR_R},
    {SDL_SCANCODE_TAB,   MCU_BUTTON_DATA},
    {SDL_SCANCODE_Q,     MCU_BUTTON_TONE_SELECT},
    {SDL_SCANCODE_A,     MCU_BUTTON_PATCH_PERFORM},
    {SDL_SCANCODE_W,     MCU_BUTTON_EDIT},
    {SDL_SCANCODE_E,     MCU_BUTTON_SYSTEM},
    {SDL_SCANCODE_R,     MCU_BUTTON_RHYTHM},
    {SDL_SCANCODE_T,     MCU_BUTTON_UTILITY},
    {SDL_SCANCODE_S,     MCU_BUTTON_MUTE},
    {SDL_SCANCODE_D,     MCU_BUTTON_MONITOR},
    {SDL_SCANCODE_F,     MCU_BUTTON_COMPARE},
    {SDL_SCANCODE_G,     MCU_BUTTON_ENTER},
};

void LCD_VolumeChanged(lcd_t& lcd) {
    if (lcd.volume > 1.0f) {
        lcd.volume = 1.0f;
    }
    if (lcd.volume < 0.0f) {
        lcd.volume = 0.0f;
    }
    if (lcd.volume != 0.0f) {
        float vol = powf(10.0f, (-80.0f * (1.0f - lcd.volume)) / 20.0f); // or volume ^ 8 (0 < volume < 1)
        MCU_SetVolume(*lcd.mcu, (uint16_t) (vol * UINT16_MAX));
    } else {
        MCU_SetVolume(*lcd.mcu, 0);
    }
}

LCD_SDL_Backend::~LCD_SDL_Backend()
{
    Stop();
}

bool LCD_SDL_Backend::Start(lcd_t& lcd)
{
    m_lcd                 = &lcd;
    int32_t screen_width  = (int32_t)m_lcd->width; 
    int32_t screen_height = (int32_t)m_lcd->height;

    if (m_lcd->mcu->romset == Romset::JV880)
    {
        screen_width  = (int32_t)m_lcd->width;
        screen_height = (int32_t)m_lcd->height;
    }
    else
    {
        screen_width  = (int32_t)m_lcd->width;
        screen_height = (int32_t)m_lcd->height;
    }

    std::string title = "Nuked SC-55: ";

    title += EMU_RomsetName(m_lcd->mcu->romset);

    if(m_lcd->mcu->romset == Romset::MK1 || m_lcd->mcu->romset == Romset::MK2)
    {
        m_image = SDL_LoadBMP((const char*)(base_path / "sc55_background.bmp").u8string().c_str());
        if(m_image)
        {
            background_enabled = true;
            screen_width       = 1120;
            screen_height      = 233;
        }
    }

    m_window = SDL_CreateWindow(title.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                screen_width,
                                screen_height,
                                SDL_WINDOW_SHOWN);
    if (!m_window)
        return false;

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer)
        return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "BEST");

    m_texture = SDL_CreateTexture(
        m_renderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, (int32_t)m_lcd->width, (int32_t)m_lcd->height);
    if (!m_texture)
        return false;

    if(background_enabled)
    {
        m_background = SDL_CreateTextureFromSurface(m_renderer, m_image);
        if (!m_background)
            return false;
    }

    return true;
}

void LCD_SDL_Backend::Stop()
{
    if(m_background)
    {
        SDL_DestroyTexture(m_background);
        m_background = nullptr;
    }

    if(m_image)
    {
        SDL_FreeSurface(m_image);
        m_image = nullptr;
    }

    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void LCD_SDL_Backend::HandleEvent(const SDL_Event& sdl_event)
{
    // Do not respond if not acting on a particular window
    switch (sdl_event.type)
    {
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:        
        case SDL_MOUSEWHEEL:        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (sdl_event.key.windowID != SDL_GetWindowID(m_window))
            {
                return;
            }
            break;
        case SDL_WINDOWEVENT:
            if (sdl_event.window.windowID != SDL_GetWindowID(m_window))
            {
                return;
            }
            break;
        default:
            break;
    }

    // JV880 Encoder dial
    if (sdl_event.type == SDL_KEYDOWN && m_lcd->mcu->romset == Romset::JV880)
    {
        if (sdl_event.key.keysym.scancode == SDL_SCANCODE_COMMA)
            MCU_EncoderTrigger(*m_lcd->mcu, 0);
        if (sdl_event.key.keysym.scancode == SDL_SCANCODE_PERIOD)
            MCU_EncoderTrigger(*m_lcd->mcu, 1);
    }

    // GUI Controls for SC-55
    if ((m_lcd->mcu->romset == Romset::MK1 || m_lcd->mcu->romset == Romset::MK2) && background_enabled) 
    {
        switch (sdl_event.type)
        {
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN: {
            if (sdl_event.button.button == 1) {
                if (drag_volume_knob || (sdl_event.button.x >= 153 && sdl_event.button.x <= 212 && sdl_event.button.y >= 42 && sdl_event.button.y <= 101)) {
                    drag_volume_knob = (sdl_event.type == SDL_MOUSEBUTTONDOWN) || (drag_volume_knob && sdl_event.type != SDL_MOUSEBUTTONUP);
                }
                if (sdl_event.button.clicks == 2 && (sdl_event.button.x >= 153 && sdl_event.button.x <= 212 && sdl_event.button.y >= 42 && sdl_event.button.y <= 101)) {
                    m_lcd->volume = 0.8f;
                    LCD_VolumeChanged(*m_lcd);
                } 
            }
            int32_t x = sdl_event.button.x;
            int32_t y = sdl_event.button.y;
            int mask  = 0;
            uint32_t button_pressed = m_lcd->mcu->button_pressed;
            for (int i = 0; i < 32; i++) {
                const SDL_Rect *rect = &lcd_button_regions_sc55[i];
                if (rect->x == 0 && rect->y == 0 && rect->w == 0 && rect->h == 0) continue;
                if (x >= rect->x && x <= rect->x + rect->w && y >= rect->y && y <= rect->y + rect->h) {
                    mask |= 1 << i;
                }
            }
            if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
                button_pressed |= mask;
            else
                button_pressed &= ~mask;
            m_lcd->mcu->button_pressed = button_pressed;
            break;
        }
        case SDL_MOUSEMOTION:
            if (drag_volume_knob) {
                float angle = (float) (atan2(sdl_event.motion.y - 72, sdl_event.motion.x - 183) + 270.0f / 180.0f * M_PI);
                if (isnan(angle)) {
                    angle = (float)(270.0 / 180.0 * M_PI);
                }
                if (angle > 2.0 * M_PI) {
                    angle = angle - (float)(2.0 * M_PI);
                }
                if (angle > 330.0f / 180.0f * M_PI)
                    angle = float(330.0 / 180.0 * M_PI);
                if (angle < 30.0f / 180.0f * M_PI)
                    angle = float(30.0f / 180.0f * M_PI);
                float delta = (float)((angle - 30.0 / 180.0 * M_PI) / (300.0 / 180.0 * M_PI)) - m_lcd->volume;
                if (abs(delta) > 0.5f) {
                    delta = 0.0f;
                }
                if (m_lcd->volume > 0.8f || m_lcd->volume + delta > 0.8f) {
                    if (delta > 0.005f) {
                        delta = 0.005f;
                    }
                    if (delta < -0.005f) {
                        delta = -0.005f;
                    }
                }
                m_lcd->volume += delta;
                LCD_VolumeChanged(*m_lcd);
            }
            break;
        case SDL_MOUSEWHEEL:
            if (sdl_event.wheel.mouseX >= 153 && sdl_event.wheel.mouseX <= 212 && sdl_event.wheel.mouseY >= 42 && sdl_event.wheel.mouseY <= 101) {
                int32_t relval = sdl_event.wheel.y;
                if (relval > 10) { // Maximum ±2dB incremental
                    relval = 10;
                }
                if (relval < -10) {
                    relval = -10;
                }
                m_lcd->volume += (float)relval / 400.0f;
                LCD_VolumeChanged(*m_lcd);
                }
            break;

        default:
            break;
        }
    }

    // Keyboard control
    switch (sdl_event.type)
    {
        case SDL_WINDOWEVENT:
            if (sdl_event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                m_quit_requested = true;
            }
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (sdl_event.key.repeat)
                break;

            int      mask           = 0;
            uint32_t button_pressed = m_lcd->mcu->button_pressed;

            auto button_map  = m_lcd->mcu->is_jv880 ? button_map_jv880 : button_map_sc55;
            auto button_size =
                (m_lcd->mcu->is_jv880 ? sizeof(button_map_jv880) : sizeof(button_map_sc55)) / sizeof(button_map_sc55[0]);
            for (size_t i = 0; i < button_size; i++)
            {
                if (button_map[i][0] == sdl_event.key.keysym.scancode)
                    mask |= (1 << button_map[i][1]);
            }

            if (sdl_event.type == SDL_KEYDOWN)
                button_pressed |= mask;
            else
                button_pressed &= ~mask;

            m_lcd->mcu->button_pressed = button_pressed;

#if 0
                if (sdl_event.key.keysym.scancode >= SDL_SCANCODE_1 && sdl_event.key.keysym.scancode < SDL_SCANCODE_0)
                {
#if 0
                    int kk = sdl_event.key.keysym.scancode - SDL_SCANCODE_1;
                    if (sdl_event.type == SDL_KEYDOWN)
                    {
                        MCU_PostUART(0xc0);
                        MCU_PostUART(118);
                        MCU_PostUART(0x90);
                        MCU_PostUART(0x30 + kk);
                        MCU_PostUART(0x7f);
                    }
                    else
                    {
                        MCU_PostUART(0x90);
                        MCU_PostUART(0x30 + kk);
                        MCU_PostUART(0);
                    }
#endif
                    int kk = sdl_event.key.keysym.scancode - SDL_SCANCODE_1;
                    const int patch = 47;
                    if (sdl_event.type == SDL_KEYDOWN)
                    {
                        static int bend = 0x2000;
                        if (kk == 4)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0x7f);
                        }
                        else if (kk == 3)
                        {
                            bend += 0x100;
                            if (bend > 0x3fff)
                                bend = 0x3fff;
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                        }
                        else if (kk == 2)
                        {
                            bend -= 0x100;
                            if (bend < 0)
                                bend = 0;
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                        }
                        else if (kk)
                        {
                            MCU_PostUART(0xc1);
                            MCU_PostUART(patch);
                            MCU_PostUART(0xe1);
                            MCU_PostUART(bend & 127);
                            MCU_PostUART((bend >> 7) & 127);
                            MCU_PostUART(0x91);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0x7f);
                        }
                        else if (kk == 0)
                        {
                            //MCU_PostUART(0xc0);
                            //MCU_PostUART(patch);
                            MCU_PostUART(0xe0);
                            MCU_PostUART(0x00);
                            MCU_PostUART(0x40);
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x37);
                            MCU_PostUART(0x7f);
                        }
                    }
                    else
                    {
                        if (kk == 1)
                        {
                            MCU_PostUART(0x91);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0);
                        }
                        else if (kk == 0)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x37);
                            MCU_PostUART(0);
                        }
                        else if (kk == 4)
                        {
                            MCU_PostUART(0x99);
                            MCU_PostUART(0x32);
                            MCU_PostUART(0);
                        }
                    }
                }
#endif
            break;
        }
    }
}

void LCD_SDL_Backend::Render()
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = (int32_t)m_lcd->width;
    rect.h = (int32_t)m_lcd->height;
    SDL_UpdateTexture(m_texture, &rect, m_lcd->buffer, lcd_width_max * 4);

    if ((m_lcd->mcu->romset == Romset::MK1 || m_lcd->mcu->romset == Romset::MK2) && background_enabled) {
        SDL_Rect srcrect, dstrect;
        srcrect.x =    0;
        srcrect.y =    0;
        srcrect.w = 2240;
        srcrect.h =  466;
        SDL_RenderCopy(m_renderer, m_background, &srcrect, NULL);
        if((m_lcd->button_enable & 1) != 0 || (m_lcd->button_enable & 2) != 0) {
            srcrect.x =   0;
            srcrect.y = 466;
            srcrect.w =  52;
            srcrect.h =  52;
            dstrect.x = 754;
            dstrect.w =  26;
            dstrect.h =  26;
            if ((m_lcd->button_enable & 1) != 0) { // ALL
                dstrect.y = 35;
                SDL_RenderCopy(m_renderer, m_background, &srcrect, &dstrect);
            }
            if ((m_lcd->button_enable & 2) != 0) { // MUTE
                dstrect.y = 82;
                SDL_RenderCopy(m_renderer, m_background, &srcrect, &dstrect);
            }
        }
        if ((m_lcd->button_enable & 4) != 0) { // STANDBY
            srcrect.x =   0;
            srcrect.y = 518;
            srcrect.w =  20;
            srcrect.h =  20;
            dstrect.x = 118;
            dstrect.y =  42;
            dstrect.w =  10;
            dstrect.h =  10;
            SDL_RenderCopy(m_renderer, m_background, &srcrect, &dstrect);
        }
        { // Volume
            srcrect.x =  54;
            srcrect.y = 468;
            srcrect.w = 118;
            srcrect.h = 118;
            dstrect.x = 153;
            dstrect.y =  42;
            dstrect.w =  59;
            dstrect.h =  59;
            SDL_RenderCopyEx(m_renderer, m_background, &srcrect, &dstrect, (m_lcd->volume - 0.5f) * 300.0, NULL, SDL_FLIP_NONE);
        }
        {
            int type = 1;
            if (m_lcd->mcu->romset == Romset::MK1) {
                switch (m_lcd->mcu->revision) {
                    case MK1version::REVISION_SC55_100:
                        type = 0;
                        break;
                    case MK1version::REVISION_SC55_110:
                        type = 2;
                        break;
                    case MK1version::REVISION_SC55_120:
                    case MK1version::REVISION_SC55_121:
                    case MK1version::REVISION_SC55_200:
                        type = -1;
                        break;
                    default:
                        type = 1;
                        break;
                }
            } 
            else if (m_lcd->mcu->romset == Romset::MK2) {
                type = 3;
            }
            if (type >= 0) {
                srcrect.x = 804 + 262 * (type & 1);
                srcrect.y = 466 + 50 * (type >> 1);
                srcrect.w = 262;
                srcrect.h =  50;
                dstrect.x = 533;
                dstrect.y = 195;
                dstrect.w = 131;
                dstrect.h =  25;
                SDL_RenderCopy(m_renderer, m_background, &srcrect, &dstrect);
            }
            if (type == -1 || type == 3) {
                if (type == -1) {
                    srcrect.x = 1592;
                }
                if (type == 3) {
                    srcrect.x = 1392;
                }
                srcrect.y = 466;
                srcrect.w = 200;
                srcrect.h = 104;
                dstrect.x = 696;
                dstrect.y = 174;
                dstrect.w = 100;
                dstrect.h =  52;
                SDL_RenderCopy(m_renderer, m_background, &srcrect, &dstrect);
            }
        }
        srcrect.x =   0;
        srcrect.y =   0;
        srcrect.w = 740;
        srcrect.h = 268;
        dstrect.x = 283;
        dstrect.y =  49;
        dstrect.w = 370;
        dstrect.h = 134;
        SDL_RenderCopy(m_renderer, m_texture, &srcrect, &dstrect);
    }
    else {
        SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    }
    SDL_RenderPresent(m_renderer);
}

bool LCD_SDL_Backend::IsQuitRequested() const
{
    return m_quit_requested;
}