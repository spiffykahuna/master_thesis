#ifndef GET_INFO_H_
#define GET_INFO_H_

#include "jansson.h"
#include "messages.h"
#include "system/error.h"
#include "coffee_machine_lib/coffee_machine_lib.h"

json_t * getInfo(const json_t *requestJson);

#endif /* GET_INFO_H_ */
