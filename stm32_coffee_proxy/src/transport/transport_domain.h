/*
 * transport_type.h
 *
 *  Created on: May 23, 2013
 *      Author: Kasutaja
 */

#ifndef TRANSPORT_TYPE_H_
#define TRANSPORT_TYPE_H_

#include "jansson.h"
#include "strbuffer.h"

typedef enum _transport_type_t {
	TRANSPORT_UNKNOWN = 0,
	TRANSPORT_UART1 = 1,
	TRANSPORT_UART2 = 2,
	TRANSPORT_UART3 = 3,
	TRANSPORT_UART4 = 4,
	TRANSPORT_UART5 = 5
} transport_type_t;



typedef enum _transport_direction_t {
	DIRECTION_INPUT=1,
	DIRECTION_OUTPUT=2,
} transport_direction_t;



typedef enum _packet_type_t {
	PKG_TYPE_UNKNOWN,
	PKG_TYPE_ERROR,
	PKG_TYPE_INCOME_MESSAGE_STRING,
	PKG_TYPE_OUTGOING_MESSAGE_STRING,
	PKG_TYPE_INCOME_JSONRPC_REQUEST,
	PKG_TYPE_OUTCOME_JSONRPC_RESPONSE,
	PKG_TYPE_OUTCOME_JSONRPC_NOTIFICATION,
} packet_type_t;

typedef struct _packet_t {
	json_int_t			id;
	packet_type_t		type;
	transport_type_t 	transport;
	union {
		strbuffer_t		 	*stringData;
		json_t				*jsonDoc;
	} payload;

	int locked;
} packet_t;

#endif /* TRANSPORT_TYPE_H_ */
