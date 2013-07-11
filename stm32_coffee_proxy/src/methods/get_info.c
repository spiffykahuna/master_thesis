#include "get_info.h"


json_t * getInfo(json_t **requestJson) {
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
