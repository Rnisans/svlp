#include "Delay_us.h"
#include "stm32f10x.h"

volatile uint32_t msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;
}

void SysTick_Init(void) {
    SysTick_Config(SystemCoreClock / 1000); // 1 ms tick
}

void Delay_ms(uint32_t ms) {
    uint32_t curTicks = msTicks;
    while ((msTicks - curTicks) < ms);
}

void Delay_ms_USB(uint32_t ms) {
    uint32_t curTicks_USB = msTicks;
    while ((msTicks - curTicks_USB) < ms) USB_MANAGEMENT();
}

void Delay_us(uint32_t us) {
    volatile uint32_t count = us * (SystemCoreClock / 8000000);
    while (count--) __NOP();
}
