/**************************************************************************************************
 TIMER_SERVICE.C

 Служба времени
   
 Пакеты данных отправляются с интервалом в 14 мс.
 Задержка реализовывается на таймере SysTick либо на прерываниях таймера общего назначения.
 
 Используется TIM4.
 
 Разработчик: Подлесный Василий
**************************************************************************************************/

#include "timerService.h"

/**************************************************************************************************
                                          ВНЕШНИЕ ПЕРЕМЕННЫЕ
**************************************************************************************************/
extern uint8_t SBUSDataMessage[SBUS_PacketSize];		// пакет SBUS протокола
extern int16_t SBUSChannelValues[SBUS_ChannelSize];		// массива значений, заносимых в каналы управления

/**************************************************************************************************
                                       ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
**************************************************************************************************/
static volatile uint32_t TimingDelay;

/**************************************************************************************************
                                        ГЛОБАЛЬНЫЕ ФУНКЦИИ
**************************************************************************************************/

/**************************************************************************************************
Описание:  Обеспечивает задержку
Аргументы: Время задержки в миллисекундах
Возврат:   Нет
Замечания: 
**************************************************************************************************/
void Delay(__IO uint32_t nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0); 
}

/**************************************************************************************************
Описание:  Декремент занчений счетчика таймера
Аргументы: Нет
Возврат:   Нет
Замечания: 
**************************************************************************************************/
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}

/**************************************************************************************************
Описание:  Обработчик прерываний от системного таймера
Аргументы: Нет
Возврат:   Нет
Замечания: 
**************************************************************************************************/
void SysTick_Handler(void) 
{
	TimingDelay_Decrement();
}

/**************************************************************************************************
Описание:  Инициализация таймера общего назначения для отправки пакетов в 14 мс
Аргументы: Нет
Возврат:   Нет
Замечания:
**************************************************************************************************/
void InitTimer(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	NVIC_InitTypeDef  NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef timer;
	
	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = PRESCALER;
	timer.TIM_Period = PERIOD;
	TIM_TimeBaseInit(TIM4, &timer);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_EnableIRQ(TIM4_IRQn);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

/**************************************************************************************************
Описание:  Обработчик прерываний по таймеру TIM4
Аргументы: Нет
Возврат:   Нет
Замечания:
**************************************************************************************************/

void TIM4_IRQHandler()
{
 	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	USART_ITConfig( USART2, USART_IT_TXE, ENABLE );
// 	SendSBUS(SBUSDataMessage, SBUS_PacketSize);
}


/**************************************************************************************************
Описание:  Инициализация таймера общего назначения для создание пауз в 1000 мс 
Аргументы: Нет
Возврат:   Нет
Замечания:
**************************************************************************************************/
void InitTimerPause(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	NVIC_InitTypeDef  NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef timer;
	
	TIM_TimeBaseStructInit(&timer);
	timer.TIM_Prescaler = PRESCALER_PAUSE;
	timer.TIM_Period = PERIOD_PAUSE;
	TIM_TimeBaseInit(TIM3, &timer);
		
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

/**************************************************************************************************
Описание:  Обработчик прерываний по таймеру TIM3
Аргументы: Нет
Возврат:   Нет
Замечания:
**************************************************************************************************/
int j = VALUE_MIN;
int flag = 0;
void TIM3_IRQHandler()
{
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
// 	for(int i = 0; i <= 15; i++)
// 	{
// 		SBUSChannelValues[i] = VALUE_ZERO;
// 	}
	if(flag == 0)
	{
		for(int i = 0; i <= 15; i++)
		{
			SBUSChannelValues[i] = VALUE_MIN;
		}
		flag = 1;
	}
	else
	{
		for(int i = 0; i <= 15; i++)
		{
			SBUSChannelValues[i] = VALUE_MAX;
		}
		flag = 0;
	}
	
	MakeSBUSmsg(SBUSDataMessage, SBUSChannelValues);
}
