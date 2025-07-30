
extern "C" {
	#include "stm32f10x.h"
	#include "RCC_CLOCK.h"
	#include "USB-VCP.h"
	#include "string.h"
	#include "stdio.h"
	#include "Temperature18B20.h"
	#include "stm32f10x_tim.h"
	#include "delay/Delay_us.h"
#include "headers/svlp_writer.h"
}


// Р¤СѓРЅРєС†РёСЏ РїРµСЂРµРІРѕРґР° РёР· СЃС‚СЂРѕРєРё РІ float
float custom_atof(const char *str) {
    float result = 0.0f;
    float fraction = 0.1f;
    int sign = 1;

    // РџСЂРѕРїСѓСЃРєР°РµРј РїСЂРѕР±РµР»С‹
    while (*str == ' ') str++;

    // РћР±СЂР°Р±Р°С‚С‹РІР°РµРј Р·РЅР°Рє
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // РћР±СЂР°Р±Р°С‚С‹РІР°РµРј С†РµР»СѓСЋ С‡Р°СЃС‚СЊ
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0f + (*str - '0');
        str++;
    }

    // РћР±СЂР°Р±Р°С‚С‹РІР°РµРј РґСЂРѕР±РЅСѓСЋ С‡Р°СЃС‚СЊ
    if (*str == '.') {
        str++;

        while (*str >= '0' && *str <= '9') {
            result += (*str - '0') * fraction;
            fraction *= 0.1f;
            str++;
        }
    }
    return sign * result;
}

float targetTemperature = 0; // Р¦РµР»РµРІР°СЏ С‚РµРјРїРµСЂР°С‚СѓСЂР°
char temp_str[20];
uint32_t cmp_len = 0;
float SendTemp = 0;

// PID-РєРѕСЌС„С„РёС†РёРµРЅС‚С‹
const float Kp = 12.0f;
const float Ki = 0.4f;
const float Kd = 4.0f;

// РћРіСЂР°РЅРёС‡РµРЅРёРµ РёРЅС‚РµРіСЂР°Р»СЊРЅРѕР№ С‡Р°СЃС‚Рё
const float IntMax = 50;
const float IntMin = -50;

// РћС€РёР±РєРё
float errorPrevious = 0; // РџСЂРµРґС‹РґСѓС‰Р°СЏ РѕС€РёР±РєР°
float Integral = 0; // РќР°РєРѕРїР»РµРЅРЅС‹Р№ РёРЅС‚РµРіСЂР°Р»

// PID-СЂРµРіСѓР»СЏС‚РѕСЂ
int PID(){
	Read_VCP((uint8_t*)temp_str, &cmp_len);
	targetTemperature = custom_atof(temp_str);

	float error = targetTemperature - temperature; // РћС€РёР±РєР° С‚РµРєСѓС‰РµР№ С‚РµРјРїРµСЂР°С‚СѓСЂС‹
	float P = Kp * error;// РљРѕРјРїРѕРЅРµРЅС‚ P
	Integral += Ki * error;

	//РћРіСЂР°РЅРёС‡РµРЅРёРµ РёРЅС‚РµРіСЂР°Р»СЊРЅРѕР№ С‡Р°СЃС‚Рё
	if (Integral > IntMax){
		Integral = IntMax;
	}
	if (Integral < IntMin){
		Integral = IntMin;
	}
	float I = Integral;// РљРѕРјРїРЅРµРЅС‚ I

	float derivative = error - errorPrevious;
	float D = Kd * derivative;// РљРѕРјРїРѕРЅРµРЅС‚ D

	int Power = P + I + D;
	if (Power > 100){
		Power = 100;
	}
	if (Power < 0){
		Power = 0;
	}
	errorPrevious = error;
	TIM3->CCR3 = 100 - Power;
	return Power;
}

// Р¤СѓРЅРєС†РёСЏ РґР»СЏ РѕС‚РїСЂР°РІРєРё СЃРѕРѕР±С‰РµРЅРёР№ С‡РµСЂРµР· USB VCP
void SendMsg(const char *text) {
	if(text == NULL) return;
	int length = -1;
	const char *str = text;
	while (*str) {
		if ((*str & 0xC0) != 0x80) {
			length++;
		}
		str++;
	}
	Write_VCP((uint8_t*)text, length);
}

// Р�РЅРёС†РёР°Р»РёР·Р°С†РёСЏ РЁР�Рњ РґР»СЏ СѓРїСЂР°РІР»РµРЅРёСЏ РЅР°РіСЂРµРІР°С‚РµР»РµРј
void TIMEInit(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef heater;
	heater.GPIO_Pin = GPIO_Pin_0;
	heater.GPIO_Speed = GPIO_Speed_2MHz;
	heater.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &heater);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef timer;
	TIM_OCInitTypeDef timerPWM;

	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = 7200 - 1;	//72MHz / 7200 = 10kHz
	timer.TIM_Period = 100;	//10kHz / 100 = 100Hz
	timer.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM3, &timer);

	TIM_OCStructInit(&timerPWM);
	timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
	timerPWM.TIM_OutputState = TIM_OutputState_Enable;
	timerPWM.TIM_Pulse = 0;
	timerPWM.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC3Init(TIM3, &timerPWM);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

int main(void) {
    // РќР°СЃС‚СЂРѕР№РєР° РїР°СЂР°РјРµС‚СЂРѕРІ USB РїРѕ СѓРјРѕР»С‡Р°РЅРёСЋ
    LC_DATA_RATE = 115200;
    LC_STOP_BITS = 0;
    LC_PARITY_TYPE = 0;
    LC_N_BITS_DATA = 8;

    // РћС‚РєР»СЋС‡РµРЅРёРµ РІСЃРµС… РїСЂРµСЂС‹РІР°РЅРёР№
    NVIC->ICER[0] = 0xFFFFFFFF;

    ClockInit();
    USBInit();
    TIMEInit();
    SysTick_Init();
    temperatureInit();

    svlp::SVLP_Writer writer(socket, true);

    while(1) {
        USB_MANAGEMENT();
        updateTemp();
        temperature = getTemp();
        char message[20];
        memset(message, 0, 20);
        snprintf(message, sizeof(message), "%.2f,%d\r\n", temperature, PID());
		SendMsg(message);
		Delay_ms_USB(1000);
    }
}
