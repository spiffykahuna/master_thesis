#include <stdarg.h>
#include "transport.h"

char error_space[ERROR_BUFFER_SIZE];

extern xQueueHandle  msgOutComeQueue;
extern xSemaphoreHandle xUART1WriteMutex;
extern xSemaphoreHandle xUART1ReadSemaphore;

extern xSemaphoreHandle xUART3WriteMutex;
extern xSemaphoreHandle xUART3ReadSemaphore;


inline
void send_packet_to_client(packet_t *packet) {
	portBASE_TYPE xStatus;
	strbuffer_t *logMsg;

	switch(packet->transport) {
	case TRANSPORT_UART1:
		do {
			xStatus =  xQueueSendToBack( msgOutComeQueue, &packet, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
			if(xStatus == pdPASS) { break; }
			vTaskDelay( SYSTEM_TASK_DELAY);
		} while( xStatus != pdPASS );
		break;

	default:
		logMsg = strbuffer_new();
		strbuffer_append(logMsg, "method send_packet_to_client -> Received unknown transport type: ");
		strbuffer_append(logMsg, int_to_string(packet->transport));
		logger(LEVEL_ERR, logMsg->value);
		strbuffer_destroy(&logMsg);

		break;
	}
}

/* this function transmit data directly to the destination avoiding queue mechanism */
inline
void send_data_to_client(transport_type_t transport, char *data, size_t dataLength) {
	strbuffer_t *logMsg;

	switch(transport) {
	case TRANSPORT_UART1:
//		if(transport_lock(TRANSPORT_UART1, DIRECTION_OUTPUT) == pdPASS) {
			UART1_send( (uint8_t*) data, dataLength);
//			transport_unlock(TRANSPORT_UART1, DIRECTION_OUTPUT);
//		}
		break;


	default:
		logMsg = strbuffer_new();
		strbuffer_append(logMsg, "method send_packet_to_client -> Received unknown transport type: ");
		strbuffer_append(logMsg, int_to_string(transport));
		logger(LEVEL_ERR, logMsg->value);
		strbuffer_destroy(&logMsg);

		break;
	}
}

//void report_error_to_sender(transport_type_t transport, const char *msgFormat, ...) {
//	va_list args;
//
//
//	// error json should be made using static buffers
//
//
//	// format error
//	memset(error_space, 0, ERROR_BUFFER_SIZE);
//	va_start (args, msgFormat );
//	vsnprintf(error_space, ERROR_BUFFER_SIZE, msgFormat, args);
//	va_end (args);
//
//	send_data_to_client(transport, error_space, strlen(error_space));
//
//}



const char * transport_type_to_str(transport_type_t transport) {
	switch(transport) {
	case TRANSPORT_UART1:		return "TRANSPORT_UART1";

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
				case TRANSPORT_UART3:		return wait_for_semaphore(xUART3ReadSemaphore);


				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return wait_for_semaphore(xUART1WriteMutex);
				case TRANSPORT_UART3:		return wait_for_semaphore(xUART3WriteMutex);

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
				case TRANSPORT_UART3:		return xSemaphoreGive(xUART3ReadSemaphore);



				default:				return pdFALSE;
			}
			break;

		case DIRECTION_OUTPUT:
			switch(transport) {
				case TRANSPORT_UART1:		return xSemaphoreGive(xUART1WriteMutex);
				case TRANSPORT_UART3:		return xSemaphoreGive(xUART3WriteMutex);
				default:				return pdFALSE;
			}
			break;
		default:	return pdFALSE;
	}

	return pdFALSE;
}

