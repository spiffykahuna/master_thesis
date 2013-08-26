#include "get_info.h"
#include "methods.h"


//extern const msg_maintasks MSG_MAINTASKS;
//extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;


json_t * getInfo(const json_t *requestJson) {
	json_t *responseJson = NULL;

	json_t* result = json_object();

	char * response = get_version();
	if(response) {
		json_object_set_new(result, "machineFirmvareVersion", json_string(response));
	}  else {
		logger(LEVEL_WARN, "get_info()  invalid version response");
		return server_error(requestJson);
	}

	json_object_set_new(result, "machineName", json_string("Good Coffee Machine"));

	responseJson = jsonrpc_response(requestJson, result, FALSE);

	return responseJson;
}



