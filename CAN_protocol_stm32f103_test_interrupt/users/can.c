/**
  *****************************************************************************
  * @title		can.c
  * @platform	STM32F103
  * @author		Dmitry Ovcharenko (https://www.smartmode.info/)
  * @version	V1.1.0
  * @date		27.07.2016
  *
  * @brief		������ CAN - �������������, �����-��������
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
  * @brief  ��������� ������ �����-������, CAN
  * @note   ��� ������� ������������� ��� ��������� ����������������� ����� STM32F103.
  * @param  None
  * @retval None
  */
void init_CAN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* CAN GPIOs configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 		// �������� ������������ AFIO
	RCC_APB2PeriphClockCmd(CAN1_Periph, ENABLE); 				// �������� ������������ �����

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);		// �������� ������������ CAN-����

	// ����������� CAN RX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_RX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	// ����������� CAN TX pin
	GPIO_InitStructure.GPIO_Pin   = CAN1_TX_SOURCE;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN1_GPIO_PORT, &GPIO_InitStructure);

	#ifdef CAN1_ReMap
		GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);			// ��������� Can1 �� PB8, PB9
	#endif

	// ������������� ����
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
//	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;			// ������� ����� ������ ����������
	CAN_InitStructure.CAN_Mode = CAN_Mode_Silent_LoopBack;	// ��� ������������ ��� ������������ ��������� ����
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = CAN1_SPEED_PRESCALE;	// �������� ������ ��������
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
	// �������������� � ���������� USB_HP_CAN1_TX_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);			// ���������� ��� ������������ ���������� ��������� �����

	// CAN Receive Interrupt enable
	// �������������� � ���������� USB_LP_CAN1_RX0_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);		// ���������� ��������� ������ � ����� FIFO 0
	CAN_ITConfig(CAN1, CAN_IT_FF0, ENABLE);			// ���������� ��� ���������� ������ FIFO 0
	CAN_ITConfig(CAN1, CAN_IT_FOV0, ENABLE);		// ���������� ��� ������������ ������ FIFO 0

	// �������������� � ���������� CAN1_RX1_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);		// ���������� ��������� ������ � ����� FIFO 1
	CAN_ITConfig(CAN1, CAN_IT_FF1, ENABLE);			// ���������� ��� ���������� ������ FIFO 1
	CAN_ITConfig(CAN1, CAN_IT_FOV1, ENABLE);		// ���������� ��� ������������ ������ FIFO 1

	// CAN Operating Mode Interrupt enable
	// �������������� � ���������� CAN1_SCE_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_WKU, ENABLE);			// ���������� ��� "�����������" - ����� �� "�������" ������
	CAN_ITConfig(CAN1, CAN_IT_SLK, ENABLE);			// ���������� ��� �������� � "������" �����

	// CAN Error Interrupts
	// �������������� � ���������� CAN1_SCE_IRQHandler
	CAN_ITConfig(CAN1, CAN_IT_EWG, ENABLE);			// Error warning Interrupt (error counter >= 96)
	CAN_ITConfig(CAN1, CAN_IT_EPV, ENABLE);			// Error passive Interrupt (error counter > 127)
	CAN_ITConfig(CAN1, CAN_IT_BOF, ENABLE);			// Bus-off Interrupt (error counter > 255)
	CAN_ITConfig(CAN1, CAN_IT_LEC, ENABLE);			// Last error code - ��� ������������� ������ ������-��������
	CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);			// ���������� ��� ������������� ������ bxCan


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
  * @brief  ���������� ���������� CAN ����
  * @note   CAN1 TX0 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void USB_HP_CAN1_TX_IRQHandler(void)
{
	// CAN Transmit mailbox empty Interrupt enable
	// ���������� ���������� ��� ������������ ���������� ��������� �����
	if (CAN_GetITStatus(CAN1, CAN_IT_TME)==SET) { 			// ���������� ��� ������������ ���������� ��������� �����
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME);

		// ��������� ���� ��� �� ��������� ����������
		}
}



/**
  * @brief  ���������� ���������� CAN ����
  * @note   CAN1 RX0 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;

	// CAN Receive Interrupt enable FIFO 0
	// ���������� ���������� ��������� ������ FIFO 0
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET) {		// ���������� ��������� ������ � ����� FIFO 0

		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);			// ������� ���� ����������

		// ������� ������ ������
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

		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);			// ������� ���������

		// ��������� ����� ���� ��� ��������� ��������� ������

		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FF0)==SET) { 			// ���������� ��� ���������� ������ FIFO 0
		CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);

		// ��������� ���� ��� �� ��������� ����������

		// �� �������� ����� ��������� �������� ���� ������
		CAN_ClearFlag(CAN1, CAN_FLAG_FF0);
		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FOV0)==SET) { 			// ���������� ��� ������������ ������ FIFO 0
		CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);

		// ��������� ���� ��� �� ��������� ����������

		// �� �������� ����� ��������� �������� ���� ������
		CAN_ClearFlag(CAN1, CAN_FLAG_FOV0);
		}
}



/**
  * @brief  ���������� ���������� CAN ����
  * @note   CAN1 RX1 interrupt IRQ channel
  * @param  None
  * @retval None
  */
void CAN1_RX1_IRQHandler(void)
{
	CanRxMsg RxMessage;

	// CAN Receive Interrupt enable FIFO 1
	// ���������� ���������� �������� ������ FIFO 1
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP1) == SET) {		// ���������� ��������� ������ � ����� FIFO 1

		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);			// ������� ���� ����������

		// ������� ������ ������
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

		CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);			// ������� ���������

		// ��������� ����� ���� ��� ��������� ��������� ������

		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FF1)==SET) { 			// ���������� ��� ���������� ������ FIFO 1
		CAN_ClearITPendingBit(CAN1, CAN_IT_FF1);

		// ��������� ���� ��� �� ��������� ����������

		// �� �������� ����� ��������� �������� ���� ������
		CAN_ClearFlag(CAN1, CAN_FLAG_FF1);
		}

	if (CAN_GetITStatus(CAN1, CAN_IT_FOV1)==SET) { 			// ���������� ��� ������������ ������ FIFO 1
		CAN_ClearITPendingBit(CAN1, CAN_IT_FOV1);

		// ��������� ���� ��� �� ��������� ����������

		// �� �������� ����� ��������� �������� ���� ������
		CAN_ClearFlag(CAN1, CAN_FLAG_FF1);
		}
}



/**
  * @brief  ���������� ���������� CAN ����
  * @note   CAN1 SCE (Status Change Error) interrupt IRQ channel
  * @param  None
  * @retval None
  */
void CAN1_SCE_IRQHandler(void)
{
	uint8_t errorcode = 0;

	// ��������� ���������� errcnt ��� �������
	// ����� ���������� ��� ������ ������� ������ ������ � ��������
	// � ����� CAN ����������� � ����� Bus-Off
	errcnt = CAN_GetReceiveErrorCounter(CAN1);
	errcnt = CAN_GetLSBTransmitErrorCounter(CAN1);



	if (CAN_GetITStatus(CAN1, CAN_IT_ERR)==SET)	{		// ���������� ��� ������������� ������
		CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);

		// CAN Error Interrupts
		// ��������� ���������� �� ������
		if (CAN_GetITStatus(CAN1, CAN_IT_EWG)==SET) { 	// Error warning Interrupt (������� ������ >= 96)
			CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);

			// ��������� ���� ��� �� ��������� ����������
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_EPV)==SET) { 	// Error passive Interrupt  (������� ������ > 127)
			CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);

			// ��������� ���� ��� �� ��������� ����������
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_BOF)==SET) { 	// Bus-off. ���������� ��� ������������ �������� ������ (>255)
			CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);	// bxCan ������ � ����� Bus-OFF

			// ��������� ���� ��� �� ��������� ����������
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_LEC)==SET) { 	// ���������� ��� ������ ������ �������� ���������
			CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);

			errorcode = CAN_GetLastErrorCode(CAN1);			// ������� ��� ������

			// ��������� ���� ��� �� ��������� ����������

			// �� �������� ����� ��������� �������� ���� ������
			CAN_ClearFlag(CAN1, CAN_FLAG_LEC);
			}

	} else {

		// CAN Operating Mode Interrupt
		// ��������� ���������� �� ������� ���/�����������
		if (CAN_GetITStatus(CAN1, CAN_IT_WKU)==SET) {	// ���������� ��� "�����������" - ����� �� "�������" ������
			CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);

			// ��������� ���� ��� �� ��������� ����������

			// �� �������� ����� ��������� �������� ���� ������
			CAN_ClearFlag(CAN1, CAN_FLAG_WKU);
			}

		if (CAN_GetITStatus(CAN1, CAN_IT_SLK)==SET)	{	// ���������� ��� �������� � "������" �����
			CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);

			// ��������� ���� ��� �� ��������� ����������

			// �� �������� ����� ��������� �������� ���� ������
			CAN_ClearFlag(CAN1, CAN_FLAG_SLAK);
			}
	}
}





/**
  * @brief  �������� �������� ������� � ����
  * @note   ���������� ��� �������� �����
  * @param  None
  * @retval None
  */
void CAN_Send_Test(void)
{
    uint32_t i = 0;
    uint8_t TransmitMailbox = 0;
	CanTxMsg TxMessage;

	TxMessage.StdId = CAN_CMD_Test_Send;				// ������� ����

	TxMessage.ExtId = 0x00;								// ����������� ������� ��������� ��� ������

	TxMessage.IDE = CAN_Id_Standard;					// ������ �����
	TxMessage.RTR = CAN_RTR_DATA;						// ��� ���������
	TxMessage.DLC = 3;									// ����� ����� ������ 3 - ��������� ��� �����

	TxMessage.Data[0] = 0x00;							// ���� ������ �1
	TxMessage.Data[1] = 0x01;							// ���� ������ �2
	TxMessage.Data[2] = 0x02;							// ���� ������ �3

    TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);	// �������� ��������� � ������ ����� ��������� �����
    i = 0;

    while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK)
            && (i != 0xFF))
    {
        i++;
    }

    if (i==0xFF) {
    	// �������� ���� �� ����������
    	// ���������� ���������� ������

    }

}


/**
  * @brief  �������� ������������� ��������� �������� ������� � ����
  * @note   ���������� ������ ������� ��� ������
  * @param  None
  * @retval None
  */
void CAN_Send_Ok(void)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = CAN_CMD_Test_Ok;				// ������� ����

	TxMessage.ExtId = 0x00;							// ����������� ������� ��������� ��� ������

	TxMessage.IDE = CAN_Id_Standard;				// ������ �����
	TxMessage.RTR = CAN_RTR_DATA;					// ��� ���������
	TxMessage.DLC = 0;								// ����� ����� ������ 0 - ������ �� ��������

	CAN_Transmit(CAN1, &TxMessage);
}

