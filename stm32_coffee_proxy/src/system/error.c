/*
 * error.c
 *
 *  Created on: Jun 20, 2013
 *      Author: Kasutaja
 */

#include "error.h"

char error_space[ERROR_BUFFER_SIZE];
extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;

inline
json_t * create_error(int errorCode, char* errorMsg) {
	json_t* errorObj = json_object();
	json_object_set_new(errorObj, "code", json_integer(errorCode));
	json_object_set_new(errorObj, "message", json_string(errorMsg));

	return errorObj;
}

inline
json_t * create_response_error(int errorCode, char* errorMsg) {
	json_t *responseJson = NULL;
	responseJson = json_object();
	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));
	json_object_set_new(responseJson, "id", json_null());

	json_t* errorObj = create_error(errorCode, errorMsg);
	json_object_set_new(responseJson, "error", errorObj);
	return responseJson;
}

char * format_jsonrpc_error(proxy_error_t errorNum, char* errorMsg, char* errorData, json_int_t id) {

	snprintf(error_space, ERROR_BUFFER_SIZE,
			MSG_JSONRPC_ERRORS.general_error_json,
			errorNum,
			errorMsg,
			errorData,
			id
	);
	return error_space;
}

inline json_t* server_error(const json_t *requestJson) {
	json_t *responseJson = NULL;

	json_t* errorObj = create_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);

	responseJson = jsonrpc_response(requestJson, errorObj, TRUE);

	return responseJson;
}





