/*
 * messages.h
 *
 *  Created on: May 17, 2013
 *      Author: Kasutaja
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_


typedef struct _msg_maintasks {
	struct {
		char * const unable_to_alloc_n_bytes;
		char * const device_is_busy_timeout;
		char * const incoming_buffer_overflow;
		char * const unable_to_alloc_new_json_packet;
	} tskUSBReader;
	struct {
		char * const invalid_json_document;
		char * const invalid_jsonrpc_2_0;
	} parseJsonPacket;
} msg_maintasks;

typedef struct _msg_methods {
	struct {
		char * const unable_to_alloc_n_bytes;
		char * const device_is_busy_timeout;
		char * const incoming_buffer_overflow;
		char * const unable_to_alloc_new_json_packet;
	} subtract;
	char * const invalid_method_parameters;
} msg_methods;

typedef struct _msg_error {
	char * const PROXY_MEMORY_ALLOC_FAILED;
	char * const PROXY_DEVICE_IS_BUSY;
	char * const PROXY_INCOMING_BUFFER_OVERFLOW;
	char * const PROXY_INVALID_INCOME_PACKET;
	char * const PROXY_INVALID_JSONRPC_2_0;
	char * const PROXY_INVALID_RPC_METHOD_PARAMETERS;
	char * const UNKNOWN;
} msg_error;

typedef struct _msg_jsonrpc_errors {
	char * const general_error_json;
	char * const parse_error; 					/* code -32700 */
	char * const invalid_request;  				/* code -32600 */
	char * const method_not_found; 				/* code -32601 */
	char * const invalid_params;				/* code -32602 */
	char * const internal_error;				/* code -32603 */
	char * const server_error;
	char * const application_error;				/* code -32500 */
	char * const system_error;					/* code -32400 */
	char * const transport_error;				/* code -32300 */
} msg_jsonrpc_errors;

#endif /* MESSAGES_H_ */
