#pragma once

#include <cstdint>
#include <string>

struct FE_Application;

bool REMOTE_Init(FE_Application& fe, std::string_view rc_pipe);
void REMOTE_Quit();
