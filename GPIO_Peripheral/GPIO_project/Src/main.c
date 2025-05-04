#include "stm32f4xx.h"

void gpio_init(void) {
    // Enable GPIOA, GPIOC clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    // Set PA5 as output, PA9/PA10 as AF, PC13 as input
    GPIOA->MODER &= ~(3 << (5 * 2));           // Clear mode
    GPIOA->MODER |= (1 << (5 * 2));            // PA5 Output

    GPIOA->MODER &= ~((3 << (9*2)) | (3 << (10*2)));  // Clear PA9, PA10
    GPIOA->MODER |= (2 << (9*2)) | (2 << (10*2));     // AF mode

    GPIOC->MODER &= ~(3 << (13 * 2));          // PC13 Input

    // OTYPER: Push-pull for PA5
    GPIOA->OTYPER &= ~(1 << 5);

    // OSPEEDR: High speed for PA5
    GPIOA->OSPEEDR |= (3 << (5 * 2));

    // PUPDR: Pull-up for PC13
    GPIOC->PUPDR &= ~(3 << (13 * 2));
    GPIOC->PUPDR |= (1 << (13 * 2));

    // AFR: Set PA9 and PA10 to AF7 (USART1)
    GPIOA->AFR[1] |= (7 << ((9 - 8) * 4)) | (7 << ((10 - 8) * 4));

    // Lock PA5
    GPIOA->LCKR = (1 << 16) | (1 << 5);
    GPIOA->LCKR = (1 << 5);
    GPIOA->LCKR = (1 << 16) | (1 << 5);
    uint32_t temp = GPIOA->LCKR; // Dummy read to complete lock
}

void delay(void) {
    for (int i = 0; i < 500000; i++);
}

int main(void) {
    gpio_init();

    while (1) {
        // If button is pressed (PC13 = 0)
        if (!(GPIOC->IDR & (1 << 13))) {
            GPIOA->BSRR = (1 << 5);  // Set PA5 (LED ON)
        } else {
            GPIOA->BSRR = (1 << (5 + 16));  // Reset PA5 (LED OFF)
        }

        delay();
    }
}
