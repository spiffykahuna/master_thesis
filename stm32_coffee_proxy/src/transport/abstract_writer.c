/*
 * abstract_writer.c
 *
 *  Created on: May 23, 2013
 *      Author: Kasutaja
 */

#include "abstract_writer.h"

extern void logger(log_level_t level, char *msg);


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
						strbuffer_append(errorMsg, (char*) taskName);
						strbuffer_append(errorMsg, " : Received packed has different transport type. Expected => ");
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

					int result = ERROR;

					if(transport_lock(config->transport_type, DIRECTION_OUTPUT) == pdPASS) {
						result = write_func(data, strlen(data) + 1);
						if(result != SUCCESS) {
							logger(LEVEL_ERR, (char*) taskName);
							logger(LEVEL_ERR, " : Unable to write output data\n");
						}
						transport_unlock(config->transport_type, DIRECTION_OUTPUT);
					}


					packet_destroy(&dataPacket);
				}
			}
			taskYIELD();
		}
	}
}



//void tskHandleResponses(void *pvParameters) {
//	json_t *responceJson = NULL;
//	portBASE_TYPE xStatus;
//
//	while(1) {
//		xStatus = xQueueReceive( responseQueue, &responceJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
//		if( (xStatus == pdPASS) && responceJson) {
//			transport_type_t transport = json_integer_value(json_object_get(responceJson, "transport"));
//			json_object_del(responceJson, "transport");
//
////			json_t *idObj = json_object_get(responceJson, "id");
////			json_int_t id = json_integer_value(idObj);
////
////			json_t *resultObj = json_object_get(responceJson, "result");
////			double result = json_real_value(resultObj);
////
////			json_t *versionObj = json_object_get(responceJson, "jsonrpc");
////			char * version = json_string_value(versionObj);
//
//
//			char *jsonData = json_dumps(responceJson, JSON_ENCODE_ANY );
//			if(jsonData) {
//				send_data_to_client(transport, jsonData, strlen(jsonData));
//				vPortFree(jsonData);
//			}
//			json_decref(responceJson);
//		}
//		if(uxQueueMessagesWaiting(responseQueue) > 0) continue;
//		taskYIELD();
//	}
//}
