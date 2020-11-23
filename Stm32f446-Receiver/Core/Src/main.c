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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// �?ндекс комманды в массиве, пришедшего по радиоканалу
#define CommIndex 0
// �?ндекс комманды в массиве, пришедшего c Linux
#define CommIndexLinux 42
// Количество элементов в массиве для радиопередачи
#define RadioMaxBuff 42
// Количество элементов, приходящих с ЦКТ
#define MaxBuffOfCKT 45
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
 /*  Режимы работы светодиодов:
 *
 *  1) Поочередное включение - �?ндикация того, что УС�? ПРД включился
 *  2) 1 светодиод  мигает - идет передача( ожидание) посылки, горит - посылка пришла, перестал гореть - посылка не пришла
 *  3) 2 светодиод дублирует первый за исключением ожидания посылки (мигания)
 *  4) 3 светодиод не горит, если в данный момент образовалась очередь из команд от Linux,в противном случае горит всегда
 */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
//CRC8
uint8_t CRC_8c=0;
//CRC16
uint16_t CRC_c=0;
// Массив приемопередачи по радиоканалу
uint8_t TX_RX_Radio[RadioMaxBuff];
// Буффер TX usart2
uint8_t BuffTx[MaxBuffOfCKT];
// Буффер RX usart2
uint8_t BuffRx[MaxBuffOfCKT];
// Счетчик количества пришедших байт с linux
uint8_t countRx=0;
// Переменная для получения 1 байта, пришедшего по usartу
uint8_t data=0;
// Маркер того, что пришли новые данные с usart
uint8_t Readflag=0;
// Переменные, отвечающие за режимы светодиодов
uint8_t LedMode=0;
// Для фильтрации помех с Usart
uint8_t ReadRdy=0;
// Текущее время в мс
uint32_t reciveTime=0;
// Запрет/разрешение на отправку команд
uint8_t AccessRadio=0;
// Переменная по таймауту
uint8_t RadioTimeoutRx=0;
// Задержка в мс
uint32_t Ms_Delay=0;
// Начало задержки
uint8_t Delay_start=0;
// Последняя отправленная команда
uint8_t Com4Active=0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
// Таблица CRC8
const unsigned char Crc8Table[256] = {
    0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
    0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
    0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
    0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
    0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
    0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
    0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
    0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
    0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
    0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
    0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
    0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
    0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
    0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
    0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
    0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
    0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
    0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
    0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
    0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
    0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
    0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
    0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
    0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
    0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
    0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
    0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
    0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
    0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
    0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
};
// Расчет CRC8
unsigned char Crc8(unsigned char *pcBlock, unsigned char len)
{
    unsigned char crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];

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
	if(command==3)
	{
		// Заносим задержку в секундах
		TX_RX_Radio[1]=BuffRx[43];
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
// Отправка команды на Linux
void Command_to_Linux(uint8_t cmd)
{
	for(uint8_t i=0;i<MaxBuffOfCKT;i++)
		BuffTx[i]=0;
	for(uint8_t i=0;i<MaxBuffOfCKT-6;i++)
		BuffTx[i+4]=TX_RX_Radio[i+1];
	BuffTx[0]=0x7C;
	BuffTx[1]=0x6E;
	BuffTx[2]=0xA1;
	BuffTx[3]=0x2C;
	BuffTx[42]=cmd;
	BuffTx[44]=Crc8(BuffTx,MaxBuffOfCKT-1);
	HAL_UART_Transmit(&huart2, BuffTx, MaxBuffOfCKT,100);
}
// Отправка 4 команды по радио
void Command4_send(void)
{
	if(Com4Active==1) // Если разрешена 4 команда
	{
		// Посылка принята успешно, отправляем запрос на данные, если нет команд с Linux
		if(Readflag!=1)
		{
			LedMode=1; // Режим мигания - посылка передается
			CommandToRadio(4);
			// Ожидаем команду
			Rf96_Lora_RX_mode();
			// Запуск таймера для отслеживания таймаута
			Delay_start=1;
			Ms_Delay=0;
			AccessRadio=1;
		} else AccessRadio=0;
	} else   AccessRadio=0;
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
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  // �?ндикация включения УС�? ПРД
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_Delay(300);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_Delay(300);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
  HAL_Delay(300);

  // �?нициализация lora sx1272
  Rf96_Lora_init();
  // Запуск приема команд с Linux
  HAL_UART_Receive_IT(&huart2, &data, 1);
  // Запуск таймера для работы светодиода
  HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		// Фильтрация помех 1 разъема
		if((HAL_GetTick()-reciveTime >200) && ReadRdy) // Если пришел 1 байт и в течении секунды больше ничего не пришло, считаем, что мы поймали помеху
		{
			ReadRdy=0;
			HAL_UART_Abort(&huart2);
			HAL_UART_Receive_IT(&huart2, &data, 1);
			countRx=0;
		}
		// Пришла какая-то посылка по linux
		if(Readflag==1 && AccessRadio==0)
		{
			Readflag=0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

			CRC_8c=BuffRx[MaxBuffOfCKT-1];
			if(CRC_8c==Crc8(BuffRx,MaxBuffOfCKT-1)) // если CRC совпало
			{
				switch(BuffRx[CommIndexLinux])
				{
					case 1:
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(1);  // Команда начала записи: Создаем новый файл и начинаем прием данных
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 2:
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(2); // Команда открытия клапана
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 3:
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(3); // Команда запуска двигателя
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 4:  // Внутренняя команда (запрос данных с ЦКТ)
					break;
					case 5:
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(5); // Команда закрытия файла на SD и запрет записи на SD
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 6:
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(6); // Команда закрытия клапана
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 7: // Подача еденицы на оптопару на закрытие двигателя
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(7); // Команда Подача еденицы на оптопару на закрытие двигателя
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
					case 8: // Снятие еденицы с оптопары на закрытие двигателя
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(8); // Команда Снятие еденицы с оптопары на закрытие двигателя
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
					break;
				}
			} else // Если CRC8 не совпало продолжаем отправлять 4 команду, если она отправлялась до этого
			{
				if(Com4Active==1) // Если мы попадаем сюда во время приема данных
				{
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(4);
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					// Занимаем радиоканал
					AccessRadio=1;
				}
			}
			// Продолжаем ожидать посылку с Linux
			countRx=0;
			HAL_UART_Abort(&huart2);
			HAL_UART_Receive_IT(&huart2, &data, 1);
		}

		// Прерывание по приему по радиоканалу
		if(Get_NIRQ_Di0())
		{
			// Достаем посылку из буфера
			Rf96_DataRX_From_FiFO((char*)TX_RX_Radio);
			// Считаем CRC
			CRC_c=(TX_RX_Radio[RadioMaxBuff-2]<<8)+TX_RX_Radio[RadioMaxBuff-1];
			// Очистка флагов
			Rf96_LoRaClearIrq();

			if(CRC_c==Crc16(TX_RX_Radio, RadioMaxBuff-2)) // Если CRC16 совпало
			{
				// Останавливаем таймер
				Delay_start=0;
				Ms_Delay=0;
				switch(TX_RX_Radio[CommIndex])
				{
					case 1:   // Команда начала записи: Создаем файл
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					// Посылка принята успешно, отправляем запрос на данные, если нет команд с Linux
					if(Readflag!=1)
					{
					LedMode=1; // Режим мигания - посылка передается
					Com4Active=1; // 4 команда будет посылаться всегда
					CommandToRadio(4);
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					AccessRadio=1;
					} else AccessRadio=0;
					break;
					case 2:   // Команда открытия клапана
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Command_to_Linux(2);
					Command4_send();
					break;
					case 3:   // Команда запуска двигателя
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Command_to_Linux(3);
					Command4_send();
					break;
					case 4:   // Команда запроса данных с ЦКТ
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					// Пересылаем принятый пакет на linux
					Command_to_Linux(1);
					// Посылка принята успешно, отправляем запрос на данные
					if(Com4Active==1 && Readflag!=1  ) // Если нет запрета на 4 команду, то отправляем ее
					{
					LedMode=1; // Режим мигания - посылка передается
					CommandToRadio(4);
					// Ожидаем команду
					Rf96_Lora_RX_mode();
					// Запуск таймера для отслеживания таймаута
					Delay_start=1;
					Ms_Delay=0;
					AccessRadio=1;
					} else AccessRadio=0;
					// радиоканал не занят
					break;
					case 5:   // Команда закрытия файла на SD и запрет записи на SD
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Com4Active=0; // запрещаем отправку 4 команды
					Command_to_Linux(5);
					// радиоканал не занят
					AccessRadio=0;
					break;
					case 6:  // Команда закрытия клапана
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Command_to_Linux(6);
					Command4_send();
					break;
					case 7: //Подача еденицы на оптопару на закрытие двигателя
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Command_to_Linux(7);
					Command4_send();
					break;
					case 8: // Снятие еденицы с оптопары на закрытие двигателя
					LedMode=0; // посылка принята (просто зажигаем светодиод)
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					Command_to_Linux(8);
					Command4_send();
					break;
				}
			}
		}
	// Если радиосигнал не был принят (таймаут)
	if(RadioTimeoutRx==1)
	{
		// Останавливаем таймер
		Delay_start=0;
		Ms_Delay=0;
		RadioTimeoutRx=0;
		LedMode=0; // посылка пропущена (тушим светодиод)
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		Rf96_LoRaClearIrq();
		// Отправляем еще один запрос на данные, иначе просто ожидаем команд с linux
		if(Com4Active==1 && Readflag!= 1)
		{
			LedMode=1; // Режим мигания - посылка передается
			CommandToRadio(4); // Команда запроса данных
			// Ожидаем команду
			Rf96_Lora_RX_mode();
			// Запуск таймера для отслеживания таймаута
			Delay_start=1;
			Ms_Delay=0;
			AccessRadio=1 ;
		} else   AccessRadio=0;

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
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
  htim6.Init.Prescaler = 8400-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2500;
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
  huart2.Init.BaudRate = 9600;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
// Обработчик прерываний по приему usart2
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)
	{
		ReadRdy=1;
		reciveTime=HAL_GetTick();
		// Заносим пришедший байт в массив
		BuffRx[countRx]=data;
		if(countRx==MaxBuffOfCKT-1)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
			// Устанавливаем флаг того, что посылка принята
			Readflag=1;
		}
		else countRx++;

		HAL_UART_Receive_IT(&huart2, &data, 1);
	}
}
// Обработчик прерываний таймера
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim6)  // мигание светодиодов
	{
		if(LedMode==1) // 1 светодиод
		{
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
		}
	}
}
// Прерывание по системному таймеру
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	if(Delay_start==1)
	{
		if(Ms_Delay<5000)
		{
			Ms_Delay++;
		}
		else
		{
			Delay_start=0;
			Ms_Delay=0;
			RadioTimeoutRx=1; // произошло прерывание
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
