/*
 * methods.c
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */

#include "methods.h"

#define METHOD_IS(actual, expected)		(strcmp((actual), (expected)) == 0)

//extern msg_methods MSG_METHODS;
extern msg_jsonrpc_errors MSG_JSONRPC_ERRORS;

inline
void delete_rpc_method(json_t *requestJson) {
	json_object_del(requestJson, "method");
}

inline
void add_jsonrpc2_0_version(json_t *requestJson) {
	json_t * versionObj = json_string("2.0");
	json_object_set_new(requestJson, "jsonrpc", versionObj);
}

json_t * subtract(json_t **requestJson) {
	json_t *responseJson = NULL;

	json_t * arguments = NULL;
	json_t * numberObj = NULL;

	double minuend  	= 0.0;
	double subtrahend   = 0.0;
	double difference   = 0.0;

	arguments = json_object_get(*requestJson, "params");
	if(arguments &&
		(json_is_array(arguments)) &&
		(json_array_size(arguments) == 2)) {

		numberObj = json_array_get(arguments, 0);
		minuend = json_number_value(numberObj);

		numberObj = json_array_get(arguments, 1);
		subtrahend = json_number_value(numberObj);

		difference = minuend - subtrahend;
		numberObj = json_real(difference);

		json_object_set_new(*requestJson, "result", numberObj);

	} else {
		json_t* errorObj = json_pack(
			"{s:i, s:s}",
			"code", (json_int_t) JSONRPC_INVALID_PARAMS,
			"message", MSG_JSONRPC_ERRORS.invalid_params
		);
		json_object_set_new(*requestJson, "error", errorObj);
	}
	json_incref(*requestJson);
	responseJson = *requestJson;

	return responseJson;
}

json_t * handle_request(json_t **requestJson) {
	json_t *responseJson = NULL;
	json_t *methodNameObj = NULL;
	json_t *errorObj = NULL;
	const char *methodName = NULL;

	json_t *id = json_object_get(*requestJson, "id");
	json_t *transport = json_object_get(*requestJson, "transport");

	methodNameObj = json_object_get(*requestJson, "method");
	methodName	= json_string_value(methodNameObj);

	if(methodNameObj && methodName) {
		if(METHOD_IS("subtract", methodName)) { responseJson = subtract(requestJson);}


	} else {
		errorObj = json_pack(
			"{s:i, s:s}",
			"code", (json_int_t) JSONRPC_INVALID_REQUEST,
			"message", MSG_JSONRPC_ERRORS.invalid_request
		);
		json_object_set_new(*requestJson, "error", errorObj);
		responseJson = *requestJson;
		logger(LEVEL_WARN, "handle_request\tMSG_JSONRPC_ERRORS.invalid_request\n");
	}

	if(!responseJson) {
		json_incref(*requestJson);
		errorObj = json_pack(
			"{s:i, s:s}",
			"code", (json_int_t) JSONRPC_INVALID_REQUEST,
			"message", MSG_JSONRPC_ERRORS.invalid_request
		);
		json_object_set_new(*requestJson, "error", errorObj);
		responseJson = *requestJson;

	}

	json_object_del(responseJson, "params");
	json_object_del(responseJson, "method");
	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));

	if(!json_object_get(responseJson, "result")) {
		errorObj = json_pack(
			"{s:i, s:s}",
			"code", (json_int_t) JSONRPC_SERVER_ERROR,
			"message", MSG_JSONRPC_ERRORS.server_error
		);
		json_object_set_new(responseJson, "error", errorObj);
	}

	if(!json_equal(id, json_object_get(responseJson, "id"))) {
		json_object_set_new(responseJson, "id", id);
	}

	if(!json_equal(transport, json_object_get(responseJson, "transport"))) {
		json_object_set_new(responseJson, "transport", transport);
	}

	json_decref(*requestJson);
	return responseJson;
}
