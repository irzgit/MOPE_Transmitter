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
/* USER CODE BEGIN PFP */
#define PacketSize 28
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//  FATfs
FATFS FatFs;
//File object
FIL fil;
FRESULT fres;
///


///uint8_t str[PacketSize*73] = {0};

uint8_t package[3][14*PacketSize] = {0};
uint8_t packageCut[3][9*PacketSize] = {0};

uint8_t Txcomplite=0;

//SD карта
uint32_t blocksNum;
uint32_t blockAddr = 0;
uint8_t block[512];
//

// Парсер
//int xIntSum, yIntSum, zIntSum;
//int x5, x60, x65, y8, y90, y95, z11, z120, z125;
////char xval[3][7], yval[3][7], zval[3][7], str1[3][21];
//char xval[7], yval[7], zval[7], str1[511];
//

////первый акселерометр
int xIntSumFirst, yIntSumFirst, zIntSumFirst;
int x5First, x60First, x65First, y8First, y90First, y95First, z11First, z120First, z125First;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalFirst[7], yvalFirst[7], zvalFirst[7];
//
//второй акслерометр
int xIntSumSecond, yIntSumSecond, zIntSumSecond;
int x5Second, x60Second, x65Second, y8Second, y90Second, y95Second, z11Second, z120Second, z125Second;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalSecond[7], yvalSecond[7], zvalSecond[7];

//третий акселерометр
int xIntSumThird, yIntSumThird, zIntSumThird;
int x5Third, x60Third, x65Third, y8Third, y90Third, y95Third, z11Third, z120Third, z125Third;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalThird[7], yvalThird[7], zvalThird[7], str1[PacketSize*73];

// Для синхронизации с датчиками
uint8_t readFlag;
uint8_t readFlag2;
uint8_t readFlag3;
// После того, как получено с каждого датчика измерение запись в память и отсылка по радиоканалу
uint8_t UsartCount=0;
// Считаем сколько записей сделано
uint8_t CountOfAccel=0;
// время отсчета микрконтроллера в милисекундах
uint32_t reciveTime;
// Буффер записи на SD карту
uint8_t SD_Buff[74];
// Буффер посредник
uint8_t Buff_Mid[36*PacketSize];
// Буфер передачи по радио
uint8_t RadioBuff[28];
uint8_t ToSD[PacketSize*73];


uint8_t metka=0;

// 1 - акселерометр с адресом 50
// 2 - акселерометр с адресом 100
// 3 - акселерометр с адресом 150
uint8_t accelSelect[3][5] = {{0x68, 0x04, 0x32, 0x04, 0x3a},{0x68, 0x04, 0x64, 0x04, 0x6c},{0x68, 0x04, 0x96, 0x04, 0x9e}};



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
	//  srtAll[0]='1';
	//  srtAll[1]='\n';
	//  HAL_UART_Transmit_IT(&huart2, srtAll,2);

}












/*
void transmit(uint8_t str[3][9])
{

	for(int i = 0; i < 3; i++)
	{
		//маркер начала команды
//		if(str[i][0] == 0x68)
//		{
			//str[1] - длина команды в байтах
			//0x68 - команда возврата данных с акселерометра
//			if(str[i][3] == 0x84)
//			{
				// Ось Х
				xIntSum = (str[i][0] & 0x0F) * 10 + (str[i][1] >> 4); // целочисленная сумма X
				x5 = str[i][1] & 0x0F; //перевод правого бита 5го байта
				x60 = str[i][2] >> 4; //перевод левого бита 6го байта
				x65 = str[i][2] & 0x0F; //перевод правого бита 6го байта
				sprintf(xval[i], "+%02d.%d%d;",xIntSum, x5, x60);
				if((str[i][0] >> 4) == 0x01)
					xval[i][0] = '-';


				// Ось Y
				yIntSum = (str[i][3] & 0x0F) * 10 + (str[i][4] >> 4);	// целочисленная сумма Y
				y8 = str[i][4] & 0x0F; //перевод правого бита 8го байта
				y90 = str[i][5] >> 4; //перевод левого бита 9го байта
				y95 = str[i][5] & 0x0F; //перевод правого бита 9го байта
				sprintf(yval[i], "+%02d.%d%d;", yIntSum, y8, y90);
				if((str[i][3] >> 4) == 0x01)
					yval[i][0] = '-';

				// Ось Z
				zIntSum = (str[i][6] & 0x0F) * 10 + (str[i][7] >> 4); //целочисленная сумма Z
				z11 = str[i][7] & 0x0F; //перевод правого бита 11го байта
				z120 = str[i][8] >> 4; //перевод левого бита 12го байта
				z125 = str[i][8] & 0x0F; //перевод правого бита 12го байта
				sprintf(zval[i], "+%02d.%d%d;", zIntSum, z11, z120);
				if((str[i][6] >> 4) == 0x01)
					zval[i][0] = '-';
//			}
//		}
		sprintf(str1[i], "%s%s%s", xval[i], yval[i], zval[i]);
	}


	//str1[0][21]=' ';
	//str1[1][21]=' ';
	//str1[2][21]='\n';


	//HAL_UART_Transmit_IT(&huart2,"\n",1);


}
*/
void transmit(uint8_t* str)
{


		//маркер начала команды
//		if(str[0] == 0x68)
//		{
			//str[1] - длина команды в байтах
			//0x68 - команда возврата данных с акселерометра
//			if(str[3] == 0x84)
//			{
				// первый акселерометр
				// Ось Х

	for(uint8_t i=0;i<PacketSize;i++)
	{
				xIntSumFirst = (str[1+8+i*36] & 0x0F) * 10 + (str[1+9+i*36] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5First = str[1+9+i*36] & 0x0F; //перевод правого бита 5го байта
				x60First = str[1+10+i*36] >> 4; //перевод левого бита 6го байта
				x65First = str[1+10+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalFirst, "+%02d.%d%d;",xIntSumFirst, x5First, x60First);
				if((str[1+8+i*36] >> 4) == 0x01)
					xvalFirst[0] = '-';

				// Ось Y
				yIntSumFirst = (str[1+11+i*36] & 0x0F) * 10 + (str[1+12+i*36] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8First = str[1+12+i*36] & 0x0F; //перевод правого бита 8го байта
				y90First = str[1+13+i*36] >> 4; //перевод левого бита 9го байта
				y95First = str[1+13+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalFirst, "+%02d.%d%d;", yIntSumFirst, y8First, y90First);
				if((str[1+11+i*36] >> 4) == 0x01)
					yvalFirst[0] = '-';

				// Ось Z
				zIntSumFirst = (str[1+14+i*36] & 0x0F) * 10 + (str[1+15+i*36] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11First = str[1+15+i*36] & 0x0F; //перевод правого бита 11го байта
				z120First = str[1+16+i*36] >> 4; //перевод левого бита 12го байта
				z125First = str[1+16+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalFirst, "+%02d.%d%d;", zIntSumFirst, z11First, z120First);
				if((str[1+14+i*36] >> 4) == 0x01)
					zvalFirst[0] = '-';

				// второй акселерометр
				// Ось Х
				xIntSumSecond = (str[1+17+i*36] & 0x0F) * 10 + (str[1+18+i*36] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5Second = str[1+18+i*36] & 0x0F; //перевод правого бита 5го байта
				x60Second = str[1+19+i*36] >> 4; //перевод левого бита 6го байта
				x65Second = str[1+19+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalSecond, "+%02d.%d%d;",xIntSumSecond, x5Second, x60Second);
				if((str[1+17+i*36] >> 4) == 0x01)
					xvalSecond[0] = '-';

				// Ось Y
				yIntSumSecond = (str[1+20+i*36] & 0x0F) * 10 + (str[1+21+i*36] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8Second = str[1+21+i*36] & 0x0F; //перевод правого бита 8го байта
				y90Second = str[1+22+i*36] >> 4; //перевод левого бита 9го байта
				y95Second = str[1+22+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalSecond, "+%02d.%d%d;", yIntSumSecond, y8Second, y90Second);
				if((str[1+20+i*36] >> 4) == 0x01)
					yvalSecond[0] = '-';

				// Ось Z
				zIntSumSecond = (str[1+23+i*36] & 0x0F) * 10 + (str[1+24+i*36] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11Second = str[1+24+i*36] & 0x0F; //перевод правого бита 11го байта
				z120Second = str[1+25+i*36] >> 4; //перевод левого бита 12го байта
				z125Second = str[1+25+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalSecond, "+%02d.%d%d;", zIntSumSecond, z11Second, z120Second);
				if((str[1+23+i*36] >> 4) == 0x01)
					zvalSecond[0] = '-';

				// третий акселерометр
				// Ось Х
				xIntSumThird = (str[1+26+i*36] & 0x0F) * 10 + (str[1+27+i*36] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5Third = str[1+27+i*36] & 0x0F; //перевод правого бита 5го байта
				x60Third = str[1+28+i*36] >> 4; //перевод левого бита 6го байта
				x65Third = str[1+28+i*36] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalThird, "+%02d.%d%d;",xIntSumThird, x5Third, x60Third);
				if((str[1+26+i*36] >> 4) == 0x01)
					xvalThird[0] = '-';

				// Ось Y
				yIntSumThird = (str[1+29+i*36] & 0x0F) * 10 + (str[1+30+i*36] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8Third = str[1+30+i*36] & 0x0F; //перевод правого бита 8го байта
				y90Third = str[1+31+i*36] >> 4; //перевод левого бита 9го байта
				y95Third = str[1+31+i*36] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalThird, "+%02d.%d%d;", yIntSumThird, y8Third, y90Third);
				if((str[1+29+i*36] >> 4) == 0x01)
					yvalThird[0] = '-';

				// Ось Z
				zIntSumThird = (str[1+32+i*36] & 0x0F) * 10 + (str[1+33+i*36] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11Third = str[1+33+i*36] & 0x0F; //перевод правого бита 11го байта
				z120Third = str[1+34+i*36] >> 4; //перевод левого бита 12го байта
				z125Third = str[1+34+i*36] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalThird, "+%02d.%d%d;", zIntSumThird, z11Third, z120Third);
				if((str[1+32+i*36] >> 4) == 0x01)
					zvalThird[0] = '-';



				for(uint8_t j=0;j<8;j++)
				{

					str1[j+i*73]=str[j+i*36];
				}
				for(uint8_t j=0;j<8;j++)
							{

								str1[j+8+i*73]=';';
							}


				sprintf(&str1[1+8+i*73], "%s%s%s%s%s%s%s%s%s\n", xvalFirst, yvalFirst, zvalFirst, xvalSecond, yvalSecond, zvalSecond, xvalThird, yvalThird, zvalThird);

	}
//			}
//		}

	//HAL_UART_Abort(&huart2);
	//HAL_UART_Transmit(&huart2, (uint8_t*)str1, 510,10);

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
	  	HAL_UART_Receive_DMA(&huart3, &package[0][0], 14*PacketSize);
	  }
	  if(readFlag2==1)
	  {
		UsartCount=0;
	  	readFlag2=0;
	  	HAL_Delay(1);
	  	HAL_UART_Abort(&huart1);
	  	HAL_UART_Receive_DMA(&huart1, &package[2][0], 14*PacketSize);
	  }
	  if(readFlag3==1)
	  {
		UsartCount=0;
	  	readFlag3=0;
	  	HAL_Delay(1);
	  	HAL_UART_Abort(&huart5);
	  	HAL_UART_Receive_DMA(&huart5, &package[1][0], 14*PacketSize);
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
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  MX_SPI3_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
  int kolZapis = 0;



	Rf96_Lora_init();
	Rf96_Lora_TX_mode();


	  ///////////////////////////////
	  //int code;
	  //code = SDCARD_Init();


	 // code = SDCARD_GetBlocksNumber(&blocksNum);



	    // Запись одного блока
	   // code = SDCARD_WriteSingleBlock(blockAddr, block);
	  ////////////////////////////////

	    // Чтение одного блока
	    //code = SDCARD_ReadSingleBlock(blockAddr, block);

	    /*
	       startBlockAddr=0;
	       blockAddr = startBlockAddr + 1;
	       for(uint16_t i=0;i<512;i++)
	       {
	    	   block[i]='1';
	       }

           code = SDCARD_ReadBegin(blockAddr);
	                   for(int i = 0; i < 3; i++) {
	                           code = SDCARD_ReadData(block);
	                   }
	                           code = SDCARD_ReadEnd();


	       code = SDCARD_WriteBegin(blockAddr);


	       for(int i = 0; i < 3; i++)
	       {
	               //snprintf((char*)block, sizeof(block), "0x%08X", (int)blockAddr);
	               code = SDCARD_WriteData(block);
	       }

	               code = SDCARD_WriteEnd();


	               blockAddr = startBlockAddr + 1;


	                   code = SDCARD_ReadBegin(blockAddr);
	                   for(int i = 0; i < 3; i++) {
	                           code = SDCARD_ReadData(block);
	                   }
	                           code = SDCARD_ReadEnd();

*/



		HAL_UART_Receive_DMA(&huart3, &package[0][0], 14*PacketSize);
		HAL_UART_Receive_DMA(&huart5, &package[1][0], 14*PacketSize);
		HAL_UART_Receive_DMA(&huart1, &package[2][0], 14*PacketSize);


	//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	//  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
     // HAL_UART_Transmit_DMA(&huart3, &accelSelect[0][0], 5);
     // HAL_UART_Transmit_DMA(&huart5, &accelSelect[1][0], 5);
    //  HAL_UART_Transmit_DMA(&huart1, &accelSelect[2][0], 5);


        //Rf96_LoRaTxPacket((char*)packageCut,27);


		PacketToRadio();





		////////////////ФЛЕШКА



	    fres = f_mount(&FatFs, "", 1); //1=mount now


		  if (fres != FR_OK) {
		   // myprintf("f_mount error (%i)\r\n", fres);

			//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
		  }



	  //fres = f_open(&fil, "write.txt", FA_OPEN_APPEND | FA_WRITE);


		  fres = f_open(&fil, "write.txt", FA_CREATE_ALWAYS | FA_WRITE);

	  if(fres == FR_OK) {
	 //   myprintf("I was able to open 'write.txt' for writing\r\n");
	  } else {
		  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
			while(1);
	 //   myprintf("f_open error (%i)\r\n", fres);
	  }


	  UINT bytesWrote;
/////////////////////////////////////////////////////////////////////////

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

      // Кнопка
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

		if(metka==1)
		{

			// парсер  Buff_Mid

			metka=0;
		//	transmit(Buff_Mid);



		//	str1[510]=';';
		//	str1[511]='\n';

			// Запись на SD
			 // code = SDCARD_WriteSingleBlock(blockAddr++, str1);
			//  memset(block, 0, sizeof(block));


			  //fres = f_write(&fil, packageCut, 27, &bytesWrote);


			  /*

			  reciveTime = HAL_GetTick();
			  uint32_TO_charmass(reciveTime, SD_Buff, 0, 8);

			  SD_Buff[8]=';';
			  for(uint8_t i=0;i<21;i++)
						  {
							  SD_Buff[i+9]=str1[0][i];

						  }
			  for(uint8_t i=0;i<21;i++)
						  {
							  SD_Buff[i+30]=str1[1][i];

						  }
			  for(uint8_t i=0;i<21;i++)
						  {
							  SD_Buff[i+51]=str1[2][i];

						  }

			  SD_Buff[72]='\n';


			  if(kolZapis==7)
			  {
				  kolZapis=0;
				  block[510]=';';
				  block[511]='\n';
				  code = SDCARD_WriteSingleBlock(blockAddr++, block);
				  memset(block, 0, sizeof(block));

			  }
			  else
			  {
				  uint8_t j=0;
                  for(uint16_t i=36*kolZapis;i<36+kolZapis*73;i++)
                  {
                	  block[i]=SD_Buff[j++];
                  }
                  kolZapis++;

			  }

*/


			          fres = f_write(&fil, &str1, sizeof(str1), &bytesWrote);

					  //fres = f_write(&fil, byte, sizeof(byte), &bytesWrote);
					  if(fres == FR_OK) {
					  //  myprintf("Wrote %i bytes to 'write.txt'!\r\n", bytesWrote);
					  } else {

					 //   myprintf("f_write error (%i)\r\n");
					  }

					  fres= f_sync(&fil);

					//  HAL_UART_Transmit_IT(&huart2, "\n", 1);
/*

					  kolZapis++;

					  if(kolZapis==5)
					  {


						  kolZapis=0;


						 if(fres!=FR_OK)
						 {
							 while(1);
						 }

					  }
*/


			 // HAL_UART_Transmit_IT(&huart2, "\n", 1);

/*
			  if(kolZapis==0){
			    fres = f_mount(&FatFs, "", 1); //1=mount now


				  if (fres != FR_OK) {
				   // myprintf("f_mount error (%i)\r\n", fres);

					//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
					while(1);
				  }



			  //fres = f_open(&fil, "write.txt", FA_OPEN_APPEND | FA_WRITE);


				  fres = f_open(&fil, "write.txt", FA_OPEN_APPEND | FA_WRITE);

			  if(fres == FR_OK) {
			 //   myprintf("I was able to open 'write.txt' for writing\r\n");
			  } else {
				  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
					while(1);
			 //   myprintf("f_open error (%i)\r\n", fres);
			  }

		}
*/


/*
			  kolZapis++;
			  if (kolZapis == 1000)
			  {

			  //Close file, don't forget this!
				  f_close(&fil);

				   //De-mount drive
				 f_mount(NULL, "", 0);
			  	  kolZapis = 0;


			  } else
			  {


			  }
*/

			//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
			//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
			//  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
	//	HAL_UART_Transmit_DMA(&huart3, &accelSelect[0][0], 5);
	//	HAL_UART_Transmit_DMA(&huart1, &accelSelect[2][0], 5);
	//	HAL_UART_Transmit_DMA(&huart5, &accelSelect[1][0], 5);

		//UsartCount=0;
		//  transmit(packageCut);


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

if(package[0][0]!=0x68 && package[0][14]!=0x68)
{
	readFlag=1;
	UsartCount=0;
	//HAL_UART_Abort(&huart3);

}else
{
	for(uint8_t j=0;j<PacketSize;j++)
	{
	for(uint8_t i=0;i<9;i++)
	{
		packageCut[0][i+j*9]=package[0][i+4+j*14];
	}
	}
	/*
	for(uint8_t i=0;i<9*20;i++)
	{
		packageCut[0][i]=package[0][i+4];
	}
	*/
	UsartCount++;

}
}
if(huart==&huart1)
{

	if(package[2][0]!=0x68 && package[2][14]!=0x68)
	{
		readFlag2=1;
		//HAL_UART_Abort(&huart3);
		UsartCount=0;

	}else
	{

		for(uint8_t j=0;j<PacketSize;j++)
		{
		for(uint8_t i=0;i<9;i++)
		{
			packageCut[2][i+j*9]=package[2][i+4+j*14];
		}
		}
		/*
		for(uint8_t i=0;i<9*20;i++)
		{
			packageCut[2][i]=package[2][i+4];
		}
		*/
		UsartCount++;
	}

}

if(huart==&huart5)
{

	if(package[1][0]!=0x68 && package[1][14]!=0x68)
	{
		readFlag3=1;
		UsartCount=0;
		//HAL_UART_Abort(&huart3);

	}else
	{
		for(uint8_t j=0;j<PacketSize;j++)
		{
		for(uint8_t i=0;i<9;i++)
		{
			packageCut[1][i+j*9]=package[1][i+4+j*14];
		}
		}
		/*
		for(uint8_t i=0;i<9*20;i++)
		{
			packageCut[1][i]=package[1][i+4];
		}
		*/
		UsartCount++;

	}

}
if(UsartCount==3 && readFlag==0 && readFlag2==0 && readFlag3==0)  // Получено 1 измерение с каждого датчика
{
	UsartCount=0;



	reciveTime = HAL_GetTick();
	for(uint8_t i=0, j=PacketSize;i<PacketSize;i++, j--)
	{
		uint32_TO_charmass(reciveTime-j, Buff_Mid, i*36, 8);
	}

	for(uint8_t j=0;j<PacketSize;j++)
	{
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Mid[i+9+j*36]=packageCut[0][i+j*9];

		}
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Mid[i+18+j*36]=packageCut[1][i+j*9];

		}
		for(uint8_t i=0;i<9;i++)
		{
			Buff_Mid[i+27+j*36]=packageCut[2][i+j*9];

		}

	}

	metka=1;

	//CountOfAccel++;

/*
	if(CountOfAccel==PacketSize)  // Считано 7 измерений с каждого датчика
	{
		CountOfAccel=0;
		metka=1;
	}


	if(package[0][0]!=0x68 && package[1][0]!=0x68  && package[2][0]!=0x68 )
		{
		CountOfAccel=0;
		UsartCount=0;
		} else
		{

		}
*/


/*

	for(uint8_t j=0;j<3;j++)
	{

	if(packageCut[j][0]==0 && packageCut[j][1]==0 && packageCut[j][2]==0
			&& packageCut[j][3]==0 && packageCut[j][4]==0 && packageCut[j][5]==0 && packageCut[j][6]==0
			&& packageCut[j][7]==0 && packageCut[j][8]==0)
	{
		UsartCount=0;
	    CountOfAccel=0;

	} else
	{
		break;

	}

	}
	*/

}

}




void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{


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
