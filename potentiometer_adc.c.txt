#include "stm32l4xx.h"
#include <stdint.h>
#include <stdio.h>


void delay_ms(int ms)
{
    for (volatile int i = 0; i < ms * 4000; i++);
}


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

int main(void)
{

	RCC->AHB2ENR  |= RCC_AHB2ENR_ADCEN;
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOCEN;
	RCC->APB2ENR  |= RCC_APB2ENR_USART1EN;
	GPIOB->MODER &= ~((3 << 12) | (3 << 14));
	GPIOB->MODER |=  ((2 << 12) | (2 << 14));
	GPIOB->AFR[0] |= (7 << 24) | (7 << 28);
	GPIOC->MODER |= (3 << (5 * 2));
	USART1->BRR = 35; // 115200 @ 16MHz
	USART1->CR1 = USART_CR1_TE | USART_CR1_UE;


	UART_SendString("UART OK\r\n");
	RCC->CCIPR &= ~(3 << 28);
	RCC->CCIPR |=  (3 << 28);
	ADC1_COMMON->CCR &= ~(3 << 16);
	ADC1_COMMON->CCR |= ADC_CCR_VREFEN;
	    // EXIT DEEP POWER DOWN
	ADC1->CR &= ~ADC_CR_DEEPPWD;    // Exit Deep Power Down
	ADC1->CR |= ADC_CR_ADVREGEN;    // Enable Voltage Regulator
	delay_ms(20);

    ADC1->CR &= ~ADC_CR_ADEN;
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);
    UART_SendString("ADC CALIBRATED\r\n");
    // 1. CLEAR ADRDY flag by writing a 1 to it
    ADC1->ISR |= ADC_ISR_ADRDY;

    // 2. ACTUALLY ENABLE THE ADC
    ADC1->CR |= ADC_CR_ADEN;

    // 3. NOW WAIT for the hardware to report it is ready
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
        // Optional: Add a timeout here if you want to be safe
    }

    UART_SendString("ADC READY\r\n");
       // 2. Configure Sequence (Channel 14 for PC5)
    ADC1->SQR1 &= ~(0x1F << 6);     // L=0 (1 conversion in sequence)
    ADC1->SQR1 |= (14 << 6);  // Set 1st conversion in sequence to Channel 14

       // 3. Set Sample Time (Optional but recommended for accuracy)
       // Channel 14 is in SMPR2. Setting it to '010' (12.5 ADC clock cycles)
    ADC1->SMPR2 &= ~(7<<12);
    ADC1->SMPR2 |= (2<<12);

       while(1)
       {
    	   ADC1->CR |= ADC_CR_ADSTART;
    	   UART_SendString("START ADC\r\n");

    	   while (!(ADC1->ISR & ADC_ISR_EOC));
    	   UART_SendString("START value \r\n");

    	       // Read Data (This also clears the EOC flag automatically)
    	       uint32_t adc_val = ADC1->DR;

    	       // Send to UART
    	       char msg[20];
    	       sprintf(msg, "Value: %lu\r\n", adc_val);
    	       UART_SendString(msg);

    	       delay_ms(100);
       }


}
