#include "get_info.h"
#include "methods.h"

//extern const msg_maintasks MSG_MAINTASKS;
extern const msg_jsonrpc_errors MSG_JSONRPC_ERRORS;

json_t * getInfo(const json_t *requestJson) {
	json_t *responseJson = NULL;

		const char *version = "ty:LTB2010M 044";

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

			json_t* result = json_object();

			json_object_set_new(result, "value", numberObj);
			json_object_set_new(result, "version", json_string(version));


			responseJson = jsonrpc_response(requestJson, result, FALSE);


		} else {
			logger(LEVEL_WARN, "subtract()  invalid params");
			json_t* errorObj = create_error(JSONRPC_INVALID_PARAMS, MSG_JSONRPC_ERRORS.invalid_params);
			responseJson = jsonrpc_response(requestJson, errorObj, TRUE);
		}


		return responseJson;
}
