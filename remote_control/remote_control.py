from collections import namedtuple
import os
import pygame
import sys

if os.name == 'nt':
    from pipe_win32 import PipeHandler
elif os.name == 'posix':
    from pipe_posix import PipeHandler

class RemoteControl:
    def __init__(self, rc_pipe: str):
        self.Button  = namedtuple('Button', ('id', 'x', 'y', 'w', 'h', 'name'))
        self.buttons = (
            self.Button(0x00,  80,  92, 80, 80, 'All'),
            self.Button(0x01, 202,  92, 80, 80, 'Mute'),
            self.Button(0x03, 446,  92, 80, 80, 'Power'),
            self.Button(0x04,  80, 228, 76, 76, 'Part L'),
            self.Button(0x05, 202, 228, 76, 76, 'Part R'),
            self.Button(0x06, 326, 228, 76, 76, 'Inst L'),
            self.Button(0x07, 448, 228, 76, 76, 'Inst R'),
            self.Button(0x08,  80, 360, 76, 76, 'Level L'),
            self.Button(0x09, 202, 360, 76, 76, 'Level R'),
            self.Button(0x0A, 326, 360, 76, 76, 'Reverb L'),
            self.Button(0x0B, 448, 360, 76, 76, 'Reverb R'),
            self.Button(0x0C,  80, 492, 76, 76, 'Song L'),
            self.Button(0x0D, 202, 492, 76, 76, 'Song R'),
            self.Button(0x0E, 326, 492, 76, 76, 'Tempo L'),
            self.Button(0x0F, 448, 492, 76, 76, 'Tempo R'),
            self.Button(0x10,  80, 626, 76, 76, 'Stop'),
            self.Button(0x11, 202, 626, 76, 76, 'Play'),
            self.Button(0x12, 326, 626, 76, 76, 'Rew'),
            self.Button(0x13, 448, 626, 76, 76, 'FF')
        )

        self.remote_height = 960
        self.remote_width  = 600
        self.scaling       = 2

        self.clock = pygame.time.Clock()
        
        self.ph = PipeHandler(rc_pipe)

    def init_remote(self):
        if not self.ph.open_connection():
            sys.exit(1)

        pygame.init()

        width, height = int(self.remote_width / self.scaling), int(self.remote_height / self.scaling)
        screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("Remote Control")

        image = pygame.image.load("rc_background.png").convert_alpha()
        image = pygame.transform.smoothscale(image, (width, height))

        screen.blit(image, (0, 0))
        pygame.display.flip()

    def __send_rc_message(self, button: namedtuple) -> None:
        self.ph.write_byte(button.id.to_bytes(1, "little"))

    def __is_hovered(self, button: namedtuple, x: int, y: int) -> int:
        if button.x <= x and button.y <= y and (button.x + button.w) >= x and (button.y + button.h) >= y:
            center_x = button.x + button.w / 2
            center_y = button.y + button.h / 2
            dx = x - center_x
            dy = y - center_y
            return (dx * dx) / ((button.w / 2) ** 2) + (dy * dy) / ((button.h / 2) ** 2) <= 1
        return False

    def run_remote(self) -> None:
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.MOUSEBUTTONUP:
                    x, y = event.pos
                    x *= self.scaling
                    y *= self.scaling

                    for button in self.buttons:
                        if self.__is_hovered(button, x, y):
                            self.__send_rc_message(button)
                            break

            self.clock.tick(30)

    def quit_remote(self) -> None:
        self.ph.close_connection()
        pygame.quit()

def main():
    if len(sys.argv) == 1:
        print("Missing argument: Named Pipe")
        sys.exit()

    rc = RemoteControl(sys.argv[1])
    rc.init_remote()
    rc.run_remote()
    rc.quit_remote()

if __name__ == "__main__":
    main()
