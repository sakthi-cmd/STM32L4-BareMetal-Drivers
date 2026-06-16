# STM32L4-BareMetal-Drivers

A collection of register-level, bare-metal configuration drivers written in C for the **STM32L4S5xx** microcontroller platform. This repository demonstrates hardware peripheral initialization, clock configuration, and interrupt-driven subsystems without relying on high-level abstraction layers (like STM32 HAL/LL).

## Peripherals Implemented
* **UART1 Transmit Driver:** Initialized at 115200 baud (@16MHz) for deterministic telemetry and logging via data registers.
* **ADC1 Configurations:** Includes both asynchronous polling-based sampling and NVIC hardware-interrupt-driven conversions for internal temperature monitoring (Channel 17) and external inputs (PC5 Channel 14).
* **EXTI External Interrupts:** Register setup for EXTI Line 2 mapping to Port B with software debouncing validation.
* **TIM2 & TIM1 Implementations:** Configured hardware timers for a deterministic 1-second update interrupt loop and an efficient hardware-generated PWM (Mode 1) LED dimmer.

## Hardware Context
* **Target MCU:** ARM Cortex-M4 (STM32L4S5xx)
* **Development Environment:** Bare-Metal C compilation environment using CMSIS device headers.
