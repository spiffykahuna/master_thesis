#include "circular_buffer.h"

/* Circular buffer example, keeps one slot open */


void cbInit(CircularBuffer *cb, int size) {
    cb->size  = size + 1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->s_msb = 0;
    cb->e_msb = 0;
    cb->elems = (ElemType *)calloc(cb->size, sizeof(ElemType));
}

void cbFree(CircularBuffer *cb) {
    free(cb->elems); /* OK if null */ }

int cbIsFull(CircularBuffer *cb) {
    return cb->end == cb->start && cb->e_msb != cb->s_msb; }

int cbIsEmpty(CircularBuffer *cb) {
    return cb->end == cb->start && cb->e_msb == cb->s_msb; }

void cbIncr(CircularBuffer *cb, int *p, int *msb) {
    *p = *p + 1;
    if (*p == cb->size) {
        *msb ^= 1;
        *p = 0;
    }
}

void cbWrite(CircularBuffer *cb, ElemType *elem) {
    cb->elems[cb->end] = *elem;
    if (cbIsFull(cb)) /* full, overwrite moves start pointer */
        cbIncr(cb, &cb->start, &cb->s_msb);
    cbIncr(cb, &cb->end, &cb->e_msb);
}

void uart_fifo_read_byte(CircularBuffer *cb, ElemType *elem) {
    *elem = cb->elems[cb->start];
    cbIncr(cb, &cb->start, &cb->s_msb);
}

int main(int argc, char **argv) {
    CircularBuffer cb;
    ElemType elem = {0};

    int testBufferSize = 10; /* arbitrary size */
    cbInit(&cb, testBufferSize);

    /* Fill buffer with test elements 3 times */
    for (elem.value = 0; elem.value < 3 * testBufferSize; ++ elem.value)
        cbWrite(&cb, &elem);

    /* Remove and print all elements */
    while (!cbIsEmpty(&cb)) {
        uart_fifo_read_byte(&cb, &elem);
        printf("%d\n", elem.value);
    }

    cbFree(&cb);
    return 0;
}

