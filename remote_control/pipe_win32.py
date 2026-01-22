import win32file, win32pipe
import pywintypes


class PipeHandler:
    def __init__(self, rc_pipe: str) -> None:
        self.pipe_path = rc_pipe
        self.handle = pywintypes.HANDLE

    def __open_connection(self) -> bool:
        try:
            self.handle = win32file.CreateFile(
                self.pipe_path,
                win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None
            )
            win32pipe.SetNamedPipeHandleState(self.handle, win32pipe.PIPE_READMODE_BYTE, None, None)

            return True
        except Exception as e:
            print(e)
            
            return False
        
    def __close_connection(self) -> None:
        win32file.CloseHandle(self.handle)

    def write_byte(self, pipe_data: bytes) -> None:
        if self.__open_connection():
            win32file.WriteFile(self.handle, pipe_data)
            self.__close_connection()
        else:
            print("Failed to send singal")

    def open_connection(self) -> bool:
        return True
    
    def close_connection(self) -> None:
        return
