/*
 * transport_type.h
 *
 *  Created on: May 23, 2013
 *      Author: Kasutaja
 */

#ifndef TRANSPORT_TYPE_H_
#define TRANSPORT_TYPE_H_

#include "strbuffer.h"

typedef enum _transport_type_t {
	TRANSPORT_USB = 1
} transport_type_t;

typedef struct _packet_t {
	transport_type_t transport;
	strbuffer_t		 *jsonDoc;
} packet_t;

#endif /* TRANSPORT_TYPE_H_ */
