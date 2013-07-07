/*
 * circular_buffer.h
 *
 *  Created on: Jul 6, 2013
 *      Author: Kasutaja
 */

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_


#include <stdio.h>

#include <malloc.h>

/* Opaque buffer element type.  This would be defined by the application. */
typedef struct { int value; } ElemType;

/* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    int         s_msb;
    int         e_msb;
    ElemType   *elems;  /* vector of elements                   */
} CircularBuffer;

#endif /* CIRCULAR_BUFFER_H_ */
