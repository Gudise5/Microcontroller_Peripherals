#include "stm32f4xx.h"

// PA5 = Green, PA6 = Yellow, PA7 = Red
// PC13 = Pedestrian Button
// USART1 (TX = PA9)

void gpio_init(void) {
    // Enable GPIOA, GPIOC, USART1
    RCC->AHB1ENR |= (1 << 0) | (1 << 2);      // GPIOAEN | GPIOCEN
    RCC->APB2ENR |= (1 << 4);                 // USART1EN

    // Set PA5, PA6, PA7 as General Purpose Output
    GPIOA->MODER &= ~((3 << (5*2)) | (3 << (6*2)) | (3 << (7*2)));
    GPIOA->MODER |= (1 << (5*2)) | (1 << (6*2)) | (1 << (7*2));

    // Set Push-Pull (OTYPER)
    GPIOA->OTYPER &= ~((1 << 5) | (1 << 6) | (1 << 7));

    // High speed
    GPIOA->OSPEEDR |= (3 << (5*2)) | (3 << (6*2)) | (3 << (7*2));

    // Lock PA5–PA7 using LCKR
    GPIOA->LCKR = (1 << 16) | (1 << 5) | (1 << 6) | (1 << 7);
    GPIOA->LCKR = (1 << 5) | (1 << 6) | (1 << 7);
    GPIOA->LCKR = (1 << 16) | (1 << 5) | (1 << 6) | (1 << 7);
    (void)GPIOA->LCKR;

    // PC13 as input, pull-up
    GPIOC->MODER &= ~(3 << (13*2));
    GPIOC->PUPDR &= ~(3 << (13*2));
    GPIOC->PUPDR |=  (1 << (13*2));

    // USART1 PA9 (TX) AF7
    GPIOA->MODER &= ~(3 << (9*2));
    GPIOA->MODER |=  (2 << (9*2));           // Alternate function
    GPIOA->AFR[1] |= (7 << ((9-8)*4));       // AF7 for PA9

    // USART config: 9600 baud @ 16 MHz
    USART1->BRR = 0x683;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 8000; i++) __NOP();
}

void usart_send_char(char c) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

void usart_send_string(const char *str) {
    while (*str) usart_send_char(*str++);
}

void traffic_light_cycle(void) {
    // Green ON
    GPIOA->BSRR = (1 << 5);                      // ON
    GPIOA->BSRR = (1 << (6 + 16)) | (1 << (7 + 16)); // OFF
    usart_send_string("GREEN\r\n");
    delay_ms(2000);

    // Yellow ON
    GPIOA->BSRR = (1 << 6);
    GPIOA->BSRR = (1 << (5 + 16)) | (1 << (7 + 16));
    usart_send_string("YELLOW\r\n");
    delay_ms(1000);

    // Red ON
    GPIOA->BSRR = (1 << 7);
    GPIOA->BSRR = (1 << (5 + 16)) | (1 << (6 + 16));
    usart_send_string("RED\r\n");
    delay_ms(2000);
}

int main(void) {
    gpio_init();

    while (1) {
        traffic_light_cycle();

        // If button pressed (PC13 = 0)
        if (!(GPIOC->IDR & (1 << 13))) {
            usart_send_string("PEDESTRIAN WAITING\r\n");
            GPIOA->BSRR = (1 << 7);  // Keep Red ON
            delay_ms(2000);
        }
    }
}
