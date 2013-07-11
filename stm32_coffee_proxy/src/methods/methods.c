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

json_t * getSystemHelp(json_t **requestJson) {
	json_int_t id = json_integer_value(json_object_get(*requestJson, "id"));
	transport_type_t transport = json_integer_value(json_object_get(*requestJson, "transport"));
	json_decref(*requestJson);

	logger_format(LEVEL_INFO, "getSystemHelp : Requested system help message. client_id = %d from_transport=%s"
			, (int) id, transport_type_to_str(transport)
	);

	json_t *responseJson = NULL;

	extern const char *SERVICE_SCHEMA;
	logger(LEVEL_INFO, "getSystemHelp : Starting to send service schema");

	system_flush_messages();

	if(transport_lock(transport, DIRECTION_OUTPUT) == pdPASS) {
		strbuffer_t *schemaMsg = strbuffer_new();

		strbuffer_append(schemaMsg, "{\"jsonrpc\": \"2.0\", \"id\": " );
		strbuffer_append(schemaMsg, int_to_string((int) id));
		strbuffer_append(schemaMsg, ", \"result\": ");
		send_data_to_client(transport, schemaMsg->value, schemaMsg->length);

		send_data_to_client(transport, SERVICE_SCHEMA, strlen(SERVICE_SCHEMA));

		strbuffer_clear(schemaMsg);
		strbuffer_append(schemaMsg, "}\n");
		send_data_to_client(transport, schemaMsg->value, schemaMsg->length);
		strbuffer_destroy(&schemaMsg);

		transport_unlock(transport, DIRECTION_OUTPUT);
	} else {
		logger_format(LEVEL_WARN, "getSystemHelp : Unable to get transport <%s> handler", transport_type_to_str(transport));
		return create_response_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);
	}
	logger(LEVEL_INFO, "getSystemHelp : Finished to send service schema");



//	system_msg_t * helpMsg = system_msg_new(MSG_TYPE_PRINT_HELP);
//	helpMsg->transport = transport;
//
//	int result = system_msg_add_to_queue(helpMsg);
//	if(!result) {
//		snprintf(error_space, ERROR_BUFFER_SIZE, "getSystemHelp : Unable to add system message. Request id = %d", (int) id);
//		logger(LEVEL_INFO, error_space);
//		responseJson = create_response_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);
//		json_object_set_new(responseJson, "id", json_integer(id));
//		json_object_set_new(responseJson, "transport", json_integer(transport));
//
//		return responseJson;
//	}

	responseJson = json_object();
	json_object_set_new(responseJson, "no_reply", json_true());
	return responseJson;
}


json_t * handle_request(json_t **requestJson) {
	json_t *responseJson = NULL;
	json_t *methodNameObj = NULL;
	json_t *idObj = NULL;
	json_t *errorObj = NULL;
	static char methodName[32];
	methodName[0] = '\0';

	if(!(*requestJson)) {
		logger(LEVEL_ERR, "handle_request: invalid request was received");
		return create_response_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request);
	}

	idObj = json_object_get(*requestJson, "id");
	json_int_t id = json_integer_value(idObj);
	transport_type_t transport = json_integer_value(json_object_get(*requestJson, "transport"));
	methodNameObj = json_object_get(*requestJson, "method");
	strncpy(methodName, json_string_value(methodNameObj), 32);


	logger_format(LEVEL_INFO, "%s.handle_request():  Received request. id = %d method = %s transport=%s"
			, pcTaskGetTaskName(NULL), (int) id, methodName, transport_type_to_str(transport)
	);
	system_flush_messages();

	if(methodName[0] != '\0') {
		if(METHOD_IS("subtract", methodName)) 		{ responseJson = subtract(requestJson);}
		if(METHOD_IS("system.help", methodName)) 	{ responseJson = getSystemHelp(requestJson);}
		if(METHOD_IS("get_info", methodName))		{ responseJson = getInfo(requestJson);}

	} else {
		json_decref(*requestJson);

		logger_format(LEVEL_WARN, "%s.handle_request():  %s"
					, pcTaskGetTaskName(NULL), MSG_JSONRPC_ERRORS.invalid_request
		);

		errorObj = create_response_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request);
		json_object_set_new(errorObj, "id", json_integer(id));
		json_object_set_new(errorObj, "transport", json_integer(transport));

		return errorObj;
	}

	system_flush_messages();

	if(!responseJson) {
		json_decref(*requestJson);

		logger_format(LEVEL_WARN, "handle_request method not found: %s", methodName);

		errorObj = create_response_error(JSONRPC_METHOD_NOT_FOUND, MSG_JSONRPC_ERRORS.method_not_found);
		json_object_set_new(errorObj, "id", json_integer(id));
		json_object_set_new(errorObj, "transport", json_integer(transport));
		return errorObj;
	}

	if(json_object_get(responseJson, "no_reply")) {
		/* Mehtod was notification only */
		json_decref(responseJson);
		return NULL;
	}

	json_object_del(responseJson, "params");
	json_object_del(responseJson, "method");
	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));

	if(!json_object_get(responseJson, "result") && !json_object_get(responseJson, "error")) {
		logger_format(LEVEL_WARN, "handle_request method <%s> returned invalid response", methodName);
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
