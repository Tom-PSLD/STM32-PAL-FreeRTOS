#include "main.h"
#include "factory_io.h"
#include "trcRecorder.h"


#define SIZE_SUBSCRIPTIONS 12

uint8_t tx_dma_buffer[7];
extern uint8_t rx_dma_buffer[14];

volatile uint8_t system_started = 1;

xQueueHandle xWriteQueue;
xQueueHandle xReadQueue;
xSemaphoreHandle xSem1, xSem2, xSem3, xSem4;
xSemaphoreHandle xSemStartStop;
xSemaphoreHandle xSem_LoadingReady;

xTaskHandle vTask_Write_handle;
xTaskHandle vTask_chargement_carton_handle;
xTaskHandle vTask_ascenseur_handle;
xTaskHandle vTask_chargement_palette_handle;

static void SystemClock_Config(void);
void vTask_Conv_carton(void *pvParameters);
void vTask_chargement_carton(void *pvParameters);
void vTask_ascenseur(void *pvParameters);
void vTask_chargement_palette(void *pvParameters);
void vTask_Write(void *pvParameters);
void vTask_Read(void *pvParameters);



void Wait_Sensor(xSemaphoreHandle sem, uint8_t sensor_idx, uint8_t state) {
    subscribe_message_t subscription;
    subscription.Sem = sem;
    subscription.sensor_idx = sensor_idx;
    subscription.target_state = state;

    xQueueSendToBack(xReadQueue, &subscription, 10);
    xSemaphoreTake(sem, portMAX_DELAY);
}


int main() {
    SystemClock_Config();
    BSP_LED_Init();
    BSP_PB_Init();
    BSP_Console_Init();

    BSP_LED_On();

    vTraceEnable(TRC_START);

    xSem1 = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSem1, "xSem1_Capteurs");

    xSem2 = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSem2, "xSem2_Poussoir");

    xSem3 = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSem3, "xSem3_Ascenseur");

    xSem4 = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSem4, "xSem4_Palette");

    xSemStartStop = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSemStartStop, "xSem_StartStop");
    xSemaphoreGive(xSemStartStop);

    xSem_LoadingReady = xSemaphoreCreateBinary();
    vTraceSetSemaphoreName(xSem_LoadingReady, "xSem_Jeton_Chargeur");
    xSemaphoreGive(xSem_LoadingReady);

    xWriteQueue = xQueueCreate(16, sizeof(command_message_t));
    vTraceSetQueueName(xWriteQueue, "Queue_Commandes");

    xReadQueue = xQueueCreate(8, sizeof(subscribe_message_t));
    vTraceSetQueueName(xReadQueue, "Queue_Abonnements");

    xTaskCreate(vTask_Write,              "Task_Write",   128, NULL, 5, &vTask_Write_handle);
    xTaskCreate(vTask_Read,               "Task_Read",    148, NULL, 6, NULL);
    xTaskCreate(vTask_chargement_palette, "Task_Pal",     128, NULL, 4, &vTask_chargement_palette_handle);
    xTaskCreate(vTask_Conv_carton,        "Task_Conv",    128, NULL, 3, NULL);
    xTaskCreate(vTask_chargement_carton,  "Task_carton",  128, NULL, 3, &vTask_chargement_carton_handle);
    xTaskCreate(vTask_ascenseur,          "Task_Elev",    128, NULL, 3, &vTask_ascenseur_handle);

    vTaskStartScheduler();

    while (1) {}
}


void vTask_Conv_carton(void *pvParameters) {
    while (1) {
        FACTORY_IO_SetActuator(0, 1);
        Wait_Sensor(xSem1, S_CARTON_DISTRIBUE, 0);

        FACTORY_IO_SetActuator(1, 1);
        FACTORY_IO_SetActuator(11, 1);

        Wait_Sensor(xSem1, S_CARTON_ENVOYE, 0);
        Wait_Sensor(xSem1, S_CARTON_DISTRIBUE, 0);

        FACTORY_IO_SetActuator(0, 0);

        Wait_Sensor(xSem1, S_PAL_PRESENCE_ENTREE, 0);
        vTaskDelay(1000);
        Wait_Sensor(xSem1, S_PAL_PRESENCE_ENTREE, 0);

        xSemaphoreTake(xSem_LoadingReady, portMAX_DELAY);
        xTaskNotifyGive(vTask_chargement_carton_handle);
        vTaskDelay(200);
    }
}


void vTask_chargement_carton(void *pvParameters) {
    uint8_t count = 0;
    FACTORY_IO_SetActuator(2, 1);
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        FACTORY_IO_SetActuator(2, 0);
        FACTORY_IO_SetActuator(14, 1);
        vTaskDelay(2100);
        FACTORY_IO_SetActuator(14, 0);
        FACTORY_IO_SetActuator(2, 1);
        FACTORY_IO_SetActuator(4, 1);
        vTaskDelay(500);

        Wait_Sensor(xSem2, S_POUSSOIR_BUTTEE, 1);

        FACTORY_IO_SetActuator(4, 0);
        xSemaphoreGive(xSem_LoadingReady);
        count++;
        if (count == 3)
        { count = 0; xTaskNotifyGive(vTask_ascenseur_handle); }
    }
}


void vTask_ascenseur(void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        FACTORY_IO_SetActuator(5, 1);
        vTaskDelay(1000);
        FACTORY_IO_SetActuator(3, 1);
        vTaskDelay(1000);
        FACTORY_IO_SetActuator(5, 0);
        vTaskDelay(500);
        FACTORY_IO_SetActuator(7, 1);

        Wait_Sensor(xSem3, S_ASC_NIV1, 0);

        FACTORY_IO_SetActuator(7, 0);
        vTaskDelay(500);
        FACTORY_IO_SetActuator(3, 0);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        FACTORY_IO_SetActuator(5, 1);
        vTaskDelay(1000); FACTORY_IO_SetActuator(3, 1);
        vTaskDelay(1000);
        FACTORY_IO_SetActuator(5, 0);
        vTaskDelay(500);
        FACTORY_IO_SetActuator(7, 1);
        FACTORY_IO_SetActuator(8, 1);

        Wait_Sensor(xSem3, S_ASC_RDC, 1);

        FACTORY_IO_SetActuator(8, 0);
        FACTORY_IO_SetActuator(7, 0);
        FACTORY_IO_SetActuator(3, 0);
        xTaskNotifyGive(vTask_chargement_palette_handle);
    }
}

void vTask_chargement_palette(void *pvParameters) {
    while (1) {
        Wait_Sensor(xSem4, S_PAL_SORTIE, 0);

        FACTORY_IO_SetActuator(9, 1);
        FACTORY_IO_SetActuator(17, 1);
        vTaskDelay(500);
        FACTORY_IO_SetActuator(9, 0);
        FACTORY_IO_SetActuator(16, 1);

        Wait_Sensor(xSem4, S_PAL_ENTREE, 1);

        FACTORY_IO_SetActuator(10, 1);

        Wait_Sensor(xSem4, S_PAL_SORTIE, 1);

        FACTORY_IO_SetActuator(10, 0);
        FACTORY_IO_SetActuator(18, 0);
        FACTORY_IO_SetActuator(19, 0);
        FACTORY_IO_SetActuator(6, 1);
        FACTORY_IO_SetActuator(8, 1);

        Wait_Sensor(xSem4, S_ASC_NIV1, 1);

        FACTORY_IO_SetActuator(8, 0);
        FACTORY_IO_SetActuator(6, 0);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        FACTORY_IO_SetActuator(10, 1);
        FACTORY_IO_SetActuator(18, 1);

        Wait_Sensor(xSem4, S_PAL_SORTIE, 0);

        FACTORY_IO_SetActuator(10, 0);
        FACTORY_IO_SetActuator(19, 1);
    }
}


void vTask_Write(void *pvParameters) {
    NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 3);
    NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
    command_message_t cmd_msg;
    uint32_t n;
    while (1) {
        xQueueReceive(xWriteQueue, &cmd_msg, portMAX_DELAY);
        for (n = 0; n < 7; n++) tx_dma_buffer[n] = cmd_msg[n];
            DMA1_Channel4->CNDTR = 7;
            DMA1_Channel4->CCR |= DMA_CCR_EN;
            USART2->CR3 |= USART_CR3_DMAT;
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            DMA1_Channel4->CCR &= ~DMA_CCR_EN;
            USART2->CR3 &= ~USART_CR3_DMAT;
    }
}

void vTask_Read(void *pvParameters) {
    portTickType xLastWakeTime = xTaskGetTickCount();
    subscribe_message_t subscription;
    uint8_t sensors[28] = {0};
    subscribe_message_t subscriptions[SIZE_SUBSCRIPTIONS];

    for (int i = 0; i < SIZE_SUBSCRIPTIONS; i++) {
        subscriptions[i].Sem = NULL;
        subscriptions[i].sensor_idx = 0;
        subscriptions[i].target_state = 0;
    }

    while (1) {
        while (xQueueReceive(xReadQueue, &subscription, 0)) {
            for (int i = 0; i < SIZE_SUBSCRIPTIONS; i++) {
                if (subscriptions[i].Sem == NULL) {
                    subscriptions[i] = subscription;
                    break;
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            uint8_t data = rx_dma_buffer[i + 1];
            for (int j = 0; j < 7; j++) {
                if ((i * 7 + j) < 28) sensors[i * 7 + j] = (data >> j) & 0x01;
            }
        }

        for (int i = 0; i < SIZE_SUBSCRIPTIONS; i++) {
            if (subscriptions[i].Sem != NULL) {

                uint8_t idx = subscriptions[i].sensor_idx;
                uint8_t expected = subscriptions[i].target_state;

                if (sensors[idx] == expected) {
                    xSemaphoreGive(subscriptions[i].Sem);
                    subscriptions[i].Sem = NULL;
                }
            }
        }
        vTaskDelayUntil(&xLastWakeTime, 10);
    }
}


static void SystemClock_Config()
{
    uint32_t HSE_Status;
    uint32_t PLL_Status;
    uint32_t SW_Status;
    uint32_t timeout = 0;
    timeout = 1000000;
    RCC->CR |= RCC_CR_HSEBYP;
    RCC->CR |= RCC_CR_HSEON;
    do
    {
        HSE_Status = RCC->CR & RCC_CR_HSERDY_Msk;
        timeout--;
    } while ((HSE_Status == 0) && (timeout > 0));
    RCC->CFGR &= ~RCC_CFGR_PLLSRC_Msk;
    RCC->CFGR |= (0x02 << RCC_CFGR_PLLSRC_Pos);
    RCC->CFGR2 = 0x00000000;
    RCC->CFGR &= ~RCC_CFGR_PLLMUL_Msk;
    RCC->CFGR |= (0x00 << RCC_CFGR_PLLMUL_Pos);
    RCC->CR |= RCC_CR_PLLON;
    do
    {
        PLL_Status = RCC->CR & RCC_CR_PLLRDY_Msk;
        timeout--;
    } while ((PLL_Status == 0) && (timeout > 0));
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV2;
    RCC->CFGR &= ~RCC_CFGR_PPRE_Msk;
    RCC->CFGR |= RCC_CFGR_PPRE_DIV1;
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    do
    {
        SW_Status = (RCC->CFGR & RCC_CFGR_SWS_Msk);
        timeout--;
    } while ((SW_Status != RCC_CFGR_SWS_PLL) && (timeout > 0));
    SystemCoreClockUpdate();
}






