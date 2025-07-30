
extern "C" {
	#include "stm32f10x.h"
	#include "RCC_CLOCK.h"
	#include "USB-VCP.h"
	#include "string.h"
	#include "stdio.h"
	#include "Temperature18B20.h"
	#include "stm32f10x_tim.h"
	#include "delay/Delay_us.h"
}

// Include SVLP library headers
#include "svlp_lib/headers/svlp_writer.h"
#include "usb_socket.h"

// Temperature and power data structure for SVLP
struct TemperatureData {
    float current_temperature;
    float target_temperature;
    int power_output;
};

// Function to convert string to float
float custom_atof(const char *str) {
    float result = 0.0f;
    float fraction = 0.1f;
    int sign = 1;

    // Skip spaces
    while (*str == ' ') str++;

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Handle integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0f + (*str - '0');
        str++;
    }

    // Handle decimal part
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

float targetTemperature = 0; // Target temperature
char temp_str[20];
uint32_t cmp_len = 0;
float SendTemp = 0;

// PID coefficients
const float Kp = 12.0f;
const float Ki = 0.4f;
const float Kd = 4.0f;

// Integral limits
const float IntMax = 50;
const float IntMin = -50;

// Errors
float errorPrevious = 0; // Previous error
float Integral = 0; // Accumulated integral

// PID controller
int PID(){
	Read_VCP((uint8_t*)temp_str, &cmp_len);
	targetTemperature = custom_atof(temp_str);

	float error = targetTemperature - temperature; // Current temperature error
	float P = Kp * error;// P component
	Integral += Ki * error;

	//Integral limitation
	if (Integral > IntMax){
		Integral = IntMax;
	}
	if (Integral < IntMin){
		Integral = IntMin;
	}
	float I = Integral;// I component

	float derivative = error - errorPrevious;
	float D = Kd * derivative;// D component

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

// Global USB socket and SVLP writer
svlp::USBSocket usbSocket;
svlp::SVLP_Writer* writer = nullptr;

// SVLP message codes (you can define your own protocol)
#define MSG_CODE_TEMPERATURE_DATA  0x01
#define MSG_CODE_STATUS           0x02
#define MSG_CODE_ERROR            0x03

// Function to send temperature data using SVLP
void SendTemperatureData(float currentTemp, float targetTemp, int power) {
    if (!writer) return;
    
    TemperatureData data;
    data.current_temperature = currentTemp;
    data.target_temperature = targetTemp;
    data.power_output = power;
    
    // Create SVLP message with temperature data
    svlp::svlp_message msg = writer->create_svlp_message(
        0x00,  // info_code (you can define your own)
        MSG_CODE_TEMPERATURE_DATA,
        data   // The temperature data structure
    );
    
    // Send the message
    writer->send_message(msg);
}

// Function to send status messages using SVLP
void SendStatusMsg(const char* status) {
    if (!writer || !status) return;
    
    // Create SVLP message with status string
    svlp::svlp_message msg = writer->create_svlp_message(
        0x00,  // info_code
        MSG_CODE_STATUS,
        status,
        strlen(status)
    );
    
    // Send the message
    writer->send_message(msg);
}

// Legacy function for backward compatibility (now uses SVLP)
void SendMsg(const char *text) {
	SendStatusMsg(text);
}

// Timer initialization for heater control
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
    // USB default parameter setup
    LC_DATA_RATE = 115200;
    LC_STOP_BITS = 0;
    LC_PARITY_TYPE = 0;
    LC_N_BITS_DATA = 8;

    // Disable all interrupts
    NVIC->ICER[0] = 0xFFFFFFFF;

    ClockInit();
    USBInit();
    TIMEInit();
    SysTick_Init();
    temperatureInit();

    // Initialize USB socket and SVLP writer
    usbSocket.open();
    writer = new svlp::SVLP_Writer(usbSocket, true); // Enable CRC
    
    // Send initialization message
    SendStatusMsg("System initialized");

    while(1) {
        USB_MANAGEMENT();
        updateTemp();
        temperature = getTemp();
        
        int powerOutput = PID();
        
        // Send structured temperature data using SVLP
        SendTemperatureData(temperature, targetTemperature, powerOutput);
        
        Delay_ms_USB(1000);
    }
}
