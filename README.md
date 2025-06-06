# FreeRTOS
This repository contains various STM32 projects aimed at better understanding FreeRTOS, along with lessons learned throughout the development process.

## Lessons Learned
* The CMSIS-RTOS layer sits on top of the FreeRTOS layer and provides a common interface for various RTOSes. This allows programmers to write portable applications using a standardized API. In essence, CMSIS-RTOS is a wrapper around an existing RTOS.
