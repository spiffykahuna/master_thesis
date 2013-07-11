/*
 * messages.c
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */
#include "maintasks.h"

const msg_maintasks MSG_MAINTASKS = {
	.tskAbstractReader.unable_to_alloc_n_bytes   		= "Unable to allocate %d bytes for received string.",
	.tskAbstractReader.device_is_busy_timeout 	  		= "Unable to handle request. Device is busy. Timeout reached.\n",
	.tskAbstractReader.incoming_buffer_overflow  		= "Incoming message is too big. Max message length should be less than " MAX_INCOME_MSG_SIZE_MSG " .\n",
	.tskAbstractReader.unable_to_alloc_new_json_packet	= "{\"jsonrpc\": \"2.0\", \"error\": \{\"code\": -32500, \"message\": \"Application error\", \"data\" : \"Error: Unable to allocate memory for new json packet.\" }, \"id\": null}\n",

	.parseJsonPacket.invalid_json_document 			= "Unable to parse incoming json document\n",
	.parseJsonPacket.invalid_jsonrpc_2_0   			= "Provided json document is not compatible with json-rpc version 2.0\n"
};

const msg_methods MSG_METHODS = {
	.invalid_method_parameters 						= "Invalid method parameters specified: ",
};

const msg_error MSG_ERROR = {
	.PROXY_MEMORY_ALLOC_FAILED 						= "PROXY_MEMORY_ALLOC_FAILED",
	.PROXY_DEVICE_IS_BUSY 							= "PROXY_DEVICE_IS_BUSY",
	.PROXY_INCOMING_BUFFER_OVERFLOW					= "PROXY_INCOMING_BUFFER_OVERFLOW",
	.PROXY_INVALID_INCOME_PACKET					= "PROXY_INVALID_INCOME_PACKET",
	.PROXY_INVALID_JSONRPC_2_0						= "PROXY_INVALID_JSONRPC_2_0",
	.UNKNOWN 										= "UNKNOWN ERROR"
};

const msg_jsonrpc_errors MSG_JSONRPC_ERRORS = {
	.general_error_json								= "{\"jsonrpc\": \"2.0\", \"error\": \{\"code\": %ld, \"message\": \"%s\", \"data\" : \"%s\" }, \"id\": %s }\n",
	.parse_error									= "Parse error",
	.invalid_request								= "Invalid Request",
	.method_not_found								= "Method not found",
	.invalid_params									= "Invalid params",
	.internal_error									= "Internal error",
	.server_error									= "Server error",
	.application_error								= "Application error",
	.system_error									= "System error",
	.transport_error								= "Transport error"
};




//typedef struct _msg_maintasks {
//	struct {
//		char * unable_to_alloc_n_bytes = "Error: Unable to allocate %s bytes for received string";
//		char * device_is_busy_timeout = "Error: Unable to handle request. Device is busy. Timeout reached";
//	} tskUSBHandling;
//} msg_maintasks;
//
//typedef struct _msg_error {
//	char * const PROXY_MEMORY_ALLOC_FAILED = "PROXY_MEMORY_ALLOC_FAILED";
//	char * const PROXY_DEVICE_IS_BUSY = "PROXY_DEVICE_IS_BUSY";
//	char * const UNKNOWN = "UNKNOWN ERROR";
//} msg_error;
