/*
 * transport.c
 *
 *  Created on: May 19, 2013
 *      Author: Kasutaja
 */

#include <stdarg.h>
#include "transport.h"

char error_space[ERROR_BUFFER_SIZE];

extern xQueueHandle  usbOutComeQueue;
extern xSemaphoreHandle xUART1WriteMutex;
extern xSemaphoreHandle xUART1ReadSemaphore;

//inline
//char * format_error_code(proxy_error_t errorCode) {
//	extern const msg_error MSG_ERROR;
//	switch(errorCode) {
//	case PROXY_MEMORY_ALLOC_FAILED:			return MSG_ERROR.PROXY_MEMORY_ALLOC_FAILED;
//	case PROXY_DEVICE_IS_BUSY:				return MSG_ERROR.PROXY_DEVICE_IS_BUSY;
//	case PROXY_INCOMING_BUFFER_OVERFLOW: 	return MSG_ERROR.PROXY_INCOMING_BUFFER_OVERFLOW;
//	case PROXY_INVALID_INCOME_PACKET:		return MSG_ERROR.PROXY_INVALID_INCOME_PACKET;
//	case PROXY_INVALID_JSONRPC_2_0:			return MSG_ERROR.PROXY_INVALID_JSONRPC_2_0;
//
//	default:								return MSG_ERROR.UNKNOWN;
//	}
//}



inline
void send_packet_to_client(packet_t *packet) {
	portBASE_TYPE xStatus;

	switch(packet->transport) {
	case TRANSPORT_UART1:
		do {
			xStatus =  xQueueSendToBack( usbOutComeQueue, &packet, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
			if(xStatus == pdPASS) { break; }
			vTaskDelay( SYSTEM_TASK_DELAY);
		} while( xStatus != pdPASS );
		break;

	default:
		logger_format(LEVEL_ERR, "%s method send_packet_to_client -> Received unknown transport type: %d", pcTaskGetTaskName(NULL), packet->transport);
		break;
	}
}


/* this function transmit data directly to the destination avoiding queue mechanism */
inline
void send_data_to_client(transport_type_t transport, char *data, size_t dataLength) {
	switch(transport) {
	case TRANSPORT_UART1:
			UART1_send( (uint8_t*) data, dataLength);
		break;

	default:
		logger_format(LEVEL_ERR, "%s method send_packet_to_client -> Received unknown transport type: %d", pcTaskGetTaskName(NULL), transport);
		break;
	}
}

void report_error_to_sender(transport_type_t transport, const char *msgFormat, ...) {
	va_list args;

	// format error
	memset(error_space, 0, ERROR_BUFFER_SIZE);
	va_start (args, msgFormat );
	vsnprintf(error_space, ERROR_BUFFER_SIZE, msgFormat, args);
	va_end (args);

	send_data_to_client(transport, error_space, strlen(error_space));
	//TODO uncomment here
}


inline
char * transport_type_to_str(transport_type_t transport) {
	switch(transport) {
	case TRANSPORT_UART1:		return "TRANSPORT_USB";

	default:				return "TRANSPORT_UNKNOWN";
	}
}

inline
int wait_for_semaphore(xSemaphoreHandle semaphore) {
	portBASE_TYPE xStatus;
	int retries = 10;
	while(retries--) {
		xStatus = xSemaphoreTake(semaphore, QUEUE_RECEIVE_WAIT_TIMEOUT);
		if(xStatus == pdPASS) {
			return xStatus;
		} else {
			vTaskDelay( SYSTEM_TASK_DELAY );
		}
	}

	return xStatus;
}

inline
int transport_lock(transport_type_t transport, transport_direction_t direction) {
	switch(direction) {
		case DIRECTION_INPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return wait_for_semaphore(xUART1ReadSemaphore);



				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return wait_for_semaphore(xUART1WriteMutex);
				default:				return pdFALSE;
			}
			break;
		default:	return pdFALSE;
	}

	return pdFALSE;
}

inline
int transport_unlock(transport_type_t transport, transport_direction_t direction) {
	switch(direction) {
		case DIRECTION_INPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return xSemaphoreGive(xUART1ReadSemaphore);



				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return xSemaphoreGive(xUART1WriteMutex);
				default:				return pdFALSE;
			}
			break;
		default:	return pdFALSE;
	}

	return pdFALSE;
}

