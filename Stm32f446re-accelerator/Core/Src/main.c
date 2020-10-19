/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdarg.h>
#include "sdcard.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart5_rx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE BEGIN PV */

/*
 *  Режимы работы светодиодов:
 *
 *  1) Поочередное включение, затем выключение всех сразу, кроме 3го - Индикация того, что УСИ ПРД включился
 *  2) 1 светодиод горит - сд карта исправна, 1 светодиод не горит - проблема с сд картой
 *  3) 2 светодиод горит - прием с ЦКТ идет, 2 светодиод не горит - данные с ЦКТ не приходят
 *  4) 3 светодиод горит - остается гореть после включения УСИ ПРД
 */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI3_Init(void);
static void MX_UART5_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */
// Индекс комманды в массиве, пришедшего по радиоканалу
#define CommIndex 0
// Количество элементов в массиве для радиопередачи
#define RadioMaxBuff 42
// Количество элементов, приходящих с ЦКТ
#define MaxBuffOfCKT 43
// Адрес ячейки памяти
#define FilesAdr 0x08040000
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//  FATfs
FATFS FatFs;
//File object
FIL fil;
FRESULT fres;
UINT bytesWrote;
// время отсчета микрконтроллера в милисекундах
uint32_t reciveTime=0;
// CRC16
uint16_t CRC_c=0;
// Массив приемопередачи по радиоканалу
uint8_t TX_RX_Radio[RadioMaxBuff];
// Разрешение на запись на SD карту
uint8_t ResolveSDWrite=0;
// Готовность к записи на SD карту
uint8_t ReadyToWrite=0;
// Массив данных, пришедших с ЦКТ
uint8_t BuffCkt[MaxBuffOfCKT];
// Массив на запись на SD карту ( буфер посредник)
uint8_t BuffMidW[MaxBuffOfCKT];
// Для синхронизации с ЦКТ
uint8_t readFlag;
// Переменная запроса на отправку данных по радио
uint8_t RadioIrq=0;
// Буфер для записи на sd карту
uint8_t SDbufWrite[163];
// Счетчик данного файла
uint8_t CountFileNow=0;
// Массив с названием файла
const char MassFileName[10][10]=
	{{"Data0.txt\0"},
	{"Data1.txt\0"},
	{"Data2.txt\0"},
	{"Data3.txt\0"},
	{"Data4.txt\0"},
	{"Data5.txt\0"},
	{"Data6.txt\0"},
	{"Data7.txt\0"},
	{"Data8.txt\0"},
	{"Data9.txt\0"}};

// Функция чтения слова (8 бита) из памяти по адресу
uint32_t Flash_Read_single8bit(uint32_t Adr)
{
	return *(uint8_t*)Adr;
}

// Функция записи слова(8 бита) в память
void Flash_Write_single8bit(uint32_t Adr, uint32_t Data)
{
     HAL_FLASH_Unlock();
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,Adr,Data);
	 HAL_FLASH_Lock();

}
// Функция очистки Flash памяти
void flashErasePage(uint32_t sector)
{

	uint32_t secError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_SECTORS ;
	EraseInitStruct.Sector   = sector;
	EraseInitStruct.NbSectors = 1;
     HAL_FLASH_Unlock();
     HAL_FLASHEx_Erase(&EraseInitStruct, &secError);
	 HAL_FLASH_Lock();
}
// Таблица CRC16
const unsigned short Crc16Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
// Расчет CRC16
unsigned short Crc16(unsigned char * pcBlock, unsigned short len)
{
    unsigned short crc = 0xFFFF;

    while (len--)
        crc = (crc << 8) ^ Crc16Table[(crc >> 8) ^ *pcBlock++];

    return crc;
}

// Функция передачи по радиоканалу
void CommandToRadio(uint8_t command)
{
	// Вход в режим передачи
	 Rf96_Lora_TX_mode();
	// Обнуляем массив
	for(uint8_t i=0;i<RadioMaxBuff;i++)
	{
		TX_RX_Radio[i]=0;
	}
    // Заносим команду
	TX_RX_Radio[CommIndex]=command;
	//Подсчет CRC16
	CRC_c=Crc16(TX_RX_Radio,RadioMaxBuff-2);
	TX_RX_Radio[RadioMaxBuff-2]=(uint8_t)(CRC_c>>8);
	TX_RX_Radio[RadioMaxBuff-1]=(uint8_t)CRC_c;
	// Установка адреса TX в буфере FIFO
	Rf96_TX_FifoAdr(0x80);
	// Устанавливает указатель на адрес начала массива TX в FIFO
	Rf96_FIFO_point(0x80);
    // Очистка флагов
	Rf96_LoRaClearIrq();
    // Отправка посылки
	Rf96_LoRaTxPacket((char*)TX_RX_Radio,RadioMaxBuff);
}

// Перевод Числа в массив
/*
Number - переводимое число
mass - массив, в который число будет переведено
startMass - начальный элемент массива
len - длина переводимого числа
*/
void uint32_TO_charmass(uint32_t Number, uint8_t *mass, uint16_t startMass, uint16_t len)
{

	for (uint16_t i = 0; i < len; i++)
	{
		mass[len - 1 - i + startMass] = Number % 10 + 48;
		Number = Number / 10;
	}
}


// Функция синхронизации Usartа с ЦКТ
void SyncCKT(void)
{
	// Синхронизация
	if(readFlag==1)
	{
		readFlag=0;
		HAL_Delay(1);
		HAL_UART_Abort(&huart5);
		HAL_UART_Receive_DMA(&huart5,BuffCkt, MaxBuffOfCKT);
	}
}
// Команда начала записи на SD карту
void RXCommande1(void)
{
    // Запись в память номера файла, на котором мы находимся

	//CountFileNow=Flash_Read_single8bit(FilesAdr);
	if(CountFileNow==0xFF || CountFileNow>=9 ) // Максимальное количество создаваемых файлов =9
	{
		CountFileNow=0;
	}
	CountFileNow++;
	//flashErasePage(FLASH_SECTOR_6);
	//Flash_Write_single8bit(FilesAdr,CountFileNow);
	///ФЛЕШКА
	///  Создание файла
		fres = f_mount(&FatFs, "", 1); //1=mount now

		if (fres != FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
		// Открываем или создаем новый файл
		fres = f_open(&fil, &(MassFileName[CountFileNow][0]), FA_CREATE_ALWAYS | FA_WRITE);

		if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

		} else
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
	    ResolveSDWrite=1; // Открываем доступ к записи на SD
    // Отсылаем ответ
    CommandToRadio(1);
    // Ожидаем команду
    Rf96_Lora_RX_mode();
}
// Команда включения клапаном
void RXCommande2(void)
{
	// Подаем единицу на клапан
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	// Отсылаем ответ
    CommandToRadio(2);
    // Ожидаем команду
    Rf96_Lora_RX_mode();
}

// Команда включения двигателя
void RXCommande3(void)
{
	// Подаем единицу на двигатель
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
	// Ждем 5 секунд
	HAL_Delay(5000);
	// Убираем единицу с двигателя
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
	// Отсылаем ответ
    CommandToRadio(3);
    // Ожидаем команду
    Rf96_Lora_RX_mode();
}
// Команда - запрос на отправку данных
void RXCommande4(void)
{

	RadioIrq=1;

}
// Команда начала закрытия файла на SD карте
void RXCommande5(void)
{
	///ФЛЕШКА
    // Закрываем файл
	fres=f_close(&fil);
	if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

	} else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	fres=f_mount(NULL, "", 0);
	if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

	} else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
    ResolveSDWrite=0; // Закрываем доступ к записи на SD
    // Отсылаем ответ
    CommandToRadio(5);
    // Ожидаем команду
    Rf96_Lora_RX_mode();
}
// Команда закрытия клапана
void RXCommande6(void)
{
	// Подаем единицу на клапан
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	// Отсылаем ответ
    CommandToRadio(6);
    // Ожидаем команду
    Rf96_Lora_RX_mode();
}

// Проверка SD карты на работоспособность
void CheckSD(void)
{
	///  Создание файла
		fres = f_mount(&FatFs, "", 1); //1=mount now

		if (fres != FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
		// Открываем или создаем новый файл
		fres = f_open(&fil, "CheckSD.txt", FA_CREATE_ALWAYS | FA_WRITE);

		if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

		} else
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
	    fres=f_close(&fil);
		if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

		} else
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
	    fres=f_mount(NULL, "", 0);
		if(fres == FR_OK) { // Если проблема с флешкой  выключаем 1 светодиод

		} else
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

}

// Парсер
void DataConv(void)
{
	// Запись времени в буфер
	uint32_TO_charmass(reciveTime, SDbufWrite, 0, 8);
	for(uint8_t i=0;i<38;i++)
	{
		uint32_TO_charmass(&BuffMidW[i+4], SDbufWrite, 9+i*4, 3);
		SDbufWrite[8+i*4]=',';
	}
	SDbufWrite[162]='\n';

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  MX_SPI3_Init();
  MX_UART5_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */

	// Индикация включения УСИ ПРД
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
	//Проверка SD карты
	CheckSD();
	// Инициализация радиоканала (sx1272)
	Rf96_Lora_init();
	// Режим приема
	Rf96_Lora_RX_mode();
	// Запуск приема в дма с аксселерометров
	HAL_UART_Receive_DMA(&huart5, BuffCkt, MaxBuffOfCKT);
    // Запуск таймера с целью определения подключения ЦКТ
    HAL_TIM_Base_Start_IT(&htim10);
    //RXCommande1();

/////////////////////////////////////////////////////////////////////////

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		// Синхронизация с ЦКТ
	    SyncCKT();
	    // Прерывание по приему по радиоканалу
		if(Get_NIRQ_Di0())
		{
            // Достаем посылку из буфера
			Rf96_DataRX_From_FiFO((char*)TX_RX_Radio);
			// Считаем CRC
			CRC_c=(TX_RX_Radio[RadioMaxBuff-2]<<8)+TX_RX_Radio[RadioMaxBuff-1];
			// Очистка флагов
			Rf96_LoRaClearIrq();
			// Если CRC совпадает
			if(CRC_c==Crc16(TX_RX_Radio, RadioMaxBuff-2))
			{
				switch(TX_RX_Radio[CommIndex])
				{
				case 1:   // Команда начала записи: Создаем файл
					RXCommande1();
					break;
				case 2:   // Команда открытия клапана
					RXCommande2();
					break;
				case 3:   // Команда запуска двигателя
					RXCommande3();
					break;
				case 4:   // Команда запроса данных
					RXCommande4();
					break;
				case 5:   // Команда закрытия файла на SD и запрет записи на SD
					RXCommande5();
					break;
				case 6:    // Команда закрытия клапана
					RXCommande6();
					break;
				}
			}
		}

		if(ResolveSDWrite==1 && ReadyToWrite==1) //  Если разрешена запись на Sd карту и если есть что записывать
		{
			// Запись на SD
			DataConv();
			fres = f_write(&fil, SDbufWrite, 163, &bytesWrote);
			if (fres != FR_OK)
			{
				while(1)
				{
					// Выключение 1 светодиода, если какая-то проблема с записью на SD
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);

				}
			}
			if(RadioIrq==1)  // Если пришел запрос на отправку измерений по радио
			{
				for(uint8_t i=0;i<RadioMaxBuff-3;i++)
				{
					TX_RX_Radio[i+1]=BuffMidW[i+4];
				}
				// заносим в 1 элемент 4 команду
				TX_RX_Radio[0]=4;
				// Отсылаем ответ
			    Rf96_Lora_TX_mode();
				//Подсчет CRC16
				CRC_c=Crc16(TX_RX_Radio,RadioMaxBuff-2);
				TX_RX_Radio[RadioMaxBuff-2]=(uint8_t)(CRC_c>>8);
				TX_RX_Radio[RadioMaxBuff-1]=(uint8_t)CRC_c;
				// Установка адреса TX в буфере FIFO
				Rf96_TX_FifoAdr(0x80);
				// Устанавливает указатель на адрес начала массива TX в FIFO
				Rf96_FIFO_point(0x80);
			    // Очистка флагов
				Rf96_LoRaClearIrq();
			    // Отправка посылки
				Rf96_LoRaTxPacket((char*)TX_RX_Radio,RadioMaxBuff);
			    // Ожидаем команду
			    Rf96_Lora_RX_mode();
				RadioIrq=0;
			}
			// Синхронизация файла и sd карты
			fres = f_sync(&fil);
			ReadyToWrite=0;
		}


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 9000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 9000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 9000-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 10000;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_9B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_EVEN;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 230400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 230400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 230400;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, acel3_Pin|acel3_3_Pin|SPI3_nss_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|acel1_Pin|acel1_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|SSV_Pin|acel2_Pin|acel2_2_Pin 
                          |Motor_Pin|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : acel3_Pin acel3_3_Pin SPI3_nss_Pin */
  GPIO_InitStruct.Pin = acel3_Pin|acel3_3_Pin|SPI3_nss_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 acel1_Pin acel1_1_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|acel1_Pin|acel1_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SSV_Pin acel2_2_Pin Motor_Pin PB6 */
  GPIO_InitStruct.Pin = SSV_Pin|acel2_2_Pin|Motor_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : acel2_Pin */
  GPIO_InitStruct.Pin = acel2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(acel2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

// Обработчик прерываний Usartа по приему
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart5)
	{
		// Остановка таймера, говорящая о том, что ЦКТ подключен
		HAL_TIM_Base_Stop_IT(&htim10);
		TIM10->CNT=0;
        // Включение 2 светодиода, если ЦКТ подключен
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
		// Если данные синхронизированы
		if(BuffCkt[0]==0x7C && BuffCkt[1]==0x6E && BuffCkt[2]==0xA1 && BuffCkt[3]==0x2C )
		{
			// Готов к записи
			ReadyToWrite=1;
			//  Время в мс, когда пришли данные
			reciveTime = HAL_GetTick();
			// Перезаписываем данные в массив посредник
			for(uint8_t i=0;i<MaxBuffOfCKT;i++)
			{
				BuffMidW[i]=BuffCkt[i];
			}
		} else  // Если данные не синхронизированы
		{
			readFlag=1;
		}
		// стартуем таймер для дальнейшей проверки на подключение ЦКТ
		HAL_TIM_Base_Start_IT(&htim10);
	}

}
// Обработчик прерываний Usartа по передаче
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{


}
// Обработчик прерываний таймера
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim10)  // Если на ЦКТ ничего не пришло, выключаем 2 светодиод
	{
		TIM10->CNT=0;
		HAL_TIM_Base_Start_IT(&htim10);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_UART_Abort(&huart5);
		HAL_UART_Receive_DMA(&huart5, BuffCkt, MaxBuffOfCKT);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
