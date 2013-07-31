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
#include "get_products.h"
#include "transport/transport.h"

inline void delete_rpc_method(json_t *requestJson);

inline
void add_jsonrpc2_0_version(json_t *requestJson);

inline json_t * jsonrpc_response(const json_t* rpcRequest, json_t *resultOrError, int error);

void handle_request(packet_t *requestPacket);

#endif /* METHODS_H_ */
