/**************************************************************************************************
 FUTABA SBUS PROTOCOL COMMUNICATION
 
 Низкоуровневая реализация протокола SBUS
 
 Процессор: STM32F103RBT6
 Среда и компилятор: Keil uVision 4
 Язык: C99
 
 Разработчик: Подлесный Василий
**************************************************************************************************/

#include <CMSIS/stm32f10x.h>
#include "futaba_sbus.h"
#include "timerService.h"
#include "NAZA_example.h"

extern uint8_t SBUSDataMessage[SBUS_PacketSize];		// пакет SBUS протокола
extern int16_t SBUSChannelValues[SBUS_ChannelSize];		// массива значений, заносимых в каналы управления

void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
	;
  }
}

int main(void)
{
	// включаем тактирование от кварца 8МГц
	RCC_DeInit(); //    сброс настроек тактового генератора
	RCC_HSICmd(DISABLE); // выключение внутреннего RC HSI 8МГц генератора
	RCC_HSEConfig(RCC_HSE_ON); //   включение внешнего тактового генератора
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE); //  На системную шину подаем тактирование с внешнего резонатора
	
	
// 	SysTick_Config(72000000/1000);
	
	__disable_irq();
	
	InitSBUSuart();
 	InitTimer();
  	InitTimerPause();	// пауза в 1 с	
	for(int i = 0; i <= 15; i++)
	{
		SBUSChannelValues[i] = VALUE_ZERO;
	}
 	
	MakeSBUSmsg( SBUSDataMessage, SBUSChannelValues);
// SBUSDataMessage[0]=(0xaa);
// SBUSDataMessage[1]=(0x7);
	
// 	

	__enable_irq ();
	
	while(1)
	{		
// 		__disable_irq();
// 		for( int i=0; i<SBUS_PacketSize; i++ )
// 		//for( int i=0; i<2; i++ )
// 		{
// 			
// 			//while( ! USART_GetFlagStatus( USART2, USART_FLAG_TXE ) );
// 			while( ! (USART2->SR & USART_SR_TXE) );
// 			USART2->DR = SBUSDataMessage[i];			
// 		}
// 		__enable_irq();
// 		
// 		Delay(15);
	}
	return 0;
}



