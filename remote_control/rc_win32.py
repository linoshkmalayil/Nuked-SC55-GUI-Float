from collections import namedtuple
import win32gui
import pygame
import sys


class RemoteControl_WIN32:
    def __init__(self):
        self.Button = namedtuple('Button', ('id', 'x', 'y', 'w', 'h', 'name'))

        self.buttons = (
            Button(0x00,  80,  92, 80, 80, 'All'),
            Button(0x01, 202,  92, 80, 80, 'Mute'),
            Button(0x03, 446,  92, 80, 80, 'Power'),
            Button(0x04,  80, 228, 76, 76, 'Part L'),
            Button(0x05, 202, 228, 76, 76, 'Part R'),
            Button(0x06, 326, 228, 76, 76, 'Inst L'),
            Button(0x07, 448, 228, 76, 76, 'Inst R'),
            Button(0x08,  80, 360, 76, 76, 'Level L'),
            Button(0x09, 202, 360, 76, 76, 'Level R'),
            Button(0x0A, 326, 360, 76, 76, 'Reverb L'),
            Button(0x0B, 448, 360, 76, 76, 'Reverb R'),
            Button(0x0C,  80, 492, 76, 76, 'Song L'),
            Button(0x0D, 202, 492, 76, 76, 'Song R'),
            Button(0x0E, 326, 492, 76, 76, 'Tempo L'),
            Button(0x0F, 448, 492, 76, 76, 'Tempo R'),
            Button(0x10,  80, 626, 76, 76, 'Stop'),
            Button(0x11, 202, 626, 76, 76, 'Play'),
            Button(0x12, 326, 626, 76, 76, 'Rew'),
            Button(0x13, 448, 626, 76, 76, 'FF')
        )
        
        WM_APP = 0x8000