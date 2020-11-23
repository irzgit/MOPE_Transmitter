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
 *  1) Поочередное включение, затем выключение всех сразу, кроме 3го - �?ндикация того, что УС�? ПРД включился
 *  2) 1 светодиод горит - сд карта исправна, 1 светодиод не горит - проблема с сд картой
 *  3) 2 светодиод горит - прием с ЦКТ идет, 2 светодиод не горит - данные с ЦКТ не приходят
 *  4) 3 светодиод горит - остается гореть после включения УС�? ПРД
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
// �?ндекс комманды в массиве, пришедшего по радиоканалу
#define CommIndex 0
// Количество элементов в массиве для радиопередачи
#define RadioMaxBuff 42
// Количество элементов, приходящих с ЦКТ
#define MaxBuffOfCKT 43
// Количество пакетов, через которые мы начинаем записывать
#define CountOfWriteToSD 5
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//  FATfs
/////////////////////////
FATFS FatFs;
//File object
FIL fil;
// Результат операций над SD картой
FRESULT fres;
// Для записи
UINT bytesWrote;
// Для чтения
UINT bytesRead;
/////////////////////////
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
// Буфер для записи на sd карту данных с ЦКТ (после парсера)
uint8_t SDbufWrite[163];
// Счетчик данного файла
uint8_t CountFileNow=0;
// Массив с названием файла
const char MassFileName[10][10]=
   {
    {"Data0.txt\0"},
	{"Data1.txt\0"},
	{"Data2.txt\0"},
	{"Data3.txt\0"},
	{"Data4.txt\0"},
	{"Data5.txt\0"},
	{"Data6.txt\0"},
	{"Data7.txt\0"},
	{"Data8.txt\0"},
	{"Data9.txt\0"}
   };
// Передача или прием по радио в данный момент
uint8_t ModeRadio=0;
// Массив чтения с SD
uint8_t BuffSDRead[50];
// Массив для записи количества созданых файлов на SD в файл InfoSD.txt
uint8_t BuffSDfileinfo[30]="\t\tFile info\nNumber of files:X;";
//Массив для записи данных на SD в  файл InfoSD.txt
uint8_t BufFileInfoWr[30];
// Задержка в мс
uint32_t Ms_Delay=0;
// Начало задержки
uint8_t Delay_start=0;
// Метка об окончании задержки
uint8_t Timeout=0;
// Длительность задержки
uint32_t TimeDelay=5000;
// Счетчик пришедших с ЦКт посылок
uint8_t CountCKT=0;

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
// Функция пользовательской задержки
void UserDelayStart(uint32_t Delay)
{
	TimeDelay=Delay;
	Delay_start=1;
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
// Чтение количества созданных файлов из файла InfoSD.txt
uint8_t ReadNumofFileSD(void)
{
	FIL filInform;
	uint8_t Num=255;
	UINT bytesWroteInform;
	// Открываем или создаем новый файл
	fres = f_open(&filInform, "InfoSD.txt", FA_OPEN_ALWAYS | FA_READ);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	//Читаем количество файлов
	fres=f_read(&filInform,BuffSDRead,40,&bytesRead);
	//Если на флешке не было этого файла
	if(BuffSDRead[0]=='\t' && BuffSDRead[1]=='\t' && BuffSDRead[2]=='F' && BuffSDRead[3]=='i' && BuffSDRead[4]=='l')
	{
		Num=BuffSDRead[28]-'0'; // количество файлов
	}
	fres=f_close(&filInform);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	if(Num==255)
	{
		Num=0;
		fres = f_open(&filInform, "InfoSD.txt", FA_OPEN_ALWAYS | FA_WRITE);
		// Количество файлов
		fres = f_write(&filInform, "\t\tFile info\nNumber of files:0;\n", 31, &bytesWroteInform);
		// �?стория команд
		fres = f_write(&filInform, "\t\tCommand History\nTime\tcommand\n", 31, &bytesWroteInform);
		fres=f_close(&filInform);
		if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

	return Num;
}
// Запись в фаил  InfoSD.txt количество созданных файлов
void WriteNumofFileSD(uint8_t Num)
{
	FIL filInform;
	UINT bytesWroteInform;
	// Открываем или создаем новый файл
	fres = f_open(&filInform, "InfoSD.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	//Записываем количество данных
	BuffSDfileinfo[28]=Num+'0';
	fres = f_write(&filInform, BuffSDfileinfo, 30, &bytesWroteInform);
	fres=f_close(&filInform);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
}
// Запись в файл InfoSD.txt команд и времени
void CommandHistoryWrite(uint8_t command)
{
	FIL filInform;
	UINT bytesWroteInform;
	// Открываем или создаем новый файл
	fres = f_open(&filInform, "InfoSD.txt", FA_OPEN_APPEND | FA_WRITE);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	//Записываем количество данных
	//  Время в мс, когда ,прибыла команда
	uint32_t TimeHistory = HAL_GetTick();
	uint32_TO_charmass(TimeHistory, BufFileInfoWr, 0, 8);
	BufFileInfoWr[8]=' ';
	BufFileInfoWr[9]=command+'0';
	BufFileInfoWr[10]=';';
	BufFileInfoWr[11]='\n';
	fres = f_write(&filInform, BufFileInfoWr, 12, &bytesWroteInform);
	fres=f_close(&filInform);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
}
// Запись в файл InfoSD.txt метки о включении питания
void HistoryOnOffUSI(void)
{
	FIL filInform;
	UINT bytesWroteInform;
	// Открываем или создаем новый файл
	fres = f_open(&filInform, "InfoSD.txt", FA_OPEN_APPEND | FA_WRITE);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	//Записываем количество данных
	fres = f_write(&filInform, "Power On\n", 9, &bytesWroteInform);
	fres=f_close(&filInform);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
}
// Запрет записи на SD карту и закрытие открытого файла
void StopWriteToSD(void)
{
	if(ResolveSDWrite==1)
	{
		fres=f_close(&fil);
		if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		}
	}
	ResolveSDWrite=0; // закрываем доступ к записи на SD данных с ЦКТ
}
// Функция синхронизации Usartа с ЦКТ
void SyncCKT(void)
{
	// Синхронизация
	if(readFlag==1)
	{
		readFlag=0;
		HAL_Delay(2);
		HAL_UART_Abort(&huart5);
		HAL_UART_Receive_DMA(&huart5,BuffCkt, MaxBuffOfCKT);
	}
}
// Функция передачи по радиоканалу
void CommandToRadio(uint8_t Comm)
{
	for(uint8_t i=0;i<RadioMaxBuff-3;i++)
	{
		TX_RX_Radio[i+1]=BuffMidW[i+4];
	}
	// заносим в 1 элемент 4 команду
	TX_RX_Radio[0]=Comm;
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
	// следующее Прерывание будет по передаче
	ModeRadio=1;
}
// Команда начала записи на SD карту
void RXCommande1(void)
{
	// Запрещаем запись на SD
	ResolveSDWrite=0;
	// Запись в память номера файла, на котором мы находимся
	CountFileNow=ReadNumofFileSD();
	if(CountFileNow>=9) CountFileNow=0;
	CountFileNow++;
	WriteNumofFileSD(CountFileNow);
	//Записываем команду в историю
	CommandHistoryWrite(1);
	// создаем новый файл и сразу его закрываем
	fres = f_open(&fil, &(MassFileName[CountFileNow][0]), FA_CREATE_ALWAYS | FA_WRITE);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	// Закрытие файла и sd карты
	fres=f_close(&fil);
	if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	// Открываем доступ к записи на SD
	ResolveSDWrite=1;
	// Отсылаем ответ
	CommandToRadio(1);
}
// Команда включения клапаном
void RXCommande2(void)
{
	// Подаем единицу на клапан
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    //Записываем команду в историю
    CommandHistoryWrite(2);
	// Отсылаем ответ
    CommandToRadio(2);
}

// Команда включения двигателя
void RXCommande3(void)
{
	// Подаем единицу на двигатель
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    //Записываем команду в историю
    CommandHistoryWrite(3);
    if(TX_RX_Radio[1]==0)
    {
        // Запускаем задержку
        UserDelayStart(5000);
    } else
    {
    	// Запускаем задержку
    	UserDelayStart((uint32_t)(TX_RX_Radio[1]*1000));
    }
	// Отсылаем ответ
    CommandToRadio(3);
}
// Команда - запрос на отправку данных
void RXCommande4(void)
{
	// Отсылаем ответ
	CommandToRadio(4);
}
// Команда начала закрытия файла на SD карте
void RXCommande5(void)
{
	// запрещаем запись на SD
	ResolveSDWrite=0;
    //Записываем команду в историю
    CommandHistoryWrite(5);
    // Отсылаем ответ
    CommandToRadio(5);
}
// Команда закрытия клапана
void RXCommande6(void)
{
	// Подаем единицу на клапан
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    //Записываем команду в историю
    CommandHistoryWrite(6);
	// Отсылаем ответ
    CommandToRadio(6);
}
// Команда закрытия клапана
void RXCommande7(void)
{
	// Подаем единицу на оптопару
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
    //Записываем команду в историю
    CommandHistoryWrite(7);
	// Отсылаем ответ
    CommandToRadio(7);
}
// Команда закрытия клапана
void RXCommande8(void)
{
	// Подаем нуля на оптопару
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
    //Записываем команду в историю
    CommandHistoryWrite(8);
	// Отсылаем ответ
    CommandToRadio(8);
}
// Парсер
void DataConv(void)
{
	// Запись времени в буфер
	uint32_TO_charmass(reciveTime, SDbufWrite, 0, 8);
	for(uint8_t i=0;i<38;i++)
	{
		uint32_TO_charmass(BuffMidW[i+4], SDbufWrite, 9+i*4, 3);
		SDbufWrite[8+i*4]=',';
	}
	SDbufWrite[160]='\n';
}
// Расчет CRC8 с ЦКТ
uint8_t CKTcrc(uint8_t* data, uint8_t len)
{
	uint8_t sign=0;
	for(uint8_t i=4;i<len;i++)
	{
		sign+=data[i];
		if(sign & 0x80)
		{
			sign=sign<<1;
			sign|=0x01;
		} else sign=sign<<1;
	}
	return sign;
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

	// �?ндикация включения УС�? ПРД
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);

    // Монтирование флешки
	fres = f_mount(&FatFs, "", 1); //1=mount now
	if (fres != FR_OK)
	{ // Если проблема с флешкой  выключаем 1 светодиод
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		while(1);
	}
	// Создание файла infoSD.txt
	ReadNumofFileSD();
	// Запись в историю информацию о подаче питания
	HistoryOnOffUSI();
	// �?нициализация радиоканала (sx1272)
	Rf96_Lora_init();
	// Режим приема
	Rf96_Lora_RX_mode();
	// Запуск приема в дма с аксселерометров
	HAL_UART_Receive_DMA(&huart5, BuffCkt, MaxBuffOfCKT);
    // Запуск таймера с целью определения подключения ЦКТ
    HAL_TIM_Base_Start_IT(&htim10);

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
			if(ModeRadio==0) // если прерывание по приему
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
				case 7:// Подача еденицы на оптопару на закрытие двигателя
					RXCommande7();
					break;
				case 8: // Снятие еденицы с оптопары на закрытие двигателя
					RXCommande8();
					break;
				}
			}
			} else if(ModeRadio==1) // если прерывание по передаче
			{
				ModeRadio=0;
				// Сбрасываем флаги
				Rf96_LoRaClearIrq();
	            // Заходим в Standby
				Rf96_Standby();
			    // Ожидаем команду
			    Rf96_Lora_RX_mode();
			}
		}
		//  Если разрешена запись на Sd карту и если есть что записывать
		if(ResolveSDWrite==1 && ReadyToWrite==1)
		{
			// Запись на SD
			DataConv();
			// создаем новый файл
			fres = f_open(&fil, &(MassFileName[CountFileNow][0]), FA_OPEN_APPEND | FA_WRITE);
			if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				while(1);
			}
			fres = f_write(&fil, SDbufWrite, 161, &bytesWrote);
			if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				while(1);
			}
			// Закрытие файла и sd карты
			fres=f_close(&fil);
			if(fres != FR_OK) // Если проблема с флешкой  выключаем 1 светодиод
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				while(1);
			}
            // Нечего записывать
			ReadyToWrite=0;
		}
		// Окончание задержки
		if(Timeout==1)
		{
			// Для 3 команды
			// Убираем единицу с двигателя
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
			Timeout=0;
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
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|acel3_Pin|acel3_3_Pin|SPI3_nss_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|acel1_Pin|acel1_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_2|SSV_Pin|acel2_Pin 
                          |acel2_2_Pin|Motor_Pin|GPIO_PIN_6, GPIO_PIN_RESET);

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

  /*Configure GPIO pins : PA0 acel3_Pin acel3_3_Pin SPI3_nss_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|acel3_Pin|acel3_3_Pin|SPI3_nss_Pin;
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

  /*Configure GPIO pins : PB2 SSV_Pin acel2_2_Pin Motor_Pin 
                           PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|SSV_Pin|acel2_2_Pin|Motor_Pin 
                          |GPIO_PIN_6;
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

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
			if(CountCKT==CountOfWriteToSD)
			{
				CountCKT=0;
				// Готов к записи
				ReadyToWrite=1;
				//  Время в мс, когда пришли данные
				reciveTime = HAL_GetTick();
				//Если CRC8 с ЦКТ совпадает
				if(BuffCkt[MaxBuffOfCKT-1]==CKTcrc(BuffCkt,MaxBuffOfCKT-1))
				{
					// Перезаписываем данные в массив посредник
					for(uint8_t i=0;i<MaxBuffOfCKT;i++)
					{
						BuffMidW[i]=BuffCkt[i];
					}
				}
			} else CountCKT++;
		} else readFlag=1;  // Если данные не синхронизированы
		// стартуем таймер для дальнейшей проверки на подключение ЦКТ
		HAL_TIM_Base_Start_IT(&htim10);
	}
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

// Прерывание по системному таймеру
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	if(Delay_start==1)
	{
		if(Ms_Delay<TimeDelay)
		{
			Ms_Delay++;
		}
		else
		{
			// произошло прерывание
			Timeout=1;
			Delay_start=0;
			Ms_Delay=0;
		}
	}
  /* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
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
