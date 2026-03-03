#include "factory_io.h"

// Buffer de réception DMA
uint8_t rx_dma_buffer[FRAME_LENGTH];

static uint8_t actuators_state[28] = {0};
extern xQueueHandle xWriteQueue;

static uint8_t calc_crc(uint8_t *data, uint8_t len) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t val = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((val >> j) & 0x01) count++;
        }
    }
    return (count << 3);
}


void FACTORY_IO_SetActuator(uint8_t actuator_id, uint8_t state) {
    if (actuator_id >= 28) return;

    actuators_state[actuator_id] = state ? 1 : 0;

    uint8_t packed_data[4] = {0};
    for (int i = 0; i < 28; i++) {
        if (actuators_state[i]) {
            uint8_t byte_idx = i / 7;
            uint8_t bit_idx  = i % 7;
            packed_data[byte_idx] |= (1 << bit_idx);
        }
    }

    command_message_t msg;
    msg[0] = TAG_ACTUATORS;
    msg[1] = packed_data[0];
    msg[2] = packed_data[1];
    msg[3] = packed_data[2];
    msg[4] = packed_data[3];
    msg[5] = calc_crc(packed_data, 4);
    msg[6] = '\n';

    if (xWriteQueue != NULL) {
        xQueueSendToBack(xWriteQueue, &msg, 10);
    }
}

uint8_t FACTORY_IO_Sensors_Get(uint32_t msk) { return 0; }
void FACTORY_IO_update(void) {}
