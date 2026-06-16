
#include "stm32l4xx.h"
#include <stdint.h>
#include <stdio.h>


void delay_ms(int ms)
{
    for (volatile int i = 0; i < ms * 4000; i++);
}



int main(void)
{

	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
	GPIOB->MODER &= ~((3 << 12) | (3 << 14));
	GPIOB->MODER &= ~(3<<28);
	GPIOB->MODER |= (2<<28);
	GPIOB->AFR[1] |= (1 << (6 * 4));
	GPIOB->MODER |=  ((2 << 12) | (2 << 14));
	GPIOB->AFR[0] |= (7 << 24) | (7 << 28);
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->ARR = 999;
	TIM1->PSC = 50;
	TIM1->CR1 = 0;
	TIM1->CR1 |= (3<<5);
	TIM1->CCMR1 &= ~(7 << 12); // Clear OC2M bits
	TIM1->CCMR1 |=  (6 << 12); // Set OC2M to 110 (PWM Mode 1)

	// Enable Channel 2 Output
	TIM1->CCER |= (1<<6);

	// MASTER SWITCH: Main Output Enable (MOE)
	TIM1->BDTR |= (1 << 15);

	// Start the Counter
	TIM1->CR1 |= (1 << 0);
	// Enable the Output Compare preload
	TIM1->CCMR1 |= (1 << 11);

	int brightness = 0;
	int step = 15;

	while(1) {
	    TIM1->CCR2 = brightness; // Set the compare value
	    

	    brightness += step; // Change brightness

	    // Reverse direction if we hit the limits (0 or ARR)
	    if (brightness <= 0 || brightness >= 1000) {
	        step = -step;
	    }

	   delay_ms(10); // Simple software delay to see the "breathing"
	}
}

