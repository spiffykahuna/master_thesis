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

inline json_t * jsonrpc_response(const json_t* rpcRequest, json_t *resultOrError, int error) {
	json_t *responseJson = NULL;

	responseJson = json_object();
	json_object_set(responseJson, "id",  json_object_get(rpcRequest, "id"));
	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));

	if(error) {
		json_object_set_new(responseJson, "error", resultOrError);
	} else {
		json_object_set_new(responseJson, "result", resultOrError);
	}

	return responseJson;
}


json_t * subtract(const json_t *requestJson) {
	json_t *responseJson = NULL;

	json_t * arguments = NULL;
	json_t * numberObj = NULL;

	double minuend  	= 0.0;
	double subtrahend   = 0.0;
	double difference   = 0.0;


	arguments = json_object_get(requestJson, "params");
	if(arguments &&
		(json_is_array(arguments)) &&
		(json_array_size(arguments) == 2)) {

		numberObj = json_array_get(arguments, 0);
		minuend = json_number_value(numberObj);

		numberObj = json_array_get(arguments, 1);
		subtrahend = json_number_value(numberObj);

		difference = minuend - subtrahend;
		numberObj = json_real(difference);


		responseJson = jsonrpc_response(requestJson, numberObj, FALSE);


	} else {
		logger(LEVEL_WARN, "subtract()  invalid params");
		json_t* errorObj = create_error(JSONRPC_INVALID_PARAMS, MSG_JSONRPC_ERRORS.invalid_params);
		responseJson = jsonrpc_response(requestJson, errorObj, TRUE);
	}


	return responseJson;
}

json_t * getSystemHelp(const json_t *requestJson, transport_type_t transport) {


	json_int_t id = json_integer_value(json_object_get(requestJson, "id"));

	strbuffer_t *logMsg = strbuffer_new();
	strbuffer_append(logMsg, "getSystemHelp : Requested system help message. client_id = ");
	strbuffer_append(logMsg, int_to_string((int) id));
	strbuffer_append(logMsg, " from_transport=");
	strbuffer_append(logMsg, int_to_string(transport));
	logger(LEVEL_INFO, logMsg->value);
	strbuffer_destroy(&logMsg);

	json_t *responseJson = NULL;

	extern const char *SERVICE_SCHEMA;
	logger(LEVEL_INFO, "getSystemHelp : Starting to send service schema");


	if(transport_lock(transport, DIRECTION_OUTPUT) == pdPASS) {
		strbuffer_t *schemaMsg = strbuffer_new();



		strbuffer_append(schemaMsg, "{\"jsonrpc\": \"2.0\", \"id\": " );
		strbuffer_append(schemaMsg, int_to_string((int) id));
		strbuffer_append(schemaMsg, ", \"result\": ");

		int totalLength = schemaMsg->length + strlen(SERVICE_SCHEMA) + strlen("}");

		char *len = int_to_string(totalLength);

		send_data_to_client(transport, len, strlen(len));
		send_data_to_client(transport, ":", 1);

		send_data_to_client(transport, schemaMsg->value, schemaMsg->length);

		send_data_to_client(transport, SERVICE_SCHEMA, strlen(SERVICE_SCHEMA));

		strbuffer_clear(schemaMsg);
		strbuffer_append(schemaMsg, "}");
		send_data_to_client(transport, schemaMsg->value, schemaMsg->length);
		strbuffer_destroy(&schemaMsg);

		send_data_to_client(transport, ",", 1);

		transport_unlock(transport, DIRECTION_OUTPUT);
	} else {

		logMsg = strbuffer_new();
		strbuffer_append(logMsg, "getSystemHelp : Unable to get transport <");
		strbuffer_append(logMsg, int_to_string(transport));
		strbuffer_append(logMsg, "> handler");
		logger(LEVEL_WARN, logMsg->value);
		strbuffer_destroy(&logMsg);


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
	json_object_set_new(responseJson, "notification", json_true());
	return responseJson;
}


void handle_request(packet_t *requestPacket) {
	json_t *responseJson = NULL;
	json_t *methodNameObj = NULL;
	json_t *idObj = NULL;
	json_t *errorObj = NULL;

	int methodFound;

	static char methodName[32];
	methodName[0] = '\0';

	strbuffer_t *logMsg;

//	packet_lock(requestPacket);
//	{
//
//
//		requestPacket->type = PKG_TYPE_OUTCOME_JSONRPC_RESPONSE;
//
//	}
//	packet_unlock(requestPacket);
//	return;

	if(requestPacket->type != PKG_TYPE_INCOME_JSONRPC_REQUEST) {
		logger(LEVEL_ERR, "Wrong data packet from requestQueue. Deleting packet" );
		return;
	}

	json_t *rpcRequest = requestPacket->payload.jsonDoc;


	methodNameObj = json_object_get(rpcRequest, "method");
	strncpy(methodName, json_string_value(methodNameObj), 32);

	logMsg = strbuffer_new();
	strbuffer_append(logMsg, "handle_request():  Received request. id = ");
	strbuffer_append(logMsg, int_to_string(requestPacket->id));
	strbuffer_append(logMsg, " method = ");
	strbuffer_append(logMsg, methodName);
	strbuffer_append(logMsg, " transport=");
	strbuffer_append(logMsg, int_to_string(requestPacket->transport));
	logger(LEVEL_INFO, logMsg->value);
	strbuffer_destroy(&logMsg);

	responseJson = NULL;
	methodFound = 0;

	if(METHOD_IS("subtract", methodName)) 		{ responseJson = subtract(rpcRequest); methodFound = 1;}
	if(METHOD_IS("system.help", methodName)) 	{ responseJson = getSystemHelp(rpcRequest, requestPacket->transport); methodFound = 1;}
	if(METHOD_IS("get_info", methodName))		{ responseJson = getInfo(rpcRequest); methodFound = 1;}

	if(responseJson) {

		if(json_object_get(responseJson, "notification")) {
			json_decref(responseJson);
			packet_lock(requestPacket);
			{
				requestPacket->type = PKG_TYPE_OUTCOME_JSONRPC_NOTIFICATION;
			}
			packet_unlock(requestPacket);

			return;
		}
		logger(LEVEL_INFO, "Received response json object");

		packet_lock(requestPacket);
		{
			json_decref(requestPacket->payload.jsonDoc);

			requestPacket->type = PKG_TYPE_OUTCOME_JSONRPC_RESPONSE;
			requestPacket->payload.jsonDoc = responseJson;
		}
		packet_unlock(requestPacket);
		return;

	} else {
		if( methodFound == 1) {
			logMsg = strbuffer_new();
			strbuffer_append(logMsg, "%s.handle_request():  ");
			strbuffer_append(logMsg, MSG_JSONRPC_ERRORS.invalid_request);
			logger(LEVEL_WARN, logMsg->value);
			strbuffer_destroy(&logMsg);

			errorObj = create_response_error(JSONRPC_INVALID_REQUEST, MSG_JSONRPC_ERRORS.invalid_request);
		} else {
			logMsg = strbuffer_new();
			strbuffer_append(logMsg, "handle_request method not found: ");
			strbuffer_append(logMsg, methodName);
			logger(LEVEL_WARN, logMsg->value);
			strbuffer_destroy(&logMsg);

			errorObj = create_response_error(JSONRPC_METHOD_NOT_FOUND, MSG_JSONRPC_ERRORS.method_not_found);
		}

		packet_lock(requestPacket);
		{
			json_decref(requestPacket->payload.jsonDoc);

			requestPacket->type = PKG_TYPE_OUTCOME_JSONRPC_RESPONSE;
			requestPacket->payload.jsonDoc = errorObj;
		}
		packet_unlock(requestPacket);
		return;

	}


//
//	if(json_object_get(responseJson, "no_reply")) {
//		/* Mehtod was notification only */
//		logger_format(LEVEL_DEBUG, "Method was notification only. Returning no response. Method: %s", methodName);
//		json_decref(responseJson);
////		return NULL;
//	}
//
////	json_object_del(responseJson, "params");
////	json_object_del(responseJson, "method");
////	json_object_set_new(responseJson, "jsonrpc", json_string("2.0"));
//
//	if(!json_object_get(responseJson, "result") && !json_object_get(responseJson, "error")) {
//		logger_format(LEVEL_WARN, "handle_request method <%s> returned invalid response", methodName);
//		errorObj = create_error(JSONRPC_SERVER_ERROR, MSG_JSONRPC_ERRORS.server_error);
//		json_object_set_new(responseJson, "error", errorObj);
//	}
//
//	if( id != json_integer_value(json_object_get(responseJson, "id"))) {
//		logger_format(LEVEL_WARN, "Response is wrong. Response id=%d is not equal to request id=%d. Setting id from request",
//				json_integer_value(json_object_get(responseJson, "id")), id
//		);
//		json_object_set_new(responseJson, "id", json_integer(id));
//	}
//
//	if(transport != json_integer_value(json_object_get(responseJson, "transport"))) {
//		logger_format(LEVEL_WARN, "Response is wrong. Response transport=%s is not equal to request transport=%s. Setting transport type from request",
//				transport_type_to_str((transport_type_t) json_integer_value(json_object_get(responseJson, "transport"))),
//				transport_type_to_str((transport_type_t) transport)
//		);
//		json_t *transport = json_integer(transport);
//		json_object_set_new(responseJson, "transport", transport);
//		logger_format(LEVEL_WARN, "Transport is now: %s", transport_type_to_str((transport_type_t) json_integer_value(json_object_get(responseJson, "transport"))));
//	}
//
//	json_decref(*requestPacket);

//	return responseJson;
}
