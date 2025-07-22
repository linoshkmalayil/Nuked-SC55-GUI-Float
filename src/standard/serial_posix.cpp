
#include "serial.h"
#include <cstdio>
#include <cstring> // strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer codes
#include <mutex>
#include <termios.h> // Contains POSIX terminal control definitions
#include <thread>
#include <unistd.h> // open(), write(), read(), close()
#include <vector>

#define INVALID_VALUE -1
#define BUFFER_SIZE 4096

std::vector<uint8_t> read_buffer;
std::vector<uint8_t> write_buffer;

typedef void (*serial_read_callback)(FE_Application& fe, uint8_t data);
void FE_RouteSerial(FE_Application& fe, uint8_t byte);

std::thread linux_io_thread;
std::thread serial_read_thread;
std::mutex  serial_io_mutex;
bool        thread_run = false;
void        Linux_IO_Serial();
void        SERIAL_Read_Updater(FE_Application& fe, serial_read_callback read_callback);

class Serial_Handler
{
    public:
        bool SerialOpen(std::string_view serial_port);
        void SerialClose();

        bool IsSerialInit() { return serial_init; }

        bool IsReadPending() { return read_pending; }
        bool IsWritePending() { return write_pending; }

        std::vector<uint8_t> Read() 
        {   
            auto read_data = linux_read_buffer;
            linux_read_buffer.clear(); 
            
            return read_data;
        }

        void Write(std::vector<uint8_t>& write_data)
        {
            linux_write_buffer = write_data;
            write_data.clear();
        }

        void SetReadPending(bool value) {   read_pending = value; }
        void SetWritePending(bool value) { write_pending = value; }

        void ReadSerialPort();
        void WriteSerialPort();
    private:
        bool serial_init = false;
        int port_handle;

        std::vector<uint8_t> linux_read_buffer;
        std::vector<uint8_t> linux_write_buffer;

        bool read_pending  = false;
        bool write_pending = false;
};

bool Serial_Handler::SerialOpen(std::string_view serial_port)
{
    port_handle = open(std::string(serial_port).c_str(), O_RDWR|O_NOCTTY);
    if(port_handle == INVALID_VALUE)
    {
        fprintf(stderr, "Failed to open serial port: %s\n", std::string(serial_port).c_str());
        return false;
    }

    termios tty_handle;
    // Read in existing settings, and handle any error
    if(tcgetattr(port_handle, &tty_handle) != 0) {
        fprintf(stderr, "Error %i from getting tty attributes: %s\n", errno, strerror(errno));
        return false;
    }

    // Clear parity bit, stop field (use only one stop bit) and data size bits.
    tty_handle.c_cflag &= ~(PARENB|CSTOPB|CSIZE);
    // 8 bits per byte, turn on READ and ignore ctrl lines
    tty_handle.c_cflag |= (CS8|CREAD|CLOCAL);

    // Disable echo, erasure, new-line echo ,interpretation of INTR, QUIT and SUSP & Turn off s/w flow ctrl
    tty_handle.c_iflag &= ~(ICANON|ECHO|ECHOE|ECHONL|ISIG|IXON|IXOFF|IXANY);
    tty_handle.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty_handle.c_oflag &= ~(OPOST|ONLCR); // Prevent special interpretation of output bytes (e.g. newline chars)
    // Prevent conversion of newline to carriage return/line feed

    tty_handle.c_cc[VTIME] = 0;    // Do not wait, process asap
    tty_handle.c_cc[VMIN]  = 0;

    // Save tty settings, also checking for error
    if (tcsetattr(port_handle, TCSANOW, &tty_handle) != 0) {
        fprintf(stderr, "Error %i from getting tty attributes: %s\n", errno, strerror(errno));
        return false;
    }
    tcflush(port_handle, TCIFLUSH);

    fprintf(stderr, "Opened serial port: %s\n", std::string(serial_port).c_str());
    serial_init = true;

    return true;
}

void Serial_Handler::SerialClose()
{
    if (!serial_init)
        return;

    close(port_handle);
    port_handle = INVALID_VALUE;
    serial_init = false;
}

void Serial_Handler::ReadSerialPort()
{
    uint8_t read_linux[BUFFER_SIZE];
    int16_t read_bytes = (int16_t)read(port_handle, read_linux, BUFFER_SIZE*sizeof(uint8_t));

    std::vector<uint8_t> read_data;

    if (read_bytes == INVALID_VALUE)
    {
        fprintf(stderr, "Error %i reading from serial port: %s\n", errno, strerror(errno));
    }

    if (read_bytes > 0)
    {
        for (uint16_t i=0; i < read_bytes; i++)
        {
            linux_read_buffer.push_back(read_linux[i]);
        }
    }
}

void Serial_Handler::WriteSerialPort()
{
    uint8_t write_linux[BUFFER_SIZE];
    uint16_t write_size = (uint16_t)linux_write_buffer.size();
    for (uint16_t i=0; i<write_size && i<BUFFER_SIZE; i++)
    {
        write_linux[i] = linux_write_buffer[i];
    }

    int16_t write_bytes = (int16_t)write(port_handle, write_linux, write_size);

    if (write_bytes == INVALID_VALUE)
    {
        fprintf(stderr, "Error %i writing to serial port: %s\n", errno, strerror(errno));
    }

    linux_write_buffer.clear();
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
    linux_io_thread    = std::thread(&Linux_IO_Serial);
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

    if (read_buffer.empty())
    {
        s_handler->SetReadPending(false);
    }

    if (write_buffer.empty())
    {
        s_handler->SetWritePending(false);
    }

    if (!s_handler->IsReadPending())
    {
        read_buffer = s_handler->Read();
        s_handler->SetReadPending(true);
    }

    if (s_handler->IsWritePending())
    {
        s_handler->Write(write_buffer);
        s_handler->SetWritePending(false);
    }
}

bool SERIAL_HasData()
{
    if (!s_handler || !s_handler->IsSerialInit())
    {
        return false;
    }

    return !read_buffer.empty();
}

uint8_t SERIAL_ReadUART()
{
    if (!s_handler || !s_handler->IsSerialInit())
    {
        return 0;
    }

    if (!read_buffer.empty())
    {
        uint8_t read_byte = *read_buffer.cbegin();
        read_buffer.erase(read_buffer.begin());

        return read_byte;
    }
    else
    {
        s_handler->SetReadPending(false);
        return 0;
    }
}

void SERIAL_PostUART(uint8_t data)
{
    std::lock_guard<std::mutex> lock(serial_io_mutex);

    if (!s_handler || !s_handler->IsSerialInit())
    {
        return;
    }

    if(write_buffer.size() < BUFFER_SIZE)
    {
        write_buffer.push_back(data);
    }
    else
    {
        s_handler->SetWritePending(true);
    }
}

void SERIAL_Quit()
{
    thread_run = false;
    if(linux_io_thread.joinable())
    {
        thread_run = false;
        linux_io_thread.join();
        serial_read_thread.join();
    }

    if (s_handler)
    {
        s_handler->SerialClose();
        delete s_handler;
        s_handler = nullptr;
    }

    return;
}

void Linux_IO_Serial()
{
    while (thread_run)
    {
        std::lock_guard<std::mutex> lock(serial_io_mutex);

        s_handler->ReadSerialPort();
        s_handler->WriteSerialPort();
    }
}

void SERIAL_Read_Updater(FE_Application& fe, serial_read_callback read_callback)
{
    while (thread_run)
    {
        std::lock_guard<std::mutex> lock(serial_io_mutex);

        if(SERIAL_HasData())
        {
            uint8_t byte = SERIAL_ReadUART();
            read_callback(fe, byte);        
        }
    }
}
