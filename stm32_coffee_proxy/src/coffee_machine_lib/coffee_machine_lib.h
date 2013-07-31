#ifndef COFFEE_MACHINE_LIB_H_
#define COFFEE_MACHINE_LIB_H_

//typedef union _coffee_message_value_t {
//	int int_value;
//	double double_value;
//	char* 	text_value;
//} coffee_message_value_t;
//
//typedef struct _coffee_message_t {
//	char*						messageName;
//
//	coffee_message_param_t 		*values;
//	size_t						paramsCount;
//
//	uint8_t						*data;
//	size_t						dataSize;
//} coffee_message_t;
//
//
//coffee_message_t* coffee_message_new(size_t valuesCount);
//void	coffee_message_destroy(coffee_message_t**);
//
///* returns encoding result FAIL/SUCCESS. Stores/Reads coffee_message_t data and dataSize fields */
//int		coffee_message_encode(coffee_message_t**);
//int		coffee_message_decode(coffee_message_t**);

#include "strbuffer.h"

typedef enum _product_currency_t {
	EUR,
	USD,
	LTL,
	LVL,
	RUB,
	SEK
} product_currency_t;


struct _product_list_t {
	unsigned long long id;
	char *productName;

	double productPrice;
	product_currency_t currency;
	struct _product_list_t *nextProduct;
} __attribute__ ((aligned));

typedef struct _product_list_t product_list_t;

char * get_version(void);

//product_list_t* getProductList(void);

#endif /* COFFEE_MACHINE_LIB_H_ */
