#ifndef BSP_INC_FACTORY_IO_H_
#define BSP_INC_FACTORY_IO_H_

#include "stm32f0xx.h"
#include "FreeRTOS.h"
#include "queue.h"

#define FRAME_LENGTH        14
#define FRAME_LENGTH_ACT    7
#define FRAME_LENGTH_SENS   7

#define TAG_SENSORS         0xA8
#define TAG_ACTUATORS       0xAD
#define TAG_UPDATE          0xA3


typedef uint8_t command_message_t[7];

// --- Variables Globales ---
extern uint8_t rx_dma_buffer[FRAME_LENGTH];


void FACTORY_IO_SetActuator(uint8_t actuator_id, uint8_t state);

uint8_t FACTORY_IO_Sensors_Get(uint32_t msk);
void FACTORY_IO_update(void);

#endif /* BSP_INC_FACTORY_IO_H_ */
