#include "rc.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>
#include <windows.h>
#include <thread>

#define BUFFER_SIZE 2

typedef void (*rc_read_callback)(FE_Application& fe, uint8_t data);
void FE_RouteRC(FE_Application& fe, uint8_t byte);

std::thread rc_read_thread;
std::mutex  rc_io_mutex;
bool        rc_thread_run = false;
void        REMOTE_Thread_updater(FE_Application& fe, rc_read_callback read_callback);

class RC_Handler
{
    public:

        bool RCOpen(std::string_view rc_pipe);
        void RCClose();

        bool IsRCInit() { return rc_init; }

        void ReadRCPipe();

        uint8_t Read()
        {
            uint8_t rc_message = rc_data;
            rc_data = 0xFF;
            
            return rc_message;
        }
    private:
        bool isNamedPipe(std::string rc_pipe);
        
        LPCSTR GetErrorString(DWORD error);
        void ReportIOError(DWORD error);
        
        bool rc_init   = false;
        HANDLE handle;

        uint8_t rc_data = 0xFF;
};

LPCSTR RC_Handler::GetErrorString(DWORD error) {
    LPSTR buffer = NULL;
    DWORD size   = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPSTR) &buffer, 0, NULL);
    if (buffer != NULL) {
        if (buffer[size - 1] == '\n') {
            buffer[size - 1] = '\0';
        }
    }
    return buffer;
}

void RC_Handler::ReportIOError(DWORD error) {
    LPCSTR str = GetErrorString(error);
    fprintf(stderr, "Code: %lu, Remote Control I/O Error: ", error);
    if (str != NULL) {
        fprintf(stderr, "%s\n", str);
        LocalFree((HLOCAL) str);
    } else {
        fprintf(stderr, "%ld\n", error);
    }
    fflush(stderr);
}

bool RC_Handler::isNamedPipe(std::string rc_pipe)
{
    if(rc_pipe.size() < 10 || rc_pipe.size() > 256)
    {
        return false;
    }

    if(rc_pipe.compare(0, 2, "\\\\"))
    {
        return false;
    }

    std::string hostname = rc_pipe.substr(2);
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

bool RC_Handler::RCOpen(std::string_view rc_pipe)
{
    std::string pipe = std::string(rc_pipe);

    if (!isNamedPipe(pipe))
    {
        fprintf(stderr, "Can't open '%s': Not a named pipe\n", std::string(rc_pipe).c_str());
        fflush(stderr);
        return false;
    }

    handle = CreateNamedPipeA(
        pipe.c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        PIPE_UNLIMITED_INSTANCES,
        0,
        0,
        0,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        LPCSTR str = GetErrorString(error);
        fprintf(stderr, "Unable to open named port: '%s' Error: ", std::string(rc_pipe).c_str());
        if (str != NULL) 
        {
            fprintf(stderr, "%s\n", str);
            LocalFree((HLOCAL) str);
        } 
        else 
        {
            fprintf(stderr, "%ld\n", error);
        }
        fflush(stderr);
        return false;
    }

    printf("Opened named pipe '%s'\n", std::string(rc_pipe).c_str());

    rc_init = true;

    return rc_init;
}

void RC_Handler::RCClose()
{
    if (!rc_init)
    {
        return;
    }

    CloseHandle(handle);
    handle  = INVALID_HANDLE_VALUE;

    rc_init = false;
}

void RC_Handler::ReadRCPipe()
{
    if (ConnectNamedPipe(handle, NULL) != FALSE)
    {
        return;
    }

    DWORD dwReads;
    uint8_t read_buffer[BUFFER_SIZE];
    
    ReadFile(handle, read_buffer, (DWORD)(BUFFER_SIZE - 1), &dwReads, NULL);
    if (dwReads > 0)
    {
        rc_data = read_buffer[0];

        DisconnectNamedPipe(handle);
    }


}

RC_Handler *rc_handler = nullptr;

bool REMOTE_Init(FE_Application& fe, std::string_view rc_pipe)
{
    (void)fe;

    if (rc_handler)
    {
        fprintf(stderr, "Remote IO Already running\n");
        return false;
    }
    rc_handler = new RC_Handler();
    
    if (!rc_handler->RCOpen(rc_pipe))
    {
        delete rc_handler;
        rc_handler = nullptr;

        return false;
    }

    rc_thread_run  = true;
    rc_read_thread = std::thread(&REMOTE_Thread_updater, std::ref(fe), std::ref(FE_RouteRC));

    return true;
}

void REMOTE_Quit()
{
    rc_thread_run = false;
    if (rc_read_thread.joinable())
    {
        rc_read_thread.join();
    }

    if (rc_handler)
    {
        rc_handler->RCClose();
        delete rc_handler;
        rc_handler = nullptr;
    }

    return;
}

void REMOTE_Thread_updater(FE_Application& fe, rc_read_callback read_callback)
{
    while (rc_thread_run)
    {
        std::lock_guard<std::mutex> lock(rc_io_mutex);

        if (!rc_handler->IsRCInit())
        {
            continue;
        }

        rc_handler->ReadRCPipe();

        uint8_t rc_message = rc_handler->Read();
        if (rc_message != 0xFF)
        {
            read_callback(fe, rc_message);
        }
    }
}
