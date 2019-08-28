/**
  *****************************************************************************
  * @title		main.c
  * @platform	STM32F103
  * @author		Dmitry Ovcharenko (https://www.smartmode.info/)
  * @version	V1.0.0
  * @date		27.07.2016
  *
  * @brief		Проект для тестирования CAN на базе микроконтроллеров STM32F103
  *				Изучение прерываний bxCan, обработка ошибок
  *
  *******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 SmartMODE</center></h2>
*/


#include "stm32f10x.h"
#include "stm32f10x_rcc.h"

#include "can.h"

/**
  * @brief  Настройка тактирования микроконтроллера
  * @brief  Ядро - 64MHz, перефирия APB1 - 16 MHz
  *
  * @note   Эта функция предназначена для настройки микроконтроллеров серия STM32F103C8.
  * @param  None
  * @retval None
  */
void RCC_Config(void)
{
	// Для настройки CAN в максимальном режиме работы на скорости до 1Mb нам необходимо
	// Настроить частоту перефирии APB1 на 16 MHz

	RCC_ClocksTypeDef RCC_Clocks;
	ErrorStatus HSEStartUpStatus;


	// Сбросим настройки тактирования системы
	RCC_DeInit();												// RCC system reset

	// Включим внешний кварц, как источник сигнала
	RCC_HSEConfig(RCC_HSE_ON);									// Enable HSE

	HSEStartUpStatus = RCC_WaitForHSEStartUp();					// Подождем включения HSE

	if (HSEStartUpStatus == SUCCESS)							// Если включился кварц
	{
		// Настроим тактирование так, как нам требуется
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						// HCLK = SYSCLK     (64MHz)
		RCC_PCLK1Config(RCC_HCLK_Div4);							// PCLK1 = HCLK / 8  (16MHz)
		RCC_PCLK2Config(RCC_HCLK_Div1);							// PCLK2 = HCLK	     (64MHz)
		RCC_ADCCLKConfig(RCC_PCLK2_Div2);						// ADC CLK

		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_8); 	// PLLCLK = 8MHz * 8 = 64 MHz
		RCC_PLLCmd(ENABLE);										// Включаем PLL

		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}	// Ждем включения PLL

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 				// Выбираем PLL как источник
																// системного тактирования

		while (RCC_GetSYSCLKSource() != 0x08) {}				// Ждем, пока не установится PLL,
																// как источник системного тактирования
	}

	// Предназначен для отладки, проверяем, как настроены частоты устройства
	RCC_GetClocksFreq (&RCC_Clocks);
}


int main(void)
{
	uint16_t Count = 0;	// Настроим счетчик

	// Настраиваем тактирование
	RCC_Config();

	// Выполним инициализацию CAN
	init_CAN();

	while(1)
	{
		// Используем счетчик
		if (Count == 0xffff)
		{
			// Если дошли до максимума, то отправим сообщение в шину
			Count = 0;
			CAN_Send_Test();

		} else {
			// Иначе - увеличим счетчик на единицу
			Count++;
		}
    }

}

