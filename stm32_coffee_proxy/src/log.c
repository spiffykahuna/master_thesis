#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log.h"

inline strbuffer_t * log_if_level_set(log_level_t level, char *msg);

extern xSemaphoreHandle xLogMutex;
log_level_t systemLogLevel;

static char logBuffer[STRING_BUFFER_SIZE];

log_func_t log_func = log_to_UART3;	/* <== specify logging function here*/

int log_to_UART3(char *str) {
	size_t length = strlen(str);
	UART3_send((uint8_t*) str, length);
	return SUCCESS;
}

void logger(log_level_t level, char *msg) {

	strbuffer_t *logMsg = NULL;

	switch(level) {
	case LEVEL_OFF:
		return; /* no message here */
	case LEVEL_FATAL:
		logMsg = log_if_level_set(level, msg, "FATAL : ");
		break;

	case LEVEL_ERR:
		logMsg = log_if_level_set(level, msg, "ERROR : ");
		break;
	case LEVEL_WARN:
		logMsg = log_if_level_set(level, msg, "WARN : ");
		break;
	case LEVEL_INFO:
		logMsg = log_if_level_set(level, msg, "INFO : ");
		break;
	case LEVEL_DEBUG:
		logMsg = log_if_level_set(level, msg, "DEBUG : ");
		break;
	case LEVEL_TRACE:
		logMsg = log_if_level_set(level, msg, "TRACE : ");
		break;
	}

	if(logMsg) {
		system_msg_t *systemMsg = system_msg_new(MSG_TYPE_LOGGING);
		systemMsg->logMsg = logMsg;

		system_msg_add_to_queue(systemMsg);
	}
}

void logger_format(log_level_t level, char *msgFormat, ...) {

	va_list args;
	if(level > LEVEL_OFF) {
		if(wait_for_semaphore(xLogMutex) == pdPASS) {
			// format error
				memset(logBuffer, 0, STRING_BUFFER_SIZE);
				va_start (args, msgFormat );
				vsnprintf(logBuffer, STRING_BUFFER_SIZE, msgFormat, args);
				va_end(args);

			logger(level, logBuffer);
			xSemaphoreGive(xLogMutex);
		}
	}
}

inline char * getCurrentSystemState() {
	static char temp[64];
	snprintf(temp, 64, "%d %d %s : ", (int) xTaskGetTickCount(), (int) xPortGetFreeHeapSize(), pcTaskGetTaskName(NULL));
	return temp;
}

inline strbuffer_t *  log_if_level_set(log_level_t level, char *msg, const char *levelMessage) {
	if(level > systemLogLevel) return NULL;

	strbuffer_t *logMsg = strbuffer_new();

	strbuffer_append(logMsg, getCurrentSystemState());
	strbuffer_append(logMsg, levelMessage);
	strbuffer_append(logMsg, *msg);
	strbuffer_append(logMsg, "\n");

	return logMsg;
}

// TODO proper logging everywhere ( on method start, finish, between on different events)
void setSystemLogLevel(log_level_t level) {
	if(level >= LEVEL_OFF && level <= LEVEL_TRACE) {
		systemLogLevel = level;
	}
}
