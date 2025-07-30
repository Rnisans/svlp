#ifndef DELAY_US_H
#define DELAY_US_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Delay_ms_USB(uint32_t ms);

#ifdef __cplusplus
}
#endif

void SysTick_Init(void);
void Delay_ms(uint32_t ms);
void Delay_ms_USB(uint32_t ms);
void Delay_us(uint32_t us);

#endif
