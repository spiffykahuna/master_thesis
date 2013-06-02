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
extern char  error_space[ERROR_BUFFER_SIZE];

inline
void delete_rpc_method(json_t *requestJson) {
	json_object_del(requestJson, "method");
}

inline
void add_jsonrpc2_0_version(json_t *requestJson) {
	json_t * versionObj = json_string("2.0");
	json_object_set_new(requestJson, "jsonrpc", versionObj);
}

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


json_t * subtract(json_t **requestJson) {
	json_t *responseJson = NULL;

	json_t * arguments = NULL;
	json_t * numberObj = NULL;

	double minuend  	= 0.0;
	double subtrahend   = 0.0;
	double difference   = 0.0;

	responseJson = json_object();
	json_object_set(responseJson, "id",  json_object_get(*requestJson, "id"));


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



		json_object_set_new(responseJson, "result", numberObj);

	} else {
		json_t* errorObj = create_error(JSONRPC_INVALID_PARAMS, MSG_JSONRPC_ERRORS.invalid_params);
		json_object_set_new(responseJson, "error", errorObj);
	}

	json_decref(*requestJson);
	return responseJson;
}

json_t * handle_request(json_t **requestJson) {
	json_t *responseJson = NULL;
	json_t *methodNameObj = NULL;
	json_t *errorObj = NULL;
	strbuffer_t* errorMsg;
	static char methodName[32];
	methodName[0] = '\0';


	if(!*requestJson) {
		return create_response_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request);
	}

	json_int_t id = json_integer_value(json_object_get(*requestJson, "id"));
	transport_type_t transport = json_integer_value(json_object_get(*requestJson, "transport"));
	methodNameObj = json_object_get(*requestJson, "method");
	strncpy(methodName, json_string_value(methodNameObj), 32);

	if(methodName[0] != '\0') {
		if(METHOD_IS("subtract", methodName)) { responseJson = subtract(requestJson);}


	} else {
		json_decref(*requestJson);

		errorMsg = strbuffer_new();
		strbuffer_append(errorMsg, "handle_request : ");
		strbuffer_append(errorMsg, MSG_JSONRPC_ERRORS.invalid_request);
		strbuffer_append(errorMsg, "\n");


		logger(LEVEL_WARN, errorMsg->value);
		strbuffer_destroy(&errorMsg);

		errorObj = create_response_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request);
		json_object_set_new(errorObj, "id", json_integer(id));
		json_object_set_new(errorObj, "transport", json_integer(transport));

		return errorObj;
	}

	if(!responseJson) {
		json_decref(*requestJson);

//		errorMsg = strbuffer_new();
//		strbuffer_append(errorMsg, "handle_request method not found: ");
//		strbuffer_append(errorMsg, methodName);
//		strbuffer_append(errorMsg, "\n");



		snprintf(error_space, ERROR_BUFFER_SIZE, "handle_request method not found: %s\n", methodName);
		logger(LEVEL_WARN, error_space);
//		strbuffer_destroy(&errorMsg);

		errorObj = create_response_error(JSONRPC_METHOD_NOT_FOUND, MSG_JSONRPC_ERRORS.method_not_found);
		json_object_set_new(errorObj, "id", json_integer(id));
		json_object_set_new(errorObj, "transport", json_integer(transport));
		return errorObj;
	}

	json_object_del(responseJson, "params");
	json_object_del(responseJson, "method");
	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));

	if(!json_object_get(responseJson, "result") && !json_object_get(responseJson, "error")) {
		errorObj = create_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);
		json_object_set_new(responseJson, "error", errorObj);
	}

	if( id != json_integer_value(json_object_get(responseJson, "id"))) {
		json_object_set_new(responseJson, "id", json_integer(id));
	}

	if(transport != json_integer_value(json_object_get(responseJson, "transport"))) {
		json_object_set_new(responseJson, "transport", json_integer(transport));
	}

	json_decref(*requestJson);

	return responseJson;
}
