#include "uart_fifo.h"

inline
void uart_fifo_init(UartFifo *fifo, uint8_t *buffer, int bufferSize) {
    fifo->size  = bufferSize;
    fifo->start = 0;
    fifo->end   = 0;
    fifo->s_msb = 0;
    fifo->e_msb = 0;
    fifo->elems = buffer;
    memset(fifo->elems, 0, fifo->size);
}

inline
void uart_fifo_clear(UartFifo *fifo) {
	fifo->start = 0;
	fifo->end   = 0;
	fifo->s_msb = 0;
	fifo->e_msb = 0;
	memset(fifo->elems, 0, fifo->size);
}

inline
int uart_fifo_is_full(UartFifo *fifo) {
    return fifo->end == fifo->start && fifo->e_msb != fifo->s_msb; }

inline
int uart_fifo_is_empty(UartFifo *fifo) {
    return fifo->end == fifo->start && fifo->e_msb == fifo->s_msb; }

inline
void uart_fifo_incr(UartFifo *fifo, int *p, int *msb) {
    *p = *p + 1;
    if (*p == fifo->size) {
        *msb ^= 1;
        *p = 0;
    }
}

inline
void uart_fifo_write_byte(UartFifo *fifo, uint8_t *elem) {
    fifo->elems[fifo->end] = *elem;
    if (uart_fifo_is_full(fifo)) /* full, overwrite moves start pointer */
        uart_fifo_incr(fifo, &fifo->start, &fifo->s_msb);
    uart_fifo_incr(fifo, &fifo->end, &fifo->e_msb);
}

inline
void uart_fifo_read_byte(UartFifo *fifo, uint8_t *elem) {
    *elem = fifo->elems[fifo->start];
    uart_fifo_incr(fifo, &fifo->start, &fifo->s_msb);
}
