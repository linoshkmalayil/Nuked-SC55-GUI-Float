#include "serial.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>

#define BUFFER_SIZE 4096 

static int SERIAL_strnicmp(const char* a, const char* b, int len)
{
    int diff = 0;
    char chrA, chrB;
    for (int i = 0; i < len && !diff; i++)
    {
        chrA = *(a + i);
        chrA |= 0x20 * (chrA >= 'A' && chrA <= 'Z');
        chrB = *(b + i);
        chrB |= 0x20 * (chrB >= 'A' && chrB <= 'Z');
        diff |= chrA ^ chrB;
    }
    return diff;
}

static inline bool IsSerialPort(const char * path) {
    size_t len = strlen(path);
    if (*path == '\\' && len >= 4) {
        if (memcmp(path, "\\\\.\\", 4) != 0)
            return false;
        len -= 4;
        path += 4;
    }
    return len > 3 && SERIAL_strnicmp(path, "COM", 3) == 0 && atoi(path + 3) != 0;
}

static inline bool IsNamedPipe(const char * path) {
    size_t pathlen = strnlen(path, 257);
    if (pathlen < 10 || pathlen > 256)
        return false;
    if (memcmp(path, "\\\\", 2) != 0)
        return false;
    const char * hostname = path + 2;
    const char * pipename = hostname;
    while (*pipename != 0 && *pipename != '\\') pipename++;
    if (hostname == pipename)
        return false; // No hostname
    if (strnlen(pipename, 7) < 7 || SERIAL_strnicmp(pipename, "\\pipe\\", 6) != 0)
        return false;
    pipename += 7;
    return true;
}

static bool serial_init = false;
static bool is_serial_port = false;
static bool is_named_pipe = false;

static char * serial_path;
static HANDLE handle;
static OVERLAPPED olRead;
static OVERLAPPED olWrite;
static uint8_t read_buffer[BUFFER_SIZE];
static uint8_t * read_ptr = read_buffer;
static uint8_t * read_end = read_buffer;
static const uint8_t * read_limit = read_buffer + BUFFER_SIZE;
static uint8_t write_buffer[BUFFER_SIZE];
static uint8_t * write_ptr = write_buffer;
static uint8_t * write_end = write_buffer;
static const uint8_t * write_limit = write_buffer + BUFFER_SIZE;
static bool read_pending = false;
static bool write_pending = false;

static void Cleanup() {
    CloseHandle(handle);
    CloseHandle(olRead.hEvent);
    CloseHandle(olWrite.hEvent);
    handle = INVALID_HANDLE_VALUE;
    olRead.hEvent = INVALID_HANDLE_VALUE;
    olWrite.hEvent = INVALID_HANDLE_VALUE;
}

static LPCSTR GetErrorString(DWORD error) {
    LPSTR buffer = NULL;
    DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPSTR) &buffer, 0, NULL);
    if (buffer != NULL) {
        if (buffer[size - 1] == '\n') {
            buffer[size - 1] = '\0';
        }
    }
    return buffer;
}

static void ReportIOError(DWORD error) {
    LPCSTR str = GetErrorString(error);
    fprintf(stderr, "Serial I/O Error: ");
    if (str != NULL) {
        fprintf(stderr, "%s\n", str);
        LocalFree((HLOCAL) str);
    } else {
        fprintf(stderr, "%ld\n", error);
    }
    fflush(stderr);
}

bool SERIAL_Init(FE_Application& fe, std::string_view serial_port) {
    (void)fe;

    char *path = new char[serial_port.size()+1];
    strcpy(path, std::string(serial_port).c_str());

    if (serial_init) return 0;
    if (!IsSerialPort(path) && !IsNamedPipe(path)) {
        fprintf(stderr, "Can't open '%s': Not a serial port or named pipe\n", path);
        fflush(stderr);
        return false;
    }
    is_serial_port = IsSerialPort(path);
    is_named_pipe = !is_serial_port;

    serial_path = (char *)calloc(strlen(path) + 5, sizeof(char));
    if (is_serial_port && *path != '\\') {
        memcpy(serial_path, "\\\\.\\", 4);
    }
    strcat(serial_path, path);

    handle = CreateFileA(
        serial_path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        LPCSTR str = GetErrorString(error);
        fprintf(stderr, "Unable to open serial port: '%s' Error: ", path);
        if (str != NULL) {
            fprintf(stderr, "%s\n", str);
            LocalFree((HLOCAL) str);
        } else {
            fprintf(stderr, "%ld\n", error);
        }
        fflush(stderr);
        free((void *) serial_path);
        return false;
    }

    memset(&olRead, 0, sizeof(OVERLAPPED));
    memset(&olWrite, 0, sizeof(OVERLAPPED));

    olRead.hEvent = CreateEventA(NULL, true, false, NULL);
    olWrite.hEvent = CreateEventA(NULL, true, false, NULL);

    serial_init = 1;
    printf("Opened serial port '%s'\n", path);
    return true;
}
void SERIAL_Update(submcu_t& sm) {
    (void) sm;

    if (!serial_init || handle == INVALID_HANDLE_VALUE) return;
    // printf("readptr: %04x readend: %04x\n", read_ptr - read_buffer, read_end - read_buffer);
    if (read_ptr == read_end && !read_pending) {
        if (read_end == read_limit) {
            read_ptr = read_buffer;
            read_end = read_buffer;
        }
        DWORD dwReads;
        bool success = ReadFile(handle, read_end, (DWORD) (read_limit - read_end), &dwReads, &olRead);
        if (success) {
            read_end += dwReads;
            read_pending = false;
        } else {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                ReportIOError(error);
                Cleanup();
            } else {
                read_pending = true;
            }
        }
    }
    if (read_pending) {
        DWORD dwReads;
        bool success = GetOverlappedResult(handle, &olRead, &dwReads, false);
        if (success) {
            read_end += dwReads;
            read_pending = false;
        } else {
            DWORD error = GetLastError();
            if (error != ERROR_IO_INCOMPLETE) {
                ReportIOError(error);
                Cleanup();
            }
        }
    }
    if (write_ptr != write_end && !write_pending) {
        DWORD dwWrite;
        int32_t len = (int32_t) (write_end - write_ptr);
        uint8_t *towrite = write_ptr;
        if (len < 0) {
            if (towrite == write_limit) {
                len += BUFFER_SIZE;
                towrite = write_buffer;
                write_ptr = write_buffer;
            } else {
                len = (int32_t) (write_limit - towrite);
            }
        }
        bool success = WriteFile(handle, towrite, len, &dwWrite, &olWrite);
        if (success) {
            write_ptr += dwWrite;
            write_pending = false;
        } else {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                ReportIOError(error);
                Cleanup();
            } else {
                write_pending = true;
            }
        }
    }
    if (write_pending) {
        DWORD dwWrite;
        bool success = GetOverlappedResult(handle, &olWrite, &dwWrite, false);
        if (success) {
            write_ptr += dwWrite;
            write_pending = false;
        } else {
            DWORD error = GetLastError();
            if (error != ERROR_IO_INCOMPLETE) {
                ReportIOError(error);
                Cleanup();
            }
        }
    }
    if (write_ptr == write_limit) {
        write_ptr = write_buffer;
    }
}
bool SERIAL_HasData() {
    return read_ptr < read_end;
}
uint8_t SERIAL_ReadUART() {
    if (read_ptr < read_end) {
        return *read_ptr++;
    }
    return 0;
}
void SERIAL_PostUART(uint8_t data) {
    if (!serial_init || handle == INVALID_HANDLE_VALUE) return;
    if (write_end == write_ptr - 1) {
        fprintf(stderr, "SERIAL TX OVERFLOW, THIS IS A BUG\n");
        fflush(stderr);
        return;
    }
    *write_end++ = data;
    if (write_end == write_limit) {
        write_end = write_buffer;
    }
}
void SERIAL_Quit() {
    if (!serial_init) return;
    Cleanup();
    free((void *) serial_path);
}
