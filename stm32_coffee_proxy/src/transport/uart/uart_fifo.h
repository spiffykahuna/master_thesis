#ifndef UART_BUFFER_H_
#define UART_BUFFER_H_


#include <stdio.h>
#include <stdint.h>

/* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    int         s_msb;
    int         e_msb;
    uint8_t     *elems;  /* vector of elements                   */
} UartFifo;


inline
void uart_fifo_init(UartFifo *fifo, uint8_t *buffer, int bufferSize);

inline
void uart_fifo_clear(UartFifo *fifo);

inline
int uart_fifo_is_full(UartFifo *fifo);

inline
int uart_fifo_is_empty(UartFifo *fifo);

inline
void uart_fifo_incr(UartFifo *fifo, int *p, int *msb);

inline
void uart_fifo_write_byte(UartFifo *fifo, uint8_t *elem);

inline
void uart_fifo_read_byte(UartFifo *fifo, uint8_t *elem);

#endif /* UART_BUFFER_H_ */
