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
//#include "stm32_eval.h"
//#include "stm3210b_eval_lcd.h"

#include "log.h"	/* <== specify logging function there*/

//#include "hw_config.h"

//
//#include "usb_lib.h"
//#include "usb_desc.h"
//#include "usb_pwr.h"

#include "transport/uart/uart.h"

#include "app_config.h"
#include "mainTasks.h"
#include "tasks/systemTask.h"

#include "jansson.h"
#include "strbuffer.h"
//#include "json_parse/json.h"

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

xSemaphoreHandle xUSBReadSemaphore = NULL;
xSemaphoreHandle xUSBWriteMutex = NULL;

xQueueHandle  msgIncomeQueue = NULL;
xQueueHandle  usbOutComeQueue = NULL;

extern xQueueHandle  systemMsgQueue;

xQueueHandle  requestQueue;
xQueueHandle  responseQueue;

writer_params_t writerConfig;

int main(void)
{
	HardwareSetup();
	/* Setup STM32 system (clock, PLL and Flash configuration) */
	json_set_alloc_funcs(pvPortMalloc, vPortFree);


//	ledsInit();
	USART_init();
//	USB_initialize();

	int status = initLogger();
	if( status != SUCCESS) {
		logger(LEVEL_ERR, "Unable to initialize system logger. System stopped.\n\r");
		return 1;
	}

	if(initSystemHandlers() != SUCCESS) {
		logger(LEVEL_ERR, "Unable to initialize system handlers. System stopped.\n\r");
		return 1;
	}

	if(initTransport() != SUCCESS) {
		logger(LEVEL_ERR, "Unable to create USB reader and writer. System stopped.\n\r");
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


//void ledsInit(void)
//{
///* Initialize LEDs, Key Button, LCD and COM port(USART) available on
//	 STM3210X-EVAL board ******************************************************/
//	STM_EVAL_LEDInit(LED1);
//	STM_EVAL_LEDInit(LED2);
//	STM_EVAL_LEDInit(LED3);
//	STM_EVAL_LEDInit(LED4);
//
//
//	/* Turn on leds available on STM3210X-EVAL **********************************/
//	STM_EVAL_LEDOn(LED1);
//	STM_EVAL_LEDOn(LED2);
//	STM_EVAL_LEDOn(LED3);
//	STM_EVAL_LEDOn(LED4);
//
//}

void USART_init(void)
{
//	USART_InitStructure.USART_BaudRate = 115200;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//
//	STM_EVAL_COMInit(COM1, &USART_InitStructure);

	UART1_Init();
//	UART2_Init();
	UART3_Init();
	UART4_Init();
	UART5_Init();

}

//void USB_initialize(void)
//{
//	Set_System();
//	Set_USBClock();
//	USB_Interrupts_Config();
//	USB_Init();
//}

inline
int initTransport(void) {
	portBASE_TYPE xStatus;
	vSemaphoreCreateBinary( xUSBReadSemaphore );
	if( xUSBReadSemaphore == NULL ) {
		return ERROR;
	};

	xUSBWriteMutex = xSemaphoreCreateMutex( );
	if( xUSBWriteMutex == NULL ) {
		return ERROR;
	};

	msgIncomeQueue = xQueueCreate( INCOME_MSG_QUEUE_SIZE, sizeof(packet_t *) );
	if( msgIncomeQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create USB incoming queue\n\r");
		return ERROR;
	};

	usbOutComeQueue = xQueueCreate( OUTCOME_MSG_QUEUE_SIZE, sizeof(packet_t *) );
	if( usbOutComeQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create USB outcoming queue\n\r");
		return ERROR;
	};

	xStatus  = xTaskCreate( tskUART1Reader, ( signed char * ) "tskUART1Reader", configMINIMAL_STACK_SIZE + configMINIMAL_STACK_SIZE , NULL, PRIORITY_USB_READER_TASK, NULL );
	if(xStatus != pdPASS) {
		logger(LEVEL_ERR, "Unable to create USB reader task\n\r");
		return ERROR;
	}


	writerConfig.transport_type = TRANSPORT_UART1;
	writerConfig.dataOutputQueue = usbOutComeQueue;
	writerConfig.dataInputQueueTimeout = QUEUE_RECEIVE_WAIT_TIMEOUT;
	writerConfig.write_func = UART1_send_chars;
	writerConfig.writeMutex = xUSBWriteMutex;

	xStatus  = xTaskCreate( tskAbstractWriter, ( signed char * ) "tskUART1Writer", configMINIMAL_STACK_SIZE , (void *) &writerConfig, PRIORITY_USB_WRITER_TASK, NULL );
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

	responseQueue = xQueueCreate( RESPONSE_QUEUE_SIZE, sizeof(json_t *) );
	if( responseQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create response queue\n\r");
		return ERROR;
	};

	systemMsgQueue = xQueueCreate( SYSTEM_MSG_QUEUE_SIZE, sizeof(system_msg_t *) );
	if( systemMsgQueue == NULL ) {
		logger(LEVEL_ERR, "Unable to create queue for system messages\n\r");
		return ERROR;
	};

	xStatus = xTaskCreate( tskSystem, ( signed char * ) "tskSystem", configMINIMAL_STACK_SIZE, NULL, PRIORITY_SYSTEM_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	xStatus = xTaskCreate( tskParseJson, ( signed char * ) "tskParseJson", configMINIMAL_STACK_SIZE + 400, NULL, PRIORITY_PARSE_JSON_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	xStatus = xTaskCreate( tskHandleRequests, ( signed char * ) "tskHandleRequests", configMINIMAL_STACK_SIZE, NULL, PRIORITY_HANDLE_REQUESTS_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	xStatus = xTaskCreate( tskHandleResponses, ( signed char * ) "tskHandleResponses", configMINIMAL_STACK_SIZE, NULL, PRIORITY_HANDLE_RESPONSES_TASK, NULL );
	if(xStatus != pdPASS) {
		return ERROR;
	}

	return SUCCESS;
}

inline
int initLogger(void) {

	xLogMutex = xSemaphoreCreateMutex( );
	if( xLogMutex == NULL ) {
		logger(LEVEL_ERR, "Unable to create log mutex\n\r");
		return ERROR;
	};

	return SUCCESS;
}

