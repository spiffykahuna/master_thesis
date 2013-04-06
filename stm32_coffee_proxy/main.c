
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eval.h"
/* Standard includes. */
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"


#include "stm3210b_eval_lcd.h"

#include <stdio.h>

/* The time between cycles of the 'check' task - which depends on whether the
check task has detected an error or not. */
#define mainCHECK_DELAY_NO_ERROR			( ( portTickType ) 5000 / portTICK_RATE_MS )
#define mainCHECK_DELAY_ERROR				( ( portTickType ) 500 / portTICK_RATE_MS )

/* The LED controlled by the 'check' task. */
#define mainCHECK_LED						( 3 )

/* Task priorities. */
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainECHO_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* COM port and baud rate used by the echo task. */
#define mainCOM0							( 0 )
#define mainBAUD_RATE						( 115200 )

/*-----------------------------------------------------------*/

static void ledBlinkTask(void *pvParameters)
{
	int i;
	Led_TypeDef led = (Led_TypeDef) pvParameters;
	while (1)
	  {
			STM_EVAL_LEDToggle(led);
			for(i=0;i<(0x100000 / (led + 1));i++);
			taskYIELD();
	  }

}


int main(void)
{
	/* Setup STM32 system (clock, PLL and Flash configuration) */
	SystemInit();

	/* Initialize LEDs, Key Button, LCD and COM port(USART) available on
	 STM3210X-EVAL board ******************************************************/
	STM_EVAL_LEDInit(LED1);
	STM_EVAL_LEDInit(LED2);
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);


	/* Turn on leds available on STM3210X-EVAL **********************************/
	STM_EVAL_LEDOn(LED1);
	STM_EVAL_LEDOn(LED2);
	STM_EVAL_LEDOn(LED3);
	STM_EVAL_LEDOn(LED4);



	/* Create the 'check' task, which is also defined within this file. */
	xTaskCreate( ledBlinkTask, ( signed char * ) "Blink", configMINIMAL_STACK_SIZE, (void*) LED1, mainCHECK_TASK_PRIORITY, NULL );
	xTaskCreate( ledBlinkTask, ( signed char * ) "Blink", configMINIMAL_STACK_SIZE, (void*) LED2, mainCHECK_TASK_PRIORITY, NULL );
	xTaskCreate( ledBlinkTask, ( signed char * ) "Blink", configMINIMAL_STACK_SIZE, (void*) LED3, mainCHECK_TASK_PRIORITY, NULL );
	xTaskCreate( ledBlinkTask, ( signed char * ) "Blink", configMINIMAL_STACK_SIZE, (void*) LED4, mainCHECK_TASK_PRIORITY, NULL );

      /* Start the scheduler. */
  	vTaskStartScheduler();

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

	for( ;; );
}
/*-----------------------------------------------------------*/

void assert_failed( unsigned char *pucFile, unsigned long ulLine )
{
	( void ) pucFile;
	( void ) ulLine;

	for( ;; );
}

