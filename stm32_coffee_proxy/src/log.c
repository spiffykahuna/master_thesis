#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log.h"

extern xSemaphoreHandle xLogMutex;

extern xQueueHandle  systemMsgQueue;

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
		 }
	 }
	}
	xSemaphoreGive( xLogMutex );
	return SUCCESS;
}

void logger(log_level_t level, char *msg) {
	char temp[32];

	strbuffer_t *logMsg = strbuffer_new();
	if(logMsg) {
		snprintf(temp, 32, "%d   ", (int) xTaskGetTickCount());
		strbuffer_append(logMsg, temp);

		switch(level) {
		case LEVEL_OFF:
			return; /* no message here */
		case LEVEL_FATAL:
			strbuffer_append(logMsg, "FATAL : ");
			break;

		case LEVEL_ERR:
			strbuffer_append(logMsg, "ERROR : ");
			break;
		case LEVEL_WARN:
			strbuffer_append(logMsg, "WARN : ");
			break;
		case LEVEL_INFO:
			strbuffer_append(logMsg, "INFO : ");
			break;
		case LEVEL_DEBUG:
			strbuffer_append(logMsg, "DEBUG : ");
			break;
		case LEVEL_TRACE:
			strbuffer_append(logMsg, "TRACE : ");
			break;
		}

		strbuffer_append(logMsg, msg);

		system_msg_t *systemMsg = system_msg_new(MSG_TYPE_LOGGING);
		systemMsg->logMsg = logMsg;

		portBASE_TYPE xStatus = 0;
		while( xStatus != pdPASS) {
			xStatus = xQueueSendToBack( systemMsgQueue, &systemMsg, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
		}
	}
}

inline
int log_usb(char *msg, size_t msgLength ) {
	log_d(msg);
	return SUCCESS;
}
