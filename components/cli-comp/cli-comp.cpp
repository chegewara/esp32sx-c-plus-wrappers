#include <stdio.h>
extern "C" {
#include "FreeRTOS_CLI.h"
}
#include "cli-comp.h"



CLI::CLI()
{
}

void CLI::registerCmd(const CLI_Command_Definition_t* cmd)
{
    FreeRTOS_CLIRegisterCommand(cmd);
}

bool CLI::processCmd(const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen)
{
    return FreeRTOS_CLIProcessCommand(pcCommandInput, pcWriteBuffer, xWriteBufferLen);
}

const char* CLI::getParameter(const char *pcCommandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength)
{
    return FreeRTOS_CLIGetParameter(pcCommandString, uxWantedParameter, pxParameterStringLength);
}