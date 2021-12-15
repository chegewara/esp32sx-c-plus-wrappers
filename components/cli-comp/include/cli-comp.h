#pragma once
#include <stdio.h>
#include "esp_err.h"
extern "C" {
#include "FreeRTOS_CLI.h"
}
class CLI
{
private:
    
public:
    CLI();
    ~CLI(){}

public:
    void registerCmd(const CLI_Command_Definition_t* cmd);
    bool processCmd(const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen);
    const char* getParameter(const char *pcCommandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength);
};

