/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

/* Standard demo includes. */
#include "BlockQ.h"
#include "integer.h"
#include "semtest.h"
#include "PollQ.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "recmutex.h"
#include "flop.h"
#include "TimerDemo.h"
#include "countsem.h"
#include "death.h"
#include "dynamic.h"
#include "QueueSet.h"
#include "QueueOverwrite.h"
#include "EventGroupsDemo.h"
#include "IntSemTest.h"
#include "TaskNotify.h"
#include "TaskNotifyArray.h"
#include "QueueSetPolling.h"
#include "StaticAllocation.h"
#include "blocktim.h"
#include "AbortDelay.h"
#include "MessageBufferDemo.h"
#include "StreamBufferDemo.h"
#include "StreamBufferInterrupt.h"
#include "MessageBufferAMP.h"

/* Priorities at which the tasks are created. */
#define mainCHECK_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define mainQUEUE_POLL_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainSEM_TEST_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainBLOCK_Q_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainCREATOR_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainFLASH_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainINTEGER_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainGEN_QUEUE_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainFLOP_TASK_PRIORITY (tskIDLE_PRIORITY)
#define mainQUEUE_OVERWRITE_PRIORITY (tskIDLE_PRIORITY)

#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS(3333)
#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS(500)

#define mainINTERRUPT_NUMBER 3


static void vPeriodicTask(void)
{
    const TickType_t xDelay500ms = pdMS_TO_TICKS(500UL);
    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Block until it is time to generate the software interrupt again. */
        vTaskDelay(xDelay500ms);
        /* Generate the interrupt, printing a message both before and after
        the interrupt has been generated, so the sequence of execution is evident
        from the output.
        The syntax used to generate a software interrupt is dependent on the
        FreeRTOS port being used. The syntax used below can only be used with
        the FreeRTOS Windows port, in which such interrupts are only simulated. */
        printf("Periodic task - About to generate an interrupt.\r\n");
        vPortGenerateSimulatedInterrupt(mainINTERRUPT_NUMBER);
        printf("Periodic task - Interrupt generated.\r\n\r\n\r\n");
    }
}

static void vDeferredHandlingFunction(void *pvParameter1, uint32_t ulParameter2)
{
    /* Process the event - in this case just print out a message and the value of
    ulParameter2. pvParameter1 is not used in this example. */
    printf("Handler function - Processing event: %d\r\n", ulParameter2);
}

static uint32_t ulExampleInterruptHandler(void)
{
    static uint32_t ulParameterValue = 0;
    BaseType_t xHigherPriorityTaskWoken;
    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as it will
    get set to pdTRUE inside the interrupt safe API function if a context switch is
    required. */
    xHigherPriorityTaskWoken = pdFALSE;
    /* Send a pointer to the interrupt's deferred handling function to the daemon task.
    The deferred handling function's pvParameter1 parameter is not used so just set to
    NULL. The deferred handling function's ulParameter2 parameter is used to pass a
    number that is incremented by one each time this interrupt handler executes. */
    xTimerPendFunctionCallFromISR(vDeferredHandlingFunction, /* Function to execute. */
                                  NULL,                      /* Not used. */
                                  ulParameterValue,          /* Incrementing value. */
                                  &xHigherPriorityTaskWoken);
    ulParameterValue++;
    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR(). If
    xHigherPriorityTaskWoken was set to pdTRUE inside xTimerPendFunctionCallFromISR() then
    calling portYIELD_FROM_ISR() will request a context switch. If
    xHigherPriorityTaskWoken is still pdFALSE then calling portYIELD_FROM_ISR() will have
    no effect. Unlike most FreeRTOS ports, the Windows port requires the ISR to return a
    value - the return statement is inside the Windows version of portYIELD_FROM_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void deferred_interrupt_processing(void)
{
    /* The task that generates the software interrupt is created at a priority below the
priority of the daemon task. The priority of the daemon task is set by the
configTIMER_TASK_PRIORITY compile time configuration constant in FreeRTOSConfig.h. */
    const UBaseType_t ulPeriodicTaskPriority = configTIMER_TASK_PRIORITY - 1;
    /* Create the task that will periodically generate a software interrupt. */
    xTaskCreate(vPeriodicTask, "Periodic", 1000, NULL, ulPeriodicTaskPriority, NULL);
    /* Install the handler for the software interrupt. The syntax necessary to do
    this is dependent on the FreeRTOS port being used. The syntax shown here can
    only be used with the FreeRTOS windows port, where such interrupts are only
    simulated. */
    vPortSetInterruptHandler(mainINTERRUPT_NUMBER, ulExampleInterruptHandler);
    /* Start the scheduler so the created task starts executing. */
    vTaskStartScheduler();
    /* As normal, the following line should never be reached. */
    for (;;)
        ;
}