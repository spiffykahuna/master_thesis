#include "systemTask.h"

xQueueHandle  systemMsgQueue = NULL;


extern log_func_t log_func;

void tskSystem(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	static char tempSize[32];
	system_msg_t *sysMsg;
	//json_t *responceJson = NULL;
	portBASE_TYPE xStatus;

	portTickType xLastWakeTime = xTaskGetTickCount();

	while(1) {
		if(uxQueueMessagesWaiting(systemMsgQueue) > 0) {
			xStatus = xQueueReceive( systemMsgQueue, &sysMsg, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && sysMsg) {
				switch(sysMsg->msgType) {
				case MSG_TYPE_LOGGING:
					if(sysMsg->logMsg && sysMsg->logMsg->value) {
						if(transport_lock(LOG_TRANSPORT, DIRECTION_OUTPUT) == pdPASS) {
							log_func(sysMsg->logMsg->value);
							transport_unlock(LOG_TRANSPORT, DIRECTION_OUTPUT);
						}
					}
					break;
				// TODO system dynamic properties ( json object maybe or plain structure with mutex)
				// TODO dynamic system configuration
				default:
					break;
				}
				system_msg_destroy(&sysMsg);
			}
			continue; /* until all system messages are processed */
		}
//			xStatus = xQueueReceive( responseQueue, &responceJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
//			if( (xStatus == pdPASS) && responceJson) {
//				transport_type_t transport = json_integer_value(json_object_get(responceJson, "transport"));
//				json_object_del(responceJson, "transport");
//
//	//			json_t *idObj = json_object_get(responceJson, "id");
//	//			json_int_t id = json_integer_value(idObj);
//	//
//	//			json_t *resultObj = json_object_get(responceJson, "result");
//	//			double result = json_real_value(resultObj);
//	//
//	//			json_t *versionObj = json_object_get(responceJson, "jsonrpc");
//	//			char * version = json_string_value(versionObj);
//
//
//				char *jsonData = json_dumps(responceJson, JSON_ENCODE_ANY );
//				if(jsonData) {
//					send_data_to_client(transport, jsonData, strlen(jsonData));
//					vPortFree(jsonData);
//				}
//				json_decref(responceJson);
//			}
//			if(uxQueueMessagesWaiting(responseQueue) > 0) continue;

//		memset(tempSize, 0, 32);
//		siprintf(tempSize, "%s FREE: %d\n\r", taskName, xPortGetFreeHeapSize());
//		logger(LEVEL_DEBUG, tempSize);
//
//		memset(tempSize, 0, 32);
//		siprintf(tempSize, "%s STACK: %d\n\r", taskName, (int) uxTaskGetStackHighWaterMark( NULL ));
//		logger(LEVEL_DEBUG, tempSize);
		//memset(tempSize, 0, BUFF_SIZE);



		snprintf(tempSize, 32, "%s FREE: %d\n\r", taskName, xPortGetFreeHeapSize());
		if(transport_lock(LOG_TRANSPORT, DIRECTION_OUTPUT) == pdPASS) {
			log_func(tempSize);
			transport_unlock(LOG_TRANSPORT, DIRECTION_OUTPUT);
		}
//
//		memset(tempSize, 0, BUFF_SIZE);
//		siprintf(tempSize, "%s STACK: %d\n\r", taskName, (int) uxTaskGetStackHighWaterMark( NULL ));
//		logger(LEVEL_DEBUG, tempSize);

		vTaskDelayUntil( &xLastWakeTime, SYSTEM_TASK_DELAY  );
	}
}

inline
void system_msg_destroy(system_msg_t **sysMsg) {
	if(*sysMsg) {
		switch((*sysMsg)->msgType) {
			case MSG_TYPE_LOGGING:
				strbuffer_destroy(&(*sysMsg)->logMsg);
				break;
			default:
				break;
		}
		vPortFree(*sysMsg);
		*sysMsg = NULL;
	}
}

inline
system_msg_t* system_msg_new(system_msg_type_t msgType) {
	system_msg_t *sysMsg = NULL;

	sysMsg = (system_msg_t*) pvPortMalloc(sizeof(system_msg_t));
	if(!sysMsg) { return NULL;}

	sysMsg->msgType = msgType;
//	switch(msgType) {
//		case MSG_TYPE_LOGGING:
//			sysMsg->logMsg = strbuffer_new();
//			break;
//		default:
//			break;
//	}
	return sysMsg;
}

int system_msg_add_to_queue(system_msg_t *sysMsg) {
	int retries;
	portBASE_TYPE xStatus = 0;

	if(systemMsgQueue != NULL) {
		retries = 5;
		while(retries--) {
			xStatus = xQueueSendToBack( systemMsgQueue, &sysMsg, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
		}
	} else {
		return FALSE;
	}

	return (xStatus == pdPASS);
}

inline
void system_flush_messages() {
	while(1) {
		if(uxQueueMessagesWaiting(systemMsgQueue) > 0) {
			vTaskDelay( SYSTEM_TASK_DELAY );
		} else {
			break;
		}
	}
}
