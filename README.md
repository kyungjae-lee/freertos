# FreeRTOS
This repository contains various STM32 projects aimed at better understanding FreeRTOS, along with lessons learned throughout the development process.



## FreeRTOS Naming Conventions

### Variables

* `TickType_t`

  * Unsigned 16-bit type
  * Unsigned 32-bit type

  Based on config settings.

* `BaseType_t`

  * Unsigned 16-bit type on 16-bit architecture
  * Unsigned 32-bit type on 32-bit architecture

* Variables are prefixed with their type:

  * `c`: char
  * `s`: short / uint16_t
  * `l`: long / int32_t
  * `x`: `BaseType_t` and any other non-standard types (e.g., structures, task handles, queue handles, etc.)
  * `u`: unsigned
  * `p`: pointer

### Functions

Functions are prefixed with both the type they return, and the module they are defined in.

* e.g., `vTaskPrioritySet()` - Returns a void, defined in `task.c`.
* e.g., `xQueueReceive()` - Returns a variable of type `BaseType_t` and is defined within `queue.c`.
* e.g., `pvTimerGetTimerID()` - Returns a pointer to void and is defined within `timers.c`.

### Macros

* Macro in **uppercase**.
* Prefix in **lowercase**.
* Prefix indicates macro definition file:
  * e.g., `portMAX_DELAY` - Defined in `portable.h`.
  * e.g., `taskENTER_CRITICAL()` - Defined in `task.h`
  * e.g., `pdTRUE` - Defined in `projdefs.h`
  * e.g., `configUSE_PREEMPTION` - Defined in `FreeRTOSConfig.h`
  * e.g., `errQUEUE_FULL` - Defined in `projdefs.h`
* Macro examples
  * e.g., `pdTRUE` - 1
  * e.g., `pdFALSE` - 0
  * e.g., `pdPASS` - 1
  * e.g., `pdFAIL` - 0



## Tasks

### Task States



![image-20250607215046253](C:\Users\klee\AppData\Roaming\Typora\typora-user-images\image-20250607215046253.png)



* Causes of the Blocked state:

  - **Time-related event** – e.g., a task that is placed in the **Blocked** state for a specified duration using `vTaskDelay()` or similar.

    - To use `vTaskDelay()` the following setting must be enabled:

      ```c
      /* FreeRTOSConfig.h */
      #define INCLUDE_vTaskDelay	1
      ```

      The function prototype is:

      ```c
      vTaskDelay(TickType_t xTicksToDelay)
      ```

      This API places the calling task into the **Blocked** state for the specified number of tick interrupts, after which it transitions back into the **Ready** state.

      Since the concept of system ticks can be unintuitive, FreeRTOS provides the `pdMS_TO_TICKS()` macro to convert time from milliseconds to ticks.
      For example:

      ```c
      vTaskDelay(pdMS_TO_TICKS(100));  /* Delay for 100 milliseconds */
      ```

  - **Synchronization event** – e.g., a task waiting for a signal, such as a semaphore, event group, or notification from another task or ISR.

### Idle Task

* There must be at least one task in the **Running** state at any given time. Because of this, the Idle task automatically enters the Running state when no other task is available to run.
* The Idle task is created automatically when the scheduler starts.
* The Idle task has the lowest priority (0), ensuring it never prevents a higher-priority application task from entering the **Running** state.

### Idle Hook Function

* We can add application-specific functionality directly into the Idle task through the use of an Idle hook (or Idle callback) function.

* This function is called automatically by the Idle task once per iteration of the Idle task loop.

* Enabling idle hook function:

  ```c
  /* FreeRTOSConfig.h */
  #define configUSE_IDLE_HOOK()  1
  ```

* `configIDLE_SHOULD_YIELD` is used to prevent the Idle task from unnecessarily consuming CPU time by allowing it to yield the processor to ready tasks of equal priority.



## Tick Hook

* A tick hook (or tick callback) is a function called by the kernel during each tick interrupt.

* Tick hook functions execute in the context of the tick interrupt and therefore must be kept very short, use only a moderate amount of stack space, and must not call any FreeRTOS API functions that do not end with `FromISR()`.

* Enabling tick hook function:

  ```c
  /* FreeRTOSConfig.h */
  #define configUSE_TICK_HOOK()  1
  ```




## Queues

* Queues are used as FIFO buffers, where data is inserted at the back and removed from the front.
* Queues can hold a finite number of fixed data items.
* The maximum number of items a queue can hold is called the length.

### Passing Data to a Queue

* Pass-by-value
  * Simply insert data into the queue.
* Pass-by-reference
  * Involves using queues to transfer pointers to the data, rather than copy the data itself into and out of the queue byte by byte.
  * Preferred choice for passing large queue data.
  * Saves memory.

### Blocking

* Blocking on queue reads

  When a task attempts to read from a queue, it can optionally specify a 'block' time. This is the time the task will be kept in the **Blocked** state to wait for data to be available from the queue, if the queue is empty. As soon as data becomes available the task is automatically moved to the **Ready** state.

* Blocking on queue writes

  Task is placed in blocked state if queue is full, as soon as space becomes available in the queue task is moved to **Ready** state.

### Commonly Used APIs

* `xQueueSend()`, `xQueueSendToFront()`, `xQueueSendToBack()`

  ```c
  BaseType_t xQueueSend(QueueHandle_t xQueue,
                        const void * pvItemToQueue,
                        TickType_t xTickToWait);
  ```

* `xQueueReceive()`

  ```c
  BaseType_t xQueueReceive(QueueHandle_t xQueue,
                           void * pvBuffer,
                           TickType_t xTickToWait);
  ```

* `xQueueCreate()`

  ```c
  QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength,
                             UBaseType_t uxItemSize);
  ```



## Queuesets

* Queue sets allow a task to receive data from more than one queue without the task polling each queue in turn to determine which, if any, contains data.

* Enabling queue sets:

  ```c
  /* FreeRTOSConfig.h */
  #define configUSE_QUEUE_SETS() 	1
  ```

### Commonly Used APIs

* `xQueueCreateSet()`

  ```c
  QueueSetHandle_t xQueueCreateSet(const UBaseType_t uxEventQueueLength);
  ```

* `xQueueAddToSet()`

  ```c
  BaseType_t xQueueAddToSet(QueueSetMemberHandle_t xQueueOrSemaphore,
                            QueueSetHandle_t xQueueSet);
  ```



## Lessons Learned
* The CMSIS-RTOS layer sits on top of the FreeRTOS layer and provides a common interface for various RTOSes. This allows programmers to write portable applications using a standardized API. In essence, CMSIS-RTOS is a wrapper around an existing RTOS.
* The `cmcsis_os.h` header file includes `FreeRTOS.h`.
* Shortcut to commenting out multiple lines: highlight the area -> `ctrl` + `/`
* The larger the priority number, the higher the priority.
* `vTaskDelay()` puts a task to sleep, and the scheduler will remove it from the Running state and place it into the Blocked state for the specified number of ticks. Once the delay expires, the task is moved back to the Ready state and will be scheduled to run again according to its priority.
  * If you want to add a delay to a task but don't want to block it (i.e., keep it in Running state), simply use the `for` loop to implement a delay instead of using `vTaskDelay()`.
* FreeRTOS configuration options can be modified through the `FreeRTOSConfig.h` file.
* A deleted task cannot be resumed.

### GPIO

* Push-pull mode (for normal output pins) vs. Open-drain mode (for I2C, SPI pins)?



## Reference

Gbati, I. (2023). *FreeRTOS  From Ground Up™  on ARM Processors (REVISED)* [Video file]. Retrieved from https://www.udemy.com/course/freertos-on-arm-processors/?couponCode=PLOYALTY0923
