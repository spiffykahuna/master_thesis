/*
 * abstract_writer.c
 *
 *  Created on: May 23, 2013
 *      Author: Kasutaja
 */

#include "abstract_writer.h"

extern void logger(log_level_t level, char *msg);

inline
void write_data(packet_t *dataPacket, writer_params_t *config);

void tskAbstractWriter(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	portBASE_TYPE xStatus;
	strbuffer_t *temp = NULL;
	char *terminator;
	packet_t *dataPacket = NULL;


	if(pvParameters) {
		writer_params_t *config = (writer_params_t*) pvParameters;
		write_callback_t write_func = config->write_func;

		xQueueHandle dataQueue	= config->dataOutputQueue;

		while(1) {
			if(uxQueueMessagesWaiting(dataQueue) > 0) {
				xStatus = xQueueReceive( dataQueue, &dataPacket, config->dataInputQueueTimeout );
				if( (xStatus == pdPASS) && dataPacket) {
					if(dataPacket->transport != config->transport_type) {
						strbuffer_t *errorMsg = strbuffer_new();
						strbuffer_append(errorMsg, "Received packed has different transport type. Expected => ");
						strbuffer_append(errorMsg, transport_type_to_str(config->transport_type));
						strbuffer_append(errorMsg, " Actual => ");
						strbuffer_append(errorMsg, transport_type_to_str(dataPacket->transport));
						strbuffer_append(errorMsg, ". Packet was deleted...\n");
						logger(LEVEL_ERR, errorMsg->value);
						strbuffer_destroy(&errorMsg);

						// TODO if transport is wrong, we need to put this packet somewhere, for example a special queue
						// System should put this packet to right place or destroy it
						packet_destroy(&dataPacket);
						continue;
					};
					char * data = (dataPacket->jsonDoc)->value;

					write_data(dataPacket, config);

					packet_destroy(&dataPacket);
				}
			}
			taskYIELD();
		}
	}
}

inline
void write_data(char *data, writer_params_t *config) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	int result = ERROR;

	if(transport_lock(config->transport_type, DIRECTION_OUTPUT) == pdPASS) {
		char temp[16];

		int len = snprintf(temp, 16, "%d:", strlen(data));
		result = write_func(temp, len);
		if(result != SUCCESS) {
			logger(LEVEL_ERR, "Unable to write output data\n");
		}

		result = write_func(data, strlen(data) + 1);
		if(result != SUCCESS) {
			logger(LEVEL_ERR, "Unable to write output data\n");
		}

		temp[0] = ',';
		result = write_func(temp, 1);
		if(result != SUCCESS) {
			logger(LEVEL_ERR, "Unable to write output data\n");
		}
		transport_unlock(config->transport_type, DIRECTION_OUTPUT);
	}
}
