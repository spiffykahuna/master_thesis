#include "stm32f10x.h"
#include "uart_2.h"
#include "uart_fifo.h"

uint8_t uart2_RxBuffer[UART2_RXBUFFERSIZE];
uint8_t uart2_TxBuffer[UART2_TXBUFFERSIZE];

static UartFifo  uartRxFifo;
static uint8_t rxFifoBuffer[UART2_RX_FIFO_BUFFER_SIZE];

volatile unsigned char UART2_MsgAvailable;

volatile unsigned char UART2_dmaFree = 0;

void USART2_New_data_block( uint8_t *data, size_t len );

void USART2_IRQHandler(void)
{
  unsigned short int status;


  status=USART2->SR;

  if (status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {		// check errors
    USART_ReceiveData(USART2);

  } else 

  if (status & USART_SR_RXNE) {               // read interrupt
    USART2->SR &= ~USART_SR_RXNE;	          // clear interrupt

    /* we are using dma to receice */
//    if (++usart2_In >= UART2_RXBUFFERSIZE) usart2_In = 0;
//    uart2_RxBuffer[usart2_In] = USART2->DR;
//    UART2_MsgAvailable=1;

  }

  if (status & USART_SR_TXE) {
    USART2->SR &= ~USART_SR_TXE;	          // clear interrupt
  }

  if ( status & USART_FLAG_IDLE )
  	{
  		USART_ReceiveData(USART2);

  		unsigned long cnt = UART2_RXBUFFERSIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
  		if ( cnt != UART2_RXBUFFERSIZE/2 && cnt != UART2_RXBUFFERSIZE )
  		{
  			if ( cnt < UART2_RXBUFFERSIZE/2 )
  			{
  				USART2_New_data_block( uart2_RxBuffer, cnt );
  				DMA_Cmd(DMA1_Channel6, DISABLE);
  				DMA1_Channel6->CNDTR = UART2_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel6, ENABLE);
  			}
  			else
  			{
  				USART2_New_data_block( &uart2_RxBuffer[UART2_RXBUFFERSIZE/2], cnt - UART2_RXBUFFERSIZE/2 );
  				DMA_Cmd(DMA1_Channel6, DISABLE);
  				DMA1_Channel6->CNDTR = UART2_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel6, ENABLE);
  			}
  		}
  		UART2_MsgAvailable = 1;
  	}
}




void UART2_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

	// PD5 - USART2_Tx
	GPIO_InitTypeDef    GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	//PD6 - USART2_Rx
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //Input floating
	GPIO_Init(GPIOD, &GPIO_InitStruct);


	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200; // TODO put define
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	//  	USART2->BRR  = 0x00001D4C; // right 9600 baud rate constant
	USART2->BRR = 0x00000271; //115200

	/* Available interrupts from USART2 */
	USART2->CR1 |= (USART_CR1_RE | USART_CR1_TE /*| USART_CR1_RXNEIE */ | USART_CR1_IDLEIE);

	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//TX
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart2_TxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART2_TXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel7, &DMA_InitStructure);


	//		DMA1_Channel7->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel7->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);

	//RX
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart2_RxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART2_RXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal; //
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	DMA1_Channel6->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel6->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	//		DMA1_Channel5->CCR |= DMA_CCR1_TEIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);


	USART2->CR3         |=  USART_CR3_DMAR;          // USART2 DMA receive
	USART2->CR3         |=  USART_CR3_DMAT;          // USART2 DMA transmit


	USART_Cmd(USART2, ENABLE);
	DMA_Cmd(DMA1_Channel6, ENABLE);
	DMA_Cmd(DMA1_Channel7, ENABLE);


	uart_fifo_init(&uartRxFifo, rxFifoBuffer, UART2_RX_FIFO_BUFFER_SIZE);
	UART2_MsgAvailable=0;
}


void USART2_New_data_block( uint8_t *data, size_t len )
{
	uint8_t *end = data + len;
	while(data != end && !uart_fifo_is_full(&uartRxFifo)) {
		uart_fifo_write_byte(&uartRxFifo, data++);
	}
	/* if fifo is full new bytes are ignored */
}



// Rx
void DMA1_Channel6_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_HT6) == SET)
	{

		USART2_New_data_block( uart2_RxBuffer, UART2_RXBUFFERSIZE/2 );
		DMA_ClearITPendingBit(DMA1_IT_HT6);
	}
	if (DMA_GetITStatus(DMA1_IT_TC6) == SET)
	{
		USART2_New_data_block( uart2_RxBuffer + UART2_RXBUFFERSIZE/2 , UART2_RXBUFFERSIZE/2  );
		DMA_ClearITPendingBit(DMA1_IT_TC6);
	}

//	if (DMA_GetITStatus(DMA1_IT_TE5) == SET)
//	{
////		USART2_New_data_block( uchSCI31_RxBuffer + SCI31_RXBUFFERSIZE/2 , SCI31_RXBUFFERSIZE/2  );
//		DMA_ClearITPendingBit(DMA1_IT_TC5);
//	}
}

// Tx
void DMA1_Channel7_IRQHandler(void) {
//	if (DMA_GetITStatus(DMA1_IT_HT7) == SET)
//	{
//		DMA_ClearITPendingBit(DMA1_IT_HT7);
////		USART2_New_data_block( uart_modem_arr, sizeof(uart_modem_arr)/sizeof(*uart_modem_arr)/2 );
//	}

	if (DMA_GetITStatus(DMA1_IT_TC7) == SET)
	{
		UART2_dmaFree = 1;
		DMA_ClearITPendingBit(DMA1_IT_TC7);
	}
}

inline
void UART2_send_dma(uint8_t* data, size_t length) {

	while(UART2_dmaFree != 1) { asm("nop");}
	UART2_dmaFree = 0;

	DMA1_Channel7->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA1_Channel7->CMAR = (uint32_t) data;
	DMA1_Channel7->CNDTR = length;
	DMA1_Channel7->CCR |= DMA_CCR1_EN;
}


void UART2_send(uint8_t* data, size_t length) {
	uint8_t* dataPtr;
	size_t remaining;
	size_t i = 0;
	if(length > 0) {
		if(length < UART2_TXBUFFERSIZE) {
			while(UART2_dmaFree != 1) { asm("nop");}
			memcpy(uart2_TxBuffer, data, length);
			UART2_send_dma(uart2_TxBuffer, length);
		} else {
			dataPtr = data;
			remaining = length;
			for(i = 0; i <= length; ++i) {
				if( i != 0 && (i % UART2_TXBUFFERSIZE) == 0) {
					UART2_send_dma(dataPtr, (UART2_TXBUFFERSIZE));
					dataPtr += UART2_TXBUFFERSIZE;
					remaining -= UART2_TXBUFFERSIZE;
				}
			}
			if(remaining > 0) {
				UART2_send_dma(dataPtr, remaining);
			}

			while(UART2_dmaFree != 1) { asm("nop");}
		}
	}
}

inline
void UART2_send_byte(uint8_t byte) {
	UART2_send(&byte, 1);
}

uint8_t UART2_read() {
	uint8_t byte;
	while(uart_fifo_is_empty(&uartRxFifo)) { asm("nop");}
	uart_fifo_read_byte(&uartRxFifo, &byte);
	return byte;
}


int UART2_has_bytes() {
	return !uart_fifo_is_empty(&uartRxFifo);
}



