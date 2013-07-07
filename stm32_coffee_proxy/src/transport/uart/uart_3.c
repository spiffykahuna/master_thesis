#include "stm32f10x.h"
#include "uart_3.h"
#include "uart_fifo.h"

uint8_t uart3_RxBuffer[UART3_RXBUFFERSIZE];
uint8_t uart3_TxBuffer[UART3_TXBUFFERSIZE];

static UartFifo  uartRxFifo;
static uint8_t rxFifoBuffer[UART3_RX_FIFO_BUFFER_SIZE];

volatile unsigned char UART3_MsgAvailable;

volatile unsigned char UART3_dmaFree = 0;

void USART3_New_data_block( uint8_t *data, size_t len );

void USART3_IRQHandler(void)
{
  unsigned short int status;

  status=USART3->SR;

  if (status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {		// check errors
    USART_ReceiveData(USART3);
    USART3->SR = 0;
  } else 

  if (status & USART_SR_RXNE) {               // read interrupt
    USART3->SR &= ~USART_SR_RXNE;	          // clear interrupt

    /* we are using dma to receice */
//    if (++usart3_In >= UART3_RXBUFFERSIZE) usart3_In = 0;
//    uart3_RxBuffer[usart3_In] = UART3->DR;
//    UART3_MsgAvailable=1;

  }

  if (status & USART_SR_TXE) {
    USART3->SR &= ~USART_SR_TXE;	          // clear interrupt
  }

  if ( status & USART_FLAG_IDLE )
  	{
  		USART_ReceiveData(USART3);

  		unsigned long cnt = UART3_RXBUFFERSIZE - DMA_GetCurrDataCounter(DMA1_Channel3);
  		if ( cnt != UART3_RXBUFFERSIZE/2 && cnt != UART3_RXBUFFERSIZE )
  		{
  			if ( cnt < UART3_RXBUFFERSIZE/2 )
  			{
  				USART3_New_data_block( uart3_RxBuffer, cnt );
  				DMA_Cmd(DMA1_Channel3, DISABLE);
  				DMA1_Channel3->CNDTR = UART3_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel3, ENABLE);
  			}
  			else
  			{
  				USART3_New_data_block( &uart3_RxBuffer[UART3_RXBUFFERSIZE/2], cnt - UART3_RXBUFFERSIZE/2 );
  				DMA_Cmd(DMA1_Channel3, DISABLE);
  				DMA1_Channel3->CNDTR = UART3_RXBUFFERSIZE;
  				DMA_Cmd(DMA1_Channel3, ENABLE);
  			}
  		}
  		UART3_MsgAvailable = 1;
  	}
}

void UART3_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

	// PD8 - USART3_Tx
	GPIO_InitTypeDef    GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//PB11 - USART3_Rx
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //Input floating
	GPIO_Init(GPIOB, &GPIO_InitStruct);


	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600; // TODO put define
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	//fck = 36 Mhz -> APB1 F_max
//	  	USART3->BRR  = 0x00001D4C; // right 9600 baud rate constant
	USART3->BRR = 0x00000138; // 36 Mhz/115200
//	USART3->BRR  = 0x00000EA6; // 960
	/* Available interrupts from USART3 */
	USART3->CR1 |= (/*USART_CR1_RE | USART_CR1_TE */ /*| USART_CR1_RXNEIE */  USART_CR1_IDLEIE);

	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//TX
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART3->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart3_TxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART3_TXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel2, &DMA_InitStructure);


	//		DMA1_Channel2->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel2->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);

	//RX
	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART3->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart3_RxBuffer;
	DMA_InitStructure.DMA_BufferSize = UART3_RXBUFFERSIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal; //
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	DMA1_Channel3->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
	DMA1_Channel3->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
	//		DMA1_Channel5->CCR |= DMA_CCR1_TEIE; /* enable Transfer complete interrupt  */
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);


	USART3->CR3         |=  USART_CR3_DMAR;          // USART3 DMA receive
	USART3->CR3         |=  USART_CR3_DMAT;          // USART3 DMA transmit


	USART_Cmd(USART3, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE);
	DMA_Cmd(DMA1_Channel2, ENABLE);


	uart_fifo_init(&uartRxFifo, rxFifoBuffer, UART3_RX_FIFO_BUFFER_SIZE);
	UART3_MsgAvailable=0;
}


void USART3_New_data_block( uint8_t *data, size_t len )
{
	uint8_t *end = data + len;
	while(data != end && !uart_fifo_is_full(&uartRxFifo)) {
		uart_fifo_write_byte(&uartRxFifo, data++);
	}
	/* if fifo is full new bytes are ignored */
}



// Rx
void DMA1_Channel3_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_HT3) == SET)
	{

		USART3_New_data_block( uart3_RxBuffer, UART3_RXBUFFERSIZE/2 );
		DMA_ClearITPendingBit(DMA1_IT_HT3);
	}
	if (DMA_GetITStatus(DMA1_IT_TC3) == SET)
	{
		USART3_New_data_block( uart3_RxBuffer + UART3_RXBUFFERSIZE/2 , UART3_RXBUFFERSIZE/2  );
		DMA_ClearITPendingBit(DMA1_IT_TC3);
	}

//	if (DMA_GetITStatus(DMA1_IT_TE5) == SET)
//	{
////		USART3_New_data_block( uchSCI31_RxBuffer + SCI31_RXBUFFERSIZE/2 , SCI31_RXBUFFERSIZE/2  );
//		DMA_ClearITPendingBit(DMA1_IT_TC5);
//	}
}

// Tx
void DMA1_Channel2_IRQHandler(void) {
//	if (DMA_GetITStatus(DMA1_IT_HT2) == SET)
//	{
//		DMA_ClearITPendingBit(DMA1_IT_HT2);
////		USART3_New_data_block( uart_modem_arr, sizeof(uart_modem_arr)/sizeof(*uart_modem_arr)/2 );
//	}

	if (DMA_GetITStatus(DMA1_IT_TC2) == SET)
	{
		UART3_dmaFree = 1;
		DMA_ClearITPendingBit(DMA1_IT_TC2);
	}
}

inline
void UART3_send_dma(uint8_t* data, size_t length) {

	while(UART3_dmaFree != 1) { asm("nop");}
	UART3_dmaFree = 0;

	DMA1_Channel2->CCR &= (uint16_t)(~DMA_CCR1_EN);
	DMA1_Channel2->CMAR = (uint32_t) data;
	DMA1_Channel2->CNDTR = length;
	DMA1_Channel2->CCR |= DMA_CCR1_EN;
}


void UART3_send(uint8_t* data, size_t length) {
	uint8_t* dataPtr;
	size_t remaining;
	size_t i = 0;
	if(length > 0) {
		if(length < UART3_TXBUFFERSIZE) {
			while(UART3_dmaFree != 1) { asm("nop");}
			memcpy(uart3_TxBuffer, data, length);
			UART3_send_dma(uart3_TxBuffer, length);
		} else {
			dataPtr = data;
			remaining = length;
			for(i = 0; i <= length; ++i) {
				if( i != 0 && (i % UART3_TXBUFFERSIZE) == 0) {
					UART3_send_dma(dataPtr, (UART3_TXBUFFERSIZE));
					dataPtr += UART3_TXBUFFERSIZE;
					remaining -= UART3_TXBUFFERSIZE;
				}
			}
			if(remaining > 0) {
				UART3_send_dma(dataPtr, remaining);
			}

			while(UART3_dmaFree != 1) { asm("nop");}
		}
	}
}

inline
void UART3_send_byte(uint8_t byte) {
	UART3_send(&byte, 1);
}

uint8_t UART3_read() {
	uint8_t byte;
	while(uart_fifo_is_empty(&uartRxFifo)) { asm("nop");}
	uart_fifo_read_byte(&uartRxFifo, &byte);
	return byte;
}


int UART3_has_bytes() {
	return !uart_fifo_is_empty(&uartRxFifo);
}

