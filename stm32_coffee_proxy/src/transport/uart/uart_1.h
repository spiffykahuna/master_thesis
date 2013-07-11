#ifndef UART_1_H_
#define UART_1_H_

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#ifndef UART1_RXBUFFERSIZE
#define UART1_RXBUFFERSIZE 256
#endif

#ifndef UART1_TXBUFFERSIZE
#define UART1_TXBUFFERSIZE 256
#endif

#ifndef UART1_RX_FIFO_BUFFER_SIZE
#define UART1_RX_FIFO_BUFFER_SIZE 1024
#endif



extern uint8_t uart1_RxBuffer[];
extern uint8_t uart1_TxBuffer[];

extern volatile unsigned char UART1_MsgAvailable;


void UART1_Init(void);

void UART1_send(uint8_t* data, size_t length);

inline int UART1_send_chars(char* data, size_t length);

inline void UART1_send_byte(uint8_t byte);

uint8_t UART1_read();

inline char UART1_read_char();

int UART1_has_bytes();


#define UART1_MSG_AVAILABLE_CALLBACK
void UART1_MsgAvailable_Callback(void);


#endif /* UART_1_H_ */
