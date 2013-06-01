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

void tskLedBlinkTask(void *pvParameters)
{
	int i;
	//char buffer[100];
	Led_TypeDef led = (Led_TypeDef) pvParameters;
	while (1)
	  {
			STM_EVAL_LEDToggle(led);
			for(i=0;i<(0x100000 / (led + 1));i++);
//			printf("READY\n");
//			fputs("TEST", stdout);
//			fputs("TEST", stderr);
//			fprintf(stderr, "%d", 1);
//			scanf( "%s", buffer);
//			fputs("TEST", (FILE*) pvParameters);
//			char *buf = (char *) pvPortMalloc(3000* sizeof(char));
//			free(buf);
			taskYIELD();
	  }


}

/*
void sendDataBack(uint8_t msgBuffer[MSG_BUFFER_SIZE], uint8_t msgLen) {
	if (bDeviceState == CONFIGURED)
	{
		xSemaphoreTake( xLogMutex, portMAX_DELAY );
		{
			if (bDeviceState == CONFIGURED)
			{
				if (packet_sent == 1)
				{
					CDC_Send_DATA (msgBuffer, msgLen);
				}
			}

		}
		xSemaphoreGive( xLogMutex );
	}
}


inline
void clearCharBuffer(char * buffer, size_t arraySize) {
	int i;
	for (i = 0; i < arraySize; i++) {
		buffer[i] = 0;
	}
}
*/
//bool json_token_streq(char *js, jsmntok_t *t, char *s)
//{
//    return (strncmp(js + t->start, s, t->end - t->start) == 0
//            && strlen(s) == (size_t) (t->end - t->start));
//}
//
//char * json_token_tostr(char *js, jsmntok_t *t)
//{
//	static char string[MSG_BUFFER_SIZE];
//	int i,j;
//
//	clearCharBuffer(string, MSG_BUFFER_SIZE);
//	for(i = 0, j = t->start; j < t->end; i++, j++) {
//		string[i] = js[j];
//	}
//	string[++i] = '\0';
//    return string;
//}
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
			logger(LEVEL_INFO, "parseJsonPacket\tPacket parsing succeed\n");
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
			logger(LEVEL_WARN, "parseJsonPacket\t");
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
	// return request_packet

//			json_t *resp = json_object();
//			json_t *result_json_arr = json_array();
//
//			json_t *integ = json_integer(42);



//			json_object_set_new( resp, "command", json_string( "link_status" ) );
//			json_object_set_new( resp, "result", result_json_arr );
//
//			json_object_set_new( resp, "ideal", json_string_value(integ));
//
//			memset(buffer, NULL, MSG_BUFFER_SIZE);
//			snprintf(buffer, MSG_BUFFER_SIZE, "\nhipa: %d\n", xPortGetFreeHeapSize());
//			log_d(buffer);
//
//
//			char *responce = json_dumps(resp, 0);
//			memset(buffer, NULL, MSG_BUFFER_SIZE);
//						snprintf(buffer, MSG_BUFFER_SIZE, "responce: %s\n",responce);
//						log_d(buffer);
//			log_d(responce);
//			vPortFree(responce);
//			json_decref(resp);
//
//
//
//			memset(buffer, NULL, MSG_BUFFER_SIZE);
//			snprintf(buffer, MSG_BUFFER_SIZE, "\nhipa: %d\n", xPortGetFreeHeapSize());
//			log_d(buffer);
//	char buffer[MSG_BUFFER_SIZE];
//
//
//
//	JSON_Value *root_value;
//	JSON_Array *params;
//	JSON_Object *root_object;
//	size_t i;
//
//
//
//	for(i = 0; i < MSG_BUFFER_SIZE; i++) {
//		buffer[i] = msgBuffer[i];
//	}
//	i = 0;
//
//	root_value = json_parse_string(buffer);//json_parse_file(output_filename);
//	if (json_value_get_type(root_value) != JSONObject) {
//		log_d("ERROR root is not object");
//		return;
//	}
//
//	root_object = json_value_get_object(root_value);
//
//	memset(buffer, NULL, MSG_BUFFER_SIZE);
//
//	snprintf(buffer, MSG_BUFFER_SIZE,"Method: %s\n", json_object_dotget_string(root_object, "method") );
//	log_d(buffer);
////	/* getting array from root value and printing commit info */
////	commits = json_value_get_array(root_value);
////	printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
////	for (i = 0; i < json_array_get_count(commits); i++) {
////		commit = json_array_get_object(commits, i);
////		printf("%.10s %.10s %s\n",
////			   json_object_dotget_string(commit, "commit.author.date"),
////			   json_object_get_string(commit, "sha"),
////			   json_object_dotget_string(commit, "commit.author.name"));
////	}
//
//	/* cleanup code */
//	json_value_free(root_value);

//	char buffer[MSG_BUFFER_SIZE];
//	size_t printSize;
//	size_t i;
//
//
//	memset(buffer, NULL, MSG_BUFFER_SIZE);
//
//	for(i = 0; i < MSG_BUFFER_SIZE; i++) {
//		buffer[i] = msgBuffer[i];
//	}
//
//
//
//	int resultCode;
//	size_t tokenIndex = 0, tokensLeft = 1;
//	jsmn_parser parser;
//	jsmntok_t tokens[100];
//
//	typedef enum { START, KEY, PRINT, SKIP, STOP } parse_state;
//	parse_state state = START;
//
//	size_t object_tokens = 0;
//
//	char *KEYS[] = { "method", "params", "ideal" };
//
//	jsmn_init(&parser);
//	resultCode = jsmn_parse(&parser, buffer, tokens, 100);
//
//	if(resultCode == JSMN_SUCCESS) {
//
//
//		for ( tokenIndex = 0, tokensLeft = 1; tokensLeft > 0; tokenIndex++, tokensLeft--)
//		{
//			jsmntok_t *t = &tokens[tokenIndex];
//
//			char *tokenStr = json_token_tostr(buffer, t);
//
//			if(!(t->start != -1 && t->end != -1)) {
//				log_d("Should never reach uninitialized tokens");
//			}
//
//
//			if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT)
//				tokensLeft += t->size;
//
//			switch (state)
//			{
//				case START:
//					if (!(t->type == JSMN_ARRAY || t->type == JSMN_OBJECT)) {
//						log_d("Invalid response: root element must be an object.");
//						return;
//					}
//
//
//					state = KEY;
//					object_tokens = t->size;
//
//					if (object_tokens == 0)
//						state = STOP;
//
//					if (object_tokens % 2 != 0) {
//						log_d("Invalid response: object must have even number of children.");
//						return;
//					}
//
//
//					break;
//
//				case KEY:
//					object_tokens--;
//
//					if (t->type != JSMN_STRING) {
//						log_d("Invalid response: object keys must be strings.");
//						return;
//					}
//
//
//
//					state = SKIP;
//
//					size_t keysCount = sizeof(KEYS)/sizeof(char *);
//
//					for (i = 0; i < keysCount; i++)
//					{
//						if (json_token_streq(buffer, t, KEYS[i]))
//						{
//							clearCharBuffer(msgBuffer, MSG_BUFFER_SIZE);
//							printSize = sprintf(msgBuffer, "%s: ", KEYS[i]);
//							state = PRINT;
//							break;
//						}
//					}
//
//					break;
//
//				case SKIP:
//					if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE) {
//						//log_d("Invalid response: object values must be strings or primitives.");
//						//return;
//					}
//
//
//					object_tokens--;
//					state = KEY;
//
//					if (object_tokens == 0)
//						state = STOP;
//
//					break;
//
//				case PRINT:
//					if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE){
//						//log_d("Invalid response: object values must be strings or primitives.");
//						//return;
//					}
//					if(t->type == JSMN_ARRAY) {
//						state = SKIP;
//						continue;
//					}
//
//					//if(t->type == JSMN_STRING) {
//						char *str = json_token_tostr(buffer, t);
//						printSize = sprintf(msgBuffer + printSize, "%s\n\r", str);
//						log_d(msgBuffer);
//					//}
//
////					if(t->type == JSMN_PRIMITIVE) {
////						log_d(
////					}
//
//
//					object_tokens--;
//					state = KEY;
//
//					if (object_tokens == 0)
//						state = STOP;
//
//					break;
//
//				case STOP:
//					// Just consume the tokens
//					break;
//
//				default:
//					log_d("Invalid state: ");
//					//log_d(state);
//					log_d("\n\r");
//			}
//		}
//
//
//
//
//	} else {
//
//		switch(resultCode) {
//
//		case JSMN_ERROR_INVAL:
//			log_d("failed to parse json: JSMN_ERROR_INVAL bad token, JSON string is corrupted\n\r");
//			break;
//
//		case JSMN_ERROR_NOMEM:
//			log_d("failed to parse json: JSMN_ERROR_NOMEM not enough tokens, JSON string is too large\n\r");
//			break;
//
//		case JSMN_ERROR_PART:
//			log_d("failed to parse json: JSMN_ERROR_PART JSON string is too short, expecting more JSON data\n\r");
//			break;
//
//		default:
//			log_d("failed to parse json: Unknown error\n\r");
//			break;
//		}
//
//
//	}




//		char buffer[MSG_BUFFER_SIZE];
//		int resultCode;
//		jsmn_parser p;
//		static jsmntok_t tokens[TOKENS_COUNT];
//
//		json_t* root;
////		json_t* key;
//		json_t* value;
//
//		size_t tokenIndex = 0;
//
//////		size_t heapSize = xPortGetFreeHeapSize();
////
////		snprintf(buffer, MSG_BUFFER_SIZE, "\nhipa: %d\n", heapSize);
////		log_d(buffer);
//
//		jsmn_init(&p);
//		resultCode = jsmn_parse(&p, msgBuffer, tokens, TOKENS_COUNT);
//
//		if(resultCode == JSMN_SUCCESS) {
//			root = initJsonObjects( msgBuffer, tokens, TOKENS_COUNT, &tokenIndex );
//			if(!root) {
//				log_d("PIZDEC");
//			}
//			value = json_object_get(root, "x^2 array");
//
////			char str[1024];
////			memset(str,0, 1024);
//			int charsCopied = 0;
//
//			char* result = json_to_string(root, buffer, &charsCopied);
//
//
//		//	snprintf(buffer, MSG_BUFFER_SIZE, "Value of x^2 array(length=%d): %s\n",  charsCopied, result);
//			log_d(result);
//
//			json_object_free(root);
//
//
//		} else {
//			switch(resultCode) {
//				case JSMN_ERROR_INVAL:
//					log_d("bad token, JSON string is corrupted\n");
//					break;
//
//				case JSMN_ERROR_NOMEM:
//					log_d("not enough tokens, JSON string is too large\n");
//					break;
//
//				case JSMN_ERROR_PART:
//					log_d("JSON string is too short, expecting more JSON data\n");
//					break;
//
//				default:
//					log_d("Unknown parsing error\n");
//					break;
//
//			}
//		}
}

void tskHandleResponses(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);
	json_t *responceJson = NULL;
	portBASE_TYPE xStatus;

	while(1) {
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
				jsonp_free(jsonData);

				packet->jsonDoc = payload;
				send_packet_to_client(packet);
			}
			json_decref(responceJson);
		}
		if(uxQueueMessagesWaiting(responseQueue) > 0) continue;
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
		xStatus = xQueueReceive( requestQueue, &requestJson, (portTickType) QUEUE_RECEIVE_WAIT_TIMEOUT );
		if( (xStatus == pdPASS) && requestJson) {

			idObj = json_object_get(requestJson, "id");
			id = json_integer_value(idObj);

			methodObj = json_object_get(requestJson, "method");
			methodName = json_string_value(methodObj);

			snprintf(error_space, ERROR_BUFFER_SIZE, "%s :  Received request. id = %d    method = %s\n", taskName, (int) id, methodName );
			logger(LEVEL_INFO, error_space);

			responseJson = handle_request(&requestJson);
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
		taskYIELD();
	}

}


void tskParseJson(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	packet_t *incomePacket = NULL;
	json_t 	*requestJson = NULL;

	portBASE_TYPE xStatus;
	while(1) {
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
		taskYIELD();
	}
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

					terminator = strchr(temp->value, EOT);
					if( terminator != NULL) {
						/* remove EOT character from the end */
						strbuffer_pop(temp);
						temp->value[temp->length] = '\0';

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
							strbuffer_destroy(&temp);

							snprintf(tempSize, BUFF_SIZE, "%s ", taskName);
							logger(LEVEL_WARN, tempSize);
							logger(LEVEL_WARN, MSG_MAINTASKS.tskUSBReader.unable_to_alloc_new_json_packet);

							Receive_length = 0;
							continue;
						}

						incomePacket->jsonDoc = temp;

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


		memset(tempSize, 0, BUFF_SIZE);
		siprintf(tempSize, "%s FREE: %d\n\r", taskName, xPortGetFreeHeapSize());
		log_func(tempSize);
//
//		memset(tempSize, 0, BUFF_SIZE);
//		siprintf(tempSize, "%s STACK: %d\n\r", taskName, (int) uxTaskGetStackHighWaterMark( NULL ));
//		logger(LEVEL_DEBUG, tempSize);

//		snprintf(tempSize, BUFF_SIZE, "FLOAT %f\n", -555.666);
//		log_d(tempSize);

		taskYIELD();
	}
}



