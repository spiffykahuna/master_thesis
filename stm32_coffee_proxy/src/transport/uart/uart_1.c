#include "stm32f10x.h"
#include "uart_1.h"
#include "uart_fifo.h"

uint8_t uart1_RxBuffer[UART1_RXBUFFERSIZE];
uint8_t uart1_TxBuffer[UART1_TXBUFFERSIZE];

static UartFifo  uartRxFifo;
static uint8_t rxFifoBuffer[UART1_RX_FIFO_BUFFER_SIZE];

volatile unsigned char UART1_MsgAvailable;

volatile unsigned char UART1_dmaFree = 0;

void USART1_New_data_block( uint8_t *data, size_t len );


void USART1_IRQHandler(void)
{
  unsigned short int status;



  status=USART1->SR;

  if (status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {		// check errors
    USART_ReceiveData(USART1);

  } else 

  if (status & USART_SR_RXNE) {               // read interrupt
    USART1->SR &= ~USART_SR_RXNE;	          // clear interrupt

    /* we are using dma to receice */
//    if (++usart1_In >= UART1_RXBUFFERSIZE) usart1_In = 0;
//    uart1_RxBuffer[usart1_In] = USART1->DR;
//    UART1_MsgAvailable=1;

  }

  if (status & USART_SR_TXE) {
    USART1->SR &= ~USART_SR_TXE;	          // clear interrupt
  }

  if ( status & USART_FLAG_IDLE )
  	{
  		USART_ReceiveData(USART1);

  		unsigned long cnt = UART1_RXBUFFERSIZE - DMA_GetCurrDataCounter(DMA1_Channel5);
  		if ( cnt != UART1_RXBUFFERSIZE/2 && cnt != UART1_RXBUFFERSIZE )
  		{
  			if ( cnt < UART1_RXBUFFERSIZE/2 )
  			{
  				USART1_New_data_block( uart1_RxBuffer, cnt );
  				DMA_Cmd(DMA1_Channel5, DISABLE);
  				DMA1_Channel5->CNDTR = UART1_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel5, ENABLE);
  			}
  			else
  			{
  				USART1_New_data_block( &uart1_RxBuffer[UART1_RXBUFFERSIZE/2], cnt - UART1_RXBUFFERSIZE/2 );
  				DMA_Cmd(DMA1_Channel5, DISABLE);
  				DMA1_Channel5->CNDTR = UART1_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel5, ENABLE);
  			}
  		}
  		UART1_MsgAvailable = 1;

#ifdef UART1_MSG_AVAILABLE_CALLBACK
  		UART1_MsgAvailable_Callback();
#endif
  	}
}




void UART1_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	// PA9 - USART1_Tx
	GPIO_InitTypeDef    GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; //Push-Pull
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// PA10 - USART1_Rx
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //Input floating
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	//  	USART1->BRR  = 0x00001D4C; // right 9600 baud rate constant
	USART1->BRR = 0x00000271; //115200

	/* Available interrupts from USART1 */
	USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE /*| USART_CR1_RXNEIE */ | USART_CR1_IDLEIE);

	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart1_TxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART1_TXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel4, &DMA_InitStructure);


	//		DMA1_Channel4->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel4->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);

	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart1_RxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART1_RXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal; //
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	DMA1_Channel5->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel5->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	//		DMA1_Channel5->CCR |= DMA_CCR1_TEIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);


	USART1->CR3         |=  USART_CR3_DMAR;          // USART1 DMA receive
	USART1->CR3         |=  USART_CR3_DMAT;          // USART1 DMA transmit


	USART_Cmd(USART1, ENABLE);
	DMA_Cmd(DMA1_Channel4, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);


	uart_fifo_init(&uartRxFifo, rxFifoBuffer, UART1_RX_FIFO_BUFFER_SIZE);
	UART1_MsgAvailable=0;
}


void USART1_New_data_block( uint8_t *data, size_t len )
{
	uint8_t *end = data + len;
	while(data != end && !uart_fifo_is_full(&uartRxFifo)) {
		uart_fifo_write_byte(&uartRxFifo, data++);
	}
	/* if fifo is full new bytes are ignored */
}



// Rx
void DMA1_Channel5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_HT5) == SET)
	{

		USART1_New_data_block( uart1_RxBuffer, UART1_RXBUFFERSIZE/2 );
		DMA_ClearITPendingBit(DMA1_IT_HT5);
	}
	if (DMA_GetITStatus(DMA1_IT_TC5) == SET)
	{
		USART1_New_data_block( uart1_RxBuffer + UART1_RXBUFFERSIZE/2 , UART1_RXBUFFERSIZE/2  );
		DMA_ClearITPendingBit(DMA1_IT_TC5);
	}

//	if (DMA_GetITStatus(DMA1_IT_TE5) == SET)
//	{
////		USART1_New_data_block( uchSCI31_RxBuffer + SCI31_RXBUFFERSIZE/2 , SCI31_RXBUFFERSIZE/2  );
//		DMA_ClearITPendingBit(DMA1_IT_TC5);
//	}
}

// Tx
void DMA1_Channel4_IRQHandler(void) {
//	if (DMA_GetITStatus(DMA1_IT_HT4) == SET)
//	{
//		DMA_ClearITPendingBit(DMA1_IT_HT4);
////		USART1_New_data_block( uart_modem_arr, sizeof(uart_modem_arr)/sizeof(*uart_modem_arr)/2 );
//	}

	if (DMA_GetITStatus(DMA1_IT_TC4) == SET)
	{
		UART1_dmaFree = 1;
		DMA_ClearITPendingBit(DMA1_IT_TC4);
	}
}

inline
void UART1_send_dma(uint8_t* data, size_t length) {

	while(UART1_dmaFree != 1) { asm("nop");}
	UART1_dmaFree = 0;

	DMA1_Channel4->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA1_Channel4->CMAR = (uint32_t) data;
	DMA1_Channel4->CNDTR = length;
	DMA1_Channel4->CCR |= DMA_CCR1_EN;
}


void UART1_send(uint8_t* data, size_t length) {
	uint8_t* dataPtr;
	size_t remaining;
	size_t i = 0;
	if(length > 0) {
		if(length < UART1_TXBUFFERSIZE) {
			while(UART1_dmaFree != 1) { asm("nop");}
			memcpy(uart1_TxBuffer, data, length);
			UART1_send_dma(uart1_TxBuffer, length);
		} else {
			dataPtr = data;
			remaining = length;
			for(i = 0; i <= length; ++i) {
				if( i != 0 && (i % UART1_TXBUFFERSIZE) == 0) {
					UART1_send_dma(dataPtr, (UART1_TXBUFFERSIZE));
					dataPtr += UART1_TXBUFFERSIZE;
					remaining -= UART1_TXBUFFERSIZE;
				}
			}
			if(remaining > 0) {
				UART1_send_dma(dataPtr, remaining);
			}

			while(UART1_dmaFree != 1) { asm("nop");}
		}
	}
}

inline
void UART1_send_byte(uint8_t byte) {
	UART1_send(&byte, 1);
}

inline
int UART1_send_chars(char* data, size_t length) {
	UART1_send((uint8_t*) data, length);
	return 1;
}

uint8_t UART1_read() {
	uint8_t byte;
	while(uart_fifo_is_empty(&uartRxFifo)) { asm("nop");}
	uart_fifo_read_byte(&uartRxFifo, &byte);
	return byte;
}

int UART1_has_bytes() {
	return !uart_fifo_is_empty(&uartRxFifo);
}
