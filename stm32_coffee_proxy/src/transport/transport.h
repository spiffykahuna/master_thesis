/*
 * transport.h
 *
 *  Created on: May 19, 2013
 *      Author: Kasutaja
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include "app_config.h"
#include "transport_domain.h"
#include "packet.h"
#include "abstract_writer.h"
#include "string_utils.h"







int write_usb(char *data, size_t length);


inline
void send_packet_to_client(packet_t *packet);

void report_error_to_sender(transport_type_t transport,const char *msgFormat, ...);

inline
char * transport_type_to_str(transport_type_t transport);

#endif /* TRANSPORT_H_ */
