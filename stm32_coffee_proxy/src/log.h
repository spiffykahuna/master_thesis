
#ifndef __LOG_H
#define __LOG_H

#include <string.h>
#include <stdio.h>
#include <errno.h>


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
#include "semphr.h"


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


#include "strbuffer.h"
#include "messages.h"

#include "tasks/systemTask.h"
#include "transport/transport.h"

#include "transport/uart/uart.h"

typedef enum  {
	LEVEL_OFF 		= 0,
	LEVEL_FATAL 	= 1,
	LEVEL_ERR 		= 2,
	LEVEL_WARN  	= 3,
	LEVEL_INFO		= 4,
	LEVEL_DEBUG 	= 5,
	LEVEL_TRACE		= 6
} log_level_t;

typedef int (*log_func_t)(char *msg);

int log_to_UART3(char* /*str*/);

#define log_debug(msg, ...) log_info("%s: " msg, __func__, ##__VA_ARGS__)

#define log_syserr(msg) log_die("%s: %s: %s", __func__, msg, strerror(errno))

void log_die(char *msg, ...);
void log_info(char *msg, ...);


inline
void sendUsbPacket(char* str, size_t size);

void logger(log_level_t level, char *msg);

void logger_format(log_level_t level, char *msgFormat, ...);

#endif  /*__LOG_H*/
