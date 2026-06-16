#define STM32L4S5xx
#include "stm32l4xx.h"
#include <stdio.h>
#include <stdint.h>

 void delay_ms(int ms)
 {
     for (volatile int i = 0; i < ms * 4000; i++);
 }

 // ---------- UART ----------
 void UART_SendChar(char c)
 {
     while (!(USART1->ISR & (1 << 7)));
     USART1->TDR = c;
 }

 void UART_SendString(char *str)
 {
     while (*str)
     {
         UART_SendChar(*str++);
     }
 }
 void TIM2_IRQHandler(void) {
     // Check if the "Update" flag is set
     if (TIM2->SR & TIM_SR_UIF) {
         // Clear the flag (Target down!)
         TIM2->SR &= ~TIM_SR_UIF;

         // Perform your action
         UART_SendString("1 Second Passed\r\n");

     }
 }
 //CLOCK SETUP
 int main(void)

 {
	 // 1. Turn on HSI16
	 RCC->CR |= RCC_CR_HSION;

	 // 2. Wait for HSI16 to be ready
	 while (!(RCC->CR & RCC_CR_HSIRDY));

	 // 3. Configure the System Clock Switch to use HSI
	 RCC->CFGR &= ~RCC_CFGR_SW;      // Clear switch bits
	 RCC->CFGR |= RCC_CFGR_SW_HSI;   // Select HSI as system clock

	 // 4. Wait for the switch to complete
	 while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

	 RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
	    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    // ===== UART GPIO (PA9=TX) =====
 GPIOB->MODER &= ~(3 << (6 * 2));
 GPIOB->MODER |=  (2 << (6 * 2));   // PB6 AF

 GPIOB->MODER &= ~(3 << (7 * 2));
 GPIOB->MODER |=  (2 << (7 * 2));   // PB7 AF

    // ===== SELECT AF7 (USART1) =====
 GPIOB->AFR[0] &= ~(0xF << (6 * 4));
 GPIOB->AFR[0] |=  (7 << (6 * 4));  // PB6 → AF7

 GPIOB->AFR[0] &= ~(0xF << (7 * 4));
 GPIOB->AFR[0] |=  (7 << (7 * 4));  // PB7 → AF

    // ===== UART INIT =====
 USART1->BRR = 139;
  USART1->CR1 = USART_CR1_TE | USART_CR1_UE;

  UART_SendString("UART OK\r\n");
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

  // 2. Set the Prescaler
  // Clock is 16MHz. PSC=15999 means 16MHz / 16000 = 1000Hz (1ms per tick)
  TIM2->PSC = 1599;

  // 3. Set the Auto-Reload Register
  // 1000 ticks = 1 second
  TIM2->ARR = 999;

  // 4. Enable Update Interrupt
  // This is the "Mask" - UIE = Update Interrupt Enable
  TIM2->DIER |= TIM_DIER_UIE;

  // 5. Enable in NVIC
  NVIC_EnableIRQ(TIM2_IRQn);

  // 6. Start the Timer
  TIM2->CR1 |= TIM_CR1_CEN;
  while(1)
      {
          // The CPU sits here doing nothing.
          // Every 1 second, the Hardware will "jump" to TIM2_IRQHandler
      }

 }
