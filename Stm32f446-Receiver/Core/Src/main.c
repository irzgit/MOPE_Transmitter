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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
//первый акселерометр
int xIntSumFirst, yIntSumFirst, zIntSumFirst;
int x5First, x60First, x65First, y8First, y90First, y95First, z11First, z120First, z125First;
char xvalFirst[7], yvalFirst[7], zvalFirst[7];

//второй акслерометр
int xIntSumSecond, yIntSumSecond, zIntSumSecond;
int x5Second, x60Second, x65Second, y8Second, y90Second, y95Second, z11Second, z120Second, z125Second;
char xvalSecond[7], yvalSecond[7], zvalSecond[7];

//третий акселерометр
int xIntSumThird, yIntSumThird, zIntSumThird;
int x5Third, x60Third, x65Third, y8Third, y90Third, y95Third, z11Third, z120Third, z125Third;
char xvalThird[7], yvalThird[7], zvalThird[7], str1[67];
uint16_t CRC_c=0;
uint8_t TX_RX_Radio[60];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



void transmit(uint8_t str[27])
{

				// первый акселерометр
				// Ось Х
				xIntSumFirst = (str[0] & 0x0F) * 10 + (str[1] >> 4); // целочисленная сумма X
				x5First = str[1] & 0x0F; //перевод правого бита 5го байта
				x60First = str[2] >> 4; //перевод левого бита 6го байта
				x65First = str[2] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalFirst, "+%02d.%d%d ",xIntSumFirst, x5First, x60First);
				if((str[0] >> 4) == 0x01)
					xvalFirst[0] = '-';

				// Ось Y
				yIntSumFirst = (str[3] & 0x0F) * 10 + (str[4] >> 4);	// целочисленная сумма Y
				y8First = str[4] & 0x0F; //перевод правого бита 8го байта
				y90First = str[5] >> 4; //перевод левого бита 9го байта
				y95First = str[5] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalFirst, "+%02d.%d%d ", yIntSumFirst, y8First, y90First);
				if((str[3] >> 4) == 0x01)
					yvalFirst[0] = '-';

				// Ось Z
				zIntSumFirst = (str[6] & 0x0F) * 10 + (str[7] >> 4); //целочисленная сумма Z
				z11First = str[7] & 0x0F; //перевод правого бита 11го байта
				z120First = str[8] >> 4; //перевод левого бита 12го байта
				z125First = str[8] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalFirst, "+%02d.%d%d ", zIntSumFirst, z11First, z120First);
				if((str[6] >> 4) == 0x01)
					zvalFirst[0] = '-';

				// второй акселерометр
				// Ось Х
				xIntSumSecond = (str[9] & 0x0F) * 10 + (str[10] >> 4); // целочисленная сумма X
				x5Second = str[10] & 0x0F; //перевод правого бита 5го байта
				x60Second = str[11] >> 4; //перевод левого бита 6го байта
				x65Second = str[11] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalSecond, "+%02d.%d%d ",xIntSumSecond, x5Second, x60Second);
				if((str[9] >> 4) == 0x01)
					xvalSecond[0] = '-';

				// Ось Y
				yIntSumSecond = (str[12] & 0x0F) * 10 + (str[13] >> 4);	// целочисленная сумма Y
				y8Second = str[13] & 0x0F; //перевод правого бита 8го байта
				y90Second = str[14] >> 4; //перевод левого бита 9го байта
				y95Second = str[14] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalSecond, "+%02d.%d%d ", yIntSumSecond, y8Second, y90Second);
				if((str[12] >> 4) == 0x01)
					yvalSecond[0] = '-';

				// Ось Z
				zIntSumSecond = (str[15] & 0x0F) * 10 + (str[16] >> 4); //целочисленная сумма Z
				z11Second = str[16] & 0x0F; //перевод правого бита 11го байта
				z120Second = str[17] >> 4; //перевод левого бита 12го байта
				z125Second = str[17] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalSecond, "+%02d.%d%d ", zIntSumSecond, z11Second, z120Second);
				if((str[15] >> 4) == 0x01)
					zvalSecond[0] = '-';

				// третий акселерометр
				// Ось Х
				xIntSumThird = (str[18] & 0x0F) * 10 + (str[19] >> 4); // целочисленная сумма X
				x5Third = str[19] & 0x0F; //перевод правого бита 5го байта
				x60Third = str[20] >> 4; //перевод левого бита 6го байта
				x65Third = str[20] & 0x0F; //перевод правого бита 6го байта
				sprintf(xvalThird, "+%02d.%d%d ",xIntSumThird, x5Third, x60Third);
				if((str[18] >> 4) == 0x01)
					xvalThird[0] = '-';

				// Ось Y
				yIntSumThird = (str[21] & 0x0F) * 10 + (str[22] >> 4);	// целочисленная сумма Y
				y8Third = str[22] & 0x0F; //перевод правого бита 8го байта
				y90Third = str[23] >> 4; //перевод левого бита 9го байта
				y95Third = str[23] & 0x0F; //перевод правого бита 9го байта
				sprintf(yvalThird, "+%02d.%d%d ", yIntSumThird, y8Third, y90Third);
				if((str[21] >> 4) == 0x01)
					yvalThird[0] = '-';

				// Ось Z
				zIntSumThird = (str[24] & 0x0F) * 10 + (str[25] >> 4); //целочисленная сумма Z
				z11Third = str[25] & 0x0F; //перевод правого бита 11го байта
				z120Third = str[26] >> 4; //перевод левого бита 12го байта
				z125Third = str[26] & 0x0F; //перевод правого бита 12го байта
				sprintf(zvalThird, "+%02d.%d%d ", zIntSumThird, z11Third, z120Third);
				if((str[24] >> 4) == 0x01)
					zvalThird[0] = '-';

			sprintf(str1, "%s%s%s%s%s%s%s%s%s\n", xvalFirst, yvalFirst, zvalFirst, xvalSecond, yvalSecond, zvalSecond, xvalThird, yvalThird, zvalThird);
	HAL_UART_Abort(&huart2);
	HAL_UART_Transmit_IT(&huart2, (uint8_t*)str1, 64);



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
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  Rf96_Lora_init();
  Rf96_Lora_RX_mode();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {







	  if(Get_NIRQ_Di0()) // Прерывание по приему по радиоканалу
	  	 	 {
	  	 	    //  Rf96_LoRaRxPacket((char*)TX_RX_Radio);
	  	 	   Rf96_DataRX_From_FiFO((char*)TX_RX_Radio);

	  	 	CRC_c=(TX_RX_Radio[27]<<8)+TX_RX_Radio[28];


              if(CRC_c==Crc16(TX_RX_Radio, 27))
              {
            	  transmit(TX_RX_Radio);
              }
/*
	  	 	  if(Get_NIRQ_Di3()) // Если CRC не совпадает
	  	 		{



	  	 		}
	  	 	  else              // CRC совпало
	  	 	  {
	  	 		transmit(TX_RX_Radio);
	  	 	  }
*/
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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
