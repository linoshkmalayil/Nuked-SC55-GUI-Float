/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "lcd.h"
#include "emu.h"
#include "lcd_back.h"
#include "lcd_font.h"
#include <cstring>

uint32_t inline LCD_MixColor(uint32_t color, uint8_t contrast) {
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t g = (color >>  8) & 0xFF;
    uint8_t r = color & 0xFF;

    b = (b * contrast) >> 8;
    g = (g * contrast) >> 8;
    r = (r * contrast) >> 8;

    return (color & 0xFF000000) | ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | (r & 0xFF);
}

void LCD_Enable(lcd_t& lcd, uint32_t enable)
{
    lcd.enable = enable;
}

void LCD_ButtonEnable(lcd_t& lcd, uint32_t enable)
{
    lcd.button_enable = enable;
}

void LCD_SetContrast(lcd_t& lcd, uint8_t contrast)
{
    if (lcd.mcu->is_jv880)
    {
        if (contrast > 10)
            contrast = 10;
    }
    else
    {
        if (contrast > 16)
            contrast = 16;
        else if(contrast < 1)
            contrast = 1;
    }

    lcd.contrast = contrast;
}

void LCD_Write(lcd_t& lcd, uint32_t address, uint8_t data)
{
    // No point taking a lock and updating LCD state if there's no one to observe it.
    if (!lcd.backend)
    {
        return;
    }

    if (address == 0)
    {
        if ((data & 0xe0) == 0x20)
        {
            lcd.LCD_DL = (data & 0x10) != 0;
            lcd.LCD_N  = (data & 0x8)  != 0;
            lcd.LCD_F  = (data & 0x4)  != 0;
        }
        else if ((data & 0xf8) == 0x8)
        {
            lcd.LCD_D = (data & 0x4) != 0;
            lcd.LCD_C = (data & 0x2) != 0;
            lcd.LCD_B = (data & 0x1) != 0;
        }
        else if ((data & 0xff) == 0x01)
        {
            lcd.LCD_DD_RAM = 0;
            lcd.LCD_ID     = 1;
            memset(lcd.LCD_Data, 0x20, sizeof(lcd.LCD_Data));
        }
        else if ((data & 0xff) == 0x02)
        {
            lcd.LCD_DD_RAM = 0;
        }
        else if ((data & 0xfc) == 0x04)
        {
            lcd.LCD_ID = (data & 0x2) != 0;
            lcd.LCD_S  = (data & 0x1) != 0;
        }
        else if ((data & 0xc0) == 0x40)
        {
            lcd.LCD_CG_RAM   = (data & 0x3f);
            lcd.LCD_RAM_MODE = 0;
        }
        else if ((data & 0x80) == 0x80)
        {
            lcd.LCD_DD_RAM   = (data & 0x7f);
            lcd.LCD_RAM_MODE = 1;
        }
        else
        {
            address += 0;
        }
    }
    else
    {
        if (!lcd.LCD_RAM_MODE)
        {
            lcd.LCD_CG[lcd.LCD_CG_RAM] = data & 0x1f;
            if (lcd.LCD_ID)
            {
                lcd.LCD_CG_RAM++;
            }
            else
            {
                lcd.LCD_CG_RAM--;
            }
            lcd.LCD_CG_RAM &= 0x3f;
        }
        else
        {
            if (lcd.LCD_N)
            {
                if (lcd.LCD_DD_RAM & 0x40)
                {
                    if ((lcd.LCD_DD_RAM & 0x3f) < 40)
                        lcd.LCD_Data[(lcd.LCD_DD_RAM & 0x3f) + 40] = data;
                }
                else
                {
                    if ((lcd.LCD_DD_RAM & 0x3f) < 40)
                        lcd.LCD_Data[lcd.LCD_DD_RAM & 0x3f] = data;
                }
            }
            else
            {
                if (lcd.LCD_DD_RAM < 80)
                    lcd.LCD_Data[lcd.LCD_DD_RAM] = data;
            }
            if (lcd.LCD_ID)
            {
                lcd.LCD_DD_RAM++;
            }
            else
            {
                lcd.LCD_DD_RAM--;
            }
            lcd.LCD_DD_RAM &= 0x7f;
        }
    }
    //fprintf(stderr, "%i %.2x ", address, data);
    // if (data >= 0x20 && data <= 'z')
    //     fprintf(stderr, "%c\n", data);
    //else
    //    fprintf(stderr, "\n");
}

void LCD_Init(lcd_t& lcd, mcu_t& mcu)
{
    lcd.mcu = &mcu;
}

bool LCD_Start(lcd_t& lcd)
{
    bool success = true;

    if (lcd.mcu->romset == Romset::JV880)
    {
        lcd.width  = 820;
        lcd.height = 100;
        lcd.color1 = 0x000000;
        lcd.color2 = 0x78b500;
    }
    else
    {
        lcd.width  = 741;
        lcd.height = 268;
        lcd.color1 = 0x000000;
        lcd.color2 = 0x0050c8;
    }

    if (lcd.backend)
    {
        if (!lcd.backend->Start(lcd))
        {
            success = false;
        }
    }

    return success;
}

void LCD_Stop(lcd_t& lcd)
{
    if(lcd.backend)
    {
        lcd.backend->Stop();
    }
}

uint32_t LCD_Fade(lcd_t& lcd, uint32_t color1, uint32_t color2) {
    auto NONZERO = [](uint32_t x, uint32_t y) {
        return x == 0 ? x : y;
    };

    if (color1 == color2) {
        return color2;
    }

    if (color1 == back_palette[back_data[0 * lcd.width + 0]]) {
        color1 = lcd.color2;
    }

    uint8_t sb = (color1 >> 16) & 0xFF;
    uint8_t sg = (color1 >> 8) & 0xFF;
    uint8_t sr =  color1 & 0xFF;
    uint8_t db = (color2 >> 16) & 0xFF;
    uint8_t dg = (color2 >> 8) & 0xFF;
    uint8_t dr =  color2 & 0xFF;
    uint8_t r  = (uint8_t) ((((uint32_t) sr * 2) + dr) / 3);
    uint8_t g  = (uint8_t) ((((uint32_t) sg * 2) + dg) / 3);
    uint8_t b  = (uint8_t) ((((uint32_t) sb * 2) + db) / 3);
    uint32_t color = ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | (r & 0xFF);

    if (color != lcd.color1 && color != lcd.color2) {
        if (r == sr)
            r += NONZERO((dr - sr) / 2, dr - sr);
        if (g == sg)
            g += NONZERO((dg - sg) / 2, dg - sg);
        if (b == sb)
            b += NONZERO((db - sb) / 2, db - sb);
    }
    color = ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | (r & 0xFF);
    return color;
}

void LCD_FontRenderStandard(lcd_t& lcd, uint8_t* LCD_CG, int32_t x, int32_t y, uint8_t ch, uint8_t cursor = 0)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    for (int i = 0; i < 8; i++)
    {
        if (i == 7 && cursor == 0) {
            break;
        }

        for (int j = 0; j < 5; j++)
        {
            uint32_t col;
            if (i == 7) {
                if (cursor == 2)
                {
                    col = lcd.color1;
                }
                else
                {
                    col = lcd.color2;
                }
            } else {
                if (f[i] & (1<<(4-j)))
                {
                    col = lcd.color1;
                }
                else
                {
                    col = lcd.color2;
                }
            }
            int xx = x + i * 6;
            int yy = y + j * 6;
            for (int ii = 0; ii < 5; ii++)
            {
                for (int jj = 0; jj < 5; jj++)
                {
                    lcd.buffer[xx+ii][yy+jj] = LCD_Fade(lcd, lcd.buffer[xx+ii][yy+jj], col);
                }
            }
        }
    }
}

void LCD_FontRenderLevel(lcd_t& lcd, uint8_t* LCD_CG, int32_t x, int32_t y, uint8_t ch, uint8_t width = 5)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < width; j++)
        {
            uint32_t col;
            if (f[i] & (1<<(4-j)))
            {
                col = lcd.color1;
            }
            else
            {
                col = lcd.color2;
            }
            int xx = x + i * 11;
            int yy = y + j * 26;
            for (int ii = 0; ii < 9; ii++)
            {
                for (int jj = 0; jj < 24; jj++)
                {
                    lcd.buffer[xx+ii][yy+jj] = LCD_Fade(lcd, lcd.buffer[xx+ii][yy+jj], col);
                }
            }
        }
    }
}

static const uint8_t LR[2][12][11] =
{
    {
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,0,0,0,0,0,0,0,0,0,},
        {1,1,1,1,1,1,1,1,1,1,1,},
        {1,1,1,1,1,1,1,1,1,1,1,},
    },
    {
        {1,1,1,1,1,1,1,1,1,0,0,},
        {1,1,1,1,1,1,1,1,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,1,1,1,1,1,1,1,1,0,},
        {1,1,1,1,1,1,1,1,1,0,0,},
        {1,1,0,0,0,0,0,1,1,0,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,1,1,0,},
        {1,1,0,0,0,0,0,0,0,1,1,},
        {1,1,0,0,0,0,0,0,0,1,1,},
    }
};

static const int LR_xy[2][2] = {
    { 70 , 264 },
    { 232, 264 }
};


void LCD_FontRenderLR(lcd_t& lcd, uint8_t* LCD_CG, uint8_t ch)
{
    uint8_t* f;
    if (ch >= 16)
        f = &lcd_font[ch - 16][0];
    else
        f = &LCD_CG[(ch & 7) * 8];
    int col;
    if (f[0] & 1)
    {
        col = lcd.color1;
    }
    else
    {
        col = lcd.color2;
    }
    for (int letter = 0; letter < 2; letter++)
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 11; j++)
            {
                if (LR[letter][i][j])
                    lcd.buffer[i+LR_xy[letter][0]][j+LR_xy[letter][1]] = LCD_Fade(lcd, lcd.buffer[i+LR_xy[letter][0]][j+LR_xy[letter][1]], col);
            }
        }
    }
}

void LCD_Render(lcd_t& lcd)
{
    if (!lcd.backend)
    {
        return;
    }

    if (!lcd.mcu->is_cm300 && !lcd.mcu->is_st && !lcd.mcu->is_scb55)
    {
        if (!lcd.mutex.try_lock())
        {
            // if the MCU is currently updating something, just drop the frame
            return;
        }

        // This is the only shared mutable state we need to complete rendering. Since rendering is relatively expensive,
        // we'll quickly take a copy, release the lock, and use it for this frame.
        uint32_t LCD_C      = lcd.LCD_C;
        uint32_t LCD_DD_RAM = lcd.LCD_DD_RAM;

        uint8_t LCD_CG[sizeof(lcd.LCD_CG)];
        memcpy(LCD_CG, lcd.LCD_CG, sizeof(LCD_CG));

        uint8_t LCD_Data[sizeof(lcd.LCD_Data)];
        memcpy(LCD_Data, lcd.LCD_Data, sizeof(LCD_Data));

        lcd.mutex.unlock();

        uint8_t contrast = lcd.contrast;

        if (!lcd.enable && !lcd.mcu->is_jv880)
        {
            contrast = 1;
            memset(lcd.LCD_Data, ' ', sizeof(lcd.LCD_Data));
            for (size_t i = 0; i < lcd.height; i++) 
            {
                for (size_t j = 0; j < lcd.width; j++) 
                {
                    lcd.buffer[i][j] = (back_palette[back_data[i * lcd.width + j]] & 0xFCFC0C) >> 2;
                }
            }
        }
        else
        {
            if (lcd.mcu->is_jv880)
            {
                if (lcd.buffer[0][0] != 0xFF03BE51) 
                {
                    for (size_t i = 0; i < lcd.height; i++) {
                        for (size_t j = 0; j < lcd.width; j++) {
                            lcd.buffer[i][j] = 0xFF03BE51;
                        }
                    }
                }
            }
            else
            {
                if (lcd.buffer[0][0] != back_palette[back_data[0 * lcd.width + 0]])
                {
                    for (size_t i = 0; i < lcd.height; i++) {
                        for (size_t j = 0; j < lcd.width; j++) {
                            lcd.buffer[i][j] = back_palette[back_data[i * lcd.width + j]];
                        }
                    }
                }
            }

            if (lcd.mcu->is_jv880)
            {
                uint32_t con = contrast + 1;
                con = (con * con * con);
                con = (con * 104) / 1331;
                lcd.color2 = LCD_MixColor(lcd.buffer[0][0], 0xFF - con);
                con = contrast;
                if (con > 4)
                    con = 4;
                con = con * con;
                lcd.color1 = LCD_MixColor(lcd.color2, 255 - (con * 8));
            }
            else
            {
                uint32_t con = 0x11 * (contrast - 1);
                con = (con * con) >> 8;
                lcd.color2 = LCD_MixColor(lcd.buffer[0][0], 0xFF - (con / 4 + 4));
                lcd.color1 = LCD_MixColor(lcd.color2, 0x11 * (16 - (((contrast + 1) >> 1) + 4)));
            }

            if (lcd.mcu->is_jv880)
            {
                int curX = LCD_DD_RAM % 0x40;
                int curY = LCD_DD_RAM / 0x40;

                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 24; j++)
                    {
                        uint8_t ch = LCD_Data[i * 40 + j];
                        LCD_FontRenderStandard(lcd, LCD_CG, (4 + i * 50), 4 + j * 34, ch, (i == curY && j == curX && LCD_C) + 1);
                    }
                }

            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[0 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 11, 34 + i * 35, ch);
                }
                for (int i = 0; i < 16; i++)
                {
                    uint8_t ch = LCD_Data[3 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 11, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[40 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 75, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[43 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 75, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[49 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 139, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[46 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 139, 153 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[52 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 203, 34 + i * 35, ch);
                }
                for (int i = 0; i < 3; i++)
                {
                    uint8_t ch = LCD_Data[55 + i];
                    LCD_FontRenderStandard(lcd, LCD_CG, 203, 153 + i * 35, ch);
                }

                LCD_FontRenderLR(lcd, LCD_CG, LCD_Data[58]);

                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        uint8_t ch = lcd.LCD_Data[20 + j + i * 40];
                        LCD_FontRenderLevel(lcd, LCD_CG, 71 + i * 88, 293 + j * 130, ch, j == 3 ? 1 : 5);
                    }
                }
            }
        }
        lcd.backend->Render();
    }
}
