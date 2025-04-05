#include <cstdint>
#include <string>

struct FE_Application;
struct submcu_t;

bool SERIAL_Init(FE_Application& fe, std::string_view serial_port);
void SERIAL_Update(submcu_t& sm);
bool SERIAL_HasData();
uint8_t SERIAL_ReadUART();
void SERIAL_PostUART(uint8_t data);
void SERIAL_Quit();
