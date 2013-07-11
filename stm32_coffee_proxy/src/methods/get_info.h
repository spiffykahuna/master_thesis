#ifndef GET_INFO_H_
#define GET_INFO_H_

#include "jansson.h"
#include "messages.h"
#include "system/error.h"

json_t * getInfo(json_t **requestJson);

#endif /* GET_INFO_H_ */
