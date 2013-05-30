#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log.h"

extern xSemaphoreHandle xLogMutex;

extern __IO uint32_t packet_sent;
__IO uint8_t Send_Buffer[VIRTUAL_COM_PORT_DATA_SIZE] ;
//extern __IO  uint32_t packet_receive;
//
//extern __IO  uint8_t Receive_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
//extern __IO  uint32_t Receive_length ;


log_func_t log_func = log_d;	/* <== specify logging function here*/

inline
void sendUsbPacket(char* str, size_t size) {
	while (packet_sent != 1) {
		vTaskDelay(100 / portTICK_RATE_MS);
	}
	CDC_Send_DATA((uint8_t*) str, size);
}

int log_d(char *str)
{

	size_t size = strlen(str);
	size_t i = 0;

	xSemaphoreTake( xLogMutex, portMAX_DELAY );
	{

	 if (bDeviceState == CONFIGURED)
	 {

		 if(size < VIRTUAL_COM_PORT_DATA_SIZE) {
			sendUsbPacket(str, size);
		 } else {

			 for(i = 0; i < size; ++i) {

				 if( i != 0 && (i % (VIRTUAL_COM_PORT_DATA_SIZE - 1)) == 0) {
					 sendUsbPacket(str, (VIRTUAL_COM_PORT_DATA_SIZE - 1));
					 str += (VIRTUAL_COM_PORT_DATA_SIZE - 1);
				 }
			 }
			 sendUsbPacket(str, strlen(str));
		 }						/*

						if(size > VIRTUAL_COM_PORT_DATA_SIZE) {
							CDC_Send_DATA ((uint8_t *) str, size );
						}
						CDC_Send_DATA ((uint8_t *) str, size );
					}
					*/



	 }


	}
	xSemaphoreGive( xLogMutex );
	return SUCCESS;
}

void logger(log_level_t level, char *msg) {
	switch(level) {
	case LEVEL_OFF:

		return;
	case LEVEL_FATAL:
		log_func("FATAL : ");
		break;

	case LEVEL_ERR:
		log_func("ERROR : ");
		break;
	case LEVEL_WARN:
		log_func("WARN : ");
		break;
	case LEVEL_INFO:
		log_func("INFO : ");
		break;
	case LEVEL_DEBUG:
		log_func("DEBUG : ");
		break;
	case LEVEL_TRACE:
		log_func("TRACE : ");
		break;
	}

	log_func(msg);
}


int log_usb(char *msg, size_t msgLength ) {
	log_d(msg);
	return SUCCESS;
}
/*
void log_die(char *msg, ...)
{
    va_list argp;

    //log_null(msg);

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);

    fprintf(stderr, "\n");
    //abort();
}

void log_info(char *msg, ...)
{
    va_list argp;

    //log_null(msg);

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);

    fprintf(stderr, "\n");
}
*/


