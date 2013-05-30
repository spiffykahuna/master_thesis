/*
 * string_utils.c
 *
 *  Created on: May 16, 2013
 *      Author: Kasutaja
 */

#include <stdio.h>

#include "string_utils.h"


inline
strbuffer_t * strbuffer_new(void) {
	strbuffer_t *sb = (strbuffer_t *) jsonp_malloc(sizeof(strbuffer_t));
	if(!sb) { return NULL;}

	strbuffer_init(sb);
	return sb;
}

inline
void strbuffer_destroy(strbuffer_t **string) {
	if (*string) {
		if ((*string)->value) {
			strbuffer_close(*string);
		}
		jsonp_free(*string);
		*string = NULL;
	}
}
