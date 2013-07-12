#include "abstract_reader.h"
#include "stdlib.h"

extern const msg_maintasks MSG_MAINTASKS;
extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;

extern xQueueHandle  msgIncomeQueue;

typedef enum _reader_state_t {
	WAITING_FOR_INPUT,
	READING_MESSAGE_SIZE,
	READING_MESSAGE_VALUE,
	READING_MESSAGE_TERMINATOR,
	HANDLING_NEW_MESSAGE,
} reader_state_t;

inline
packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp, reader_params_t *config);

inline int isDigit(char character);
inline int handleMessage(strbuffer_t *msg);

void tskAbstractReader(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	portBASE_TYPE xStatus;
	strbuffer_t *messageBuffer = NULL;
	char *terminator;

	packet_t *incomePacket = NULL;
	char newChar;
	int messageLength;

	reader_state_t state = WAITING_FOR_INPUT;

	if(pvParameters) {
		reader_params_t *config = (reader_params_t*) pvParameters;
		stream_read_char_function_t stream_read_char = config->read_char_func;
		stream_has_byte_function_t	stream_has_byte = config->stream_has_byte;

		while (1) {

			if( xSemaphoreTake( config->dataReadSemaphore, SYSTEM_TASK_DELAY) == pdTRUE ) {
				while(stream_has_byte()) {
					if(!messageBuffer) {
						messageBuffer = strbuffer_new();

						if(!messageBuffer) {
							report_error_to_sender(
								config->transport_type,						/* <-- which way to send  */
								MSG_JSONRPC_ERRORS.general_error_json,		/* <-- format */
								JSONRPC_SERVER_ERROR,						/* <-- code */
								MSG_JSONRPC_ERRORS.server_error,			/* <-- message */
								MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
								"null" 	/* <-- id */
							);
							strbuffer_destroy(&messageBuffer);

							logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
							break;
						}
					}



					 // State machine for receiving netstrings
					switch(state) {
						case WAITING_FOR_INPUT:
							newChar = stream_read_char();
							if(isDigit(newChar)) {
								strbuffer_append_byte(messageBuffer, newChar);
								state = READING_MESSAGE_SIZE;
								if(messageBuffer->length > MAX_INCOME_MSG_SIZE/2) {
									strbuffer_destroy(&messageBuffer);
								}
							}
							break;

						case READING_MESSAGE_SIZE:
							newChar = stream_read_char();
							if(isDigit(newChar)) {
								strbuffer_append_byte(messageBuffer, newChar);
								state = READING_MESSAGE_SIZE;
							} else if(newChar == ':') {
								messageLength = strtol(messageBuffer->value, NULL, 10);

								if((messageLength > 0) && (messageLength <= MAX_INCOME_MSG_SIZE)) {
									strbuffer_clear(messageBuffer);
									state = READING_MESSAGE_VALUE;
								} else if(messageLength > MAX_INCOME_MSG_SIZE) {
									strbuffer_destroy(&messageBuffer);
									report_error_to_sender(
										config->transport_type,
										MSG_JSONRPC_ERRORS.general_error_json,
										JSONRPC_SERVER_ERROR,				/* <-- code */
										MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
										MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow , /* <-- data */
										"null" 	/* <-- id */
									);
									logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow);
								}
							}
							break;

						case READING_MESSAGE_VALUE:
							if(messageLength > 0) {
								newChar = stream_read_char();
								strbuffer_append_byte(messageBuffer, newChar);
								--messageLength;
							} else if(messageLength == 0) {
								state = READING_MESSAGE_TERMINATOR;
							} else {
								strbuffer_close(messageBuffer);
								logger_format(LEVEL_WARN, "Reader is bound of message. Current position = %d", messageLength);
								state =  WAITING_FOR_INPUT;
							}
							break;

						case READING_MESSAGE_TERMINATOR:
							newChar = stream_read_char();
							if(newChar == ',') {
								state = HANDLING_NEW_MESSAGE;
							} else {
								logger_format(LEVEL_WARN, "Unable to get message terminator. Got %c instead of ',' ", newChar);
								state = WAITING_FOR_INPUT;
							}
							break;

						case HANDLING_NEW_MESSAGE:
							if(messageBuffer != NULL && messageBuffer->length > 0) {
								logger_format(LEVEL_INFO, "Received new message: len = %d", messageBuffer->length);

								handleMessage(messageBuffer, config);

								strbuffer_destroy(&messageBuffer);
							}
							state = WAITING_FOR_INPUT;

							break;

						default:
							logger_format(LEVEL_WARN, "Unreachable reader state: %d", state);
							continue;
					}




















//					newChar = stream_read_char();
//
//					if(strbuffer_append_byte(messageBuffer, newChar)) {
//						strbuffer_destroy(&messageBuffer);
//						report_error_to_sender(
//							config->transport_type,
//							MSG_JSONRPC_ERRORS.general_error_json,
//							JSONRPC_SERVER_ERROR,				/* <-- code */
//							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//							MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
//							"null" 	/* <-- id */
//						);
//						strbuffer_destroy(&messageBuffer);
//
//						logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
//
//						break;
//					}
//
//					terminator = strchr(messageBuffer->value, EOT_CHAR);	//TODO EOT_CHAR -> config
//					if( terminator != NULL) {
//						/* remove EOT character from the end */
//						strbuffer_pop(messageBuffer);
//						messageBuffer->value[messageBuffer->length] = '\0';
//
//						/* Check if user requested help message */
//						if( (strcmp(messageBuffer->value, "--help") == 0) ||
//							(strcmp(messageBuffer->value, "-h") == 0) )
//						{
//							strbuffer_destroy(&messageBuffer);
//							logger_format(LEVEL_WARN, "%s System help was requested", taskName);
//
//							strbuffer_t *callHelp = strbuffer_new();
//							strbuffer_append(callHelp, "{\"jsonrpc\":\"2.0\",\"method\":\"system.help\",\"id\": null}");
//							incomePacket = createNewIncomePacketFromStr(&callHelp, config);
//							strbuffer_destroy(&callHelp);
//							if(!incomePacket)
//								continue;
//							xStatus = 0;
//							while(xStatus != pdPASS) {
//								xStatus = xQueueSendToBack( config->dataInputQueue, &incomePacket, config->dataInputQueueTimeout);
//							}
//							continue;
//						}
//
//						incomePacket = createNewIncomePacketFromStr(&messageBuffer, config);
//						strbuffer_destroy(&messageBuffer);
//
//						if(!incomePacket)
//							break; // TODO error handling when unable to create new packet
//
//						xStatus = xQueueSendToBack( msgIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
//						if( xStatus != pdPASS )
//						{
//							packet_destroy(&incomePacket);
//							strbuffer_destroy(&messageBuffer);
//							report_error_to_sender(
//								config->transport_type,
//								MSG_JSONRPC_ERRORS.general_error_json,
//								JSONRPC_SERVER_ERROR,				/* <-- code */
//								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//								MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
//								"null" 	/* <-- id */
//							);
//
//							logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
//
//
//							continue;
//						}
//
//						logger_format(LEVEL_DEBUG, "%s Received new packet( len= %d )", taskName, messageBuffer->length);
//
//						messageBuffer = NULL;
//					} else {
//						/* Overflow check */
//						if(messageBuffer->length > MAX_INCOME_MSG_SIZE) {
//							strbuffer_destroy(&temp);
//							report_error_to_sender(
//								config->transport_type,
//								MSG_JSONRPC_ERRORS.general_error_json,
//								JSONRPC_SERVER_ERROR,				/* <-- code */
//								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
//								MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow , /* <-- data */
//								"null" 	/* <-- id */
//							);
//							logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow);
//						}
//					}
				}
			}
			taskYIELD();
		}
	}
}


inline
packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp, reader_params_t *config) {
	packet_t *incomePacket = NULL;
	incomePacket = packet_new(TRANSPORT_UART1);
	if(!incomePacket) {
		report_error_to_sender(
			config->transport_type,
			MSG_JSONRPC_ERRORS.general_error_json,
			JSONRPC_SERVER_ERROR,				/* <-- code */
			MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
			MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
			"null" 	/* <-- id */
		);
		packet_destroy(&incomePacket);
		strbuffer_destroy(temp);

		logger_format(LEVEL_WARN, "%s %s", pcTaskGetTaskName(NULL), MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);

		return NULL;
	}
	incomePacket->jsonDoc = strbuffer_new();
	strbuffer_append(incomePacket->jsonDoc, (*temp)->value);
	return incomePacket;
}

inline int isDigit(char character) {
	return (character > 47 && character < 58) ? TRUE : FALSE;
}

inline int handleMessage(strbuffer_t *msg, reader_params_t *config) {
	portBASE_TYPE xStatus;
	packet_t *incomePacket = createNewIncomePacketFromStr(&msg, config);

	if(!incomePacket)
		return FALSE; // TODO error handling when unable to create new packet

	strbuffer_destroy(&msg);

	int repeats = 5; // TODO maybe put repeat count into config
	do {
		xStatus = xQueueSendToBack( config->dataInputQueue, &incomePacket, config->dataInputQueueTimeout );
	} while( xStatus != pdPASS || (repeats-- > 0) );

	if(xStatus != pdPASS) {
		packet_destroy(&incomePacket);
		strbuffer_destroy(&msg);
		report_error_to_sender(
			config->transport_type,
			MSG_JSONRPC_ERRORS.general_error_json,
			JSONRPC_SERVER_ERROR,				/* <-- code */
			MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
			MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
			"null" 	/* <-- id */
		);

		logger_format(LEVEL_WARN, "%s %s", pcTaskGetTaskName(NULL), MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
	}

	logger_format(LEVEL_DEBUG, "%s Received new packet( len = %d )", pcTaskGetTaskName(NULL), msg->length);

}
