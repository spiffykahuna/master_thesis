#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H


/* Scheduler includes. */
#ifndef JSON_TEST

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#endif

#define STRING_BUFFER_SIZE 			512
#define ERROR_BUFFER_SIZE 			512
#define MAX_INCOME_MSG_SIZE 		1024
#define MAX_INCOME_MSG_SIZE_MSG 	"1024"	/* <== used in error message */

#define TOKENS_COUNT 300

/* QUEUES */
#define INCOME_MSG_QUEUE_SIZE		10
#define OUTCOME_MSG_QUEUE_SIZE		10

#define REQUEST_QUEUE_SIZE			20
#define RESPONSE_QUEUE_SIZE			20

#define SYSTEM_MSG_QUEUE_SIZE		(INCOME_MSG_QUEUE_SIZE * 2)

/* end of transmission character */
#define EOT	23


typedef enum _proxy_error_t {
	PROXY_MEMORY_ALLOC_FAILED 				= 1,
	PROXY_DEVICE_IS_BUSY 					= 2,
	PROXY_INCOMING_BUFFER_OVERFLOW 			= 3,
	PROXY_INVALID_INCOME_PACKET 			= 4,

	/* http://www.jsonrpc.org/specification */
	JSONRPC_PARSE_ERROR 					= -32700,
	JSONRPC_INVALID_REQUEST					= -32600,
	JSONRPC_METHOD_NOT_FOUND				= -32601,
	JSONRPC_INVALID_PARAMS					= -32602,
	JSONRPC_INTERNAL_ERROR					= -32603,
	JSONRPC_SERVER_ERROR					= -32604,
	JSONRPC_APPLICATION_ERROR				= -32500,
	JSONRPC_SYSTEM_ERROR					= -32400,
	JSONRPC_TRANSPORT_ERROR					= -32300
} proxy_error_t;

#define QUEUE_SEND_WAIT_TIMEOUT  			(2000 / portTICK_RATE_MS)
#define QUEUE_RECEIVE_WAIT_TIMEOUT 			(1000 / portTICK_RATE_MS)

#define SYSTEM_TASK_DELAY					(250 / portTICK_RATE_MS)

/* task priorities */
#define PRIORITY_SYSTEM_TASK				(tskIDLE_PRIORITY + 3)
#define PRIORITY_PARSE_JSON_TASK			(tskIDLE_PRIORITY + 1)
#define PRIORITY_HANDLE_REQUESTS_TASK		(tskIDLE_PRIORITY + 1)
#define PRIORITY_HANDLE_RESPONSES_TASK		(tskIDLE_PRIORITY + 1)
#define PRIORITY_USB_READER_TASK			(tskIDLE_PRIORITY + 1)
#define PRIORITY_USB_WRITER_TASK			(tskIDLE_PRIORITY + 1)

#endif  /*__APP_CONFIG_H*/
