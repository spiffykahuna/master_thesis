/*
 * error.h
 *
 *  Created on: Jun 20, 2013
 *      Author: Kasutaja
 */

#ifndef ERROR_H_
#define ERROR_H_

#include "mainTasks.h"

inline
json_t * create_error(int errorCode, char* errorMsg);

inline
json_t * create_response_error(int errorCode, char* errorMsg);

char * format_jsonrpc_error(proxy_error_t errorNum, char* errorMsg, char* errorData, json_int_t id);

inline json_t* server_error(const json_t *requestJson);

#endif /* ERROR_H_ */
