#include "mainTasks.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern __IO  uint8_t Receive_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
extern __IO  uint32_t Receive_length ;

//extern uint8_t Send_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
uint32_t packet_sent=1;
uint32_t packet_receive=1;

extern xQueueHandle  usbIncomeQueue;
extern xQueueHandle  requestQueue;
extern xQueueHandle  responseQueue;

//extern xSemaphoreHandle xLogMutex;
extern xSemaphoreHandle xUSBSemaphore;

extern log_func_t log_func;

extern char  error_space[ERROR_BUFFER_SIZE];
extern const msg_maintasks MSG_MAINTASKS;
extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

inline
int isJsonRPCVersion2_0(json_t *object) {
	if(!json_is_object(object))	return FALSE; 	 /* may be also array if it is batch request, not implemented yet */
	if(!json_object_get(object, "id")) return FALSE;
	if(!json_object_get(object, "jsonrpc")) return FALSE;
	if(!json_object_get(object, "method")) return FALSE;
//	if(!json_object_get(object, "params")) return FALSE; // <-- This member MAY be omitted.

	json_t *versionObj = json_object_get(object, "jsonrpc");
	if(!json_is_string(versionObj))	return FALSE;

	const char *version = json_string_value(versionObj);
	if(!version) return FALSE;
	if(strcmp("2.0", version) != 0) return FALSE;

	return TRUE;
}

inline
void format_error_text(strbuffer_t *errorText, json_error_t *error) {
	if(errorText && error) {
		snprintf(error_space,ERROR_BUFFER_SIZE,
				"Text: %s    Source: %s    Line: %d    Column: %d   Position: %d",
				error->text, error->source, error->line, error->column, error->position);
		strbuffer_append(errorText, error_space);
	}
}

json_t * parseJsonPacket(packet_t ** jsonPacket) {

	json_t *root;
	json_error_t error;

	transport_type_t transport = (*jsonPacket)->transport;

	snprintf(error_space, ERROR_BUFFER_SIZE,
			"parseJsonPacket    Received json packet: transport=%s    length=%d\n",
			transport_type_to_str(transport), (*jsonPacket)->jsonDoc->length
	);
	logger(LEVEL_INFO, error_space);


	root = json_loads((*jsonPacket)->jsonDoc->value, 0, &error);
	packet_destroy(jsonPacket);

	if(root != NULL) {
		/*
			decode array or object
			array --> batch (optional, but will be implemented only  if there will be enough resources mcu flash)
			if batch proceed requests one by one and put the batch flag
		*/
		if(isJsonRPCVersion2_0(root)){
			json_t *transportCode = json_integer( (json_int_t) transport);
			json_object_set_new(root, "transport", transportCode);
			logger(LEVEL_INFO, "parseJsonPacket Packet parsing succeed\n");
		} else {
			json_decref(root);
			report_error_to_sender(
				transport,
				MSG_JSONRPC_ERRORS.general_error_json,
				JSONRPC_INVALID_REQUEST,
				MSG_JSONRPC_ERRORS.invalid_request,
				MSG_MAINTASKS.parseJsonPacket.invalid_jsonrpc_2_0,
				"null"
			);
			//print_help(transport);
			logger(LEVEL_WARN, "parseJsonPacket ");
			logger(LEVEL_WARN, MSG_MAINTASKS.parseJsonPacket.invalid_jsonrpc_2_0);

			return NULL;
		}
	} else {
		strbuffer_t *errorText = strbuffer_new();
		format_error_text(errorText, &error);
		report_error_to_sender(
			transport,
			MSG_JSONRPC_ERRORS.general_error_json,
			JSONRPC_PARSE_ERROR,
			MSG_JSONRPC_ERRORS.parse_error,
			errorText->value,
			"null"
		);
		logger(LEVEL_WARN, "parseJsonPacket\t Packet parsing failed: ");
		logger(LEVEL_WARN, errorText->value);
		logger(LEVEL_WARN, "\n");
		strbuffer_destroy(&errorText);
	}
	return root;
}

void tskHandleResponses(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	json_t *responceJson = NULL;
	portBASE_TYPE xStatus;

	while(1) {
		if(uxQueueMessagesWaiting(responseQueue) > 0) {
			xStatus = xQueueReceive( responseQueue, &responceJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && responceJson) {
				transport_type_t transport = json_integer_value(json_object_get(responceJson, "transport"));
				json_object_del(responceJson, "transport");

	//			json_t *idObj = json_object_get(responceJson, "id");
	//			json_int_t id = json_integer_value(idObj);
	//
	//			json_t *resultObj = json_object_get(responceJson, "result");
	//			double result = json_real_value(resultObj);
	//
	//			json_t *versionObj = json_object_get(responceJson, "jsonrpc");
	//			char * version = json_string_value(versionObj);


				char *jsonData = json_dumps(responceJson, JSON_ENCODE_ANY );
				if(jsonData) {
					packet_t *packet = packet_new(transport);
					strbuffer_t *payload = strbuffer_new();
					strbuffer_append(payload, jsonData);
					vPortFree(jsonData);

					packet->jsonDoc = payload;
					send_packet_to_client(packet);
				}
				json_decref(responceJson);
				continue;
			}
		}
		taskYIELD();
	}
}

void tskHandleRequests(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	json_t 	*requestJson = NULL;

	json_t *responseJson = NULL;

	json_t *idObj;
	json_int_t id;
	json_t *methodObj;
	char* methodName;


	portBASE_TYPE xStatus;
	while(1) {
		if(uxQueueMessagesWaiting(requestQueue) > 0) {
			xStatus = xQueueReceive( requestQueue, &requestJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && requestJson) {

				idObj = json_object_get(requestJson, "id");
				id = json_integer_value(idObj);

				methodObj = json_object_get(requestJson, "method");
				methodName = json_string_value(methodObj);

				snprintf(error_space, ERROR_BUFFER_SIZE, "%s :  Received request. id = %d    method = %s\n", taskName, (int) id, methodName );
				logger(LEVEL_INFO, error_space);

				responseJson = handle_request(&requestJson);
				json_decref(requestJson);
				if(responseJson) {

					idObj = json_object_get(responseJson, "id");
					id = json_integer_value(idObj);



					snprintf(error_space, ERROR_BUFFER_SIZE, "%s :  Received response. id = %d\n", taskName, (int)id);
					logger(LEVEL_INFO, error_space);

					xStatus = xQueueSendToBack( responseQueue, &responseJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
					if( xStatus != pdPASS ){
						char* idStr = json_dumps(json_object_get(requestJson, "id"), JSON_ENCODE_ANY);
						transport_type_t transport = json_integer_value(json_object_get(requestJson, "transport"));
						json_object_del(requestJson, "transport");
						json_decref(requestJson);
						// send using error reporting function
						if(id && transport) {
							report_error_to_sender(
								transport,
								MSG_JSONRPC_ERRORS.general_error_json,
								JSONRPC_SERVER_ERROR,				/* <-- code */
								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
								MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout , /* <-- data */
								idStr
							);
						}
						if(idStr) vPortFree(idStr);
						snprintf(error_space, ERROR_BUFFER_SIZE, "%s :  Unable to add response to queue. id = %d\tmethod = %s\n", taskName, (int) id, methodName );
						logger(LEVEL_INFO, error_space);
					}

				}
			}
		}
		taskYIELD();
	}

}


void tskParseJson(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	packet_t *incomePacket = NULL;
	json_t 	*requestJson = NULL;

	portBASE_TYPE xStatus;
	while(1) {
		if(uxQueueMessagesWaiting(usbIncomeQueue) > 0) {
			xStatus = xQueueReceive( usbIncomeQueue, &incomePacket, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && incomePacket) {
				snprintf(error_space, ERROR_BUFFER_SIZE, "%s :  Received packet. len = %d\n", taskName, incomePacket->jsonDoc->length );
				logger(LEVEL_INFO, error_space);

				requestJson = parseJsonPacket(&incomePacket);
				if(requestJson) {
					snprintf(error_space, ERROR_BUFFER_SIZE,
						"%s :  Parsing packet was sucessful\n", taskName
					);
					logger(LEVEL_INFO, error_space);

					json_incref(requestJson);
					xStatus = xQueueSendToBack( requestQueue, &requestJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
					if( xStatus != pdPASS ){

						json_object_del(requestJson, "method");
						json_object_del(requestJson, "params");

						json_t* errorObj = json_pack(
							"{s:i, s:s, s:s}",
							"code", (json_int_t) JSONRPC_SERVER_ERROR,
							"message", MSG_JSONRPC_ERRORS.server_error,
							"data", MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout
						);

						json_object_set_new(requestJson, "error", errorObj);

						logger(LEVEL_WARN, taskName);
						logger(LEVEL_WARN, " ");
						logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout);

						// send error back to client using output queue
						xStatus = xQueueSendToBack( responseQueue, &requestJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
						if( xStatus != pdPASS ){
							char* id = json_dumps(json_object_get(requestJson, "id"), JSON_ENCODE_ANY);
							transport_type_t transport = json_integer_value(json_object_get(requestJson, "transport"));
							json_object_del(requestJson, "transport");
							json_decref(requestJson);
							// send using error reporting function
							if(id && transport) {
								report_error_to_sender(
									transport,
									MSG_JSONRPC_ERRORS.general_error_json,
									JSONRPC_SERVER_ERROR,				/* <-- code */
									MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
									MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout , /* <-- data */
									id
								);
							}
							if(id) vPortFree(id);

							logger(LEVEL_WARN, taskName);
							logger(LEVEL_WARN, " ");
							logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout);
						}
					}
				}
				json_decref(requestJson);
				packet_destroy(&incomePacket);
			}
		}
		taskYIELD();
	}
}


inline
packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp, signed char* taskName, char* tempSize) {
	packet_t *incomePacket = NULL;
	incomePacket = packet_new(TRANSPORT_USB);
	if(!incomePacket) {
		report_error_to_sender(
			TRANSPORT_USB,
			MSG_JSONRPC_ERRORS.general_error_json,
			JSONRPC_SERVER_ERROR,				/* <-- code */
			MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
			MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet , /* <-- data */
			"null" 	/* <-- id */
		);
		packet_destroy(&incomePacket);
		strbuffer_destroy(temp);

		snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
		logger(LEVEL_WARN, tempSize);
		logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet);

		Receive_length = 0;
		return NULL;
	}
	incomePacket->jsonDoc = *temp;
	return incomePacket;
}


void tskUSBReader(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	portBASE_TYPE xStatus;
	strbuffer_t *temp = NULL;
	char *terminator;


	static char tempSize[BUFF_SIZE];

	packet_t *incomePacket = NULL;

	while (1) {
		if (bDeviceState == CONFIGURED && packet_receive == 1) {
			if( xSemaphoreTake( xUSBSemaphore, portMAX_DELAY ) == pdTRUE ) {
				if(Receive_length  != 0) {
					if(!temp) {
						temp = strbuffer_new();
					}

					if(!temp) {
						report_error_to_sender(
							TRANSPORT_USB,						/* <-- which way to send  */
							MSG_JSONRPC_ERRORS.general_error_json,	/* <-- format */
							JSONRPC_SERVER_ERROR,				/* <-- code */
							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
							MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet , /* <-- data */
							"null" 	/* <-- id */
						);
						strbuffer_destroy(&temp);

						snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
						logger(LEVEL_WARN, tempSize);
						logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet);

						Receive_length = 0;
						continue;
					}

					if(strbuffer_append_bytes(temp, Receive_Buffer, Receive_length)) {
						strbuffer_destroy(&temp);
						report_error_to_sender(
							TRANSPORT_USB,
							MSG_JSONRPC_ERRORS.general_error_json,
							JSONRPC_SERVER_ERROR,				/* <-- code */
							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
							MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet , /* <-- data */
							"null" 	/* <-- id */
						);
						strbuffer_destroy(&temp);

						snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
						logger(LEVEL_WARN, tempSize);
						logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet);

						Receive_length = 0;
						continue;
					}

					terminator = strchr(temp->value, EOT_CHAR);
					if( terminator != NULL) {
						/* remove EOT character from the end */
						strbuffer_pop(temp);
						temp->value[temp->length] = '\0';

						/* Check if user requested help message */
						if( (strcmp(temp->value, "--help") == 0) ||
							(strcmp(temp->value, "-h") == 0) )
						{
							strbuffer_t *callHelp = strbuffer_new();
							strbuffer_append(callHelp, "{\"jsonrpc\":\"2.0\",\"method\":\"system.help\",\"id\": null}");
							incomePacket = createNewIncomePacketFromStr(&callHelp, taskName, tempSize);
							if(!incomePacket)
								continue;
							xStatus = 0;
							while(xStatus != pdPASS) {
								xStatus = xQueueSendToBack( usbIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
							}
						}

						incomePacket = createNewIncomePacketFromStr(&temp, taskName, tempSize);
						if(!incomePacket)
							continue;
						xStatus = xQueueSendToBack( usbIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
						if( xStatus != pdPASS )
						{
							packet_destroy(&incomePacket);
							strbuffer_destroy(&temp);
							report_error_to_sender(
								TRANSPORT_USB,
								MSG_JSONRPC_ERRORS.general_error_json,
								JSONRPC_SERVER_ERROR,				/* <-- code */
								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
								MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout , /* <-- data */
								"null" 	/* <-- id */
							);

							snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
							logger(LEVEL_WARN, tempSize);
							logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.device_is_busy_timeout);

							Receive_length = 0;
							continue;
						}

						snprintf(tempSize, BUFF_SIZE, "%s Received new packet( len= %d )\n\r", taskName, temp->length);
						logger(LEVEL_DEBUG, tempSize);

						temp = NULL;
					} else {
						/* Overflow check */
						if(temp->length > MAX_INCOME_MSG_SIZE) {
							strbuffer_destroy(&temp);
							report_error_to_sender(
								TRANSPORT_USB,
								MSG_JSONRPC_ERRORS.general_error_json,
								JSONRPC_SERVER_ERROR,				/* <-- code */
								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
								MSG_MAINTASKS.tskUSBReader.incoming_buffer_overflow , /* <-- data */
								"null" 	/* <-- id */
							);
							snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
							logger(LEVEL_WARN, tempSize);
							logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.incoming_buffer_overflow);
						}
					}
				}
				Receive_length = 0;
			}
			CDC_Receive_DATA();
		}




//		snprintf(tempSize, BUFF_SIZE, "FLOAT %f\n", -555.666);
//		log_d(tempSize);

		taskYIELD();
	}
}



