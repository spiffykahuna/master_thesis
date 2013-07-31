#ifndef GET_PRODUCTS_H_
#define GET_PRODUCTS_H_

#include "jansson.h"
#include "messages.h"
#include "system/error.h"
#include "coffee_machine_lib/coffee_machine_lib.h"

json_t * get_products(const json_t *requestJson, transport_type_t transport);



#endif /* GET_PRODUCTS_H_ */
