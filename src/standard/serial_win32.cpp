#include "serial.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <thread>

#define BUFFER_SIZE 4096

typedef void (*serial_read_callback)(FE_Application& fe, uint8_t data);
void FE_RouteSerial(FE_Application& fe, uint8_t byte);

std::thread serial_read_thread;
std::mutex  serial_io_mutex;
bool        thread_run = false;
void        SERIAL_Read_Updater(FE_Application& fe, serial_read_callback read_callback);

class Serial_Handler
{
    public:

        bool SerialOpen(std::string_view serial_port);
        void SerialClose();

        bool IsSerialInit() { return serial_init; };

        void ReadSerialPort();
        void WriteSerialPort();

        bool HasData() { return read_ptr < read_end; }

        uint8_t Read()
        {
            if (read_ptr < read_end) 
            {
                return *read_ptr++;
            }
            return 0;
        }
        void Write(uint8_t data)
        {
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

    private:
        bool isNamedPipe(std::string serial_port);
        bool isSerialPort(std::string& serial_port);
        
        LPCSTR GetErrorString(DWORD error);
        void ReportIOError(DWORD error);
        
        bool serial_init   = false;
        bool read_pending  = false;
        bool write_pending = false;
        
        HANDLE     handle;
        OVERLAPPED olRead;
        OVERLAPPED olWrite;

        uint8_t read_buffer[BUFFER_SIZE];
        uint8_t * read_ptr = read_buffer;
        uint8_t * read_end = read_buffer;
        const uint8_t * read_limit = read_buffer + BUFFER_SIZE;

        uint8_t write_buffer[BUFFER_SIZE];
        uint8_t * write_ptr = write_buffer;
        uint8_t * write_end = write_buffer;
        const uint8_t * write_limit = write_buffer + BUFFER_SIZE;

};

LPCSTR Serial_Handler::GetErrorString(DWORD error) {
    LPSTR buffer = NULL;
    DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPSTR) &buffer, 0, NULL);
    if (buffer != NULL) {
        if (buffer[size - 1] == '\n') {
            buffer[size - 1] = '\0';
        }
    }
    return buffer;
}

void Serial_Handler::ReportIOError(DWORD error) {
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

bool Serial_Handler::isSerialPort(std::string& serial_port)
{
    if(serial_port.compare(0, 4, "\\\\.\\"))
    {
        serial_port.insert(0, "\\\\.\\");
    }
    
    for (auto &c: serial_port) c = (char)toupper(c);

    if(serial_port.compare(4, 3, "COM"))
    {
        return false;
    }

    return true;
}

bool Serial_Handler::isNamedPipe(std::string serial_port)
{
    if(serial_port.size() < 10 || serial_port.size() > 256)
    {
        return false;
    }

    if(serial_port.compare(0, 2, "\\\\"))
    {
        return false;
    }

    std::string hostname = serial_port.substr(2);
    std::string pipename = hostname;
    while(pipename[0] != '\0' && pipename[0] != '\\')
    {
        pipename.erase(pipename.begin());
    }

    if(hostname == pipename)
    {
        return false;
    }

    if(pipename.size() < 7 || pipename.compare(0, 6, "\\pipe\\"))
    {
        return false;
    }
    
    return true;
}

bool Serial_Handler::SerialOpen(std::string_view serial_port)
{
    std::string port = std::string(serial_port);

    if(!isNamedPipe(port) && !isSerialPort(port))
    {
        fprintf(stderr, "Can't open '%s': Not a serial port or named pipe\n", std::string(serial_port).c_str());
        fflush(stderr);
        return false;
    }

    handle = CreateFileA(
        port.c_str(),
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
        fprintf(stderr, "Unable to open serial port: '%s' Error: ", std::string(serial_port).c_str());
        if (str != NULL) 
        {
            fprintf(stderr, "%s\n", str);
            LocalFree((HLOCAL) str);
        } else 
        {
            fprintf(stderr, "%ld\n", error);
        }
        fflush(stderr);
        return false;
    }

    memset(&olRead, 0, sizeof(OVERLAPPED));
    memset(&olWrite, 0, sizeof(OVERLAPPED));

    olRead.hEvent  = CreateEventA(NULL, true, false, NULL);
    olWrite.hEvent = CreateEventA(NULL, true, false, NULL);

    printf("Opened serial port '%s'\n", std::string(serial_port).c_str());

    serial_init = true;

    return serial_init;
}

void Serial_Handler::SerialClose()
{
    CloseHandle(handle);
    CloseHandle(olRead.hEvent);
    CloseHandle(olWrite.hEvent);
    handle         = INVALID_HANDLE_VALUE;
    olRead.hEvent  = INVALID_HANDLE_VALUE;
    olWrite.hEvent = INVALID_HANDLE_VALUE;
}

void Serial_Handler::ReadSerialPort()
{
    if (read_ptr == read_end && !read_pending) 
    {
        if (read_end == read_limit) 
        {
            read_ptr = read_buffer;
            read_end = read_buffer;
        }
        
        DWORD dwReads;
        bool success = ReadFile(handle, read_end, (DWORD) (read_limit - read_end), &dwReads, &olRead);
        if (success)
        {
            read_end    += dwReads;
            read_pending = false;
        } 
        else 
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) 
            {
                ReportIOError(error);
                SerialClose();
            } 
            else 
            {
                read_pending = true;
            }
        }
    }

    if (read_pending) 
    {
        DWORD dwReads;
        bool success = GetOverlappedResult(handle, &olRead, &dwReads, false);
        if (success) {
            read_end    += dwReads;
            read_pending = false;
        } else 
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_INCOMPLETE) 
            {
                ReportIOError(error);
                SerialClose();
            }
        }
    }
}

void Serial_Handler::WriteSerialPort()
{
    if (write_ptr != write_end && !write_pending)
    {
        DWORD dwWrite;
        int32_t len      = (int32_t) (write_end - write_ptr);
        uint8_t *towrite = write_ptr;
        if (len < 0) 
        {
            if (towrite == write_limit) 
            {
                len      += BUFFER_SIZE;
                towrite   = write_buffer;
                write_ptr = write_buffer;
            } 
            else
            {
                len = (int32_t) (write_limit - towrite);
            }
        }

        bool success = WriteFile(handle, towrite, len, &dwWrite, &olWrite);
        if (success) 
        {
            write_ptr    += dwWrite;
            write_pending = false;
        } 
        else
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) 
            {
                ReportIOError(error);
                SerialClose();
            } 
            else
            {
                write_pending = true;
            }
        }
    }
    if (write_pending)
    {
        DWORD dwWrite;
        bool success = GetOverlappedResult(handle, &olWrite, &dwWrite, false);
        if (success)
        {
            write_ptr    += dwWrite;
            write_pending = false;
        }
        else
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_INCOMPLETE)
            {
                ReportIOError(error);
                SerialClose();
            }
        }
    }
    if (write_ptr == write_limit)
    {
        write_ptr = write_buffer;
    }
}

Serial_Handler *s_handler = nullptr;

bool SERIAL_Init(FE_Application& fe, std::string_view serial_port)
{
    (void)fe;

    if (s_handler)
    {
        fprintf(stderr, "Serial IO Already running\n");
        return false;
    }
    s_handler = new Serial_Handler();
    
    if (!s_handler->SerialOpen(serial_port))
    {
        delete s_handler;
        s_handler = nullptr;

        return false;
    }

    thread_run         = true;
    serial_read_thread = std::thread(&SERIAL_Read_Updater, std::ref(fe), std::ref(FE_RouteSerial));

    return true;
}

void SERIAL_Update()
{
    std::lock_guard<std::mutex> lock(serial_io_mutex);

    if (!s_handler || !s_handler->IsSerialInit())
    {
        return;
    }

    s_handler->ReadSerialPort();
    s_handler->WriteSerialPort();
}

bool SERIAL_HasData() 
{
    if (!s_handler || !s_handler->IsSerialInit())
    {
        return false;
    }
    
    return s_handler->HasData();
}

uint8_t SERIAL_ReadUART() 
{
    if (!s_handler || !s_handler->IsSerialInit())
    {
        return 0;
    }

    return s_handler->Read();
}

void SERIAL_PostUART(uint8_t data)
{
    if (!s_handler || !s_handler->IsSerialInit())
    {
        return;
    }

    s_handler->Write(data);
}

void SERIAL_Quit()
{
    if(s_handler)
    {
        thread_run = false;
        serial_read_thread.join();

        s_handler->SerialClose();
        delete s_handler;
        s_handler = nullptr;
    }

    return;
}

void SERIAL_Read_Updater(FE_Application& fe, serial_read_callback read_callback)
{
    while (thread_run)
    {
        std::lock_guard<std::mutex> lock(serial_io_mutex);

        if(SERIAL_HasData())
        {
            uint8_t sbyte = SERIAL_ReadUART();
            read_callback(fe, sbyte);        
        }
    }
}
