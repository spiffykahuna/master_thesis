#include "systemTask.h"

xQueueHandle  systemMsgQueue = NULL;

void tskSystem(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	static char tempSize[32];
	//json_t *responceJson = NULL;
	//	portBASE_TYPE xStatus;

	portTickType xLastWakeTime = xTaskGetTickCount();
	//
		while(1) {
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

			memset(tempSize, 0, 32);
			siprintf(tempSize, "%s FREE: %d\n\r", taskName, xPortGetFreeHeapSize());
			logger(LEVEL_DEBUG, tempSize);

			memset(tempSize, 0, 32);
			siprintf(tempSize, "%s STACK: %d\n\r", taskName, uxTaskGetStackHighWaterMark( NULL ));
			logger(LEVEL_DEBUG, tempSize);

			vTaskDelayUntil( &xLastWakeTime, SYSTEM_TASK_DELAY  );
		}
}
