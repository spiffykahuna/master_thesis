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
} reader_state_t;

inline
packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp, reader_params_t *config);

inline int isDigit(char character);
inline int handleMessage(strbuffer_t *msg, reader_params_t *config);

void tskAbstractReader(void *pvParameters) {



	strbuffer_t *messageBuffer = NULL;

	char newChar;
	int messageLength;

	strbuffer_t *logMsg;

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
//							report_error_to_sender(
//								config->transport_type,						/* <-- which way to send  */
//								MSG_JSONRPC_ERRORS.general_error_json,		/* <-- format */
//								JSONRPC_SERVER_ERROR,						/* <-- code */
//								MSG_JSONRPC_ERRORS.server_error,			/* <-- message */
//								MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
//								"null" 	/* <-- id */
//							);
							strbuffer_destroy(&messageBuffer);

							logger(LEVEL_WARN, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
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
									state =  WAITING_FOR_INPUT;
								}
							}
							break;

						case READING_MESSAGE_SIZE:
							newChar = stream_read_char();
							if(isDigit(newChar)) {
								strbuffer_append_byte(messageBuffer, newChar);
								state = READING_MESSAGE_SIZE;

								if(messageBuffer->length > MAX_INCOME_MSG_SIZE/2) {
									strbuffer_destroy(&messageBuffer);
									state =  WAITING_FOR_INPUT;
								}


							} else if(newChar == ':') {
								messageLength = strtol(messageBuffer->value, NULL, 10);

								if((messageLength > 0) && (messageLength <= MAX_INCOME_MSG_SIZE)) {
									strbuffer_destroy(&messageBuffer);
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

									logMsg = strbuffer_new();
									strbuffer_append(logMsg, MSG_MAINTASKS.tskAbstractReader.incoming_buffer_overflow);
									strbuffer_append(logMsg, " Length=");
									strbuffer_append(logMsg, int_to_string(messageLength));
									logger(LEVEL_WARN, logMsg->value);
									strbuffer_destroy(&logMsg);


								}
							} else {
								strbuffer_destroy(&messageBuffer);

								// TODO make this check on java library too
								logMsg = strbuffer_new();
								strbuffer_append(logMsg, "Reader read wrong character after receiving message size. Expected ':' got '");
								strbuffer_append_byte(logMsg, newChar);
								strbuffer_append(logMsg, "'");
								logger(LEVEL_DEBUG, logMsg->value);
								strbuffer_destroy(&logMsg);


								state =  WAITING_FOR_INPUT;
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
								strbuffer_destroy(&messageBuffer);

								logMsg = strbuffer_new();
								strbuffer_append(logMsg, "Reader is bound of message. Current position = ");
								strbuffer_append(logMsg, int_to_string(messageLength));
								logger(LEVEL_WARN, logMsg->value);
								strbuffer_destroy(&logMsg);

								state =  WAITING_FOR_INPUT;
							}
							break;

						case READING_MESSAGE_TERMINATOR:
							newChar = stream_read_char();
							if(newChar == ',') {
								if(messageBuffer != NULL && messageBuffer->length > 0) {

									logMsg = strbuffer_new();
									strbuffer_append(logMsg, "Received new message: len = ");
									strbuffer_append(logMsg, int_to_string(messageBuffer->length));
									logger(LEVEL_INFO, logMsg->value);
									strbuffer_destroy(&logMsg);

									handleMessage(messageBuffer, config);
								}
							} else {
								logMsg = strbuffer_new();
								strbuffer_append(logMsg, "Unable to get message terminator. Got ");
								strbuffer_append_byte(logMsg, newChar);
								strbuffer_append(logMsg, " instead of ',' ");
								logger(LEVEL_DEBUG, logMsg->value);
								strbuffer_destroy(&logMsg);

							}
							strbuffer_destroy(&messageBuffer);
							state = WAITING_FOR_INPUT;
							break;

						default:
							logMsg = strbuffer_new();
							strbuffer_append(logMsg, "Unreachable reader state: ");
							strbuffer_append(logMsg, int_to_string(state));
							logger(LEVEL_WARN, logMsg->value);
							strbuffer_destroy(&logMsg);

							if(messageBuffer) strbuffer_destroy(&messageBuffer);
							state = WAITING_FOR_INPUT;
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
	packet_t *incomePacket = packet_new();
	if(!incomePacket) {
		char * errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, "internal server error", 0);
		strbuffer_t *errorBuffer = strbuffer_new();
		strbuffer_append(errorBuffer, errorString);

		packet_t * errorPacket = packet_new();

		errorPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
		errorPacket->payload.stringData = errorBuffer;
		errorPacket->transport = config->transport_type;

		sendOutputMessage(errorPacket);

		packet_destroy(&incomePacket);
		strbuffer_destroy(temp);

		logger(LEVEL_WARN, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);

		return NULL;
	}
	incomePacket->type = PKG_TYPE_INCOME_MESSAGE_STRING;
	incomePacket->transport = config->transport_type;

	incomePacket->payload.stringData = strbuffer_new();
	strbuffer_append(incomePacket->payload.stringData, (*temp)->value);
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

	strbuffer_t *logMsg = strbuffer_new();
	strbuffer_append(logMsg, "Adding new packet to queue( len = ");
	strbuffer_append(logMsg, int_to_string((incomePacket->payload.stringData)->length));
	strbuffer_append(logMsg, " )");
	logger(LEVEL_DEBUG, logMsg->value);
	strbuffer_destroy(&logMsg);

	int repeats = 5; // TODO maybe put repeat count into config
	do {
		xStatus = xQueueSendToBack( config->dataInputQueue, &incomePacket, config->dataInputQueueTimeout );
	} while( xStatus != pdPASS && (repeats-- > 0) );

	if(xStatus != pdPASS) {
		packet_destroy(&incomePacket);
		strbuffer_destroy(&msg);


		char * errorString = format_jsonrpc_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error, "internal server error", 0);
		strbuffer_t *errorBuffer = strbuffer_new();
		strbuffer_append(errorBuffer, errorString);

		packet_t * errorPacket = packet_new();

		errorPacket->type = PKG_TYPE_OUTGOING_MESSAGE_STRING;
		errorPacket->payload.stringData = errorBuffer;
		errorPacket->transport = config->transport_type;

		sendOutputMessage(errorPacket);

		logger(LEVEL_WARN, MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);
	}


	return TRUE;
}
