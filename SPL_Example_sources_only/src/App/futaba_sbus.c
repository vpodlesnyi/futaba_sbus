/**************************************************************************************************
 FUTABA_SBUS.C
 
 Описание протокола, формата пакета данных, настроек протокола.
 
 S.Bus – протокол передачи команд сервоприводам по цифровому последовательному порту.
 
 Параметры последовательного порта:
 1. 1 стартовый бит.
 2. 8 бит данных.
 3. 1 бит четности.
 4. 2 стоповых бита.
 5. скорость 100 000 бит/с.
 
 Параметры протокола:
 1. длина одного пакета - 25 байт.
 2. частота отправки - 14 мс.
 3. старший бит отсылается первым.
 4. передача информация о 16-и каналах передается с разрешением 11бит на каждый каждый канал.
 5. передача информации о двух цифровых каналах 1бит на каждый канал.
 6. длина посылки - 3 мс.
 7. сигнал должен быть проинвертирован. !!!!!!!
 
 !!! ACHTUNG !!!
 Длина посылки в миллисекундах должна составлять 3 мс.
 Тестирование проводилось на отладочной плате STM32F103 NUCLEO,
 при заданных 100 000 бит/с не удавалось добиться длины в 3 мс, поэтому 
 скорость пришлось снизить до 91 000 бит/с, дабы уложиться в длину посылки.
 
	Структура пакета:
 
	-----------------------------------------------------------------
	| startbyte | data1 data2 ... data ... ... data22 | flags | endbyte |
	---------------------------------------------------------------------
		 0         1     2   ... 	  ... ...   22		 23       24
		 
	startbyte = 0x0F;
   
	data 1-22 = [ch1, 11bit][ch2, 11bit] .... [ch16, 11bit]
	Размерность ch# = 0 bis 2047
	channel 1 uses 8 bits from data1 and 3 bits from data2
	channel 2 uses last 5 bits from data2 and 6 bits from data3
	etc.
   
	flags:
	bit7 = n/a
	bit6 = n/a
	bit5 = n/a
	bit4 = n/a
	bit3 = failsafe activated (0x10)
	bit2 = Frame lost, equivalent red LED on receiver (0x20)
	bit1 = ch18 = цифровой канал 1(0x40)
	bit0 = ch17 = цифровой канал 2(0x80)
   
	endbyte = 0x00;
   
   Используется UART1, DMA1_Channel4.
   
   
   Разработчик: Подлесный Василий
**************************************************************************************************/

#include "futaba_sbus.h"

/**************************************************************************************************
                                        ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
**************************************************************************************************/		
uint8_t SBUSDataMessage[SBUS_PacketSize];		// пакет SBUS протокола		
int16_t SBUSChannelValues[SBUS_ChannelSize];	// массива значений, заносимых в каналы управления


/**************************************************************************************************
                                        ГЛОБАЛЬНЫЕ ФУНКЦИИ
**************************************************************************************************/

/**************************************************************************************************
Описание:  Инициализация UART
Аргументы: Нет
Возврат:   Нет
Замечания:
**************************************************************************************************/
void InitSBUSuart(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	USART_InitTypeDef usart;
	GPIO_InitTypeDef port;
	
	GPIO_StructInit(&port);
	port.GPIO_Mode = GPIO_Mode_AF_PP;
	port.GPIO_Pin = PIN_TX;
	port.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &port);

	port.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	port.GPIO_Pin = PIN_RX;
	port.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &port);

	USART_StructInit(&usart);
	usart.USART_BaudRate = BAUDRATE;
	usart.USART_WordLength = USART_WordLength_8b;
	usart.USART_StopBits = USART_StopBits_2;
	usart.USART_Parity = USART_Parity_Even;
	usart.USART_Mode = USART_Mode_Tx;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &usart);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART1, ENABLE);
}

/**************************************************************************************************
Описание:  Реализация передачи данных UART посредством DMA
Аргументы: buf - массив передаваемых значений, len - длина передаваемого массива
Возврат:   Нет
Замечания:
**************************************************************************************************/
void SendSBUS(uint8_t* buf, uint32_t len)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel4);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init( DMA1_Channel4, &DMA_InitStructure );

	DMA_Cmd(DMA1_Channel4, ENABLE);	
}

/**************************************************************************************************
Описание:  Формирование пакета SBUS протокола
Аргументы: buf - массив данных пакета, channelValues - массив значений, заносимых в каналы управления
Возврат:   Нет
Замечания:
**************************************************************************************************/
void MakeSBUSmsg(uint8_t* buf, int16_t* channelValues)
{
	buf[0] = 0x0F;
	
	uint8_t byteInBuffer = 1;
	uint8_t bitInByte = 0;
	uint8_t tmpByte = 0;
	uint8_t channel = 0;
	uint8_t bitInChannel  = 0;
	int16_t tmpChannel = 0;
	
	while(channel < 16)
	{
		tmpChannel = channelValues[channel];
		bitInChannel = 0;
		while(bitInChannel < 11)
		{
			if(tmpChannel & (1 << bitInChannel))
			{
				tmpByte |= (1 << bitInByte);
			}
			bitInChannel++;   
			bitInByte++;
			
			if(bitInByte == 8)
			{
				buf[byteInBuffer] = tmpByte;
				tmpByte = 0;
				bitInByte = 0;
				byteInBuffer++;
			}
		}	
		channel++;
	}
	
	buf[23] = 0x00;

	buf[24] = 0x00;
}

