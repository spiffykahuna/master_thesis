/*
 * abstract_writer.h
 *
 *  Created on: May 23, 2013
 *      Author: Kasutaja
 */

#ifndef ABSTRACT_WRITER_H_
#define ABSTRACT_WRITER_H_

#include "transport.h"
#include "packet.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "log.h"
#include "strbuffer.h"

typedef int (*write_callback_t)(char *data, size_t dataLength);

typedef struct _writer_params_t {
	transport_type_t	transport_type;
	write_callback_t	write_func;
	xQueueHandle		dataQueue;
	portTickType		queueTimeout;
	xSemaphoreHandle	writeMutex;
} writer_params_t;


void tskAbstractWriter(void *pvParameters);

#endif /* ABSTRACT_WRITER_H_ */
