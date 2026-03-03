

#include "bsp.h"
#include "factory_io.h"
#include "main.h"



void BSP_LED_Init()
{
	// Enable GPIOA clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Configure PA5 as output
	GPIOA->MODER &= ~GPIO_MODER_MODER5_Msk;
	GPIOA->MODER |= (0x01 <<GPIO_MODER_MODER5_Pos);

	// Configure PA5 as Push-Pull output
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;

	// Configure PA5 as High-Speed Output
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEEDR5_Msk;
	GPIOA->OSPEEDR |= (0x03 <<GPIO_OSPEEDR_OSPEEDR5_Pos);

	// Disable PA5 Pull-up/Pull-down
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5_Msk;

	// Set Initial State OFF
	GPIOA->BSRR |= GPIO_BSRR_BR_5;
}



void BSP_LED_On()
{
	GPIOA->BSRR = GPIO_BSRR_BS_5;
}



void BSP_LED_Off()
{
	GPIOA->BSRR = GPIO_BSRR_BR_5;
}



void BSP_LED_Toggle()
{
	GPIOA->ODR ^= GPIO_ODR_5;
}




void BSP_PB_Init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	GPIOC->MODER &= ~GPIO_MODER_MODER13_Msk;
	GPIOC->MODER |= (0x00 <<GPIO_MODER_MODER13_Pos);

	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13_Msk;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	SYSCFG->EXTICR[3] &= ~ SYSCFG_EXTICR4_EXTI13_Msk;
	SYSCFG->EXTICR[3] |=   SYSCFG_EXTICR4_EXTI13_PC;

	EXTI->IMR |= EXTI_IMR_IM13;

	EXTI->RTSR &= ~EXTI_RTSR_RT13;
	EXTI->FTSR |=  EXTI_FTSR_FT13;
}



uint8_t BSP_PB_GetState()
{
	uint8_t state;
	if ((GPIOC->IDR & GPIO_IDR_13) == GPIO_IDR_13)
	{
		state = 0;
	}
	else
	{
		state = 1;
	}
	return state;
}

extern uint8_t tx_dma_buffer[7];
extern uint8_t rx_dma_buffer[14];


void BSP_Console_Init()
{
	// Enable GPIOA clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Configure PA2 and PA3 as Alternate function
	GPIOA->MODER &= ~(GPIO_MODER_MODER2_Msk | GPIO_MODER_MODER3_Msk);
	GPIOA->MODER |= (0x02 <<GPIO_MODER_MODER2_Pos) | (0x02 <<GPIO_MODER_MODER3_Pos);

	// Set PA2 and PA3 to AF1 (USART2)
	GPIOA->AFR[0] &= ~(0x0000FF00);
	GPIOA->AFR[0] |=  (0x00001100);

	// Enable USART2 clock
	RCC -> APB1ENR |= RCC_APB1ENR_USART2EN;

	// Clear USART2 configuration (reset state)
	// 8-bit, 1 start, 1 stop, CTS/RTS disabled
	USART2->CR1 = 0x00000000;
	USART2->CR2 = 0x00000000;
	USART2->CR3 = 0x00000000;

	// Select PCLK (APB1) as clock source
	// PCLK -> 48 MHz
	RCC->CFGR3 &= ~RCC_CFGR3_USART2SW_Msk;

	// Baud Rate = 115200
	// With OVER8=1 and Fck=8MHz, USARTDIV = 2*8E6/115200 = 139
	USART2->CR1 |= USART_CR1_OVER8;
	USART2->BRR = 139;

	// Enable both Transmitter and Receiver
	USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

	// ----------------- Setup TX on DMA Channel 4 ---------------------
	// Start DMA clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// Reset DMA1 Channel 4 configuration
	DMA1_Channel4->CCR = 0x00000000;
	// Set direction Memory -> Peripherique
	DMA1_Channel4->CCR |= DMA_CCR_DIR;
	// Peripheral is USART2 TDR
	DMA1_Channel4->CPAR = (uint32_t)&USART2->TDR;
	// Peripheral data size is 8-bit (byte)
	DMA1_Channel4->CCR |= (0x00 <<DMA_CCR_PSIZE_Pos);
	// Disable auto-increment Peripheral address
	DMA1_Channel4->CCR &= ~DMA_CCR_PINC;
	// Memory is tx_dma_buffer
	DMA1_Channel4->CMAR = (uint32_t)tx_dma_buffer;
	// Memory data size is 8-bit (byte)
	DMA1_Channel4->CCR |= (0x00 <<DMA_CCR_MSIZE_Pos);
	// Enable auto-increment Memory address
	DMA1_Channel4->CCR |= DMA_CCR_MINC;

	// DMA mode is one shot
	DMA1_Channel4->CCR &= ~DMA_CCR_CIRC;

	// Enable DMA HT & TC interrupts
	DMA1_Channel4->CCR |= DMA_CCR_TCIE;

	// Disable Overwrite error to be able to spam
	USART2->CR3 |= USART_CR3_OVRDIS;
	// Enable USART2
	USART2->CR1 |= USART_CR1_UE;

	// ***************** SETUP RX ON DMA CHANNEL 5 **************************
	// Reset DMA1 Channel 5 configuration
	DMA1_Channel5->CCR = 0x00000000;

	// Set direction Peripheral -> Memory
	DMA1_Channel5->CCR &= ~DMA_CCR_DIR;

	// Peripheral is USART2 RDR
	DMA1_Channel5->CPAR = (uint32_t)&USART2->RDR;
	// Peripheral data size is 8-bit (byte)
	DMA1_Channel5->CCR |= (0x00 <<DMA_CCR_PSIZE_Pos);
	// Disable auto-increment Peripheral address
	DMA1_Channel5->CCR &= ~DMA_CCR_PINC;

	// Memory is rx_dma_buffer
	DMA1_Channel5->CMAR = (uint32_t)rx_dma_buffer;
	// Memory data size is 8-bit (byte)
	DMA1_Channel5->CCR |= (0x00 <<DMA_CCR_MSIZE_Pos);
	// Enable auto-increment Memory address
	DMA1_Channel5->CCR |= DMA_CCR_MINC;

	// Set Memory Buffer size (TAILLE DE LA TRAME)
	DMA1_Channel5->CNDTR = 14;
	// DMA mode is circular
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;

	// Enable DMA1 Channel 5
	DMA1_Channel5->CCR |= DMA_CCR_EN;
	// Enable USART2 DMA Request on RX
	USART2->CR3 |= USART_CR3_DMAR;
	// Enable USART2
	USART2->CR1 |= USART_CR1_UE;
}




