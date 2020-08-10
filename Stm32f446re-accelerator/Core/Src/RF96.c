#include "RF96.h"

// Константа для выбора несущей частоты
const uint8_t Rf96_FreqTbl[3][3] =
{
		{0x6C, 0x80, 0x00},//434 Мгц
		{0xD9, 0x00, 0x00},  // 868 Мгц. (Не проверялось) //{0xD9, 0x00, 0x24},  // 868 Мгц. (Не проверялось)
		{0xE4, 0xC0, 0x00}  // 915 Мгц

};
// Константа для выбора выходной мощности
const uint8_t Rf96_PowerTbl[4] =
{
  0xFF,                   //20 Дб
  0xFC,                   //17 Дб
  0xF9,                   //14 Дб
  0xF6,                   //11 Дб
};
// Константа для выбора SpreadFactor (SF)
const uint8_t Rf96_SpreadFactorTbl[7] =
{
  6,7,8,9,10,11,12
};
//  Константа для выбора полосы спектра
const uint8_t Rf96_LoRaBwTbl[10] =
{
//7.8KHz,10.4KHz,15.6KHz,20.8KHz,31.2KHz,41.7KHz,62.5KHz,125KHz,250KHz,500KHz
  0,1,2,3,4,5,6,7,8,9
};

//  Константа для выбора максимального тока
const uint8_t Rf96_OCPTbl[4] =
{
// без OCP, 100 мА, 120 мА, 200 мА
		0x0b,0x2B,0x2F,0x37
};
// Константа для выбора LNA
const uint8_t Rf96_LNATbl[2] =
{
// LNA выключен, Максимальное усиление,
		0x00,0x23
};
// Чтение значения RSSI
int16_t Rf96_LoRaReadRSSI(void)
{
  int16_t temp=0;
  temp=SPIRead(LR_RegRssiValue);
  temp=temp-157;
  return temp;
}

// Чтение значения RSSI пакета
int16_t Rf96_LoRaReadRSSIPkT(void)
{
  int16_t temp=0;
  temp=SPIRead(LR_RegPktRssiValue);
  temp=temp-157;
  return temp;
}
// Очистка всех флагов
void Rf96_LoRaClearIrq(void)
{
  SPIWrite(LR_RegIrqFlags,0xFF);
}

// Вход в standby мод
void Rf96_Standby(void)
{
  //SPIWrite(LR_RegOpMode,0x09);                              		//Standby//Low Frequency Mode
	SPIWrite(LR_RegOpMode,0x01);                              	 //Standby//High Frequency Mode
}

// Вход в sleep мод
void Rf96_Sleep(void)
{
  //SPIWrite(LR_RegOpMode,0x08);                              		//Sleep//Low Frequency Mode
	SPIWrite(LR_RegOpMode,0x00);                            		 //Sleep//High Frequency Mode
}

// Вход в Lora мод
void Rf96_EntryLoRa(void)
{
  //SPIWrite(LR_RegOpMode,0x88);//Low Frequency Mode
	SPIWrite(LR_RegOpMode,0x80);//High Frequency Mode
}
// Выбираем несущую частоту 0 - 434 Мгц  1 - 868 Мгц
void Rf96_FreqChoose(uint8_t freq_value)
{
	BurstWrite(LR_RegFrMsb,Rf96_FreqTbl[freq_value],3);

}
// Выбираем выходную мощность 0 -20 Дб, 1- 17 Дб, 2 - 14 Дб, 3 - 11 Дб
void Rf96_OutPower(uint8_t Power_value)
{
	SPIWrite(LR_RegPaConfig,Rf96_PowerTbl[Power_value]);
	SPIWrite(0x5A,0x87);  // Для ноги PA устанавливает Pmax до +20 Дб при 0x87  и оставляет по дефолту при 0x84 ???????
}
// защита по току( максимальный ток усилителя) ( важно ее правильно настроить, поскольку выходная мощность зависит от тока)
// 0 -Без ограничения по току, 1 - 100 мА , 2 - 120 мА, 3 -200 мА
void Rf96_OCP(uint8_t OCP_value)
{

	SPIWrite(LR_RegOcp,Rf96_OCPTbl[OCP_value]);
}
// Выбираем LNA  0 - LNA выключен, 1 - Максимальное усиление
void Rf96_LNA(uint8_t LNA_value)
{
	SPIWrite(LR_RegLna,Rf96_LNATbl[LNA_value]);

}
// Устанавливает несколько параметров: полосу частоты ( signal bandwidth), Coding rate, Мод заголовка (HeaderMod):
// Полоса частоты: 0 - 7.8KHz,1- 10.4KHz,2- 15.6KHz, 3- 20.8KHz,4- 31.2KHz,5- 41.7KHz,6- 62.5KHz,7- 125KHz,8- 250KHz,9- 500KHz
// Coding rate: 1 - 4/5, 2 - 4/6, 3 - 4/7, 4 - 4/8
// Мод заголовка : 0 -  явный, 1 - неявный
void Rf96_bandwide_CR_HeadreMod(uint8_t bandwide_value, uint8_t CR_Value, uint8_t HeaderMod_value)
{

	//SPIWrite(LR_RegModemConfig1,(0x00<<4+(CR_Value<<1)+HeaderMod_value));
	SPIWrite(LR_RegModemConfig1,0x8C); // 8C    Без CRC16
	//SPIWrite(LR_RegModemConfig1,0x8E); // С CRC16
	//SPIWrite(LR_RegDetectOptimize,0xC5);
	//SPIWrite(LR_RegDetecionThreshold,0x0C);
}
// Устанавливает несколько параметров:
// Spreading factor :  0-6,1-7,2-8,3-9,4-10,5-11,6-12
// payLoadCrcc: Выкл - 0, Вкл - 1
// Таймаут по RX:  Максимальноее значение 3FF, минимальное 0. Можно поставить любое в диапозон 0-3FF
void Rf96_SF_LoadCRC_SymbTimeout(uint8_t SF_value, uint8_t PayloadCrc_value, uint16_t SymbTimeout_value)
{
	//SPIWrite(LR_RegModemConfig2,((Rf96_SpreadFactorTbl[SF_value]<<4)+(PayloadCrc_value<<2)+(SymbTimeout_value>>8)));
	SPIWrite(LR_RegModemConfig2,0x74);
	SPIWrite(LR_RegSymbTimeoutLsb,(uint8_t)SymbTimeout_value);
}
//Устанавливаем длину преамбулы в байтах: 4+PreambLen_value
void Rf96_Preamble(uint16_t PreambLen_value)
{
	SPIWrite(LR_RegPreambleMsb,PreambLen_value>>8);
	SPIWrite(LR_RegPreambleLsb,(uint8_t)PreambLen_value);
}
// Настройка вывода Di0 0 - прерывание по приему, 1 - прерывание по передаче, Di1 0- прерывание по таймауту
void Rf96_PinOut_Di0_Di1_Di2_Di3(uint8_t Di0_value, uint8_t Di1_value,uint8_t Di2_value ,uint8_t Di3_value)
{
	SPIWrite(REG_LR_DIOMAPPING1,(Di0_value<<6)+(Di1_value<<4)+ (Di2_value<<2)+(Di3_value));
}
//Снятие маски с прерывания по TX
void Rf96_irqMaskTX(void)
{
	SPIWrite(LR_RegIrqFlagsMask,0xF7);
}
//Снятие маски с прерывания по RX
void Rf96_irqMaskRX(void)
{
	SPIWrite(LR_RegIrqFlagsMask,0x3F);
}
//Установка числа передаваемых данных (в байтах)
void Rf96_PayloadLength(uint8_t LengthBytes_value)
{
	SPIWrite(LR_RegPayloadLength,LengthBytes_value);
}
//Установка Адресса  TX в буфере
void Rf96_TX_FifoAdr(uint8_t TX_adr_value)
{
	SPIWrite(LR_RegFifoTxBaseAddr,TX_adr_value);
}
//Установка Адресса  RX в буфере
void Rf96_RX_FifoAdr(uint8_t RX_adr_value)
{
	SPIWrite(LR_RegFifoRxBaseAddr,RX_adr_value);
}
// Устанавливает указатель в FIFO
void Rf96_FIFO_point(uint8_t adrPoint_value)
{
	 SPIWrite(LR_RegFifoAddrPtr,adrPoint_value);
}
// Вход в режим приема ( один пакет)
void Rf96_RX_Single_mode(void)
{
	//SPIWrite(LR_RegOpMode,0x8E);
	SPIWrite(LR_RegOpMode,0x86);                            		//High Frequency Mode
}
// Вход в режим приема (много пакетный прием)
void Rf96_RX_Continuous_mode(void)
{
	//SPIWrite(LR_RegOpMode,0x8D);
	SPIWrite(LR_RegOpMode,0x85);                            		//High Frequency Mode
}
// Вход в режим передачи
void Rf96_TX_mode(void)
{
	//SPIWrite(LR_RegOpMode,0x8B);
	SPIWrite(LR_RegOpMode,0x83);                            		 //High Frequency Mode
}

// Запись данных в FiFO
void Rf96_DataTX_to_FiFO(char* str, uint8_t LenghtStr)
{
	BurstWrite(LR_RegFifo, (uint8_t *)str, LenghtStr);
}
//  Чтение данных RX из FIFO по последнему пришедшему пакету
void Rf96_DataRX_From_FiFO(char* str)
{
	uint8_t addr;
	uint8_t packet_size;
	addr = SPIRead(LR_RegFifoRxCurrentaddr);
	SPIWrite(LR_RegFifoAddrPtr,addr);
	packet_size = SPIRead(LR_RegRxNbBytes);
	SPIBurstRead(LR_RegFifo, str, packet_size);
}
// Настройка Rf96
void Rf96_Lora_init(void)
{
	// Входим в Sleep mode
	Rf96_Sleep();
	// Входим в Lora мод
	Rf96_EntryLoRa();
	// выбираем несущую частоту
	Rf96_FreqChoose(1);
	// Выбираем выходную мощность
	Rf96_OutPower(0);
	// Выбираем ограничение по току
	Rf96_OCP(0);
	// Выбираем LNA
	Rf96_LNA(1);
	// Выбираем полосу частот, Coding rate, и мод заголовка
	Rf96_bandwide_CR_HeadreMod(7,4,0);
	// Выбираем Spreading factor, включение-выключение loadCRC,Таймаут по RX
	Rf96_SF_LoadCRC_SymbTimeout(6,1,0x3FF);
	// Устанавливаем длину преамбулы
	Rf96_Preamble(8);
	// Заходим в StandBy
	Rf96_Standby();
}
/*
// Включаем нужные выводы на антенну по Tx
void RAK811antTx(void)
{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); //RADIO_RF_CRX_RX
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //RADIO_RF_CBT_HF
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);  //RADIO_RF_CTX_PA

}
*/
/*
// Включаем нужные выводы на антенну по Rx
void RAK811antRx(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); //RADIO_RF_CRX_RX
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //RADIO_RF_CBT_HF
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);  //RADIO_RF_CTX_PA

}
*/




// Инициализация TX
void Rf96_Lora_TX_mode(void)
{
	//RAK811antTx();
	  // Настройка вывода Di0 на прерывание по отправке
	Rf96_PinOut_Di0_Di1_Di2_Di3(1,0,0,2);
      // Сброс всех флагов
	  Rf96_LoRaClearIrq();
	  // Снимаем маску с прерывания по TX
	  Rf96_irqMaskTX();
	  // Устанавливаем длину передаваемых данных (в байтах)
	  Rf96_PayloadLength(29);
	  // Установка адреса TX в буфере FIFO
	  Rf96_TX_FifoAdr(0x80);
	  // Устанавливает указатель на адрес начала массива TX в FIFO
	  Rf96_FIFO_point(0x80);

}
// Инициализация RX
void Rf96_Lora_RX_mode(void)
{
	  //RAK811antRx();
	//  SPIWrite(REG_LR_PADAC,0x84);                            //Normal and Rx
	//  SPIWrite(LR_RegHopPeriod,0xFF);   //??????                       //RegHopPeriod NO FHSS
	  SPIWrite(LR_RegHopPeriod,0x00);   //??????
	  // Настройка вывода Di0 на прерывание по приему, Di1 на прерывание по таймауту
	  Rf96_PinOut_Di0_Di1(0,0);
	  // Снимаем маску с прерывания по RX
	  Rf96_irqMaskRX();
	  // Сброс всех флагов
	  Rf96_LoRaClearIrq();
	  // Устанавливаем длину передаваемых данных (в байтах)
	  Rf96_PayloadLength(29);
      // Установка адреса RX в буфере FIFO
	  Rf96_RX_FifoAdr(0x00);
	  // Устанавливает указатель на адрес начала массива RX в FIFO
	  Rf96_FIFO_point(0x00);
	  // Входим в RX single мод
	//  Rf96_RX_Single_mode();
	  Rf96_RX_Continuous_mode();

}

// Отправка пакета данных
void Rf96_LoRaTxPacket(char* Str, uint8_t LenStr)
{
    // Записываем данные в буфер
    Rf96_DataTX_to_FiFO(Str,LenStr);
	// Вход в режим передачи
	Rf96_TX_mode();


	// Ждем пока появится прерывание
/*
	while(1)
	{
		if(Get_NIRQ_Di0())
		{
			// Сбрасываем флаги
			Rf96_LoRaClearIrq();
            // Заходим в Standby
			Rf96_Standby();

			break;
		}
	}
*/

}
// Прием пакета данных
void Rf96_LoRaRxPacket(char* Str)
{

// Ждем прерывания по приему
 if(Get_NIRQ_Di0())
  {
    // Читаем данные из FIFO
    Rf96_DataRX_From_FiFO(Str);
    // Сбрасываем флаги
    Rf96_LoRaClearIrq();
    // Заходим в Standby
    Rf96_Standby();
  }

}
