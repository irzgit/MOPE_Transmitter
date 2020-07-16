#ifndef RF96_H_
#define RF96_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "spi.h"
#include <string.h>

#define Get_NIRQ_Di0()					HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_10)//
#define Get_NIRQ_Di1()                  HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)
// Регистры RF96
/********************Lroa mode***************************/
#define LR_RegFifo                                  0x00
// Common settings
#define LR_RegOpMode                                0x01
#define LR_RegFrMsb                                 0x06
#define LR_RegFrMid                                 0x07
#define LR_RegFrLsb                                 0x08
// Tx settings
#define LR_RegPaConfig                              0x09
#define LR_RegPaRamp                                0x0A
#define LR_RegOcp                                   0x0B
// Rx settings
#define LR_RegLna                                   0x0C
// LoRa registers
#define LR_RegFifoAddrPtr                           0x0D
#define LR_RegFifoTxBaseAddr                        0x0E
#define LR_RegFifoRxBaseAddr                        0x0F
#define LR_RegFifoRxCurrentaddr                     0x10
#define LR_RegIrqFlagsMask                          0x11
#define LR_RegIrqFlags                              0x12
#define LR_RegRxNbBytes                             0x13
#define LR_RegRxHeaderCntValueMsb                   0x14
#define LR_RegRxHeaderCntValueLsb                   0x15
#define LR_RegRxPacketCntValueMsb                   0x16
#define LR_RegRxPacketCntValueLsb                   0x17
#define LR_RegModemStat                             0x18
#define LR_RegPktSnrValue                           0x19
#define LR_RegPktRssiValue                          0x1A
#define LR_RegRssiValue                             0x1B
#define LR_RegHopChannel                            0x1C
#define LR_RegModemConfig1                          0x1D
#define LR_RegModemConfig2                          0x1E
#define LR_RegSymbTimeoutLsb                        0x1F
#define LR_RegPreambleMsb                           0x20
#define LR_RegPreambleLsb                           0x21
#define LR_RegPayloadLength                         0x22
#define LR_RegMaxPayloadLength                      0x23
#define LR_RegHopPeriod                             0x24
#define LR_RegFifoRxByteAddr                        0x25
#define LR_RegDetectOptimize                        0x31
#define LR_RegDetecionThreshold                     0x37
// I/O settings
#define REG_LR_DIOMAPPING1                          0x40
#define REG_LR_DIOMAPPING2                          0x41
// Version
#define REG_LR_VERSION                              0x42
// Additional settings
#define REG_LR_PLLHOP                               0x44
#define REG_LR_TCXO                                 0x4B
#define REG_LR_PADAC                                0x4D
#define REG_LR_FORMERTEMP                           0x5B

#define REG_LR_AGCREF                               0x61
#define REG_LR_AGCTHRESH1                           0x62
#define REG_LR_AGCTHRESH2                           0x63
#define REG_LR_AGCTHRESH3                           0x64

// Используемые функции
int16_t Rf96_LoRaReadRSSI(void);
int16_t Rf96_LoRaReadRSSIPkT(void);
void Rf96_LoRaClearIrq(void);
void Rf96_Standby(void);
void Rf96_Sleep(void);
void Rf96_EntryLoRa(void);
void Rf96_FreqChoose(uint8_t freq_value);
void Rf96_OutPower(uint8_t Power_value);
void Rf96_OCP(uint8_t OCP_value);
void Rf96_LNA(uint8_t LNA_value);
void Rf96_bandwide_CR_HeadreMod(uint8_t bandwide_value, uint8_t CR_Value, uint8_t HeaderMod_value);
void Rf96_SF_LoadCRC_SymbTimeout(uint8_t SF_value, uint8_t PayloadCrc_value, uint16_t SymbTimeout_value);
void Rf96_Preamble(uint16_t PreambLen_value);
void Rf96_PinOut_Di0_Di1(uint8_t Di0_value,uint8_t Di1_value);
void Rf96_irqMaskTX(void);
void Rf96_irqMaskRX(void);
void Rf96_PayloadLength(uint8_t LengthBytes_value);
void Rf96_TX_FifoAdr(uint8_t TX_adr_value);
void Rf96_RX_FifoAdr(uint8_t RX_adr_value);
void Rf96_FIFO_point(uint8_t adrPoint_value);
void Rf96_RX_Single_mode(void);
void Rf96_RX_Continuous_mode(void);
void Rf96_TX_mode(void);
void Rf96_DataTX_to_FiFO(char* str, uint8_t LenghtStr);
void Rf96_DataRX_From_FiFO(char* str);
void Rf96_Lora_init(void);
void Rf96_Lora_TX_mode(void);
void Rf96_Lora_RX_mode(void);
void Rf96_LoRaTxPacket(char* Str, uint8_t LenStr);
void Rf96_LoRaRxPacket(char* str);

#endif /* RF96_H_ */
