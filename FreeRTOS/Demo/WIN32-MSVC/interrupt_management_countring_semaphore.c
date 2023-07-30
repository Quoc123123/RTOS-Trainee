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


SemaphoreHandle_t xSemaphore;

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

static void vHandlerTask(void)
{
    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Use the semaphore to wait for the event. The semaphore was created
        before the scheduler was started, so before this task ran for the first
        time. The task blocks indefinitely, meaning this function call will only
        return once the semaphore has been successfully obtained - so there is
        no need to check the value returned by xSemaphoreTake(). */
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        /* To get here the event must have occurred. Process the event (in this
        Case, just print out a message). */
        printf("Handler task - Processing event.\r\n");
    }
}

static uint32_t ulExampleInterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as it
    will get set to pdTRUE inside the interrupt safe API function if a context switch
    is required. */
    xHigherPriorityTaskWoken = pdFALSE;
    /* 'Give' the semaphore multiple times. The first will unblock the deferred
    interrupt handling task, the following 'gives' are to demonstrate that the
    semaphore latches the events to allow the task to which interrupts are deferred
    to process them in turn, without events getting lost. This simulates multiple
    interrupts being received by the processor, even though in this case the events
    are simulated within a single interrupt occurrence. */
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR(). If
    xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR() then
    calling portYIELD_FROM_ISR() will request a context switch. If
    xHigherPriorityTaskWoken is still pdFALSE then calling portYIELD_FROM_ISR() will
    have no effect. Unlike most FreeRTOS ports, the Windows port requires the ISR to
    return a value - the return statement is inside the Windows version of
    portYIELD_FROM_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void main_counting_semaphore(void)
{
/* Check the semaphore was created successfully. */
    
    /* Create the 'handler' task, which is the task to which interrupt
    processing is deferred. This is the task that will be synchronized with
    the interrupt. The handler task is created with a high priority to ensure
    it runs immediately after the interrupt exits. In this case a priority of
    3 is chosen. */
    
    xSemaphore = xSemaphoreCreateCounting(10, 0);
    xTaskCreate(vHandlerTask, "Handler", 1000, NULL, 3, NULL);
    /* Create the task that will periodically generate a software interrupt.
    This is created with a priority below the handler task to ensure it will
    get preempted each time the handler task exits the Blocked state. */
    xTaskCreate(vPeriodicTask, "Periodic", 1000, NULL, 1, NULL);
    /* Install the handler for the software interrupt. The syntax necessary
    to do this is dependent on the FreeRTOS port being used. The syntax
    shown here can only be used with the FreeRTOS windows port, where such
    interrupts are only simulated. */
    vPortSetInterruptHandler(mainINTERRUPT_NUMBER, ulExampleInterruptHandler);
    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* As normal, the following line should never be reached. */
    for (;;)
        ;
}