#ifndef UART_4_H_
#define UART_4_H_

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#ifndef UART4_RXBUFFERSIZE
#define UART4_RXBUFFERSIZE 256
#endif

#ifndef UART4_TXBUFFERSIZE
#define UART4_TXBUFFERSIZE 256
#endif

#ifndef UART4_RX_FIFO_BUFFER_SIZE
#define UART4_RX_FIFO_BUFFER_SIZE 1024
#endif

extern uint8_t uart4_RxBuffer[];
extern uint8_t uart4_TxBuffer[];

extern volatile unsigned char UART4_MsgAvailable;


void UART4_Init(void);

void UART4_send(uint8_t* data, size_t length);

inline void UART4_send_byte(uint8_t byte);

uint8_t UART4_read();

int UART4_has_bytes();

#endif /* UART_4_H_ */
