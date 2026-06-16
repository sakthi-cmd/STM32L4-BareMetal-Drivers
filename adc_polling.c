#define STM32L4S5xx
#include "stm32l4xx.h"
#include <stdio.h>
#include <stdint.h>
#define TS_CAL1 (*(uint16_t*)0x1FFF75A8)
#define TS_CAL2 (*(uint16_t*)0x1FFF75CA)
// ---------- DELAY ----------
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

// ---------- MAIN ----------
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
        ADC1->CR |= ADC_CR_ADSTART;
        UART_SendString("START ADC\r\n");

        while (!(ADC1->ISR & ADC_ISR_EOC));

        UART_SendString("conversion starts\r\n");

        uint16_t raw = ADC1->DR;

        // --- STEP 1: PRINT CALIBRATION VALUES (Check for 0) ---
                char cal_buf[64];
                sprintf(cal_buf, "Raw: %u | CAL1: %u | CAL2: %u\r\n",
                        (unsigned int)raw, (unsigned int)TS_CAL1, (unsigned int)TS_CAL2);
                UART_SendString(cal_buf);

                // --- STEP 2: SIMPLIFIED FLOAT MATH ---
                // We use (float) casting everywhere to be explicit
                int32_t raw_at_3v = (raw * 330) / 300; // Adjust 3.3V to 3.0V scale
                int32_t temp_x100 = 3000 + ((raw_at_3v - TS_CAL1) * (130 - 30) * 100) / (TS_CAL2 - TS_CAL1);

                char result_buf[64];
                sprintf(result_buf, "Temp: %ld.%02ld C\r\n", temp_x100 / 100, temp_x100 % 100);
                UART_SendString(result_buf);
                delay_ms(200);
    }
}
