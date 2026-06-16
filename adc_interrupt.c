#define STM32L4S5xx
#include "stm32l4xx.h"
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    // ===== CLOCK ENABLE =====
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
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
    USART1->BRR = 35; // 115200 @ 16MHz
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;

    UART_SendString("UART OK\r\n");

    // ===== ADC CLOCK SELECT (SYSCLK) =====
    RCC->CCIPR &= ~(3 << 28);
    RCC->CCIPR |=  (3 << 28);

    // ===== ADC ASYNC MODE =====
    ADC1_COMMON->CCR &= ~(3 << 16);

    // ===== ENABLE TEMP SENSOR =====
    ADC1_COMMON->CCR |= ADC_CCR_TSEN;
    delay_ms(10);
    ADC1_COMMON->CCR |= ADC_CCR_VREFEN;


    // EXIT DEEP POWER DOWN
    ADC1->CR &= ~ADC_CR_DEEPPWD;

    // ENABLE ADC VOLTAGE REGULATOR
    ADC1->CR |= ADC_CR_ADVREGEN;

    // WAIT for regulator stabilization
    delay_ms(10);

    // ===== ADC CALIBRATION =====
    ADC1->CR &= ~ADC_CR_ADEN;
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);
    UART_SendString("ADC CALIBRATED\n");
    // ===== ADC ENABLE =====
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    UART_SendString("ADC READY\n");

    ADC1->IER &= ~(1<<2);
    ADC1->IER |= (1<<2);
    NVIC_EnableIRQ(ADC1_2_IRQn);
    NVIC_SetPriority(ADC1_2_IRQn,0);

    // ===== SELECT CHANNEL 17 =====
    ADC1->SQR1 &= ~(0x1F << 6);
    ADC1->SQR1 |=  (17 << 6);

    // ===== MAX SAMPLING TIME =====
    ADC1->SMPR2 &= ~(7 << 21);
    ADC1->SMPR2 |=  (7 << 21);

    ADC1->SQR1 &= ~ADC_SQR1_L;      // L = 0 (1 conversion total)
        ADC1->SQR1 &= ~(0x1F << 6);     // Clear SQ1
        ADC1->SQR1 |= (17 << 6);        // Set SQ1 to Channel 17

    // ===== LOOP =====
        while (1)
        {
            ADC1->CR |= ADC_CR_ADSTART;       // Start the conversion

            if (adc_ready_flag)               // Only run if the interrupt set the flag
            {
                adc_ready_flag = 0;           // Reset the flag

                // Use adc_raw_data for your math here
                int32_t raw_at_3v = (adc_raw_data * 330) / 300;
                int32_t temp_x100 = 3000 + ((raw_at_3v - TS_CAL1) * 100 * 100) / (TS_CAL2 - TS_CAL1);

                char result_buf[64];
                sprintf(result_buf, "Temp: %ld.%02ld C\r\n", temp_x100 / 100, temp_x100 % 100);
                UART_SendString(result_buf);
            }

            delay_ms(100); // You can do other tasks here now!
        }
}

