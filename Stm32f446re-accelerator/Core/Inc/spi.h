#ifndef __DK_SPI_h__
#define __DK_SPI_h__

#include "main.h"
#include "stm32f4xx_hal.h"


#define NOP()                _asm("nope")


#define NSS_H()						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET)
#define NSS_L()						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET)


void SPICmd8bit(uint8_t WrPara);
uint8_t SPIRead(uint8_t adr);
uint8_t SPIRead8bit(void);
void SPIWrite(uint8_t adr, uint8_t WrPara);
void SPIBurstRead(uint8_t adr, uint8_t *ptr, uint8_t length);
void BurstWrite(uint8_t adr, const uint8_t *ptr, uint8_t length);

extern SPI_HandleTypeDef hspi1;
#endif


