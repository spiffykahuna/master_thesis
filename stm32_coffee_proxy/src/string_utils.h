/*
 * string_utils.h
 *
 *  Created on: May 16, 2013
 *      Author: Kasutaja
 */

#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include "app_config.h"
#include "strbuffer.h"

inline strbuffer_t * strbuffer_new(void);
inline void strbuffer_destroy(strbuffer_t **string);

#endif /* STRING_UTILS_H_ */
