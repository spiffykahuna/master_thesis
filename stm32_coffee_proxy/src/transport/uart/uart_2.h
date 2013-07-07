#ifndef UART_2_H_
#define UART_2_H_

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#ifndef UART2_RXBUFFERSIZE
#define UART2_RXBUFFERSIZE 256
#endif

#ifndef UART2_TXBUFFERSIZE
#define UART2_TXBUFFERSIZE 256
#endif

#ifndef UART2_RX_FIFO_BUFFER_SIZE
#define UART2_RX_FIFO_BUFFER_SIZE 1024
#endif

extern uint8_t uart2_RxBuffer[];
extern uint8_t uart2_TxBuffer[];

extern volatile unsigned char UART2_MsgAvailable;


void UART2_Init(void);

void UART2_send(uint8_t* data, size_t length);

inline void UART2_send_byte(uint8_t byte);

uint8_t UART2_read();

int UART2_has_bytes();

#endif /* UART_2_H_ */
