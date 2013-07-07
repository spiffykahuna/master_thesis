#include "stm32f10x.h"
#include "uart_5.h"
#include "uart_fifo.h"

uint8_t uart5_RxBuffer[UART5_RXBUFFERSIZE];
uint8_t uart5_TxBuffer[UART5_TXBUFFERSIZE];

static volatile size_t txAmount;
static volatile size_t txPointer;

static UartFifo  uartRxFifo;
static uint8_t rxFifoBuffer[UART5_RX_FIFO_BUFFER_SIZE];

volatile unsigned char UART5_MsgAvailable;

volatile unsigned char UART5_txFree = 1;

//void UART5_New_data_block( uint8_t *data, size_t len );
//void UART5_send_ISR();

void UART5_IRQHandler(void)
{
  unsigned short int status;

  uint8_t txByte;

  status=UART5->SR;

  if (status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) {		// check errors
    USART_ReceiveData(UART5);
    UART5->SR = 0;
  } else 

  if (status & USART_SR_RXNE) {               // read interrupt
    UART5->SR &= ~USART_SR_RXNE;	          // clear interrupt

    if(!uart_fifo_is_full(&uartRxFifo)) {
    	txByte = USART_ReceiveData(UART5);
		uart_fifo_write_byte(&uartRxFifo, &txByte);
    }

  }



  if (status & USART_SR_TXE) {
    UART5->SR &= ~USART_SR_TXE;	          // clear interrupt
  }

//  if ( status & USART_FLAG_IDLE )
//  	{
////  		USART_ReceiveData(UART5);
//  		UART5->SR &= ~USART_SR_IDLE;
////
////  		unsigned long cnt = UART5_RXBUFFERSIZE - DMA_GetCurrDataCounter(DMA2_Channel3);
////  		if ( cnt != UART5_RXBUFFERSIZE/2 && cnt != UART5_RXBUFFERSIZE )
////  		{
////  			if ( cnt < UART5_RXBUFFERSIZE/2 )
////  			{
////  				UART5_New_data_block( uart5_RxBuffer, cnt );
////  				DMA_Cmd(DMA2_Channel3, DISABLE);
////  				DMA2_Channel3->CNDTR = UART5_RXBUFFERSIZE;
////  				DMA_Cmd(DMA2_Channel3, ENABLE);
////  			}
////  			else
////  			{
////  				UART5_New_data_block( &uart5_RxBuffer[UART5_RXBUFFERSIZE/2], cnt - UART5_RXBUFFERSIZE/2 );
////  				DMA_Cmd(DMA2_Channel3, DISABLE);
////  				DMA2_Channel3->CNDTR = UART5_RXBUFFERSIZE;
////  				DMA_Cmd(DMA2_Channel3, ENABLE);
////  			}
////  		}
//  		UART5_MsgAvailable = 1;
//  	}
}




void UART5_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap_UART5, ENABLE);

	// PC12 - UART5_Tx
	GPIO_InitTypeDef    GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	//PD2 - UART5_Rx
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //Input floating
	GPIO_Init(GPIOD, &GPIO_InitStruct);


	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200; // TODO put define
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);

	//fck = 36 Mhz -> APB1 F_max
//	  	UART5->BRR  = 0x00001D4C; // right 9600 baud rate constant
	UART5->BRR = 0x00000138; // 36 Mhz/115200
//	UART5->BRR  = 0x00000EA6; // 960
	/* Available interrupts from UART5 */
	UART5->CR1 |= (USART_CR1_RE | USART_CR1_TE  | USART_CR1_RXNEIE  /*| USART_CR1_TXEIE*/  /*|  USART_CR1_IDLEIE */);

//	/* DMA clock enable */
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
//
//	//TX
//	DMA_InitTypeDef DMA_InitStructure;
//	DMA_DeInit(DMA2_Channel5);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &UART5->DR;
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart5_TxBuffer;
//	DMA_InitStructure.DMA_BufferSize = UART5_TXBUFFERSIZE;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//
//	DMA_Init(DMA2_Channel5, &DMA_InitStructure);
//
//
//	//		DMA2_Channel5->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
//	DMA2_Channel5->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
//	NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);
//
//	//RX
//	DMA_DeInit(DMA2_Channel3);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &UART5->DR;
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart5_RxBuffer;
//	DMA_InitStructure.DMA_BufferSize = UART5_RXBUFFERSIZE;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal; //
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//
//	DMA_Init(DMA2_Channel3, &DMA_InitStructure);
//
//	DMA2_Channel3->CCR |= DMA_CCR1_HTIE; /* enable Half traisfer interrupt  */
//	DMA2_Channel3->CCR |= DMA_CCR1_TCIE; /* enable Transfer complete interrupt  */
//	//		DMA1_Channel5->CCR |= DMA_CCR1_TEIE; /* enable Transfer complete interrupt  */
//	NVIC_EnableIRQ(DMA2_Channel3_IRQn);
//
//
//	UART5->CR3         |=  USART_CR3_DMAR;          // UART5 DMA receive
//	UART5->CR3         |=  USART_CR3_DMAT;          // UART5 DMA transmit


	USART_Cmd(UART5, ENABLE);
//	DMA_Cmd(DMA2_Channel3, ENABLE);
//	DMA_Cmd(DMA2_Channel5, ENABLE);

//	 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
//	//  AFIO->MAPR   &= ~(1 << 2);                              // no USART5 remap
//
//	  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;                     // enable clock for GPIOC
//	  RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;                     // enable clock for GPIOD
//
//	  GPIOC->CRH   &= ~(0x0FUL  << 16);                        // Clear PC12
//	  GPIOC->CRH   |=  (0x0BUL  << 16);                        // USART5 Tx (PC12)  alternate output push-pull
//
//	  GPIOD->CRL   &= ~(0x0FUL  << 8);                        // Clear PD2
//	  GPIOD->CRL   |=  (0x04UL  << 8);                        // USART5 Rx (PD2) input floating
//
//	  RCC->APB1ENR |= RCC_APB1ENR_UART5EN;                   // enable clock for UART5
//
//	  UART5->BRR  = 0x00000EA6;                              // set baudrate
//	  UART5->CR1  = 0x00000000;                              // set 8bits, no parity
//	  UART5->CR2  = 0x00000000;                              // set Stop bits
//	  UART5->CR3  = 0x00000000;                              // Set Flow Control
//
//	  UART5->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE /*| USART_CR1_TXEIE */);       // RX, TX enable
//
//	  NVIC->ISER[1] |= (1 << (UART5_IRQn & 0x1F));           // enable interrupt

//	  UART5->CR1 |= USART_CR1_UE;                            // USART enable


	uart_fifo_init(&uartRxFifo, rxFifoBuffer, UART5_RX_FIFO_BUFFER_SIZE);
	UART5_MsgAvailable=0;

}


//void UART5_New_data_block( uint8_t *data, size_t len )
//{
//	uint8_t *end = data + len;
//	while(data != end && !uart_fifo_is_full(&uartRxFifo)) {
//		uart_fifo_write_byte(&uartRxFifo, data++);
//	}
//	/* if fifo is full new bytes are ignored */
//}



//// Rx
//void DMA2_Channel3_IRQHandler(void)
//{
//	if (DMA_GetITStatus(DMA2_IT_HT3) == SET)
//	{
//
//		UART5_New_data_block( uart5_RxBuffer, UART5_RXBUFFERSIZE/2 );
//		DMA_ClearITPendingBit(DMA2_IT_HT3);
//	}
//	if (DMA_GetITStatus(DMA2_IT_TC3) == SET)
//	{
//		UART5_New_data_block( uart5_RxBuffer + UART5_RXBUFFERSIZE/2 , UART5_RXBUFFERSIZE/2  );
//		DMA_ClearITPendingBit(DMA2_IT_TC3);
//	}
//
////	if (DMA_GetITStatus(DMA2_IT_TE4) == SET)
////	{
//////		UART5_New_data_block( uchSCI31_RxBuffer + SCI31_RXBUFFERSIZE/2 , SCI31_RXBUFFERSIZE/2  );
////		DMA_ClearITPendingBit(DMA2_IT_TC4);
////	}
//}
//
//// Tx
//void DMA2_Channel4_5_IRQHandler(void) {
////	if (DMA_GetITStatus(DMA1_IT_HT2) == SET)
////	{
////		DMA_ClearITPendingBit(DMA1_IT_HT2);
//////		UART5_New_data_block( uart_modem_arr, sizeof(uart_modem_arr)/sizeof(*uart_modem_arr)/2 );
////	}
//
//	if (DMA_GetITStatus(DMA2_IT_TC5) == SET)
//	{
//		UART5_dmaFree = 1;
//		DMA_ClearITPendingBit(DMA2_IT_TC5);
//	}
//}
//

//void UART5_send_ISR() {
//
//	if(txAmount > 0) {
//		USART_SendData(UART5, uart5_TxBuffer[txPointer % UART5_TXBUFFERSIZE]);
//		++txPointer;
//		--txAmount;
//	} else  {
//		UART5_txFree = 1;
//	}
//
//
//
//}


inline
void UART5_send_async(uint8_t* data, size_t length) {

	txAmount = length % UART5_TXBUFFERSIZE;
	txPointer = 0;
	while(txAmount--) {
		UART5_send_byte(uart5_TxBuffer[txPointer % UART5_TXBUFFERSIZE]);
		++txPointer;
	}
}


void UART5_send(uint8_t* data, size_t length) {
	uint8_t* dataPtr;
	if(length > 0) {
		txAmount = length % UART5_TXBUFFERSIZE;
		dataPtr = data;
		while(txAmount--) {
			UART5_send_byte(*dataPtr++);
			++txPointer;
		}
	}
}

inline
void UART5_send_byte(uint8_t byte) {
//	 while (!(UART5->SR & USART_SR_TXE));
//	 UART5->DR = (byte & 0x1FF);
	 USART_SendData(UART5, byte);
	 while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET) {}

}

uint8_t UART5_read() {
	uint8_t byte;
	while(uart_fifo_is_empty(&uartRxFifo)) { asm("nop");}
	uart_fifo_read_byte(&uartRxFifo, &byte);
	return byte;
}


int UART5_has_bytes() {
	return !uart_fifo_is_empty(&uartRxFifo);
}



