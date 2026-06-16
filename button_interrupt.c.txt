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
 void EXTI2_IRQHandler(void)
 {
     // IMPORTANT: You must clear the "Pending" bit in EXTI
     if (EXTI->PR1 & (1 << 2))
     {
    	 EXTI->PR1 |= (1 << 2);

    	         // 2. Your Action
    	         UART_SendString("Button Pressed!\r\n");
     }
 }
 //CLOCK SETUP
 int main(void)
 {
 RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
 GPIOB->MODER &= ~(3<<4);
 GPIOB->PUPDR &= ~(3<<4);

 RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
 RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

 SYSCFG->EXTICR[0] &= ~(0xF << 8); // Clear the 4 bits for Line 2
 SYSCFG->EXTICR[0] |=  (1 << 8);    // Set it to Port B

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
 USART1->BRR = 35;
  USART1->CR1 = USART_CR1_TE | USART_CR1_UE;

  UART_SendString("UART OK\r\n");

  EXTI->IMR1 |= (1 << 2);
  EXTI->FTSR1 |= (1 << 2);
  EXTI->RTSR1 &= ~(1<<2);



  NVIC_EnableIRQ(EXTI2_IRQn);
  NVIC_SetPriority(EXTI2_IRQn,0);
 }
