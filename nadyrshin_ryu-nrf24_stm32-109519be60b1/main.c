//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "main.h"
#include <delay.h>
#include <disp1color.h>
#include <nrf24.h>
#include <gpio.h>
#include <font.h>


#define LED_Port        GPIOC
#define LED_Pin         GPIO_Pin_13
#define LED_ON()        GPIO_WriteBit(LED_Port, LED_Pin, Bit_RESET)
#define LED_OFF()       GPIO_WriteBit(LED_Port, LED_Pin, Bit_SET)


#define nRF_CHANNEL     70
#define nRF_isTX_DEMO   1       // Флаг работы демо-проекта в качестве передатчика
#define nRF_PaketLen    32      // Если размер пакета = 0, то используется переменная длина пакета


// Адрес соединения с другим микроконтроллером
uint8_t Addr[] = {0x01, 0x02, 0x03, 0x04, 0x05};
// Буфер пакета
uint8_t Buff[] = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x11, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x31, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

uint8_t Pipe = 0;       // Номер соединения
uint8_t Len = 0;        // Длина пакета
uint32_t Counter = 0;   // Счётчик пакетов
uint32_t TryCounter = 0;// Счётчик повторных попыток отправки пакета
uint32_t Err = 0;       // Счётчик ошибок при отправке
  

void main()
{
  SystemInit();

  gpio_PortClockStart(LED_Port);
  gpio_SetGPIOmode_Out(LED_Port, LED_Pin);
  
  // Инициализация nRF24
  nrf24_init(SPI1, nRF_CHANNEL);
  // Настройка соединения, установка адреса
  nrf24_RxPipe_Setup(0, Addr, nRF_PaketLen);      // Если размер пакета = 0, то используется переменная длина пакета
  // Инициализация дисплея
  disp1color_Init();
  // Установка яркости на максимум
  disp1color_SetBrightness(255);
  
#if (nRF_isTX_DEMO)
  Len = nRF_PaketLen;
  while (1)
  {
    LED_ON();
    int8_t TryNum = nrf24_Send(Addr, Buff, Len);        // Отправка пакета
    if (TryNum >= 0)            // Пакет успешно был отправлен
    {
      LED_OFF();  
      Counter++;                // Считаем кол-во отправленных пакетов
      TryCounter += TryNum;     // Считаем общее кол-во попыток отправки
    }    
    else
      Err++;                    // Считаем кол-во ошибок при отправке

    // Выводим статистику на дисплей
    disp1color_FillScreenbuff(0);
    disp1color_printf(0, 0, FONTID_10X16F, "250 kbps");
    disp1color_printf(0, 16, FONTID_10X16F, "Пакетов %d", Counter);
    disp1color_printf(0, 32, FONTID_10X16F, "Повторов %d", TryCounter);
    disp1color_printf(0, 48, FONTID_10X16F, "Ошибок %d", Err);
    disp1color_UpdateFromBuff();
    
    delay_ms(50);
  }
#else
  while (1)
  {
    Len = nrf24_Recv(&Pipe, Buff);      // Проверяем, есть ли входящие пакеты
    if (Len)    // Принят пакет из соединения № Pipe длиной Len
    {
      // Тут ваша обработка пакета
      
      Counter++;// Считаем кол-во принятых пакетов
    }
  }
#endif
}
