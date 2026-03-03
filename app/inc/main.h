#ifndef INC_MAIN_H_
#define INC_MAIN_H_

#include "stm32f0xx.h"

#include "bsp.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "stream_buffer.h"


typedef enum {
    S_CARTON_DISTRIBUE      = 0,
    S_CARTON_ENVOYE         = 1,
    S_PAL_PRESENCE_ENTREE   = 2,
    S_POUSSOIR_BUTTEE       = 4,
    S_ASC_RDC               = 6,
    S_ASC_NIV1              = 7,
    S_PAL_SORTIE            = 9,
    S_PAL_ENTREE            = 12
} Sensor_Index_t;


typedef uint8_t command_message_t[7];


typedef struct {
    xSemaphoreHandle Sem;
    uint8_t sensor_idx;
    uint8_t target_state;
} subscribe_message_t;


int my_printf (const char *format, ...);
int my_sprintf (char *out, const char *format, ...);

void Wait_Sensor(xSemaphoreHandle sem, uint8_t sensor_idx, uint8_t state);

extern xTaskHandle vTask_Write_handle;
extern xTaskHandle vTask_chargement_carton_handle;
extern xTaskHandle vTask_ascenseur_handle;
extern xTaskHandle vTask_chargement_palette_handle;

#endif /* INC_MAIN_H_ */
