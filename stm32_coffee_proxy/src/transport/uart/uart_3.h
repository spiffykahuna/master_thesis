#ifndef UART_3_H_
#define UART_3_H_

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#ifndef UART3_RXBUFFERSIZE
#define UART3_RXBUFFERSIZE 256
#endif

#ifndef UART3_TXBUFFERSIZE
#define UART3_TXBUFFERSIZE 256
#endif

#ifndef UART3_RX_FIFO_BUFFER_SIZE
#define UART3_RX_FIFO_BUFFER_SIZE 1024
#endif

extern uint8_t uart3_RxBuffer[];
extern uint8_t uart3_TxBuffer[];

extern volatile unsigned char UART3_MsgAvailable;


void UART3_Init(void);

void UART3_send(uint8_t* data, size_t length);

inline void UART3_send_byte(uint8_t byte);

uint8_t UART3_read();

int UART3_has_bytes();

#endif /* UART_3_H_ */
