/**
 *******************************************************************************
 * @file  hc32_ll_rmu.h
 * @brief This file contains all the functions prototypes of the RMU driver
 *        library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2024-09-13       CDT             First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2025, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#ifndef __HC32_LL_RMU_H__
#define __HC32_LL_RMU_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_def.h"

#include "hc32f4xx.h"
#include "hc32f4xx_conf.h"
/**
 * @addtogroup LL_Driver
 * @{
 */

/**
 * @addtogroup LL_RMU
 * @{
 */
#if (LL_RMU_ENABLE == DDL_ON)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup RMU_Global_Macros RMU Global Macros
 * @{
 */

/**
 * @defgroup RMU_ResetCause Rmu reset cause
 * @{
 */
#define RMU_FLAG_PWR_ON                 (RMU_RSTF0_PORF)        /*!< Power on reset */
#define RMU_FLAG_PIN                    (RMU_RSTF0_PINRF)       /*!< Reset pin reset */
#define RMU_FLAG_BROWN_OUT              (RMU_RSTF0_BORF)        /*!< Brown-out reset */
#define RMU_FLAG_PVD1                   (RMU_RSTF0_PVD1RF)      /*!< Program voltage Detection 1 reset */
#define RMU_FLAG_PVD2                   (RMU_RSTF0_PVD2RF)      /*!< Program voltage Detection 2 reset */
#define RMU_FLAG_WDT                    (RMU_RSTF0_WDRF)        /*!< Watchdog timer reset */
#define RMU_FLAG_SWDT                   (RMU_RSTF0_SWDRF)       /*!< Special watchdog timer reset */
#define RMU_FLAG_PWR_DOWN               (RMU_RSTF0_PDRF)        /*!< Power down reset */
#define RMU_FLAG_SW                     (RMU_RSTF0_SWRF)        /*!< Software reset */
#define RMU_FLAG_MPU_ERR                (RMU_RSTF0_MPUERF)      /*!< Mpu error reset */
#define RMU_FLAG_RAM_ECC                (RMU_RSTF0_RAECRF)      /*!< Ram ECC reset */
#define RMU_FLAG_CLK_ERR                (RMU_RSTF0_CKFERF)      /*!< Clk frequency error reset */
#define RMU_FLAG_XTAL_ERR               (RMU_RSTF0_XTALERF)     /*!< Xtal error reset */
#define RMU_FLAG_CPU_LOCKUP             (RMU_RSTF0_LKUPRF)      /*!< M4 Lockup reset */
#define RMU_FLAG_FLASH_ECC              (RMU_RSTF0_FLECRF)      /*!< Flash ECC reset */
#define RMU_FLAG_ERMU                   (RMU_RSTF0_ERMURF)      /*!< ERMU reset */
#define RMU_FLAG_MX                     (RMU_RSTF0_MULTIRF)     /*!< Multiply reset cause */
#define RMU_FLAG_ALL                    (RMU_FLAG_PWR_ON | RMU_FLAG_PIN | RMU_FLAG_BROWN_OUT | RMU_FLAG_PVD1 | \
                                        RMU_FLAG_PVD2 | RMU_FLAG_WDT | RMU_FLAG_SWDT | RMU_FLAG_PWR_DOWN | \
                                        RMU_FLAG_SW | RMU_FLAG_MPU_ERR | RMU_FLAG_RAM_ECC | RMU_FLAG_CLK_ERR | \
                                        RMU_FLAG_XTAL_ERR | RMU_FLAG_CPU_LOCKUP | RMU_FLAG_FLASH_ECC | \
                                        RMU_FLAG_ERMU | RMU_FLAG_MX)
/**
 * @}
 */

/**
 * @defgroup RMU_FRST0_Peripheral RMU FRST0 peripheral
 * @{
 */
#define RMU_FRST0_PERIPH_ERMU           (RMU_FRST0_ERMU)
#define RMU_FRST0_PERIPH_PKE            (RMU_FRST0_PKE)
#define RMU_FRST0_PERIPH_KEYSCAN        (RMU_FRST0_KEYSCAN)
#define RMU_FRST0_PERIPH_DMA1           (RMU_FRST0_DMA1)
#define RMU_FRST0_PERIPH_DMA2           (RMU_FRST0_DMA2)
#define RMU_FRST0_PERIPH_FCM            (RMU_FRST0_FCM)
#define RMU_FRST0_PERIPH_AOS            (RMU_FRST0_AOS)
#define RMU_FRST0_PERIPH_CTC            (RMU_FRST0_CTC)
#define RMU_FRST0_PERIPH_MAU            (RMU_FRST0_MAU)
#define RMU_FRST0_PERIPH_SKE            (RMU_FRST0_SKE)
#define RMU_FRST0_PERIPH_HASH           (RMU_FRST0_HASH)
#define RMU_FRST0_PERIPH_TRNG           (RMU_FRST0_TRNG)
#define RMU_FRST0_PERIPH_CRC            (RMU_FRST0_CRC)
#define RMU_FRST0_PERIPH_DCU1           (RMU_FRST0_DCU1)
#define RMU_FRST0_PERIPH_DCU2           (RMU_FRST0_DCU2)
#define RMU_FRST0_PERIPH_DCU3           (RMU_FRST0_DCU3)
#define RMU_FRST0_PERIPH_DCU4           (RMU_FRST0_DCU4)
#define RMU_FRST0_PERIPH_DCU5           (RMU_FRST0_DCU5)
#define RMU_FRST0_PERIPH_DCU6           (RMU_FRST0_DCU6)
#define RMU_FRST0_PERIPH_DCU7           (RMU_FRST0_DCU7)
#define RMU_FRST0_PERIPH_DCU8           (RMU_FRST0_DCU8)
#define RMU_FRST0_PERIPH_ALL            (0xFFFFF100UL)
/**
 * @}
 */

/**
 * @defgroup RMU_FRST1_Peripheral RMU FRST1 peripheral
 * @{
 */
#define RMU_FRST1_PERIPH_CAN1           (RMU_FRST1_CAN1)
#define RMU_FRST1_PERIPH_CAN2           (RMU_FRST1_CAN2)
#define RMU_FRST1_PERIPH_ETHMAC         (RMU_FRST1_ETHMAC)
#define RMU_FRST1_PERIPH_QSPI           (RMU_FRST1_QSPI)
#define RMU_FRST1_PERIPH_I2C1           (RMU_FRST1_I2C1)
#define RMU_FRST1_PERIPH_I2C2           (RMU_FRST1_I2C2)
#define RMU_FRST1_PERIPH_I2C3           (RMU_FRST1_I2C3)
#define RMU_FRST1_PERIPH_I2C4           (RMU_FRST1_I2C4)
#define RMU_FRST1_PERIPH_I2C5           (RMU_FRST1_I2C5)
#define RMU_FRST1_PERIPH_I2C6           (RMU_FRST1_I2C6)
#define RMU_FRST1_PERIPH_SDIOC1         (RMU_FRST1_SDIOC1)
#define RMU_FRST1_PERIPH_SDIOC2         (RMU_FRST1_SDIOC2)
#define RMU_FRST1_PERIPH_I2S1           (RMU_FRST1_I2S1)
#define RMU_FRST1_PERIPH_I2S2           (RMU_FRST1_I2S2)
#define RMU_FRST1_PERIPH_I2S3           (RMU_FRST1_I2S3)
#define RMU_FRST1_PERIPH_I2S4           (RMU_FRST1_I2S4)
#define RMU_FRST1_PERIPH_SPI1           (RMU_FRST1_SPI1)
#define RMU_FRST1_PERIPH_SPI2           (RMU_FRST1_SPI2)
#define RMU_FRST1_PERIPH_SPI3           (RMU_FRST1_SPI3)
#define RMU_FRST1_PERIPH_SPI4           (RMU_FRST1_SPI4)
#define RMU_FRST1_PERIPH_SPI5           (RMU_FRST1_SPI5)
#define RMU_FRST1_PERIPH_SPI6           (RMU_FRST1_SPI6)
#define RMU_FRST1_PERIPH_USBFS          (RMU_FRST1_USBFS)
#define RMU_FRST1_PERIPH_USBHS          (RMU_FRST1_USBHS)
#define RMU_FRST1_PERIPH_FMAC1          (RMU_FRST1_FMAC1)
#define RMU_FRST1_PERIPH_FMAC2          (RMU_FRST1_FMAC2)
#define RMU_FRST1_PERIPH_FMAC3          (RMU_FRST1_FMAC3)
#define RMU_FRST1_PERIPH_FMAC4          (RMU_FRST1_FMAC4)
#define RMU_FRST1_PERIPH_MCAN1          (RMU_FRST1_MCAN1)
#define RMU_FRST1_PERIPH_MCAN2          (RMU_FRST1_MCAN2)
#define RMU_FRST1_PERIPH_ALL            (0x3FFFFFFFUL)
/**
 * @}
 */

/**
 * @defgroup RMU_FRST2_Peripheral RMU FRST2 peripheral
 * @{
 */
#define RMU_FRST2_PERIPH_TMR6           (RMU_FRST2_TMR6)
#define RMU_FRST2_PERIPH_TMR4_1         (RMU_FRST2_TMR4_1)
#define RMU_FRST2_PERIPH_TMR4_2         (RMU_FRST2_TMR4_2)
#define RMU_FRST2_PERIPH_TMR4_3         (RMU_FRST2_TMR4_3)
#define RMU_FRST2_PERIPH_TMR0_1         (RMU_FRST2_TMR0_1)
#define RMU_FRST2_PERIPH_TMR0_2         (RMU_FRST2_TMR0_2)
#define RMU_FRST2_PERIPH_TMR0_3         (RMU_FRST2_TMR0_3)
#define RMU_FRST2_PERIPH_EMB            (RMU_FRST2_EMB)
#define RMU_FRST2_PERIPH_TMR2_1         (RMU_FRST2_TMR2_1)
#define RMU_FRST2_PERIPH_TMR2_2         (RMU_FRST2_TMR2_2)
#define RMU_FRST2_PERIPH_TMR2_3         (RMU_FRST2_TMR2_3)
#define RMU_FRST2_PERIPH_TMR2_4         (RMU_FRST2_TMR2_4)
#define RMU_FRST2_PERIPH_TMRA_1         (RMU_FRST2_TMRA_1)
#define RMU_FRST2_PERIPH_TMRA_2         (RMU_FRST2_TMRA_2)
#define RMU_FRST2_PERIPH_TMRA_3         (RMU_FRST2_TMRA_3)
#define RMU_FRST2_PERIPH_TMRA_4         (RMU_FRST2_TMRA_4)
#define RMU_FRST2_PERIPH_TMRA_5         (RMU_FRST2_TMRA_5)
#define RMU_FRST2_PERIPH_TMRA_6         (RMU_FRST2_TMRA_6)
#define RMU_FRST2_PERIPH_TMRA_7         (RMU_FRST2_TMRA_7)
#define RMU_FRST2_PERIPH_TMRA_8         (RMU_FRST2_TMRA_8)
#define RMU_FRST2_PERIPH_TMRA_9         (RMU_FRST2_TMRA_9)
#define RMU_FRST2_PERIPH_TMRA_10        (RMU_FRST2_TMRA_10)
#define RMU_FRST2_PERIPH_TMRA_11        (RMU_FRST2_TMRA_11)
#define RMU_FRST2_PERIPH_TMRA_12        (RMU_FRST2_TMRA_12)
#define RMU_FRST2_PERIPH_ALL            (0xFFFFF701UL)
/**
 * @}
 */

/**
 * @defgroup RMU_FRST3_Peripheral RMU FRST3 peripheral
 * @{
 */
#define RMU_FRST3_PERIPH_ADC1           (RMU_FRST3_ADC1)
#define RMU_FRST3_PERIPH_ADC2           (RMU_FRST3_ADC2)
#define RMU_FRST3_PERIPH_ADC3           (RMU_FRST3_ADC3)
#define RMU_FRST3_PERIPH_DAC1           (RMU_FRST3_DAC1)
#define RMU_FRST3_PERIPH_DAC2           (RMU_FRST3_DAC2)
#define RMU_FRST3_PERIPH_CMP1_2         (RMU_FRST3_CMP12)
#define RMU_FRST3_PERIPH_CMP3_4         (RMU_FRST3_CMP34)
#define RMU_FRST3_PERIPH_TMR0_4         (RMU_FRST3_TMR0_4)
#define RMU_FRST3_PERIPH_TMR0_5         (RMU_FRST3_TMR0_5)
#define RMU_FRST3_PERIPH_OTS            (RMU_FRST3_OTS)
#define RMU_FRST3_PERIPH_DVP            (RMU_FRST3_DVP)
#define RMU_FRST3_PERIPH_SMC            (RMU_FRST3_SMC)
#define RMU_FRST3_PERIPH_DMC            (RMU_FRST3_DMC)
#define RMU_FRST3_PERIPH_NFC            (RMU_FRST3_NFC)
#define RMU_FRST3_PERIPH_USART1         (RMU_FRST3_USART1)
#define RMU_FRST3_PERIPH_USART2         (RMU_FRST3_USART2)
#define RMU_FRST3_PERIPH_USART3         (RMU_FRST3_USART3)
#define RMU_FRST3_PERIPH_USART4         (RMU_FRST3_USART4)
#define RMU_FRST3_PERIPH_USART5         (RMU_FRST3_USART5)
#define RMU_FRST3_PERIPH_USART6         (RMU_FRST3_USART6)
#define RMU_FRST3_PERIPH_USART7         (RMU_FRST3_USART7)
#define RMU_FRST3_PERIPH_USART8         (RMU_FRST3_USART8)
#define RMU_FRST3_PERIPH_USART9         (RMU_FRST3_USART9)
#define RMU_FRST3_PERIPH_USART10        (RMU_FRST3_USART10)
#define RMU_FRST3_PERIPH_ALL            (0x3FF79F37UL)
/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup RMU_Global_Functions
 * @{
 */

en_flag_status_t RMU_GetStatus(uint32_t u32RmuResetCause);
void RMU_ClearStatus(void);

void RMU_CPULockUpCmd(en_functional_state_t enNewState);

int32_t RMU_Frst0PeriphReset(uint32_t u32Frst0Periph);
int32_t RMU_Frst1PeriphReset(uint32_t u32Frst1Periph);
int32_t RMU_Frst2PeriphReset(uint32_t u32Frst2Periph);
int32_t RMU_Frst3PeriphReset(uint32_t u32Frst3Periph);

/**
 * @}
 */

#endif /* LL_RMU_ENABLE */

/**
 * @}
 */

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* __HC32_LL_RMU_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/

