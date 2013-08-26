#ifndef ABSTRACT_READER_H_
#define ABSTRACT_READER_H_

#include "transport.h"
#include "packet.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "log.h"
#include "strbuffer.h"

#include "messages.h"
#include "mainTasks.h"

typedef char (*stream_read_char_function_t)();
typedef int (*stream_has_byte_function_t)();

typedef struct _reader_params_t {
	transport_type_t				transport_type;
	stream_read_char_function_t		read_char_func;
	stream_has_byte_function_t		stream_has_byte;
	xQueueHandle					dataInputQueue;
	portTickType					dataInputQueueTimeout;
	xSemaphoreHandle				dataReadSemaphore;
} reader_params_t;

void tskAbstractReader(void *pvParameters);


#endif /* ABSTRACT_READER_H_ */
