/*
 * packet.h
 *
 *  Created on: May 18, 2013
 *      Author: Kasutaja
 */


#ifndef PACKET_H_
#define PACKET_H_

#include "string_utils.h"
#include "transport.h"


inline packet_t* packet_new();
inline void packet_destroy(packet_t **packet);

inline void packet_lock(packet_t *packet);
inline void packet_unlock(packet_t *packet);

#endif /* PACKET_H_ */
