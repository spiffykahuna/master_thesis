/*
 * packet.c
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */
#include <stdio.h>

#include "packet.h"

inline
packet_t* packet_new() {
	packet_t *packet = (packet_t*) pvPortMalloc(sizeof(packet_t));
	if(!packet) { return NULL;};

	packet->id = 0;
	packet->transport = TRANSPORT_UNKNOWN;
	packet->type = PKG_TYPE_UNKNOWN;
	packet->locked = 0;
	return packet;
}

inline
void packet_destroy(packet_t **packet) {
	packet_t *pkg = *packet;
	if (pkg) {
		switch(pkg->type) {
		case PKG_TYPE_INCOME_MESSAGE_STRING:
		case PKG_TYPE_OUTGOING_MESSAGE_STRING:
			if(pkg->payload.stringData) {
				strbuffer_destroy(&(pkg->payload.stringData));
			}
			break;

		case PKG_TYPE_OUTCOME_JSONRPC_RESPONSE:
		case PKG_TYPE_OUTCOME_JSONRPC_NOTIFICATION:
		case PKG_TYPE_INCOME_JSONRPC_REQUEST:
			if(pkg->payload.jsonDoc) {
				json_decref(pkg->payload.jsonDoc);
			}
			break;

		case PKG_TYPE_UNKNOWN:
		default:
			break;
		}

		vPortFree(*packet);
		*packet = NULL;
	}
}

inline
void packet_lock(packet_t *packet) {
	while(packet->locked == 1) { vTaskDelay(SYSTEM_TASK_DELAY); }
	packet->locked = 1;
}
inline
void packet_unlock(packet_t *packet) {
	packet->locked = 0;
}
