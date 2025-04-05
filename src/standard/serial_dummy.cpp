#include "serial.h"

bool SERIAL_Init(FE_Application& fe, std::string_view serial_port)
{
    (void)fe;
    (void)serial_port;
}

void SERIAL_Update(submcu_t& sm)
{
    (void)sm;
}

bool SERIAL_HasData()
{
    return false;
}

uint8_t SERIAL_ReadUART()
{
    return 0;
}

void SERIAL_PostUART(uint8_t data)
{
    (void)data;
}

void SERIAL_Close()
{
    return;
}
