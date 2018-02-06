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

void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
	;
  }
}

int main(void)
{
	__enable_irq ();
	InitSBUSuart();
	SysTick_Config( SystemCoreClock/1000 );
  	InitTimer();
	while(1)
	{
	}
	return 0;
}



