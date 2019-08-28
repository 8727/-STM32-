/**
  *****************************************************************************
  * @title		can.h
  * @platform	STM32F103
  * @author		Dmitry Ovcharenko (https://www.smartmode.info/)
  * @version	V1.1.0
  * @date		27.07.2016
  *
  * @brief		Заголовочный файл + список команд
  *
  *******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 SmartMODE</center></h2>
*/


#ifndef __CAN
#define __CAN


/******************************************************************************
 * Определение настройки CAN
 ******************************************************************************/
#define CAN1_ReMap // Закоментировать, если нет ремапинга портов

#ifndef CAN1_ReMap
	#define CAN1_GPIO_PORT			GPIOA
	#define CAN1_RX_SOURCE			GPIO_Pin_11				// RX-порт
	#define CAN1_TX_SOURCE			GPIO_Pin_12				// TX-порт
	#define CAN1_Periph				RCC_APB2Periph_GPIOA	// Порт перифирии
#else
	#define CAN1_GPIO_PORT			GPIOB
	#define CAN1_RX_SOURCE			GPIO_Pin_8				// RX-порт
	#define CAN1_TX_SOURCE			GPIO_Pin_9				// TX-порт
	#define CAN1_Periph				RCC_APB2Periph_GPIOB	// Порт перифирии
#endif

// Выбор скорости шины
// Необходимо раскомментировать нужную строку
//#define CAN1_SPEED_PRESCALE				1						// 1000 Kb
// #define CAN1_SPEED_PRESCALE			2						// 500 Kb
 #define CAN1_SPEED_PRESCALE			4						// 250 Kb
// #define CAN1_SPEED_PRESCALE			8						// 125 Kb
// #define CAN1_SPEED_PRESCALE			10						// 100 Kb
// #define CAN1_SPEED_PRESCALE			12						// 83.3 Kb
// #define CAN1_SPEED_PRESCALE			20						// 50 Kb
// #define CAN1_SPEED_PRESCALE			50						// 20 Kb
// #define CAN1_SPEED_PRESCALE			100						// 10 Kb



/******************************************************************************
 * Определение команд протокола
 ******************************************************************************/
#define CAN_CMD_Test_Send			0x0001		// Команда отправки тестового сообщения
#define CAN_CMD_Test_Ok				0x0002		// Команда подтверждения тестового сообщения




/******************************************************************************
 * Определение прототипов функций
 ******************************************************************************/
void init_CAN(void);
void USB_LP_CAN1_RX0_IRQHandler(void);

void CAN_Send_Test(void);
void CAN_Send_Ok(void);

#endif //__CAN
