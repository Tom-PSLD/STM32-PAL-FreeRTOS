#ifndef TRC_CONFIG_H
#define TRC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx.h"

/**
 * @def TRC_CFG_HARDWARE_PORT
 * @brief Specify what hardware port to use.
 */
#define TRC_CFG_HARDWARE_PORT TRC_HARDWARE_PORT_ARM_Cortex_M

#define TRC_CFG_SCHEDULING_ONLY             0


#define TRC_CFG_INCLUDE_IDLE_TASK           1

#define TRC_CFG_INCLUDE_USER_EVENTS         1

#define TRC_CFG_INCLUDE_MEMMANG_EVENTS      0

#define TRC_CFG_INCLUDE_READY_EVENTS        0

#define TRC_CFG_INCLUDE_ISR_TRACING         0
#define TRC_CFG_INCLUDE_OSTICK_EVENTS       0

#define TRC_CFG_ENABLE_STACK_MONITOR        1
#define TRC_CFG_STACK_MONITOR_MAX_TASKS     10
#define TRC_CFG_STACK_MONITOR_MAX_REPORTS   1

#define TRC_CFG_CTRL_TASK_PRIORITY          1
#define TRC_CFG_CTRL_TASK_DELAY             50
#define TRC_CFG_CTRL_TASK_STACK_SIZE        configMINIMAL_STACK_SIZE

#define TRC_CFG_RECORDER_BUFFER_ALLOCATION  TRC_RECORDER_BUFFER_ALLOCATION_STATIC
#define TRC_CFG_MAX_ISR_NESTING             8

#define TRC_CFG_ISR_TAILCHAINING_THRESHOLD  0
#define TRC_CFG_RECORDER_DATA_INIT          1

#define TRC_CFG_RECORDER_DATA_ATTRIBUTE 

#define TRC_CFG_USE_TRACE_ASSERT            0

#ifdef __cplusplus
}
#endif

#endif /* _TRC_CONFIG_H */
