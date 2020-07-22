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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

//первый акселерометр
int xIntSumFirst, yIntSumFirst, zIntSumFirst;
int x5First, x60First, x65First, y8First, y90First, y95First, z11First, z120First, z125First;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalFirst[7], yvalFirst[7], zvalFirst[7];

//второй акслерометр
int xIntSumSecond, yIntSumSecond, zIntSumSecond;
int x5Second, x60Second, x65Second, y8Second, y90Second, y95Second, z11Second, z120Second, z125Second;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalSecond[7], yvalSecond[7], zvalSecond[7];

//третий акселерометр
int xIntSumThird, yIntSumThird, zIntSumThird;
int x5Third, x60Third, x65Third, y8Third, y90Third, y95Third, z11Third, z120Third, z125Third;
//char xval[3][7], yval[3][7], zval[3][7], str1[3][16];
char xvalThird[7], yvalThird[7], zvalThird[7], str1[67];

uint8_t TX_RX_Radio[60];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 байт (127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/

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

unsigned char Crc8(unsigned char *pcBlock, unsigned char len)
{
    unsigned char crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];

    return crc;
}


void transmit(uint8_t str[27])
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
				xIntSumFirst = (str[0] & 0x0F) * 10 + (str[1] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5First = str[1] & 0x0F; //перевод правого бита 5го байта
				x60First = str[2] >> 4; //перевод левого бита 6го байта
				x65First = str[2] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalFirst, "+%02d.%d%d ",xIntSumFirst, x5First, x60First);
				if((str[0] >> 4) == 0x01)
					xvalFirst[0] = '-';

				// Ось Y
				yIntSumFirst = (str[3] & 0x0F) * 10 + (str[4] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8First = str[4] & 0x0F; //перевод правого бита 8го байта
				y90First = str[5] >> 4; //перевод левого бита 9го байта
				y95First = str[5] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalFirst, "+%02d.%d%d ", yIntSumFirst, y8First, y90First);
				if((str[3] >> 4) == 0x01)
					yvalFirst[0] = '-';

				// Ось Z
				zIntSumFirst = (str[6] & 0x0F) * 10 + (str[7] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11First = str[7] & 0x0F; //перевод правого бита 11го байта
				z120First = str[8] >> 4; //перевод левого бита 12го байта
				z125First = str[8] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalFirst, "+%02d.%d%d ", zIntSumFirst, z11First, z120First);
				if((str[6] >> 4) == 0x01)
					zvalFirst[0] = '-';

				// второй акселерометр
				// Ось Х
				xIntSumSecond = (str[9] & 0x0F) * 10 + (str[10] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5Second = str[10] & 0x0F; //перевод правого бита 5го байта
				x60Second = str[11] >> 4; //перевод левого бита 6го байта
				x65Second = str[11] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalSecond, "+%02d.%d%d ",xIntSumSecond, x5Second, x60Second);
				if((str[9] >> 4) == 0x01)
					xvalSecond[0] = '-';

				// Ось Y
				yIntSumSecond = (str[12] & 0x0F) * 10 + (str[13] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8Second = str[13] & 0x0F; //перевод правого бита 8го байта
				y90Second = str[14] >> 4; //перевод левого бита 9го байта
				y95Second = str[14] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalSecond, "+%02d.%d%d ", yIntSumSecond, y8Second, y90Second);
				if((str[12] >> 4) == 0x01)
					yvalSecond[0] = '-';

				// Ось Z
				zIntSumSecond = (str[15] & 0x0F) * 10 + (str[16] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11Second = str[16] & 0x0F; //перевод правого бита 11го байта
				z120Second = str[17] >> 4; //перевод левого бита 12го байта
				z125Second = str[17] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalSecond, "+%02d.%d%d ", zIntSumSecond, z11Second, z120Second);
				if((str[15] >> 4) == 0x01)
					zvalSecond[0] = '-';

				// третий акселерометр
				// Ось Х
				xIntSumThird = (str[18] & 0x0F) * 10 + (str[19] >> 4); // целочисленная сумма X
				//xSum = (str[5] & 0x0F) * 100 + (str[6] >> 4) * 10 + (str[6] & 0x0F);//цифры после запятой Х
				x5Third = str[19] & 0x0F; //перевод правого бита 5го байта
				x60Third = str[20] >> 4; //перевод левого бита 6го байта
				x65Third = str[20] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalThird, "+%02d.%d%d ",xIntSumThird, x5Third, x60Third);
				if((str[18] >> 4) == 0x01)
					xvalThird[0] = '-';

				// Ось Y
				yIntSumThird = (str[21] & 0x0F) * 10 + (str[22] >> 4);	// целочисленная сумма Y
				//ySum = (str[8] & 0x0F) * 100 + (str[9] >> 4) * 10 + (str[9] & 0x0F); //цифры после запятой Y
				y8Third = str[22] & 0x0F; //перевод правого бита 8го байта
				y90Third = str[23] >> 4; //перевод левого бита 9го байта
				y95Third = str[23] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalThird, "+%02d.%d%d ", yIntSumThird, y8Third, y90Third);
				if((str[21] >> 4) == 0x01)
					yvalThird[0] = '-';

				// Ось Z
				zIntSumThird = (str[24] & 0x0F) * 10 + (str[25] >> 4); //целочисленная сумма Z
				//zSum = (str[11] & 0x0F) * 100 + (str[12] >> 4) * 10 + (str[12] & 0x0F); //цифры после запятой Z
				z11Third = str[25] & 0x0F; //перевод правого бита 11го байта
				z120Third = str[26] >> 4; //перевод левого бита 12го байта
				z125Third = str[26] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalThird, "+%02d.%d%d ", zIntSumThird, z11Third, z120Third);
				if((str[24] >> 4) == 0x01)
					zvalThird[0] = '-';
//			}
//		}
		sprintf(str1, "%s%s%s%s%s%s%s%s%s\n", xvalFirst, yvalFirst, zvalFirst, xvalSecond, yvalSecond, zvalSecond, xvalThird, yvalThird, zvalThird);
	HAL_UART_Abort(&huart2);
	HAL_UART_Transmit_IT(&huart2, (uint8_t*)str1, 64);



	//NVIC_DisableIRQ(USART2_IRQn);
	//HAL_UART_Transmit(&huart2, (uint8_t*)str1, 45,100);
	//NVIC_EnableIRQ(USART2_IRQn);



	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	//HAL_UART_Transmit_IT(&huart3, accelSelect[accelNum], 5);
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  Rf96_Lora_init();
  Rf96_Lora_RX_mode();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  /*
	  if(Get_NIRQ_Di1()) // Прерывание по таймауту
	  	 	  {
	  	 		  Rf96_LoRaClearIrq();
	  	 		  Rf96_Lora_RX_mode();
	  	 		  //Rf96_LoRaRxPacket((char*)TX_RX_Radio);

	  	 	  }
*/
	  if(Get_NIRQ_Di0()) // Прерывание по приему по радиоканалу
	  	 	 {
	  	 	    //  Rf96_LoRaRxPacket((char*)TX_RX_Radio);
	  	 	   Rf96_DataRX_From_FiFO((char*)TX_RX_Radio);

	  	 	  if(Get_NIRQ_Di3()) // Если CRC не совпадает
	  	 		{



	  	 		}
	  	 	  else              // CRC совпало
	  	 	  {
	  	 		transmit(TX_RX_Radio);
	  	 	  }
	  	 	Rf96_LoRaClearIrq();

	  	 	//TX_RX_Radio[28]='\n';

	  	 	//str1[0]='1';
	  	 	//str1[1]='\n';
	  	 //	HAL_UART_Transmit_IT(&huart2, (uint8_t*)str1, 2);
          // if(Crc8(TX_RX_Radio,27)==TX_RX_Radio[27])
          //  {
          //  	transmit(TX_RX_Radio);
          //  }



				 // TX_RX_Radio[0]='P';
				//  TX_RX_Radio[1]='i';
				//  TX_RX_Radio[2]='n';
				//  TX_RX_Radio[3]='g';
				 // Rf96_Lora_TX_mode();
				 // Rf96_LoRaTxPacket((char*)TX_RX_Radio,19);
				  //Rf96_Lora_RX_mode();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
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
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10;
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
