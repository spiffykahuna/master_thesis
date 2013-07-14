#include "mainTasks.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/

extern xQueueHandle  msgIncomeQueue;
extern xQueueHandle  requestQueue;
//extern xQueueHandle  responseQueue;

extern xSemaphoreHandle xUART1ReadSemaphore;

extern const msg_maintasks MSG_MAINTASKS;
extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

int sendOutputMessage(packet_t *msgPacket);
int add_new_rpc_request(packet_t *incomePacket);

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
		strbuffer_append(errorText, "Text: ");
		strbuffer_append(errorText, error->text);

		strbuffer_append(errorText, "    Source: ");
		strbuffer_append(errorText, error->source);

		strbuffer_append(errorText, "    Line: ");
		strbuffer_append(errorText, int_to_string(error->line));

		strbuffer_append(errorText, "    Column: ");
		strbuffer_append(errorText, int_to_string(error->column));

		strbuffer_append(errorText, "    Position: ");
		strbuffer_append(errorText, int_to_string(error->position));
	}
}

inline void parseJsonPacket(packet_t *jsonPacket) {

	json_t *root;
	json_error_t jsonError;
	char *errorString;
	strbuffer_t *errorBuffer;
	strbuffer_t *logMsg;

	if(!jsonPacket || (jsonPacket->type != PKG_TYPE_INCOME_MESSAGE_STRING)) {
		logMsg = strbuffer_new();
		strbuffer_append(logMsg, "parseJsonPacket : Wrong packet type was received. Got ");
		strbuffer_append(logMsg, int_to_string(jsonPacket->type));
		strbuffer_append(logMsg, " instead of ");
		strbuffer_append(logMsg, int_to_string(PKG_TYPE_INCOME_MESSAGE_STRING));
		logger(LEVEL_ERR, logMsg->value);
		strbuffer_destroy(&logMsg);
		return;
	}

	strbuffer_t *jsonString = jsonPacket->payload.stringData;

	if(!jsonString) {
		char *errorMessage = "parseJsonPacket : Unable to extract json string from packet";
		logger(LEVEL_ERR, errorMessage);

		errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, errorMessage, 0);
		errorBuffer = strbuffer_new();
		strbuffer_append(errorBuffer, errorString);

		packet_lock(jsonPacket);
		{
			jsonPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
			jsonPacket->payload.stringData = errorBuffer;
		}
		packet_unlock(jsonPacket);
		return;
	}



	logMsg = strbuffer_new();
	strbuffer_append(logMsg, "parseJsonPacket    Received json packet: transport=");
	strbuffer_append(logMsg, int_to_string(jsonPacket->transport));
	strbuffer_append(logMsg, "    length=");
	strbuffer_append(logMsg, int_to_string(jsonString->length));
	logger(LEVEL_INFO, logMsg->value);
	strbuffer_destroy(&logMsg);


	root = json_loads(jsonString->value, 0, &jsonError);
	//packet_destroy(jsonPacket);
	if(root != NULL) {
		/*	TODO decode array or object
			array --> batch (optional, but will be implemented only  if there will be enough resources mcu flash)
			if batch proceed requests one by one and put the batch flag
		*/
		logger(LEVEL_DEBUG, "parseJsonPacket String parsing was successful");

		if(isJsonRPCVersion2_0(root)){

			logMsg = strbuffer_new();
			strbuffer_append(logMsg, "parseJsonPacket Packet parsing succeed. Transport=");
			strbuffer_append(logMsg, int_to_string(jsonPacket->transport));
			logger(LEVEL_INFO, logMsg->value);
			strbuffer_destroy(&logMsg);


			json_t* idObj = json_object_get(root, "id");
			json_int_t id = json_integer_value(idObj);

			packet_lock(jsonPacket);
			{
				strbuffer_destroy(&(jsonPacket->payload.stringData));

				jsonPacket->type = PKG_TYPE_INCOME_JSONRPC_REQUEST;
				jsonPacket->payload.jsonDoc = root;
				jsonPacket->id = id;
			}
			packet_unlock(jsonPacket);
			return;

		} else {


			// TODO print_help(transport);
			logMsg = strbuffer_new();
			strbuffer_append(logMsg, ".parseJsonPacket :  ");
			strbuffer_append(logMsg, MSG_MAINTASKS.parseJsonPacket.invalid_jsonrpc_2_0);
			logger(LEVEL_WARN, logMsg->value);
			strbuffer_destroy(&logMsg);



			json_int_t id = 0;
			if(json_object_get(root, "id") != NULL) {
				id = json_integer_value(json_object_get(root, "id"));
			}
			json_decref(root);

			errorString = format_jsonrpc_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request, MSG_MAINTASKS.parseJsonPacket.invalid_jsonrpc_2_0, id);
			errorBuffer = strbuffer_new();
			strbuffer_append(errorBuffer, errorString);

			packet_lock(jsonPacket);
			{
				strbuffer_destroy(&(jsonPacket->payload.stringData));

				jsonPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
				jsonPacket->payload.stringData = errorBuffer;
				jsonPacket->id = id;
			}
			packet_unlock(jsonPacket);


			return;
		}
	} else {
		strbuffer_t *errorText = strbuffer_new();
		format_error_text(errorText, &jsonError);

		logMsg = strbuffer_new();
		strbuffer_append(logMsg, "parseJsonPacket Packet parsing failed: ");
		strbuffer_append(logMsg, errorText->value);
		logger(LEVEL_WARN, logMsg->value);
		strbuffer_destroy(&logMsg);


		errorString = format_jsonrpc_error(JSONRPC_PARSE_ERROR, MSG_JSONRPC_ERRORS.parse_error, errorText->value, 0);
		errorBuffer = strbuffer_new();
		strbuffer_append(errorBuffer, errorString);
		strbuffer_destroy(&errorText);

		packet_lock(jsonPacket);
		{
			strbuffer_destroy(&(jsonPacket->payload.stringData));
			jsonPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
			jsonPacket->payload.stringData = errorBuffer;
		}
		packet_unlock(jsonPacket);
		return;
	}
}

//void tskHandleResponses(void *pvParameters) {
//	json_t *responseJson = NULL;
//	portBASE_TYPE xStatus;
//
//	while(1) {
//		if(uxQueueMessagesWaiting(responseQueue) > 0) {
//			xStatus = xQueueReceive( responseQueue, &responseJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
//			if( (xStatus == pdPASS) && responseJson) {
//				json_t *transportObject = json_object_get(responseJson, "transport");
//				if(!transportObject) {
//					logger(LEVEL_ERR, "Received response object does not contain transport");
//				}
//				json_int_t transport = json_integer_value(transportObject);
//				json_object_del(responseJson, "transport");
//
//				json_t *idObj = json_object_get(responseJson, "id");
//				json_int_t id = json_integer_value(idObj);
//
//				logger_format(LEVEL_INFO, "Received response: id=%d transport=%s", id, transport_type_to_str((transport_type_t) transport));
//
//				logger(LEVEL_DEBUG, "creating json string from json object");
//				char *jsonData = json_dumps(responseJson, JSON_ENCODE_ANY );
//				if(jsonData) {
//					logger(LEVEL_DEBUG, "json string was created. Creating packet to send.");
//
//					packet_t *packet = packet_new((transport_type_t) transport);
//					strbuffer_t *payload = strbuffer_new();
//					strbuffer_append(payload, jsonData);
//					vPortFree(jsonData);
//					packet->jsonDoc = payload;
//
//					logger_format(LEVEL_DEBUG,
//							"Sending new packet: packet_length=%d packet_transport=%s",
//							packet->jsonDoc->length, transport_type_to_str(packet->transport));
//					send_packet_to_client(packet);
//				} else {
//					logger_format(LEVEL_WARN, "Unable to create string from response json: id=%d transport=%s", (int) id, transport_type_to_str(transport));
//				}
//				json_decref(responseJson);
//				continue;
//			}
//		}
//		taskYIELD();
//	}
//}

int check_packet_type(packet_t *packet, packet_type_t pkgType) {
	char *errorString;
	strbuffer_t *errorBuffer;

	if(packet->type != pkgType) {

		strbuffer_t *logMsg = strbuffer_new();
		strbuffer_append(logMsg, "Received packet with wrong type. Got ");
		strbuffer_append(logMsg, int_to_string(packet->type));
		strbuffer_append(logMsg, " instead of ");
		strbuffer_append(logMsg, int_to_string(pkgType));
		logger(LEVEL_ERR, logMsg->value);
		strbuffer_destroy(&logMsg);

		errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, "internal server error", 0);
		errorBuffer = strbuffer_new();
		strbuffer_append(errorBuffer, errorString);

		packet_t * errorPacket = packet_new();

		errorPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
		errorPacket->payload.stringData = errorBuffer;
		errorPacket->transport = packet->transport;

		sendOutputMessage(errorPacket);

		return FALSE;

	}

	return TRUE;
}

void tskHandleRequests(void *pvParameters) {
	packet_t *requestPacket = NULL;

	packet_t *responsePacket = NULL;

	json_t *idObj;
	json_int_t id;


	char *errorString;
	strbuffer_t *errorBuffer;

	strbuffer_t *logMsg;

	portBASE_TYPE xStatus;
	while(1) {
		if(uxQueueMessagesWaiting(requestQueue) > 0) {
			xStatus = xQueueReceive( requestQueue, &requestPacket, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && requestPacket) {

				if(check_packet_type(requestPacket, PKG_TYPE_INCOME_JSONRPC_REQUEST) != TRUE) {
					logger(LEVEL_ERR, "Wrong data packet from requestQueue. Deleting packet" );
					packet_destroy(&requestPacket);
					continue;

				}

				logMsg = strbuffer_new();
				strbuffer_append(logMsg, "Received new request: id=");
				strbuffer_append(logMsg, int_to_string(requestPacket->id));
				strbuffer_append(logMsg, " transport=");
				strbuffer_append(logMsg, int_to_string(requestPacket->transport));
				logger(LEVEL_INFO, logMsg->value);
				strbuffer_destroy(&logMsg);


				handle_request(requestPacket);
				responsePacket = requestPacket;
				if(responsePacket) {

					if(requestPacket->type == PKG_TYPE_OUTCOME_JSONRPC_NOTIFICATION) {
						logger(LEVEL_INFO, "Request was notification only. Destroying response");
						packet_destroy(&requestPacket);
						packet_destroy(&responsePacket);
						continue;
					}

					if(check_packet_type(requestPacket, PKG_TYPE_OUTCOME_JSONRPC_RESPONSE) != TRUE) {
						logger(LEVEL_ERR, "Wrong response packet. Deleting packet" );
						packet_destroy(&requestPacket);
						continue;
					}

					logMsg = strbuffer_new();
					strbuffer_append(logMsg, "Received response: id=");
					strbuffer_append(logMsg, int_to_string(responsePacket->id));
					strbuffer_append(logMsg, " transport=");
					strbuffer_append(logMsg, int_to_string(responsePacket->transport));
					logger(LEVEL_INFO, logMsg->value);
					strbuffer_destroy(&logMsg);


					logger(LEVEL_DEBUG, "creating json string from json object");

					strbuffer_t *stringData = strbuffer_new();

					char *jsonData = json_dumps(responsePacket->payload.jsonDoc, JSON_ENCODE_ANY );
					if(jsonData) {
						logger(LEVEL_DEBUG, "json string was created. Creating packet to send.");
						strbuffer_append(stringData, jsonData);

						logMsg = strbuffer_new();
						strbuffer_append(logMsg, "Sending new packet: packet_length=");
						strbuffer_append(logMsg, int_to_string(stringData->length));
						strbuffer_append(logMsg, " packet_transport=");
						strbuffer_append(logMsg, int_to_string(responsePacket->transport));
						logger(LEVEL_DEBUG, logMsg->value);
						strbuffer_destroy(&logMsg);



					} else {
						logMsg = strbuffer_new();
						strbuffer_append(logMsg, "Unable to create string from response json: id=");
						strbuffer_append(logMsg, int_to_string(responsePacket->id));
						strbuffer_append(logMsg, " transport=");
						strbuffer_append(logMsg, int_to_string(responsePacket->transport));
						logger(LEVEL_WARN, logMsg->value);
						strbuffer_destroy(&logMsg);

						errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, "internal server error", responsePacket->id);
						strbuffer_append(stringData, errorString);
					}

					vPortFree(jsonData);

					packet_lock(responsePacket);
					{
						json_decref(responsePacket->payload.jsonDoc);

						responsePacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
						responsePacket->payload.stringData = stringData;
					}
					packet_unlock(responsePacket);


					sendOutputMessage(responsePacket);

//					idObj = json_object_get(responseJson, "id");
//					id = json_integer_value(idObj);
//
//					json_t *transportObject = json_object_get(responseJson, "transport");
// 					json_int_t transport = json_integer_value(transportObject);
//
//					logger_format(LEVEL_INFO, "Received response. id = %d transport=%s", (int) id, transport_type_to_str((transport_type_t) transport));
//
//					xStatus = xQueueSendToBack( responseQueue, &responseJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//					if( xStatus != pdPASS ){
//						char* idStr = json_dumps(json_object_get(requestPacket, "id"), JSON_ENCODE_ANY);
//						json_int_t transport = json_integer_value(json_object_get(requestPacket, "transport"));
//						json_object_del(requestPacket, "transport");
//						json_decref(requestJson);
//						// send using error reporting function
//						if(id && transport) {
//							report_error_to_sender(
//								(transport_type_t) transport,
//								MSG_JSONRPC_ERRORS.general_error_json,
//								JSONRPC_SERVER_ERROR,				/* <-- code */
//								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//								MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
//								idStr
//							);
//						}
//						if(idStr) vPortFree(idStr);
//						logger_format(LEVEL_INFO, "Unable to add response to queue. Request id = %d", (int) id );
//					} else {
//						logger(LEVEL_DEBUG, "Response was added to response queue");
//					}

				}
//				json_decref(requestPacket);
			}
		}
		taskYIELD();
	}

}

int sendOutputMessage(packet_t *msgPacket) {
	portBASE_TYPE xStatus;
	extern xQueueHandle  msgOutComeQueue;

	int retries;

	xStatus = 0;
	if(msgOutComeQueue != NULL && msgPacket) {
		retries = SYSTEM_MSG_QUEUE_ADD_RETRIES;
		while((xStatus != pdPASS) && (retries-- >= 0)) {
			xStatus = xQueueSendToBack( msgOutComeQueue, &msgPacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
			if(xStatus != pdPASS) { vTaskDelay(SYSTEM_TASK_DELAY); }
		}

		if((xStatus != pdPASS) && (retries < 0)) {
			strbuffer_t *error = strbuffer_new();
			strbuffer_append(error, "Unable to add message to msgOutComeQueue queue after ");
			strbuffer_append(error, int_to_string(SYSTEM_MSG_QUEUE_ADD_RETRIES));
			strbuffer_append(error, " retries");

			logger(LEVEL_ERR, error->value);
			strbuffer_destroy(&error);

			packet_destroy(&msgPacket);
		}
	} else {
		return FALSE;
	}

	return (xStatus == pdPASS);
}

int add_new_rpc_request(packet_t *incomePacket) {
	portBASE_TYPE xStatus;

	int retries;

	xStatus = 0;
	if(requestQueue != NULL && incomePacket) {
		retries = SYSTEM_MSG_QUEUE_ADD_RETRIES;
		while((xStatus != pdPASS) && (retries-- >= 0)) {
			xStatus = xQueueSendToBack( requestQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
			if(xStatus != pdPASS) { vTaskDelay(SYSTEM_TASK_DELAY); }
		}

		if((xStatus != pdPASS) && (retries < 0)) {
			strbuffer_t *error = strbuffer_new();
			strbuffer_append(error, "Unable to add message to requestQueue after ");
			strbuffer_append(error, int_to_string(SYSTEM_MSG_QUEUE_ADD_RETRIES));
			strbuffer_append(error, " retries");

			logger(LEVEL_ERR, error->value);
			strbuffer_destroy(&error);

			packet_destroy(&incomePacket);
		}
	} else {
		return FALSE;
	}

	return (xStatus == pdPASS);
}


void tskParseJson(void *pvParameters) {


	packet_t *incomePacket = NULL;
	json_t 	*requestJson = NULL;
	const char *transportString;

	char *errorString;
	strbuffer_t *errorBuffer;

	strbuffer_t *logMsg;

	portBASE_TYPE xStatus;
	while(1) {
		if(uxQueueMessagesWaiting(msgIncomeQueue) > 0) {
			xStatus = xQueueReceive( msgIncomeQueue, &incomePacket, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
			if( (xStatus == pdPASS) && incomePacket) {

				if(incomePacket->type != PKG_TYPE_INCOME_MESSAGE_STRING) {
					logMsg = strbuffer_new();
					strbuffer_append(logMsg, "Received packet with wrong type. Got ");
					strbuffer_append(logMsg, int_to_string(incomePacket->type));
					strbuffer_append(logMsg, " instead of ");
					strbuffer_append(logMsg, int_to_string(PKG_TYPE_INCOME_MESSAGE_STRING));
					logger(LEVEL_ERR, logMsg->value);
					strbuffer_destroy(&logMsg);


					errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, "internal server error", 0);
					errorBuffer = strbuffer_new();
					strbuffer_append(errorBuffer, errorString);

					packet_t * errorPacket = packet_new();

					errorPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
					errorPacket->payload.stringData = errorBuffer;
					errorPacket->transport = incomePacket->transport;

					sendOutputMessage(errorPacket);

					packet_destroy(&incomePacket);

					continue;
				}

				logMsg = strbuffer_new();
				strbuffer_append(logMsg, "Received packet. len = ");
				strbuffer_append(logMsg, int_to_string(incomePacket->payload.stringData->length));
				strbuffer_append(logMsg, "   transport=");
				strbuffer_append(logMsg, int_to_string(incomePacket->transport));
				logger(LEVEL_INFO, logMsg->value);
				strbuffer_destroy(&logMsg);

				parseJsonPacket(incomePacket);

				strbuffer_t *logMsg;
				switch(incomePacket->type) {
				case PKG_TYPE_OUTGOING_MESSAGE_STRING:
					logger(LEVEL_WARN, "Unable to parse incoming json packet. Sending message to client");
					sendOutputMessage(incomePacket);
					break;

				case PKG_TYPE_INCOME_JSONRPC_REQUEST:
					logMsg = strbuffer_new();
					strbuffer_append(logMsg, "New jsonrpc request. id=");
					strbuffer_append(logMsg, int_to_string(incomePacket->id));
					strbuffer_append(logMsg, "transport=");
					strbuffer_append(logMsg, int_to_string(incomePacket->transport));

					logger(LEVEL_INFO, logMsg->value);
					strbuffer_destroy(&logMsg);
					//logger_format(LEVEL_INFO, "New jsonrpc request. id=%d transport=%s", incomePacket->id, transportString);
					add_new_rpc_request(incomePacket);
					break;

				default:
					logMsg = strbuffer_new();
					strbuffer_append(logMsg, "Wrong packet type was returned fron json parse function. type_returned=");
					strbuffer_append(logMsg, int_to_string(incomePacket->type));
					logger(LEVEL_ERR, logMsg->value);
					strbuffer_destroy(&logMsg);
					packet_destroy(&incomePacket);
					break;
				}
//				if(requestJson) {
//					logger_format(LEVEL_INFO, "Parsing packet was successful.");
//
////					json_incref(requestJson);
//					logger(LEVEL_DEBUG, "Adding json to request queue");
//					xStatus = xQueueSendToBack( requestQueue, &requestJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//					if( xStatus != pdPASS ){
//
//						json_object_del(requestJson, "method");
//						json_object_del(requestJson, "params");
//
//						json_t* errorObj = create_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);
//						json_object_set_new(errorObj, "data", json_string(MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout));
//
//						json_object_set_new(requestJson, "error", errorObj);
//
//						logger_format(LEVEL_WARN, "%s", MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
//
//						// send error back to client using output queue
//						xStatus = xQueueSendToBack( responseQueue, &requestJson, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//						if( xStatus != pdPASS ){
//							char* id = json_dumps(json_object_get(requestJson, "id"), JSON_ENCODE_ANY);
//							transport_type_t transport = json_integer_value(json_object_get(requestJson, "transport"));
//							json_object_del(requestJson, "transport");
//							json_decref(requestJson);
//							// send using error reporting function
//							if(id && transport) {
//								report_error_to_sender(
//									transport,
//									MSG_JSONRPC_ERRORS.general_error_json,
//									JSONRPC_SERVER_ERROR,				/* <-- code */
//									MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//									MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
//									id
//								);
//							}
//							if(id) vPortFree(id);
//							logger_format(LEVEL_WARN, "%s", MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
//						}
//					} else {
//						logger(LEVEL_DEBUG, "Json was added to request queue");
//					}
//				}
//				json_decref(requestJson);
			}
		}
		taskYIELD();
	}
}


//inline
//packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp) {
//	packet_t *incomePacket = NULL;
//	incomePacket = packet_new(TRANSPORT_UART1);
//	if(!incomePacket) {
//		report_error_to_sender(
//			TRANSPORT_UART1,
//			MSG_JSONRPC_ERRORS.general_error_json,
//			JSONRPC_SERVER_ERROR,				/* <-- code */
//			MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//			MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
//			"null" 	/* <-- id */
//		);
//		packet_destroy(&incomePacket);
//		strbuffer_destroy(temp);
//
//		logger_format(LEVEL_WARN, "%s %s", pcTaskGetTaskName(NULL), MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
//
//		return NULL;
//	}
//	incomePacket->jsonDoc = strbuffer_new();
//	strbuffer_append(incomePacket->jsonDoc, (*temp)->value);
//	return incomePacket;
//}

// TODO make this reader abstract
//void tskUART1Reader(void *pvParameters) {
//	signed char *taskName = pcTaskGetTaskName(NULL);
//
//	portBASE_TYPE xStatus;
//	strbuffer_t *temp = NULL;
//	char *terminator;
//
//	packet_t *incomePacket = NULL;
//
//	while (1) {
//
//		if( xSemaphoreTake( xUART1ReadSemaphore, SYSTEM_TASK_DELAY ) == pdTRUE ) {
//			while(UART1_has_bytes()) {
//				if(!temp) {
//					temp = strbuffer_new();
//				}
//
//				if(!temp) {
//					report_error_to_sender(
//						TRANSPORT_UART1,						/* <-- which way to send  */
//						MSG_JSONRPC_ERRORS.general_error_json,	/* <-- format */
//						JSONRPC_SERVER_ERROR,				/* <-- code */
//						MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//						MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
//						"null" 	/* <-- id */
//					);
//					strbuffer_destroy(&temp);
//
//					logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
//					break;
//				}
//
//				char newChar = (char) UART1_read();
//
//				if(strbuffer_append_byte(temp, newChar)) {
//					strbuffer_destroy(&temp);
//					report_error_to_sender(
//						TRANSPORT_UART1,
//						MSG_JSONRPC_ERRORS.general_error_json,
//						JSONRPC_SERVER_ERROR,				/* <-- code */
//						MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//						MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
//						"null" 	/* <-- id */
//					);
//					strbuffer_destroy(&temp);
//
//					logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
//
//					break;
//				}
//
//				terminator = strchr(temp->value, EOT_CHAR);
//				if( terminator != NULL) {
//					/* remove EOT character from the end */
//					strbuffer_pop(temp);
//					temp->value[temp->length] = '\0';
//
//					/* Check if user requested help message */
//					if( (strcmp(temp->value, "--help") == 0) ||
//						(strcmp(temp->value, "-h") == 0) )
//					{
//						strbuffer_destroy(&temp);
//						logger_format(LEVEL_WARN, "%s System help was requested", taskName);
//
//						strbuffer_t *callHelp = strbuffer_new();
//						strbuffer_append(callHelp, "{\"jsonrpc\":\"2.0\",\"method\":\"system.help\",\"id\": null}");
//						incomePacket = createNewIncomePacketFromStr(&callHelp);
//						strbuffer_destroy(&callHelp);
//						if(!incomePacket)
//							continue;
//						xStatus = 0;
//						while(xStatus != pdPASS) {
//							xStatus = xQueueSendToBack( msgIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//						}
//						continue;
//					}
//
//					incomePacket = createNewIncomePacketFromStr(&temp);
//					strbuffer_destroy(&temp);
//
//					if(!incomePacket)
//						break; // TODO error handling when unable to create new packet
//
//					xStatus = xQueueSendToBack( msgIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//					if( xStatus != pdPASS )
//					{
//						packet_destroy(&incomePacket);
//						strbuffer_destroy(&temp);
//						report_error_to_sender(
//							TRANSPORT_UART1,
//							MSG_JSONRPC_ERRORS.general_error_json,
//							JSONRPC_SERVER_ERROR,				/* <-- code */
//							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//							MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
//							"null" 	/* <-- id */
//						);
//
//						logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
//
//
//						continue;
//					}
//
//					logger_format(LEVEL_DEBUG, "%s Received new packet( len= %d )", taskName, temp->length);
//
//					temp = NULL;
//				} else {
//					/* Overflow check */
//					if(temp->length > MAX_INCOME_MSG_SIZE) {
//						strbuffer_destroy(&temp);
//						report_error_to_sender(
//							TRANSPORT_UART1,
//							MSG_JSONRPC_ERRORS.general_error_json,
//							JSONRPC_SERVER_ERROR,				/* <-- code */
//							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//							MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow , /* <-- data */
//							"null" 	/* <-- id */
//						);
//						logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow);
//					}
//				}
//			}
//		}
//		taskYIELD();
//	}
//}

/**
 * This function will be called when new message arrives from UART1 (when UART line idle interrupt happens)
 */
void UART1_MsgAvailable_Callback(void) {
	static signed portBASE_TYPE xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR( xUART1ReadSemaphore, &xHigherPriorityTaskWoken );

	if( xHigherPriorityTaskWoken == pdTRUE) {
		portYIELD();
	}
}


