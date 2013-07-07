#ifndef UART_5_H_
#define UART_5_H_

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#ifndef UART5_RXBUFFERSIZE
#define UART5_RXBUFFERSIZE 256
#endif

#ifndef UART5_TXBUFFERSIZE
#define UART5_TXBUFFERSIZE 256
#endif

#ifndef UART5_RX_FIFO_BUFFER_SIZE
#define UART5_RX_FIFO_BUFFER_SIZE 1024
#endif

extern uint8_t uart5_RxBuffer[];
extern uint8_t uart5_TxBuffer[];

extern volatile unsigned char UART5_MsgAvailable;


void UART5_Init(void);

void UART5_send(uint8_t* data, size_t length);

inline void UART5_send_byte(uint8_t byte);

uint8_t UART5_read();

int UART5_has_bytes();

#endif /* UART_5_H_ */
