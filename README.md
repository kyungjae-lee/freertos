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
