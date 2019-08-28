/**
  *****************************************************************************
  * @title		can.c
  * @platform	STM32F103
  * @author		Dmitry Ovcharenko (https://www.smartmode.info/)
  * @version	V1.1.0
  * @date		27.07.2016
  *
  * @brief		Модуль CAN - инициализация, прием-передача
  *
  *******************************************************************************
  * @attention
  *
  *
  * <h2><center>&copy; COPYRIGHT 2016 SmartMODE</center></h2>
*/


#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"
#include "misc.h"

#include "can.h"

uint8_t errcnt = 0;

/**
  * @brief  Настройка портов ввода-вывода, CAN
  * @note   Эта функция предназначена для настройки микроконтроллеров серия STM32F103.
  * @param  None
  * @retval None
  */
void init_CAN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* CAN GPIOs configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 		// включаем тактирование AFIO
	RCC_APB2PeriphClockCmd(CAN1_Periph, ENABLE); 				// включаем тактирование порта

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);		// включаем тактирование CAN-шины

	// Настраиваем CAN RX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_RX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	// Настраиваем CAN TX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_TX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	#ifdef CAN1_ReMap
		GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);			// Переносим Can1 на PB8, PB9
	#endif

	// Инициализация шины
	CAN_InitTypeDef CAN_InitStructure;

	CAN_DeInit( CAN1);
	CAN_StructInit(&CAN_InitStructure);

	// CAN cell init
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
//	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;			// Обычный режим работы устройства
	CAN_InitStructure.CAN_Mode = CAN_Mode_Silent_LoopBack;	// Для тестирования без подключенных устройств шины
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = CAN1_SPEED_PRESCALE;	// Выбираем нужную скорость
	CAN_Init(CAN1, &CAN_InitStructure);

	// CAN filter init
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 1;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdHigh =  0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	// CAN Transmit mailbox empty Interrupt enable
	// Обрабатывается в прерывании USB_HP_CAN1_TX_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);			// Прерывание при освобождении исходящего почтового ящика

	// CAN Receive Interrupt enable
	// Обрабатывается в прерывании USB_LP_CAN1_RX0_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);		// Прерывание получения пакета в буфер FIFO 0
	CAN_ITConfig(CAN1, CAN_IT_FF0, ENABLE);			// Прерывание при заполнении буфера FIFO 0
	CAN_ITConfig(CAN1, CAN_IT_FOV0, ENABLE);		// Прерывание при переполнении буфера FIFO 0

	// Обрабатывается в прерывании CAN1_RX1_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);		// Прерывание получения пакета в буфер FIFO 1
	CAN_ITConfig(CAN1, CAN_IT_FF1, ENABLE);			// Прерывание при заполнении буфера FIFO 1
	CAN_ITConfig(CAN1, CAN_IT_FOV1, ENABLE);		// Прерывание при переполнении буфера FIFO 1

	// CAN Operating Mode Interrupt enable
	// Обрабатывается в прерывании CAN1_SCE_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_WKU, ENABLE);			// Прерывание при "пробуждении" - выход из "спящего" режима
	CAN_ITConfig(CAN1, CAN_IT_SLK, ENABLE);			// Прерывание при переходе в "спящий" режим

	// CAN Error Interrupts
	// Обрабатывается в прерывании CAN1_SCE_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_EWG, ENABLE);			// Error warning Interrupt (error counter >= 96)
	CAN_ITConfig(CAN1, CAN_IT_EPV, ENABLE);			// Error passive Interrupt (error counter > 127)
	CAN_ITConfig(CAN1, CAN_IT_BOF, ENABLE);			// Bus-off Interrupt (error counter > 255)
	CAN_ITConfig(CAN1, CAN_IT_LEC, ENABLE);			// Last error code - при возникновении ошибок приема-передачи
	CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);			// Прерывание при возникновении ошибок bxCan


	// NVIC Configuration
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable CAN1 TX0 interrupt IRQ channel
	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable CAN1 RX0 interrupt IRQ channel
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable CAN1 RX1 interrupt IRQ channel
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable CAN1 SCE (Status Change Error) interrupt IRQ channel
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}




/**
  * @brief  Обработчик прерывания CAN шины
  * @note   CAN1 TX0 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void USB_HP_CAN1_TX_IRQHandler(void)
{
	// CAN Transmit mailbox empty Interrupt enable
	// Обработаем прерывания при освобождении исходящего почтового ящика
	if (CAN_GetITStatus(CAN1, CAN_IT_TME)==SET) { 			// Прерывание при освобождении исходящего почтового ящика
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME);

		// Вставляем свой код по обработке прерывания
		}
}



/**
  * @brief  Обработчик прерывания CAN шины
  * @note   CAN1 RX0 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;

	// CAN Receive Interrupt enable FIFO 0
	// Обработаем прерывания приемного буфера FIFO 0
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET) {		// Прерывание получения пакета в буфер FIFO 0

		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);			// Сбросим флаг прерывания

		// Обнулим данные пакета
		RxMessage.DLC = 	0x00;
		RxMessage.ExtId = 	0x00;
		RxMessage.FMI = 	0x00;
		RxMessage.IDE = 	0x00;
		RxMessage.RTR = 	0x00;
		RxMessage.StdId = 	0x00;
		RxMessage.Data [0] = 0x00;
		RxMessage.Data [1] = 0x00;
		RxMessage.Data [2] = 0x00;
		RxMessage.Data [3] = 0x00;
		RxMessage.Data [4] = 0x00;
		RxMessage.Data [5] = 0x00;
		RxMessage.Data [6] = 0x00;
		RxMessage.Data [7] = 0x00;

		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);			// Получим сообщение

		// Вставляем любой свой код обработки входящего пакета

		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FF0)==SET) { 			// Прерывание при заполнении буфера FIFO 0
		CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);

		// Вставляем свой код по обработке прерывания

		// Не забываем после обработки сбросить флаг ошибки
		CAN_ClearFlag(CAN1, CAN_FLAG_FF0);
		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FOV0)==SET) { 			// Прерывание при переполнении буфера FIFO 0
		CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);

		// Вставляем свой код по обработке прерывания

		// Не забываем после обработки сбросить флаг ошибки
		CAN_ClearFlag(CAN1, CAN_FLAG_FOV0);
		}
}



/**
  * @brief  Обработчик прерывания CAN шины
  * @note   CAN1 RX1 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void CAN1_RX1_IRQHandler(void)
{
	CanRxMsg RxMessage;

	// CAN Receive Interrupt enable FIFO 1
	// Обработаем прерывания приеного буфера FIFO 1
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP1) == SET) {		// Прерывание получения пакета в буфер FIFO 1

		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);			// Сбросим флаг прерывания

		// Обнулим данные пакета
		RxMessage.DLC = 	0x00;
		RxMessage.ExtId = 	0x00;
		RxMessage.FMI = 	0x00;
		RxMessage.IDE = 	0x00;
		RxMessage.RTR = 	0x00;
		RxMessage.StdId = 	0x00;
		RxMessage.Data [0] = 0x00;
		RxMessage.Data [1] = 0x00;
		RxMessage.Data [2] = 0x00;
		RxMessage.Data [3] = 0x00;
		RxMessage.Data [4] = 0x00;
		RxMessage.Data [5] = 0x00;
		RxMessage.Data [6] = 0x00;
		RxMessage.Data [7] = 0x00;

		CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);			// Получим сообщение

		// Вставляем любой свой код обработки входящего пакета

		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FF1)==SET) { 			// Прерывание при заполнении буфера FIFO 1
		CAN_ClearITPendingBit(CAN1, CAN_IT_FF1);

		// Вставляем свой код по обработке прерывания

		// Не забываем после обработки сбросить флаг ошибки
		CAN_ClearFlag(CAN1, CAN_FLAG_FF1);
		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FOV1)==SET) { 			// Прерывание при переполнении буфера FIFO 1
		CAN_ClearITPendingBit(CAN1, CAN_IT_FOV1);

		// Вставляем свой код по обработке прерывания

		// Не забываем после обработки сбросить флаг ошибки
		CAN_ClearFlag(CAN1, CAN_FLAG_FF1);
		}
}



/**
  * @brief  Обработчик прерывания CAN шины
  * @note   CAN1 SCE (Status Change Error) interrupt IRQ channel
  * @param  None
  * @retval None
  */
void CAN1_SCE_IRQHandler(void)
{
	uint8_t errorcode = 0;

	// Заполняем переменную errcnt для отладки
	// Чтобы посмотреть как растет счетчик ошибок приема и передачи
	// и когда CAN сваливается в режим Bus-Off
	errcnt = CAN_GetReceiveErrorCounter(CAN1);
	errcnt = CAN_GetLSBTransmitErrorCounter(CAN1);



	if (CAN_GetITStatus(CAN1, CAN_IT_ERR)==SET)	{		// Прерывание при возникновении ошибки
		CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);

		// CAN Error Interrupts
		// Обработка прерываний по ошибке
		if (CAN_GetITStatus(CAN1, CAN_IT_EWG)==SET) { 	// Error warning Interrupt (счетчик ошибок >= 96)
			CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);

			// Вставляем свой код по обработке прерывания
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_EPV)==SET) { 	// Error passive Interrupt  (счетчик ошибок > 127)
			CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);

			// Вставляем свой код по обработке прерывания
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_BOF)==SET) { 	// Bus-off. Прерывание при переполнении счетчика ошибок (>255)
			CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);	// bxCan уходит в режим Bus-OFF

			// Вставляем свой код по обработке прерывания
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_LEC)==SET) { 	// Прерывание при ошибке приема передачи сообщения
			CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);

			errorcode = CAN_GetLastErrorCode(CAN1);			// Получим код ошибки

			// Вставляем свой код по обработке прерывания

			// Не забываем после обработки сбросить флаг ошибки
			CAN_ClearFlag(CAN1, CAN_FLAG_LEC);
			}

	} else {

		// CAN Operating Mode Interrupt
		// Обработка прерываний по режимам сна/пробуждения
		if (CAN_GetITStatus(CAN1, CAN_IT_WKU)==SET) {	// Прерывание при "пробуждении" - выход из "спящего" режима
			CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);

			// Вставляем свой код по обработке прерывания

			// Не забываем после обработки сбросить флаг ошибки
			CAN_ClearFlag(CAN1, CAN_FLAG_WKU);
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_SLK)==SET)	{	// Прерывание при переходе в "спящий" режим
			CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);

			// Вставляем свой код по обработке прерывания

			// Не забываем после обработки сбросить флаг ошибки
			CAN_ClearFlag(CAN1, CAN_FLAG_SLAK);
			}
	}
}





/**
  * @brief  Отправка тестовой команды в шину
  * @note   Отправляем три тестовых байта
  * @param  None
  * @retval None
  */
void CAN_Send_Test(void)
{
    uint32_t i = 0;
    uint8_t TransmitMailbox = 0;
	CanTxMsg TxMessage;

	TxMessage.StdId = CAN_CMD_Test_Send;				// Команда шины

	TxMessage.ExtId = 0x00;								// Расширенную команду указывать нет смысла

	TxMessage.IDE = CAN_Id_Standard;					// Формат кадра
	TxMessage.RTR = CAN_RTR_DATA;						// Тип сообщения
	TxMessage.DLC = 3;									// Длина блока данных 3 - передадим три байта

	TxMessage.Data[0] = 0x00;							// Байт данных №1
	TxMessage.Data[1] = 0x01;							// Байт данных №2
	TxMessage.Data[2] = 0x02;							// Байт данных №3

    TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);	// Отправим сообщение и узнаем номер почтового ящика
    i = 0;

    while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK)
            && (i != 0xFF))
    {
        i++;
    }

    if (i==0xFF) {
    	// Почтовый ящик не осободился
    	// Необходимо обработать ошибку

    }

}


/**
  * @brief  Отправка подтверждения получения тестовой команды в шину
  * @note   Отправляем только команду без данных
  * @param  None
  * @retval None
  */
void CAN_Send_Ok(void)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = CAN_CMD_Test_Ok;				// Команда шины

	TxMessage.ExtId = 0x00;							// Расширенную команду указывать нет смысла

	TxMessage.IDE = CAN_Id_Standard;				// Формат кадра
	TxMessage.RTR = CAN_RTR_DATA;					// Тип сообщения
	TxMessage.DLC = 0;								// Длина блока данных 0 - ничего не передаем

	CAN_Transmit(CAN1, &TxMessage);
}

