import io

class PipeHandler:
    def __init__(self, rc_pipe: str) -> None:
        self.pipe_path = rc_pipe
        self.pipe = io.BufferedReader

    def open_connection(self) -> bool:
        try:
            self.pipe = open(self.pipe_path, "wb")
            
            return True
        except Exception as e:
            print(e)

            return False

    def write_byte(self, pipe_data: bytes) -> None:
        self.pipe.write(pipe_data)
        self.pipe.flush()

    def close_connection(self) -> None:
        self.pipe.close()
