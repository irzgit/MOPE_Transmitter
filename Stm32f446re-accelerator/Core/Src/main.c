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

#define NumofPacket 7
#define file_name_sect 2688
#define file_conf_sect 2049
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//  FATfs
FATFS FatFs;
//File object
FIL fil;
FRESULT fres;
///

//Прием данных с аксселерометра
uint8_t package[3][14] = {0};
// Отбрасывание ненужных байт с аксселерометра
uint8_t packageCut[3][9] = {0};

//SD карта
uint32_t blockAddr = 2816;
uint32_t Sector_Namefile=0;
uint16_t NubofByte=0;
///
uint8_t block_file[512]={0x44,0x41,0x54,0x41,0x20,0x20,0x20,0x20,0x54,0x58,0x54,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//uint8_t block_file2[512]={0x52,0x52,0x61,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t block_file2[512]={0xF8,0xFF,0xFF,0xFF,0x03,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

// Парсер
////первый акселерометр
int xIntSumFirst, yIntSumFirst, zIntSumFirst;
int x5First, x60First, x65First, y8First, y90First, y95First, z11First, z120First, z125First;
char xvalFirst[7], yvalFirst[7], zvalFirst[7];
//
//второй акслерометр
int xIntSumSecond, yIntSumSecond, zIntSumSecond;
int x5Second, x60Second, x65Second, y8Second, y90Second, y95Second, z11Second, z120Second, z125Second;
char xvalSecond[7], yvalSecond[7], zvalSecond[7];

//третий акселерометр
int xIntSumThird, yIntSumThird, zIntSumThird;
int x5Third, x60Third, x65Third, y8Third, y90Third, y95Third, z11Third, z120Third, z125Third;
char xvalThird[7], yvalThird[7], zvalThird[7];

// Для синхронизации с датчиками
uint8_t readFlag;
uint8_t readFlag2;
uint8_t readFlag3;
// После того, как получено с каждого датчика измерение запись в память и отсылка по радиоканалу
uint8_t UsartCount=0;
// Считаем сколько записей сделано
uint8_t CountOfAccel=0;
// время отсчета микрконтроллера в милисекундах
uint32_t reciveTime=0;
uint32_t reciveTime1=0;
uint32_t reciveTime2=0;
uint32_t reciveTime3=0;

#define NumOfOrder 100
// Буффер для очереди
uint8_t Buf_order[36*NumofPacket*NumOfOrder];
//uint8_t Buf_order1[5040];

// Счетчик для очереди
uint8_t count_order=0;
uint8_t count_order_Point=0;
uint8_t count_order_Minus=0;
//uint8_t count_order1=0;
//uint8_t count_order_Point1=0;

// Буффер посредник
uint8_t Buff_Mid[36*NumofPacket];
uint8_t Buff_Top[36*NumofPacket];
//uint8_t Buff_str1[512];
uint8_t Buff_str2[512];
// Буфер передачи по радио
uint8_t RadioBuff[28];

uint8_t pr=0;
uint8_t metka=0;
uint8_t z=0;
uint16_t pac = 0;
uint16_t block = 0x03;

// 1 - акселерометр с адресом 50
// 2 - акселерометр с адресом 100
// 3 - акселерометр с адресом 150
uint8_t accelSelect[3][5] = {{0x68, 0x04, 0x32, 0x04, 0x3a},{0x68, 0x04, 0x64, 0x04, 0x6c},{0x68, 0x04, 0x96, 0x04, 0x9e}};

uint32_t countT=0;

void PacketToRadio(void)
{

	for(uint8_t j=0;j<3;j++)
	{
		for(uint8_t i=0;i<9;i++)
		   RadioBuff[i+j*9]=packageCut[j][i];
	}
	//RadioBuff[27]=Crc8(RadioBuff,27);
	  // Установка адреса TX в буфере FIFO
	  Rf96_TX_FifoAdr(0x80);
	  // Устанавливает указатель на адрес начала массива TX в FIFO
	  Rf96_FIFO_point(0x80);

	  Rf96_LoRaClearIrq();

	  Rf96_LoRaTxPacket((char*)RadioBuff,27);

}

void transmit(uint8_t* str, uint8_t* str2)
{

	for(uint8_t i=0;i<NumofPacket;i++)
	{


		        // первый акселерометр
			    // Ось Х
				xIntSumFirst = (str[1+8+i*36] & 0x0F) * 10 + (str[1+9+i*36] >> 4); // целочисленная сумма X
				x5First = str[1+9+i*36] & 0x0F; //перевод правого бита 5го байта
				x60First = str[1+10+i*36] >> 4; //перевод левого бита 6го байта
				x65First = str[1+10+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalFirst, "+%02d.%d%d;",xIntSumFirst, x5First, x60First);
				if((str[1+8+i*36] >> 4) == 0x01)
					xvalFirst[0] = '-';

				// Ось Y
				yIntSumFirst = (str[1+11+i*36] & 0x0F) * 10 + (str[1+12+i*36] >> 4);	// целочисленная сумма Y
				y8First = str[1+12+i*36] & 0x0F; //перевод правого бита 8го байта
				y90First = str[1+13+i*36] >> 4; //перевод левого бита 9го байта
				y95First = str[1+13+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalFirst, "+%02d.%d%d;", yIntSumFirst, y8First, y90First);
				if((str[1+11+i*36] >> 4) == 0x01)
					yvalFirst[0] = '-';

				// Ось Z
				zIntSumFirst = (str[1+14+i*36] & 0x0F) * 10 + (str[1+15+i*36] >> 4); //целочисленная сумма Z
				z11First = str[1+15+i*36] & 0x0F; //перевод правого бита 11го байта
				z120First = str[1+16+i*36] >> 4; //перевод левого бита 12го байта
				z125First = str[1+16+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalFirst, "+%02d.%d%d;", zIntSumFirst, z11First, z120First);
				if((str[1+14+i*36] >> 4) == 0x01)
					zvalFirst[0] = '-';

				// второй акселерометр
				// Ось Х
				xIntSumSecond = (str[1+17+i*36] & 0x0F) * 10 + (str[1+18+i*36] >> 4); // целочисленная сумма X
				x5Second = str[1+18+i*36] & 0x0F; //перевод правого бита 5го байта
				x60Second = str[1+19+i*36] >> 4; //перевод левого бита 6го байта
				x65Second = str[1+19+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalSecond, "+%02d.%d%d;",xIntSumSecond, x5Second, x60Second);
				if((str[1+17+i*36] >> 4) == 0x01)
					xvalSecond[0] = '-';

				// Ось Y
				yIntSumSecond = (str[1+20+i*36] & 0x0F) * 10 + (str[1+21+i*36] >> 4);	// целочисленная сумма Y
				y8Second = str[1+21+i*36] & 0x0F; //перевод правого бита 8го байта
				y90Second = str[1+22+i*36] >> 4; //перевод левого бита 9го байта
				y95Second = str[1+22+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalSecond, "+%02d.%d%d;", yIntSumSecond, y8Second, y90Second);
				if((str[1+20+i*36] >> 4) == 0x01)
					yvalSecond[0] = '-';

				// Ось Z
				zIntSumSecond = (str[1+23+i*36] & 0x0F) * 10 + (str[1+24+i*36] >> 4); //целочисленная сумма Z
				z11Second = str[1+24+i*36] & 0x0F; //перевод правого бита 11го байта
				z120Second = str[1+25+i*36] >> 4; //перевод левого бита 12го байта
				z125Second = str[1+25+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalSecond, "+%02d.%d%d;", zIntSumSecond, z11Second, z120Second);
				if((str[1+23+i*36] >> 4) == 0x01)
					zvalSecond[0] = '-';

				// третий акселерометр
				// Ось Х
				xIntSumThird = (str[1+26+i*36] & 0x0F) * 10 + (str[1+27+i*36] >> 4); // целочисленная сумма X
				x5Third = str[1+27+i*36] & 0x0F; //перевод правого бита 5го байта
				x60Third = str[1+28+i*36] >> 4; //перевод левого бита 6го байта
				x65Third = str[1+28+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalThird, "+%02d.%d%d;",xIntSumThird, x5Third, x60Third);
				if((str[1+26+i*36] >> 4) == 0x01)
					xvalThird[0] = '-';

				// Ось Y
				yIntSumThird = (str[1+29+i*36] & 0x0F) * 10 + (str[1+30+i*36] >> 4);	// целочисленная сумма Y
				y8Third = str[1+30+i*36] & 0x0F; //перевод правого бита 8го байта
				y90Third = str[1+31+i*36] >> 4; //перевод левого бита 9го байта
				y95Third = str[1+31+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalThird, "+%02d.%d%d;", yIntSumThird, y8Third, y90Third);
				if((str[1+29+i*36] >> 4) == 0x01)
					yvalThird[0] = '-';

				// Ось Z
				zIntSumThird = (str[1+32+i*36] & 0x0F) * 10 + (str[1+33+i*36] >> 4); //целочисленная сумма Z
				z11Third = str[1+33+i*36] & 0x0F; //перевод правого бита 11го байта
				z120Third = str[1+34+i*36] >> 4; //перевод левого бита 12го байта
				z125Third = str[1+34+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalThird, "+%02d.%d%d;", zIntSumThird, z11Third, z120Third);
				if((str[1+32+i*36] >> 4) == 0x01)
					zvalThird[0] = '-';


				for(uint8_t j=0;j<8;j++)
				{

					str2[j+i*73]=str[j+i*36];
				}
				for(uint8_t j=0;j<8;j++)
				{

					str2[j+8+i*73]=';';
				}


				sprintf(&str2[1+8+i*73], "%s%s%s%s%s%s%s%s%s\n", xvalFirst, yvalFirst, zvalFirst, xvalSecond, yvalSecond, zvalSecond, xvalThird, yvalThird, zvalThird);

	}

}


// Перевод Массива в число
/*

mass - массив, из которого получится число
startBuff - начальный элемент массива
len - длина переводимого числа
*/
uint32_t Mass_to_uint32(uint8_t* mass, uint16_t startBuf, uint16_t len)
{
	uint32_t temp = 0;
	for (uint16_t i = 0; i < len; i++)
	{
		temp = temp * 10 + mass[i + startBuf] - 48;
	}
	return temp;
}

// Перевод Числа в массив
/*
Number - переводимое число
mass - массив, в которй число будет переведено
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


// Функция синхронизации
void SyncAccel(void)
{
	  // Синхронизация
	  if(readFlag==1)
	  {
		UsartCount=0;
	  	readFlag=0;
	  	HAL_Delay(1);
	  	HAL_UART_Abort(&huart3);
	  	HAL_UART_Receive_DMA(&huart3, &package[0][0], 14);
	  }
	  if(readFlag2==1)
	  {
		UsartCount=0;
	  	readFlag2=0;
	  	HAL_Delay(1);
	  	HAL_UART_Abort(&huart1);
	  	HAL_UART_Receive_DMA(&huart1, &package[2][0], 14);
	  }
	  if(readFlag3==1)
	  {
		UsartCount=0;
	  	readFlag3=0;
	  	HAL_Delay(1);
	  	HAL_UART_Abort(&huart5);
	  	HAL_UART_Receive_DMA(&huart5, &package[1][0], 14);
	  }


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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_SET);
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

  for(uint16_t i=92;i<512;i++)
  {
       block_file[i]=0;
       block_file2[i]=0;
  }
//  block_file2[511]=0xAA;
//  block_file2[510]=0x55;
//  block_file2[492]=0x03;
//  block_file2[490]=0x1D;
//  block_file2[489]=0xA4;
//  block_file2[488]=0x92;
//  block_file2[487]=0x61;
//  block_file2[486]=0x41;
//  block_file2[485]=0x72;
//  block_file2[484]=0x72;





    // Инициализация радиоканала (sx1272)
	Rf96_Lora_init();
	Rf96_Lora_TX_mode();

    // Запуск приема в дма с аксселерометров
    HAL_UART_Receive_DMA(&huart3, &package[0][0], 14);
	HAL_UART_Receive_DMA(&huart5, &package[1][0], 14);
	HAL_UART_Receive_DMA(&huart1, &package[2][0], 14);

    // Отправка первого нулевого пакета
	PacketToRadio();
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);


	//SDCARD_Init();

	///ФЛЕШКА
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	UINT bytesWrote;
    fres = f_mount(&FatFs, "", 1); //1=mount now

	  if (fres != FR_OK) {
		while(1)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		}
	  }
	  fres = f_open(&fil, "Data.txt", FA_CREATE_ALWAYS | FA_WRITE);

    if(fres == FR_OK) {

    } else {
		while(1)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		}
    }

   // f_close(&fil);

    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(&htim7);
    HAL_TIM_Base_Start_IT(&htim10);

    /*
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    */
/////////////////////////////////////////////////////////////////////////

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)==RESET)
	  {
		  //Close file, don't forget this!
		  //HAL_Delay(10);
		   f_close(&fil);
		  // HAL_Delay(5);
		   //De-mount drive
		  f_mount(NULL, "", 0);
		  while(1)
		  {
		  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
		  HAL_Delay(500);
		//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
		  HAL_Delay(500);
		  }
	  }
	  // Синхронизация
	    SyncAccel();

	  // отправка по радиоканалу
		if(Get_NIRQ_Di0())
		{
			PacketToRadio();
		}




if(count_order_Minus>0)
{


			//HAL_UART_Transmit_IT(&huart2, "\n", 1);
			reciveTime2 = HAL_GetTick();
			metka=0;


			if(z==0)
			{
				z=1;

				memset(Buf_order,0,NumofPacket*36);
				for(uint8_t i=0;i<NumofPacket;i++)
				{
					uint32_TO_charmass(0, Buf_order, i*36, 8);
				}
			}

				transmit(&Buf_order[count_order_Point*252],Buff_str2);
				count_order_Point++;
				count_order_Minus--;
				if(count_order_Point==NumOfOrder)
				{
					count_order_Point=0;
				}

				Buff_str2[510]=';';
				Buff_str2[511]='\n';


				// Запись на SD 2 буфера

				fres = f_write(&fil, &Buff_str2, sizeof(Buff_str2), &bytesWrote);
				 if (fres != FR_OK)
				 {
					while(1)
					{
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
					}
				 }
				 fres = f_sync(&fil);


				 SDCARD_WriteSingleBlock(blockAddr++, Buff_str2);
				 NubofByte+=2;
				 if(NubofByte==256)
				 {
					 pac += 2;
					 NubofByte=0;
//					 block_file[30]+=1;

					 block += 1;
					 block_file2[4 + pac] = block;
					 block_file2[5 + pac] = 0x00;
					 block_file2[6 + pac] = 0xFF;
					 block_file2[7 + pac] = 0xFF;
					 SDCARD_WriteSingleBlock(file_conf_sect, block_file2);
				 }
				 block_file[29]=NubofByte;

				// Sector_Namefile+=512;
				// block_file[27]=(uint8_t)(Sector_Namefile>>24);
				// block_file[28]=(uint8_t)(Sector_Namefile>>16);
				// block_file[29]=(uint8_t)(Sector_Namefile>>8);
				// block_file[30]=(uint8_t)(Sector_Namefile);


			 SDCARD_WriteSingleBlock(file_name_sect, block_file);

/*
		    pac++;

				 if(pac == 129)
				 {
					 pac = 0;
					 block_file2[488]-=1;
				     block_file2[492]+=1;
					 SDCARD_WriteSingleBlock(file_conf_sect, block_file2);
				 }
*/
				 countT++;
				 if(countT==255)
				 {
					 countT=0;
				 }

				 reciveTime3 = HAL_GetTick();
				 reciveTime3 = HAL_GetTick();

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
  htim10.Init.Period = 1000;
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
  huart5.Init.BaudRate = 230400;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4 
                          |acel1_Pin|acel1_1_Pin, GPIO_PIN_RESET);


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, acel3_Pin|acel3_3_Pin|SPI3_nss_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|acel2_Pin|acel2_2_Pin|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC2 PC3 PC4 
                           acel1_Pin acel1_1_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4 
                          |acel1_Pin|acel1_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
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

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : acel2_Pin */
  GPIO_InitStruct.Pin = acel2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(acel2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : acel2_2_Pin PB6 */
  GPIO_InitStruct.Pin = acel2_2_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{


if(huart==&huart3)
{
		HAL_TIM_Base_Stop_IT(&htim6);
		TIM6->CNT=0;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	if(package[0][0]!=0x68)
	{
		readFlag=1;
		UsartCount=0;

	}else
	{
		for(uint8_t i=0;i<9;i++)
		{
			packageCut[0][i]=package[0][i+4];
		}
		UsartCount++;

	}
	HAL_TIM_Base_Start_IT(&htim6);
}
if(huart==&huart1)
{
	HAL_TIM_Base_Stop_IT(&htim7);
	TIM7->CNT=0;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	if(package[2][0]!=0x68)
	{
		readFlag2=1;
		UsartCount=0;

	}else
	{
		for(uint8_t i=0;i<9;i++)
		{
			packageCut[2][i]=package[2][i+4];
		}
		UsartCount++;
	}
	HAL_TIM_Base_Start_IT(&htim7);

}

if(huart==&huart5)
{
	HAL_TIM_Base_Stop_IT(&htim10);
	TIM10->CNT=0;
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	if(package[1][0]!=0x68)
	{
		readFlag3=1;
		UsartCount=0;

	}else
	{
		for(uint8_t i=0;i<9;i++)
		{
			packageCut[1][i]=package[1][i+4];
		}
		UsartCount++;

	}
	HAL_TIM_Base_Start_IT(&htim10);
}
if(UsartCount==3 && readFlag==0 && readFlag2==0 && readFlag3==0)  // Получено 1 измерение с каждого датчика
{
	    UsartCount=0;


		reciveTime = HAL_GetTick();

		uint32_TO_charmass(reciveTime, Buff_Top, CountOfAccel*36, 8);
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Top[i+9+36*CountOfAccel]=packageCut[0][i];

		}
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Top[i+18+36*CountOfAccel]=packageCut[1][i];

		}
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Top[i+27+36*CountOfAccel]=packageCut[2][i];

		}


		CountOfAccel++;


		if(CountOfAccel==NumofPacket)  // Считано 7 измерений с каждого датчика
		{
			for(uint32_t i=0;i<252;i++)
			{
				Buf_order[i+count_order*252]=Buff_Top[i];
			}
			count_order++;
			count_order_Minus++;
			if(count_order==NumOfOrder)
			{
				count_order=0;

			}

			CountOfAccel=0;
			metka=1;
			reciveTime1 = HAL_GetTick();
			reciveTime1 = HAL_GetTick();
		}





}

}




void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{


}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if(htim==&htim6)
	{
		TIM6->CNT=0;
		memset(packageCut[0],0,9);
		HAL_TIM_Base_Start_IT(&htim6);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	  	HAL_UART_Abort(&huart3);
	  	HAL_UART_Receive_DMA(&huart3, &package[0][0], 14);
	}
	if(htim==&htim7)
	{
		TIM7->CNT=0;
		memset(packageCut[2],0,9);
		HAL_TIM_Base_Start_IT(&htim7);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
	  	HAL_UART_Abort(&huart1);
	  	HAL_UART_Receive_DMA(&huart1, &package[2][0], 14);
	}
	if(htim==&htim10)
	{
		TIM10->CNT=0;
		memset(packageCut[1],0,9);
		HAL_TIM_Base_Start_IT(&htim10);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
	  	HAL_UART_Abort(&huart5);
	  	HAL_UART_Receive_DMA(&huart5, &package[1][0], 14);
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
