/* Standard includes. */
#include <string.h>
#include <stdio.h>


/* Scheduler includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include "log.h"	/* <== specify logging function there*/

#include "app_config.h"
#include "mainTasks.h"
#include "tasks/systemTask.h"

#include "jansson.h"
#include "strbuffer.h"

#include "transport/transport.h"



/*-----------------------------------------------------------*/
extern void HardwareSetup();


extern void logger(log_level_t level, char *msg);
extern log_func_t log_func;

//void ledsInit(void);
void USART_init(void);
//void USB_initialize(void);

inline int initLogger(void);
inline int initTransport(void);
inline int initSystemHandlers(void);


USART_InitTypeDef USART_InitStructure;

xSemaphoreHandle xLogMutex;

xSemaphoreHandle xUART1ReadSemaphore = NULL;
xSemaphoreHandle xUART1WriteMutex = NULL;

xSemaphoreHandle xUART3ReadSemaphore = NULL;
xSemaphoreHandle xUART3WriteMutex = NULL;

xQueueHandle  msgIncomeQueue = NULL;
xQueueHandle  msgOutComeQueue = NULL;

extern xQueueHandle  systemMsgQueue;

xQueueHandle  requestQueue;
//xQueueHandle  responseQueue;

writer_params_t writerConfig;
reader_params_t readerConfig;

int main(void)
{
	/* Setup STM32 system (clock, PLL and Flash configuration) */
	HardwareSetup();

	json_set_alloc_funcs(pvPortMalloc, vPortFree);

	USART_init();

	int status = initLogger();
	if( status != SUCCESS) {
		return 1;
	}

	if(initSystemHandlers() != SUCCESS) {
		logger(LEVEL_ERR, "Unable to initialize system handlers. System has stopped.\n\r");
		return 1;
	}

	if(initTransport() != SUCCESS) {
		logger(LEVEL_ERR, "Unable to create USB reader and writer. System has stopped.\n\r");
		return 1;
	}

      /* Start the scheduler. */
  	vTaskStartScheduler();

  	logger(LEVEL_ERR, "Unable to start system due to insufficient RAM available for the system tasks. System stopped.\n\r");
      /* Will only get here if there was insufficient memory to create the idle
      task.  The idle task is created within vTaskStartScheduler(). */
  	for( ;; );


}

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	( void ) pxTask;
	( void ) pcTaskName;

	static char buffer[64];
	sprintf(buffer, "STACK IN TASK OVERFLOW: %s\r\n", pcTaskName);
	logger(LEVEL_ERR, buffer);
	for( ;; );
}

void vApplicationMallocFailedHook( void ) {
	static char buffer[64];
	snprintf(buffer, 64,"MALLOC FAILED IN TASK: %s \r\n", pcTaskGetTaskName(NULL) );
	log_func(buffer);
	for( ;; );
}
/*-----------------------------------------------------------*/

void assert_failed( unsigned char *pucFile, unsigned long ulLine )
{
	( void ) pucFile;
	( void ) ulLine;

	for( ;; );
}

void HardFault_Handler(void) {
	static char buffer[64];
	snprintf(buffer, 64,"HARD FAULT IN TASK: %s \r\n", pcTaskGetTaskName(NULL) );
	log_func(buffer);
	for( ;; );
}

void USART_init(void)
{
	UART1_Init();
//	UART2_Init();
	UART3_Init();
	UART4_Init();
	UART5_Init();
}


inline
int initTransport(void) {
	portBASE_TYPE xStatus;
	vSemaphoreCreateBinary( xUART1ReadSemaphore );
	if( xUART1ReadSemaphore == NULL ) {
		return ERROR;
	};

	xUART1WriteMutex = xSemaphoreCreateMutex( );
	if( xUART1WriteMutex == NULL ) {
		return ERROR;
	};

	vSemaphoreCreateBinary( xUART3ReadSemaphore );
		if( xUART3ReadSemaphore == NULL ) {
			return ERROR;
		};

	xUART3WriteMutex = xSemaphoreCreateMutex( );
	if( xUART3WriteMutex == NULL ) {
		return ERROR;
	};

	msgIncomeQueue = xQueueCreate( INCOME_MSG_QUEUE_SIZE, sizeof(packet_t *) );
	if( msgIncomeQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create USB incoming queue\n\r");
		return ERROR;
	};

    msgOutComeQueue = xQueueCreate( OUTCOME_MSG_QUEUE_SIZE, sizeof(packet_t *) );
	if( msgOutComeQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create USB outcoming queue\n\r");
		return ERROR;
	};


	readerConfig.transport_type = TRANSPORT_UART1;
	readerConfig.dataInputQueue = msgIncomeQueue;
	readerConfig.dataInputQueueTimeout = QUEUE_SEND_WAIT_TIMEOUT;
	readerConfig.dataReadSemaphore = xUART1ReadSemaphore;
	readerConfig.stream_has_byte = UART1_has_bytes;
	readerConfig.read_char_func = UART1_read_char;

	xStatus  = xTaskCreate( tskAbstractReader, ( signed char * ) "tskUART1Reader", configMINIMAL_STACK_SIZE + configMINIMAL_STACK_SIZE , (void *) &readerConfig, PRIORITY_UART_READER_TASK, NULL );
	if(xStatus != pdPASS) {
		logger(LEVEL_ERR, "Unable to create USB reader task\n\r");
		return ERROR;
	}


	writerConfig.transport_type = TRANSPORT_UART1;
	writerConfig.dataOutputQueue = msgOutComeQueue;
	writerConfig.dataInputQueueTimeout = QUEUE_RECEIVE_WAIT_TIMEOUT;
	writerConfig.write_func = UART1_send_chars;
	writerConfig.writeMutex = xUART1WriteMutex; //TODO do we really need it?
	writerConfig.dataPacketType = PKG_TYPE_OUTGOING_MESSAGE_STRING;

	xStatus  = xTaskCreate( tskAbstractWriter, ( signed char * ) "tskUART1Writer", configMINIMAL_STACK_SIZE , (void *) &writerConfig, PRIORITY_UART_WRITER_TASK, NULL );
	if(xStatus != pdPASS) {
		logger(LEVEL_ERR, "Unable to create USB reader task\n\r");
		return ERROR;
	}

	return SUCCESS;
}

inline
int initSystemHandlers(void) {
	portBASE_TYPE xStatus;

	requestQueue = xQueueCreate( REQUEST_QUEUE_SIZE, sizeof(json_t *) );
	if( requestQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create request queue\n\r");
		return ERROR;
	};

//	responseQueue = xQueueCreate( RESPONSE_QUEUE_SIZE, sizeof(json_t *) );
//	if( responseQueue == NULL ) {
//		logger(LEVEL_ERR, "Unable to create response queue\n\r");
//		return ERROR;
//	};

	systemMsgQueue = xQueueCreate( SYSTEM_MSG_QUEUE_SIZE, sizeof(system_msg_t *) );
	if( systemMsgQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create queue for system messages\n\r");
		return ERROR;
	};

	xStatus = xTaskCreate( tskSystem, ( signed char * ) "tskSystem", configMINIMAL_STACK_SIZE, NULL, PRIORITY_SYSTEM_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	xStatus = xTaskCreate( tskParseJson, ( signed char * ) "tskParseJson", configMINIMAL_STACK_SIZE + 600, NULL, PRIORITY_PARSE_JSON_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	xStatus = xTaskCreate( tskHandleRequests, ( signed char * ) "tskHandleRequests", configMINIMAL_STACK_SIZE + 400, NULL, PRIORITY_HANDLE_REQUESTS_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

//	xStatus = xTaskCreate( tskHandleResponses, ( signed char * ) "tskHandleResponses", configMINIMAL_STACK_SIZE, NULL, PRIORITY_HANDLE_RESPONSES_TASK, NULL );
//	if(xStatus != pdPASS) {
//		return ERROR;
//	}

	return SUCCESS;
}

inline
int initLogger(void) {
	xLogMutex = xSemaphoreCreateMutex( );
	setSystemLogLevel(LEVEL_DEBUG);
	return xLogMutex != NULL ? SUCCESS : ERROR;
}

