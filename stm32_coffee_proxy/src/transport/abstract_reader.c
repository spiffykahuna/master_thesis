#include "abstract_reader.h"

inline
packet_t * createNewIncomePacketFromStr(strbuffer_t ** temp, reader_params_t *config);

void tskAbstractReader(void *pvParameters) {
	signed char *taskName = pcTaskGetTaskName(NULL);

	portBASE_TYPE xStatus;
	strbuffer_t *temp = NULL;
	char *terminator;

	packet_t *incomePacket = NULL;

	if(pvParameters) {
		reader_params_t *config = (reader_params_t*) pvParameters;
		stream_read_char_function_t stream_read_char = config->read_char_func;
		stream_has_byte_function_t	stream_has_byte = config->stream_has_byte;

		while (1) {

			if( xSemaphoreTake( config->dataReadSemaphore, SYSTEM_TASK_DELAY ) == pdTRUE ) {
				while(stream_has_byte()) {
					if(!temp) {
						temp = strbuffer_new();
					}

					if(!temp) {
						report_error_to_sender(
							config->transport_type,						/* <-- which way to send  */
							MSG_JSONRPC_ERRORS.general_error_json,	/* <-- format */
							JSONRPC_SERVER_ERROR,				/* <-- code */
							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
							MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
							"null" 	/* <-- id */
						);
						strbuffer_destroy(&temp);

						logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);
						break;
					}

					char newChar = stream_read_char();

					if(strbuffer_append_byte(temp, newChar)) {
						strbuffer_destroy(&temp);
						report_error_to_sender(
							config->transport_type,
							MSG_JSONRPC_ERRORS.general_error_json,
							JSONRPC_SERVER_ERROR,				/* <-- code */
							MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
							MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet , /* <-- data */
							"null" 	/* <-- id */
						);
						strbuffer_destroy(&temp);

						logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.unable_to_alloc_new_json_packet);

						break;
					}

					terminator = strchr(temp->value, EOT_CHAR);	//TODO EOT_CHAR -> config
					if( terminator != NULL) {
						/* remove EOT character from the end */
						strbuffer_pop(temp);
						temp->value[temp->length] = '\0';

						/* Check if user requested help message */
						if( (strcmp(temp->value, "--help") == 0) ||
							(strcmp(temp->value, "-h") == 0) )
						{
							strbuffer_destroy(&temp);
							logger_format(LEVEL_WARN, "%s System help was requested", taskName);

							strbuffer_t *callHelp = strbuffer_new();
							strbuffer_append(callHelp, "{\"jsonrpc\":\"2.0\",\"method\":\"system.help\",\"id\": null}");
							incomePacket = createNewIncomePacketFromStr(&callHelp);
							strbuffer_destroy(&callHelp);
							if(!incomePacket)
								continue;
							xStatus = 0;
							while(xStatus != pdPASS) {
								xStatus = xQueueSendToBack( config->dataInputQueue, &incomePacket, config->dataInputQueueTimeout);
							}
							continue;
						}

						incomePacket = createNewIncomePacketFromStr(&temp, config);
						strbuffer_destroy(&temp);

						if(!incomePacket)
							break; // TODO error handling when unable to create new packet

						xStatus = xQueueSendToBack( msgIncomeQueue, &incomePacket, (portTickType) QUEUE_SEND_WAIT_TIMEOUT );
						if( xStatus != pdPASS )
						{
							packet_destroy(&incomePacket);
							strbuffer_destroy(&temp);
							report_error_to_sender(
								config->transport_type,
								MSG_JSONRPC_ERRORS.general_error_json,
								JSONRPC_SERVER_ERROR,				/* <-- code */
								MSG_JSONRPC_ERRORS.server_error,	/* <-- message */
								MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout , /* <-- data */
								"null" 	/* <-- id */
							);

							logger_format(LEVEL_WARN, "%s %s", taskName, MSG_MAINTASKS.tskAbstractReader.device_is_busy_timeout);


							continue;
						}

						logger_format(LEVEL_DEBUG, "%s Received new packet( len= %d )", taskName, temp->length);

						temp = NULL;
					} else {
						/* Overflow check */
						if(temp->length > MAX_INCOME_MSG_SIZE) {
							strbuffer_destroy(&temp);
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
