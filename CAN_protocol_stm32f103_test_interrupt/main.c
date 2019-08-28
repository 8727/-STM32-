/**
  *****************************************************************************
  * @title		main.c
  * @platform	STM32F103
  * @author		Dmitry Ovcharenko (https://www.smartmode.info/)
  * @version	V1.0.0
  * @date		27.07.2016
  *
  * @brief		������ ��� ������������ CAN �� ���� ����������������� STM32F103
  *				�������� ���������� bxCan, ��������� ������
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
  * @brief  ��������� ������������ ����������������
  * @brief  ���� - 64MHz, ��������� APB1 - 16 MHz
  *
  * @note   ��� ������� ������������� ��� ��������� ����������������� ����� STM32F103C8.
  * @param  None
  * @retval None
  */
void RCC_Config(void)
{
	// ��� ��������� CAN � ������������ ������ ������ �� �������� �� 1Mb ��� ����������
	// ��������� ������� ��������� APB1 �� 16 MHz

	RCC_ClocksTypeDef RCC_Clocks;
	ErrorStatus HSEStartUpStatus;


	// ������� ��������� ������������ �������
	RCC_DeInit();												// RCC system reset

	// ������� ������� �����, ��� �������� �������
	RCC_HSEConfig(RCC_HSE_ON);									// Enable HSE

	HSEStartUpStatus = RCC_WaitForHSEStartUp();					// �������� ��������� HSE

	if (HSEStartUpStatus == SUCCESS)							// ���� ��������� �����
	{
		// �������� ������������ ���, ��� ��� ���������
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						// HCLK = SYSCLK     (64MHz)
		RCC_PCLK1Config(RCC_HCLK_Div4);							// PCLK1 = HCLK / 8  (16MHz)
		RCC_PCLK2Config(RCC_HCLK_Div1);							// PCLK2 = HCLK	     (64MHz)
		RCC_ADCCLKConfig(RCC_PCLK2_Div2);						// ADC CLK

		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_8); 	// PLLCLK = 8MHz * 8 = 64 MHz
		RCC_PLLCmd(ENABLE);										// �������� PLL

		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}	// ���� ��������� PLL

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 				// �������� PLL ��� ��������
																// ���������� ������������

		while (RCC_GetSYSCLKSource() != 0x08) {}				// ����, ���� �� ����������� PLL,
																// ��� �������� ���������� ������������
	}

	// ������������ ��� �������, ���������, ��� ��������� ������� ����������
	RCC_GetClocksFreq (&RCC_Clocks);
}


int main(void)
{
	uint16_t Count = 0;	// �������� �������

	// ����������� ������������
	RCC_Config();

	// �������� ������������� CAN
	init_CAN();

	while(1)
	{
		// ���������� �������
		if (Count == 0xffff)
		{
			// ���� ����� �� ���������, �� �������� ��������� � ����
			Count = 0;
			CAN_Send_Test();

		} else {
			// ����� - �������� ������� �� �������
			Count++;
		}
    }

}

