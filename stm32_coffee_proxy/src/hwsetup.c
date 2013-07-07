
/*  CPU TYPE    :STM32F103                                             */
#include "stm32f10x.h"


#define __EFI_ACR_Val       0x00000012

#define __RCC_CR_VAL        0x01010082
//#define __RCC_CFGR_VAL      0x001D8402  //PLLx9 - 8 MHz
#define __RCC_CFGR_VAL      0x00118402  //PLLx6 - 12 MHz

//#define __HSE               8000000UL
//#define __HSI               8000000UL

//#define __SYSCLK            72000000UL
//#define __HCLK              72000000UL
//#define __PCLK1             36000000UL
//#define __PCLK2             72000000UL

//#define __TIM1CLK           72000000UL
//#define __TIMXCLK           72000000UL


#define __GPIO_USED               0x7F

//0 - analog input, 3 - push/pull output, 4 - floating input
#define __GPIOA_CRL               0x00000000
#define __GPIOA_CRH               0x44444004
#define __GPIOA_ODR               0xffff

#define __GPIOB_CRL               0x33344400
#define __GPIOB_CRH               0x34334343
#define __GPIOB_ODR               0xffff

#define __GPIOC_CRL               0x33000000
#define __GPIOC_CRH               0x34434333
#define __GPIOC_ODR               0xffff

#define __GPIOD_CRL               0x30044400
#define __GPIOD_CRH               0x33334444
#define __GPIOD_ODR               0xffff

#define __GPIOE_CRL               0x33333333
#define __GPIOE_CRH               0x33333333
#define __GPIOE_ODR               0xffff

#define __GPIOF_CRL               0x00333333
#define __GPIOF_CRH               0x44433000
#define __GPIOF_ODR               0xffff

#define __GPIOG_CRL               0x44444444
#define __GPIOG_CRH               0x33344444
#define __GPIOG_ODR               0xffff




void HardwareSetup(void)
{

  // set access control register 
  FLASH->ACR = __EFI_ACR_Val;

//--------------------------------------------------------------------------------------------------

  /* Clock Configuration*/
  RCC->CFGR = __RCC_CFGR_VAL;                        // set clock configuration register
  RCC->CR   = __RCC_CR_VAL;                          // set clock control register

  if (__RCC_CR_VAL & RCC_CR_HSION) {                 // if HSI enabled
    while ((RCC->CR & RCC_CR_HSIRDY) == 0);          // Wait for HSIRDY = 1 (HSI is ready)
  }

  if (__RCC_CR_VAL & RCC_CR_HSEON) {                 // if HSE enabled
    while ((RCC->CR & RCC_CR_HSERDY) == 0);          // Wait for HSERDY = 1 (HSE is ready)
  }

  if (__RCC_CR_VAL & RCC_CR_PLLON) {                 // if PLL enabled
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);          // Wait for PLLRDY = 1 (PLL is ready)
  }

  /* Wait till SYSCLK is stabilized (depending on selected clock) */
  while ((RCC->CFGR & RCC_CFGR_SWS) != ((__RCC_CFGR_VAL<<2) & RCC_CFGR_SWS));

//--------------------------------------------------------------------------------------------------

  if (__GPIO_USED & 0x01) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;            // enable clock for GPIOA
    GPIOA->CRL = __GPIOA_CRL;
    GPIOA->CRH = __GPIOA_CRH;
    GPIOA->ODR = __GPIOA_ODR;
  }

  if (__GPIO_USED & 0x02) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;            // enable clock for GPIOB
    GPIOB->CRL = __GPIOB_CRL;
    GPIOB->CRH = __GPIOB_CRH;
    GPIOB->ODR = __GPIOB_ODR;
  }

  if (__GPIO_USED & 0x04) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;            // enable clock for GPIOC
    GPIOC->CRL = __GPIOC_CRL;
    GPIOC->CRH = __GPIOC_CRH;
    GPIOC->ODR = __GPIOC_ODR;
  }

  if (__GPIO_USED & 0x08) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;            // enable clock for GPIOD
    GPIOD->CRL = __GPIOD_CRL;
    GPIOD->CRH = __GPIOD_CRH;
    GPIOD->ODR = __GPIOD_ODR;
  }

  if (__GPIO_USED & 0x10) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;            // enable clock for GPIOE
    GPIOE->CRL = __GPIOE_CRL;
    GPIOE->CRH = __GPIOE_CRH;
    GPIOE->ODR = __GPIOE_ODR;
  }

  if (__GPIO_USED & 0x20) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;            // enable clock for GPIOF
    GPIOF->CRL = __GPIOF_CRL;
    GPIOF->CRH = __GPIOF_CRH;
    GPIOF->ODR = __GPIOF_ODR;
  }

  if (__GPIO_USED & 0x40) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;            // enable clock for GPIOG
    GPIOG->CRL = __GPIOG_CRL;
    GPIOG->CRH = __GPIOG_CRH;
    GPIOG->ODR = __GPIOG_ODR;
  }
}
