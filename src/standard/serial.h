#include <cstdint>
#include <span>
#include <string>

struct FE_Application;
struct submcu_t;

bool SERIAL_Init(FE_Application& fe, std::string_view serial_port);
void SERIAL_Update();
bool SERIAL_HasData();
std::span<uint8_t> SERIAL_ReadData();
void SERIAL_PostUART(uint8_t data);
void SERIAL_Quit();
