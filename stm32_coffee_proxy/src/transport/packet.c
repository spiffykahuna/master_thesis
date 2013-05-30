/*
 * packet.c
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */
#include <stdio.h>

#include "packet.h"

inline
packet_t* packet_new(transport_type_t transport) {
	packet_t *packet = (packet_t*) pvPortMalloc(sizeof(packet_t));
	if(!packet) { return NULL;};

	packet->transport = transport;
	packet->jsonDoc = NULL;
	return packet;
}

inline
void packet_destroy(packet_t **packet) {
	if (*packet) {
		if ((*packet)->jsonDoc) {
			strbuffer_destroy(&(*packet)->jsonDoc);
		}
		vPortFree(*packet);
		*packet = NULL;
	}
}
