#include "config.h"

void Cfg_WriteVersionInfo(FILE* file)
{
    fprintf(file, "Version: %s\n", NUKED_VERSION);
    fprintf(file, "Source: %s\n", NUKED_SOURCE);
    fprintf(file, "Configuration:\n");
    fprintf(file, "  NUKED_ENABLE_ASIO=%d\n", NUKED_ENABLE_ASIO);
}
