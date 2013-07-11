/*
 * methods.h
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */

#ifndef METHODS_H_
#define METHODS_H_

#include "mainTasks.h"
#include "get_info.h"

inline void delete_rpc_method(json_t *requestJson);

inline
void add_jsonrpc2_0_version(json_t *requestJson);

json_t * handle_request(json_t **requestJson);

#endif /* METHODS_H_ */
