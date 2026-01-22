#include "rc.h"

#include <cstring>    // strerror() function
#include <fcntl.h>    // Contains file controls like O_RDWR
#include <mutex>
#include <sys/stat.h> // For mkfifo()
#include <thread>
#include <unistd.h>   // open(), write(), read(), close()

#define INVALID_VALUE -1

typedef void (*rc_read_callback)(FE_Application& fe, uint8_t data);
void FE_RouteRC(FE_Application& fe, uint8_t sbyte);

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

        uint8_t Read() 
        {
            auto rc_message = rc_data;
            rc_data = 0xFF;

            return rc_message; 
        }

        void ReadRCPipe();
    private:
        bool rc_init = false;
        int pipe_handle;

        uint8_t rc_data = 0xFF;
};

bool RC_Handler::RCOpen(std::string_view named_pipe)
{
    mkfifo(std::string(named_pipe).c_str(), 0666);
    pipe_handle = open(std::string(named_pipe).c_str(), O_RDONLY|O_NONBLOCK);
    if(pipe_handle == INVALID_VALUE)
    {
        fprintf(stderr, "Failed to open named pipe: %s\n", std::string(named_pipe).c_str());
        return false;
    }

    fprintf(stderr, "Opened Remote Control Pipe: %s\n", std::string(named_pipe).c_str());
    rc_init = true;

    return true;
}

void RC_Handler::RCClose()
{
    if (!rc_init)
    {
        return;
    }

    close(pipe_handle);
    pipe_handle = INVALID_VALUE;
    rc_init     = false;
}

void RC_Handler::ReadRCPipe()
{
    uint8_t read_linux[2];
    int16_t read_bytes = (int16_t)read(pipe_handle, read_linux, 2*sizeof(uint8_t));

    if (read_bytes > 0)
    {
        rc_data = read_linux[0];
    }
}

RC_Handler *rc_handler = nullptr;

bool REMOTE_Init(FE_Application& fe, std::string_view rc_pipe)
{
    (void)fe;

    if (rc_handler)
    {
        fprintf(stderr, "Remote Control Already running\n");
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
