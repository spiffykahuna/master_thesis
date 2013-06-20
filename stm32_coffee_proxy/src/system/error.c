/*
 * error.c
 *
 *  Created on: Jun 20, 2013
 *      Author: Kasutaja
 */

#include "error.h"


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
