#include "stm32f10x.h"
#include "uart_4.h"
#include "uart_fifo.h"

uint8_t uart4_RxBuffer[UART4_RXBUFFERSIZE];
uint8_t uart4_TxBuffer[UART4_TXBUFFERSIZE];

static UartFifo  uartRxFifo;
static uint8_t rxFifoBuffer[UART4_RX_FIFO_BUFFER_SIZE];

volatile unsigned char UART4_MsgAvailable;

volatile unsigned char UART4_dmaFree = 0;

void UART4_New_data_block( uint8_t *data, size_t len );

void UART4_IRQHandler(void)
{
  unsigned short int status;

  status=UART4->SR;

  if (status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {		// check errors
    USART_ReceiveData(UART4);
    UART4->SR = 0;
  } else 

  if (status & USART_SR_RXNE) {               // read interrupt
    UART4->SR &= ~USART_SR_RXNE;	          // clear interrupt

    /* we are using dma to receice */
//    if (++uart4_In >= UART4_RXBUFFERSIZE) uart4_In = 0;
//    uart4_RxBuffer[uart4_In] = UART4->DR;
//    UART4_MsgAvailable=1;
  }

  if (status & USART_SR_TXE) {
    UART4->SR &= ~USART_SR_TXE;	          // clear interrupt
  }

  if ( status & USART_FLAG_IDLE )
  	{
  		USART_ReceiveData(UART4);
  		UART4->SR &= ~USART_SR_IDLE;

  		unsigned long cnt = UART4_RXBUFFERSIZE - DMA_GetCurrDataCounter(DMA2_Channel3);
  		if ( cnt != UART4_RXBUFFERSIZE/2 && cnt != UART4_RXBUFFERSIZE )
  		{
  			if ( cnt < UART4_RXBUFFERSIZE/2 )
  			{
  				UART4_New_data_block( uart4_RxBuffer, cnt );
  				DMA_Cmd(DMA2_Channel3, DISABLE);
  				DMA2_Channel3->CNDTR = UART4_RXBUFFERSIZE;
  				DMA_Cmd(DMA2_Channel3, ENABLE);
  			}
  			else
  			{
  				UART4_New_data_block( &uart4_RxBuffer[UART4_RXBUFFERSIZE/2], cnt - UART4_RXBUFFERSIZE/2 );
  				DMA_Cmd(DMA2_Channel3, DISABLE);
  				DMA2_Channel3->CNDTR = UART4_RXBUFFERSIZE;
  				DMA_Cmd(DMA2_Channel3, ENABLE);
  			}
  		}
  		UART4_MsgAvailable = 1;
  	}
}




void UART4_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap_UART4, ENABLE);

	// PC10 - UART4_Tx
	GPIO_InitTypeDef    GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	//PC11 - UART4_Rx
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //Input floating
	GPIO_Init(GPIOC, &GPIO_InitStruct);


	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200; // TODO put define
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	//fck = 36 Mhz -> APB1 F_max
//	  	UART4->BRR  = 0x00001D4C; // right 9600 baud rate constant
	UART4->BRR = 0x00000138; // 36 Mhz/115200
//	UART4->BRR  = 0x00000EA6; // 960
	/* Available interrupts from UART4 */
	UART4->CR1 |= (/*USART_CR1_RE | USART_CR1_TE */ /*| USART_CR1_RXNEIE */  USART_CR1_IDLEIE);

	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	//TX
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA2_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart4_TxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART4_TXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA2_Channel5, &DMA_InitStructure);


	//		DMA2_Channel5->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA2_Channel5->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);

	//RX
	DMA_DeInit(DMA2_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart4_RxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART4_RXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal; //
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA2_Channel3, &DMA_InitStructure);

	DMA2_Channel3->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA2_Channel3->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	//		DMA1_Channel5->CCR |= DMA_CCR1_TEIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA2_Channel3_IRQn);


	UART4->CR3         |=  USART_CR3_DMAR;          // UART4 DMA receive
	UART4->CR3         |=  USART_CR3_DMAT;          // UART4 DMA transmit


	USART_Cmd(UART4, ENABLE);
	DMA_Cmd(DMA2_Channel3, ENABLE);
	DMA_Cmd(DMA2_Channel5, ENABLE);


	uart_fifo_init(&uartRxFifo, rxFifoBuffer, UART4_RX_FIFO_BUFFER_SIZE);
	UART4_MsgAvailable=0;
}


void UART4_New_data_block( uint8_t *data, size_t len )
{
	uint8_t *end = data + len;
	while(data != end && !uart_fifo_is_full(&uartRxFifo)) {
		uart_fifo_write_byte(&uartRxFifo, data++);
	}
	/* if fifo is full new bytes are ignored */
}



// Rx
void DMA2_Channel3_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_IT_HT3) == SET)
	{

		UART4_New_data_block( uart4_RxBuffer, UART4_RXBUFFERSIZE/2 );
		DMA_ClearITPendingBit(DMA2_IT_HT3);
	}
	if (DMA_GetITStatus(DMA2_IT_TC3) == SET)
	{
		UART4_New_data_block( uart4_RxBuffer + UART4_RXBUFFERSIZE/2 , UART4_RXBUFFERSIZE/2  );
		DMA_ClearITPendingBit(DMA2_IT_TC3);
	}

//	if (DMA_GetITStatus(DMA2_IT_TE4) == SET)
//	{
////		UART4_New_data_block( uchSCI31_RxBuffer + SCI31_RXBUFFERSIZE/2 , SCI31_RXBUFFERSIZE/2  );
//		DMA_ClearITPendingBit(DMA2_IT_TC4);
//	}
}

// Tx
void DMA2_Channel4_5_IRQHandler(void) {
//	if (DMA_GetITStatus(DMA1_IT_HT2) == SET)
//	{
//		DMA_ClearITPendingBit(DMA1_IT_HT2);
////		UART4_New_data_block( uart_modem_arr, sizeof(uart_modem_arr)/sizeof(*uart_modem_arr)/2 );
//	}

	if (DMA_GetITStatus(DMA2_IT_TC5) == SET)
	{
		UART4_dmaFree = 1;
		DMA_ClearITPendingBit(DMA2_IT_TC5);
	}
}

inline
void UART4_send_dma(uint8_t* data, size_t length) {

	while(UART4_dmaFree != 1) { asm("nop");}
	UART4_dmaFree = 0;

	DMA2_Channel5->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA2_Channel5->CMAR = (uint32_t) data;
	DMA2_Channel5->CNDTR = length;
	DMA2_Channel5->CCR |= DMA_CCR1_EN;
}

void UART4_send(uint8_t* data, size_t length) {
	uint8_t* dataPtr;
	size_t remaining;
	size_t i = 0;
	if(length > 0) {
		if(length < UART4_TXBUFFERSIZE) {
			while(UART4_dmaFree != 1) { asm("nop");}
			memcpy(uart4_TxBuffer, data, length);
			UART4_send_dma(uart4_TxBuffer, length);
		} else {
			dataPtr = data;
			remaining = length;
			for(i = 0; i <= length; ++i) {
				if( i != 0 && (i % UART4_TXBUFFERSIZE) == 0) {
					UART4_send_dma(dataPtr, (UART4_TXBUFFERSIZE));
					dataPtr += UART4_TXBUFFERSIZE;
					remaining -= UART4_TXBUFFERSIZE;
				}
			}
			if(remaining > 0) {
				UART4_send_dma(dataPtr, remaining);
			}

			while(UART4_dmaFree != 1) { asm("nop");}
		}
	}
}

inline
void UART4_send_byte(uint8_t byte) {
	UART4_send(&byte, 1);
}

uint8_t UART4_read() {
	uint8_t byte;
	while(uart_fifo_is_empty(&uartRxFifo)) { asm("nop");}
	uart_fifo_read_byte(&uartRxFifo, &byte);
	return byte;
}


int UART4_has_bytes() {
	return !uart_fifo_is_empty(&uartRxFifo);
}
