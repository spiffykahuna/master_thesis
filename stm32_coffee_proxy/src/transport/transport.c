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
extern xSemaphoreHandle xUSBWriteMutex;
extern xSemaphoreHandle xUSBReadSemaphore;

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
	case TRANSPORT_USB:
		xStatus =  xQueueSendToBack( usbOutComeQueue, &packet, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
		if( xStatus != pdPASS ){
			// TODO Error reporting on queue full
		}
		break;

	default:
		/* TODO choose default transport or logging interface here */
		break;
	}
}


/* this function transmit data directly to the destination avoiding queue mechanism */
inline
void send_data_to_client(transport_type_t transport, char *data, size_t dataLength) {
	switch(transport) {
	case TRANSPORT_USB:
			write_usb(data, dataLength);
		break;

	default:
		/* TODO choose default transport or logging interface here */
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
int write_usb(char *data, size_t length) {
	log_d(data);
	return 1;
}

inline
char * transport_type_to_str(transport_type_t transport) {
	switch(transport) {
	case TRANSPORT_USB:		return "TRANSPORT_USB";

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
				case TRANSPORT_USB:		return wait_for_semaphore(xUSBReadSemaphore);



				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_USB:		return wait_for_semaphore(xUSBWriteMutex);
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
				case TRANSPORT_USB:		return xSemaphoreGive(xUSBReadSemaphore);



				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_USB:		return xSemaphoreGive(xUSBWriteMutex);
				default:				return pdFALSE;
			}
			break;
		default:	return pdFALSE;
	}

	return pdFALSE;
}

