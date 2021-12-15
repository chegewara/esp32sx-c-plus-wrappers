#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cli-comp.h"

#define cmdMAX_INPUT_SIZE	80
#define cmdMAX_OUTPUT_SIZE	1024

CLI cli;

static char cInputString[ cmdMAX_INPUT_SIZE ], cOutputString[ cmdMAX_OUTPUT_SIZE ];

// callback to handle command
static BaseType_t ramInfoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	int len = 0;
	const char* p = cli.getParameter(pcCommandString, 1, &len);
	if(p) printf("\tcmd \"ram\" arg 1: %.*s\n", len, p);
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	memset( pcWriteBuffer, 0x00, xWriteBufferLen );

	uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
	uint32_t size = esp_get_free_heap_size();
	if(p && strncmp(p, "psram", 6) == 0)
		size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

	snprintf(pcWriteBuffer, xWriteBufferLen, "free heap size: %d, min heap size: %u\r\n", size, heap_size);

	return pdFALSE;
}

static BaseType_t tasksListCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	memset( pcWriteBuffer, 0x00, xWriteBufferLen );

	vTaskList(pcWriteBuffer);
	return pdFALSE;
}

static BaseType_t versionTaskCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	memset( pcWriteBuffer, 0x00, xWriteBufferLen );

	sprintf(pcWriteBuffer, "%s", esp_get_idf_version());
	return pdFALSE;
}

// command definition
static const CLI_Command_Definition_t ramCommand =
{
	"ram", /* The command string to type. */
	"ram: \n  Get the current size of free heap memory and minimum size of free heap memory\r\n",
	ramInfoCommand, /* The function to run. */
	-1 /* No of parameters expected.  -1 means any number of params */
};

static const CLI_Command_Definition_t tasksCommand =
{
	"tasks", /* The command string to type. */
	"tasks: \n  Print the list of tasks with vTaskList\r\n",
	tasksListCommand, /* The function to run. */
	0 /* No of parameters expected.  -1 means any number of params */
};

static const CLI_Command_Definition_t versionCommand =
{
	"version", /* The command string to type. */
	"version: \n  Print ESP-IDF version\r\n",
	versionTaskCommand, /* The function to run. */
	0 /* No of parameters expected.  -1 means any number of params */
};

extern "C" void app_main(void)
{
    cli.registerCmd(&ramCommand);
    cli.registerCmd(&tasksCommand);
    cli.registerCmd(&versionCommand);

    BaseType_t res = pdFALSE;
    do{
        res = cli.processCmd( "help", cOutputString, cmdMAX_OUTPUT_SIZE );
        printf("%s", cOutputString);
    }while(res);

	printf("\r\nprocess commands:\r\n");

    cli.processCmd( "tasks", cOutputString, cmdMAX_OUTPUT_SIZE );
    printf("tasks:\r\n%s\r\n", cOutputString);

    cli.processCmd( "ram", cOutputString, cmdMAX_OUTPUT_SIZE );
    printf("ram:\r\n\t%s\r\n", cOutputString);
    cli.processCmd( "ram test", cOutputString, cmdMAX_OUTPUT_SIZE );
    printf("ram psram:\r\n\t%s\r\n", cOutputString);

    cli.processCmd( "version", cOutputString, cmdMAX_OUTPUT_SIZE );
    printf("version:\r\n%s\r\n", cOutputString);
}

