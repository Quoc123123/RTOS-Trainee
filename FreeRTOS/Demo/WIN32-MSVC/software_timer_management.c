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

uint32_t  ulCallCount;

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    TickType_t xTimeNow;
    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
    printf("One-shot timer callback executing: %d\r\n", xTimeNow);
    /* File scope variable. */
    ulCallCount++;
}

static void prvAutoReloadTimerCallback(TimerHandle_t xTimer)
{
    TickType_t xTimeNow;
    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
    printf("Auto-reload timer callback executing: %d\r\n", xTimeNow);
    ulCallCount++;
}

void  main_software_timer_management(void)
{
    TimerHandle_t xAutoReloadTimer, xOneShotTimer;
    BaseType_t xTimer1Started, xTimer2Started;
    /* Create the one shot timer, storing the handle to the created timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
        "OneShot",
        /* The software timer's period in ticks. */
        mainONE_SHOT_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
        pdFALSE,
        /* This example does not use the timer id. */
        0,
        /* The callback function to be used by the software timer being created. */
        prvOneShotTimerCallback);
    /* Create the auto-reload timer, storing the handle to the created timer in xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
        "AutoReload",
        /* The software timer's period in ticks. */
        mainAUTO_RELOAD_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
        pdTRUE,
        /* This example does not use the timer id. */
        0,
        /* The callback function to be used by the software timer being created. */
        prvAutoReloadTimerCallback);
    /* Check the software timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL))
    {
        /* Start the software timers, using a block time of 0 (no block time). The scheduler has
        not been started yet so any block time specified here would be ignored anyway. */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);
        /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart()
        will fail if the timer command queue gets full. The timer service task does not get
        created until the scheduler is started, so all commands sent to the command queue will
        stay in the queue until after the scheduler has been started. Check both calls to
        xTimerStart() passed. */
        if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS))
        {
            /* Start the scheduler. */
            vTaskStartScheduler();
        }
    }
    /* As always, this line should not be reached. */
    for (;;)
        ;
}