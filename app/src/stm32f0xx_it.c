#include "main.h"
#include "stm32f0xx_it.h"


void NMI_Handler(void)
{
}


void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
	if ((DMA1->ISR & DMA_ISR_TCIF4) != 0)
    {
        DMA1->IFCR |= DMA_IFCR_CTCIF4;

        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

        vTaskNotifyGiveFromISR(vTask_Write_handle, &xHigherPriorityTaskWoken);

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}
