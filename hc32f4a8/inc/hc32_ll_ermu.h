/**
 *******************************************************************************
 * @file  hc32_ll_ermu.h
 * @brief This file contains all the functions prototypes of the ermu driver
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
#ifndef __HC32_LL_ERMU_H__
#define __HC32_LL_ERMU_H__

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
 * @addtogroup LL_ERMU
 * @{
 */

#if (LL_ERMU_ENABLE == DDL_ON)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup ERMU_Global_Types ERMU Global Types
 * @{
 */

/**
 * @brief stc_ermu_eout_t ERMU error output structure definition
 */
typedef struct {
    en_functional_state_t enClearTmrEn;         /*!< Clear Timer Enable */
    uint32_t u32ClearTmrCmpValue;               /*!< Clear Timer Compare Value */
    en_functional_state_t enToggleTmrEn;        /*!< Toggle Timer Enable */
    uint32_t u32ToggleTmrCmpValue;              /*!< Toggle Timer Compare Value */
    uint32_t u32EoutMaskGroup0;                 /*!< Error Output Mask Register @ref ERMU_Error_Src_Group0 */
    uint32_t u32EoutMaskGroup1;                 /*!< Error Output Mask Register @ref ERMU_Error_Src_Group1 */
} stc_ermu_eout_t;

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup ERMU_Global_Macros ERMU Global Macros
 * @{
 */

/**
 * @defgroup ERMU_Eout_Unit ERMU Error Output Unit
 * @{
 */
#define ERMU_EOUT0                            (0U)
#define ERMU_EOUT1                            (1U)
#define ERMU_EOUT2                            (2U)
#define ERMU_EOUT3                            (3U)
/**
 * @}
 */

/**
 * @defgroup ERMU_Wtmr_Unit ERMU Wait timer Unit
 * @{
 */
#define ERMU_WTMR0                            (0U)
#define ERMU_WTMR1                            (1U)
#define ERMU_WTMR2                            (2U)
#define ERMU_WTMR3                            (3U)
/**
 * @}
 */

/**
 * @defgroup ERMU_Error_Group ERMU Error Group
 * @{
 */
#define ERMU_ERR_GRP0                         (0U)    /*!< Error ID Number of 0~31 */
#define ERMU_ERR_GRP1                         (1U)    /*!< Error ID Number of 32~63 */
/**
 * @}
 */

/**
 * @defgroup ERMU_Error_Src_Group0 ERMU Error Source Group 0
 * @{
 */
#define ERMU_GRP0_WTMR0_ERR                   (1UL << 0U)    /*!< ERMU wait timer0 error */
#define ERMU_GRP0_WTMR1_ERR                   (1UL << 1U)    /*!< ERMU wait timer1 error */
#define ERMU_GRP0_WTMR2_ERR                   (1UL << 2U)    /*!< ERMU wait timer2 error */
#define ERMU_GRP0_WTMR3_ERR                   (1UL << 3U)    /*!< ERMU wait timer3 error */
#define ERMU_GRP0_SWDT_ERR                    (1UL << 8U)    /*!< SWDT error */
#define ERMU_GRP0_WDT_ERR                     (1UL << 9U)    /*!< WDT error */
#define ERMU_GRP0_FLASH_CORRECT_ECC_ERR       (1UL << 11U)   /*!< FLASH ECC can correct error */
#define ERMU_GRP0_FLASH_UNCORRECT_ECC_ERR     (1UL << 12U)   /*!< FLASH ECC cannot correct error */
#define ERMU_GRP0_FLASH_OVF_ECC_ERR           (1UL << 13U)   /*!< FLASH ECC overflow error */
#define ERMU_GRP0_SRAMH_CORRECT_ECC_ERR       (1UL << 14U)   /*!< SRAMH ECC can correct error */
#define ERMU_GRP0_SRAM1_CORRECT_ECC_ERR       (1UL << 15U)   /*!< SRAM1 ECC can correct error */
#define ERMU_GRP0_SRAM2_CORRECT_ECC_ERR       (1UL << 16U)   /*!< SRAM2 ECC can correct error */
#define ERMU_GRP0_SRAM3_CORRECT_ECC_ERR       (1UL << 17U)   /*!< SRAM3 ECC can correct error */
#define ERMU_GRP0_SRAM4_CORRECT_ECC_ERR       (1UL << 18U)   /*!< SRAM4 ECC can correct error */
#define ERMU_GRP0_SRAMB_CORRECT_ECC_ERR       (1UL << 19U)   /*!< SRAMB ECC can correct error */
#define ERMU_GRP0_SRAMH_UNCORRECT_ECC_ERR     (1UL << 23U)   /*!< SRAMH ECC cannot correct error */
#define ERMU_GRP0_SRAM1_UNCORRECT_ECC_ERR     (1UL << 24U)   /*!< SRAM1 ECC cannot correct error */
#define ERMU_GRP0_SRAM2_UNCORRECT_ECC_ERR     (1UL << 25U)   /*!< SRAM2 ECC cannot correct error */
#define ERMU_GRP0_SRAM3_UNCORRECT_ECC_ERR     (1UL << 26U)   /*!< SRAM3 ECC cannot correct error */
#define ERMU_GRP0_SRAM4_UNCORRECT_ECC_ERR     (1UL << 27U)   /*!< SRAM4 ECC cannot correct error */
#define ERMU_GRP0_SRAMB_UNCORRECT_ECC_ERR     (1UL << 28U)   /*!< SRAMB ECC cannot correct error */
#define ERMU_GRP0_ERR_ALL                  (ERMU_GRP0_WTMR0_ERR | ERMU_GRP0_WTMR1_ERR | ERMU_GRP0_WTMR2_ERR |           \
                                            ERMU_GRP0_WTMR3_ERR | ERMU_GRP0_SWDT_ERR | ERMU_GRP0_WDT_ERR |              \
                                            ERMU_GRP0_FLASH_CORRECT_ECC_ERR | ERMU_GRP0_FLASH_UNCORRECT_ECC_ERR |       \
                                            ERMU_GRP0_FLASH_OVF_ECC_ERR | ERMU_GRP0_SRAMH_CORRECT_ECC_ERR |             \
                                            ERMU_GRP0_SRAM1_CORRECT_ECC_ERR | ERMU_GRP0_SRAM2_CORRECT_ECC_ERR |         \
                                            ERMU_GRP0_SRAM3_CORRECT_ECC_ERR | ERMU_GRP0_SRAM4_CORRECT_ECC_ERR |         \
                                            ERMU_GRP0_SRAMB_CORRECT_ECC_ERR | ERMU_GRP0_SRAMH_UNCORRECT_ECC_ERR |       \
                                            ERMU_GRP0_SRAM1_UNCORRECT_ECC_ERR | ERMU_GRP0_SRAM2_UNCORRECT_ECC_ERR |     \
                                            ERMU_GRP0_SRAM3_UNCORRECT_ECC_ERR | ERMU_GRP0_SRAM4_UNCORRECT_ECC_ERR |     \
                                            ERMU_GRP0_SRAMB_UNCORRECT_ECC_ERR)
/**
 * @}
 */

/**
 * @defgroup ERMU_Error_Src_Group1 ERMU Error Source Group 1
 * @{
 */
#define ERMU_GRP1_CACHERAM_CORRECT_ECC_ERR    (1UL << 0U)    /*!< CACHE RAM ECC can correct error */
#define ERMU_GRP1_CACHERAM_UNCORRECT_ECC_ERR  (1UL << 1U)    /*!< CACHE RAM ECC cannot correct error */
#define ERMU_GRP1_CAN1RAM_CORRECT_ECC_ERR     (1UL << 4U)    /*!< CAN1 RAM ECC can correct error */
#define ERMU_GRP1_CAN2RAM_CORRECT_ECC_ERR     (1UL << 5U)    /*!< CAN2 RAM ECC can correct error */
#define ERMU_GRP1_MCANRAM_CORRECT_ECC_ERR     (1UL << 6U)    /*!< MCAN RAM ECC can correct error */
#define ERMU_GRP1_CAN1RAM_UNCORRECT_ECC_ERR   (1UL << 10U)   /*!< CAN1 RAM ECC cannot correct error */
#define ERMU_GRP1_CAN2RAM_UNCORRECT_ECC_ERR   (1UL << 11U)   /*!< CAN2 RAM ECC cannot correct error */
#define ERMU_GRP1_MCANRAM_UNCORRECT_ECC_ERR   (1UL << 12U)   /*!< MCAN RAM ECC cannot correct error */
#define ERMU_GRP1_PKERAM_CORRECT_ECC_ERR      (1UL << 16U)   /*!< PKE RAM ECC can correct error */
#define ERMU_GRP1_PKERAM_UNCORRECT_ECC_ERR    (1UL << 17U)   /*!< PKE RAM ECC cannot correct error */
#define ERMU_GRP1_DMA1_BUS_ERR                (1UL << 19U)   /*!< DMA1 bus error */
#define ERMU_GRP1_DMA2_BUS_ERR                (1UL << 20U)   /*!< DMA2 bus error */
#define ERMU_GRP1_USBFS_BUS_ERR               (1UL << 21U)   /*!< USBFS bus error */
#define ERMU_GRP1_USBHS_BUS_ERR               (1UL << 22U)   /*!< USBHS bus error */
#define ERMU_GRP1_ETH_BUS_ERR                 (1UL << 23U)   /*!< ETH bus error */
#define ERMU_GRP1_DMA1_TRANS_ERR              (1UL << 25U)   /*!< DMA1 transmit error */
#define ERMU_GRP1_DMA2_TRANS_ERR              (1UL << 26U)   /*!< DMA2 transmit error */
#define ERMU_GRP1_FPU_CALC_ERR                (1UL << 28U)   /*!< FPU calculate error */
#define ERMU_GRP1_CLK_DETECT_ERR              (1UL << 29U)   /*!< CLK detection error */
#define ERMU_GRP1_ERR_ALL                  (ERMU_GRP1_CACHERAM_CORRECT_ECC_ERR | ERMU_GRP1_CACHERAM_UNCORRECT_ECC_ERR | \
                                            ERMU_GRP1_CAN1RAM_CORRECT_ECC_ERR | ERMU_GRP1_CAN2RAM_CORRECT_ECC_ERR |     \
                                            ERMU_GRP1_MCANRAM_CORRECT_ECC_ERR | ERMU_GRP1_CAN1RAM_UNCORRECT_ECC_ERR |   \
                                            ERMU_GRP1_CAN2RAM_UNCORRECT_ECC_ERR | ERMU_GRP1_MCANRAM_UNCORRECT_ECC_ERR | \
                                            ERMU_GRP1_PKERAM_CORRECT_ECC_ERR | ERMU_GRP1_PKERAM_UNCORRECT_ECC_ERR |     \
                                            ERMU_GRP1_DMA1_BUS_ERR | ERMU_GRP1_DMA2_BUS_ERR | ERMU_GRP1_USBFS_BUS_ERR | \
                                            ERMU_GRP1_USBHS_BUS_ERR | ERMU_GRP1_ETH_BUS_ERR |                           \
                                            ERMU_GRP1_DMA1_TRANS_ERR | ERMU_GRP1_DMA2_TRANS_ERR |                       \
                                            ERMU_GRP1_FPU_CALC_ERR | ERMU_GRP1_CLK_DETECT_ERR)
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
 * @addtogroup ERMU_Global_Functions
 * @{
 */

/* Error output */
int32_t ERMU_EOUT_StructInit(stc_ermu_eout_t *pstcEoutInit);
int32_t ERMU_EOUT_Init(const stc_ermu_eout_t *pstcEoutInit, uint8_t u8Unit);
en_flag_status_t ERMU_EOUT_GetErrorStatus(uint8_t u8Unit);
void ERMU_EOUT_ClearErrorStatus(uint8_t u8Unit);
void ERMU_EOUT_SetErrorStatus(uint8_t u8Unit);
/* Clock */
void ERMU_SetClockDiv(uint32_t u32Div);
uint32_t ERMU_GetClockDiv(void);
void ERMU_ClockDivCmd(en_functional_state_t enNewState);
/* Clear timer */
void ERMU_CTMR_Cmd(uint8_t u8Unit, en_functional_state_t enNewState);
en_flag_status_t ERMU_CTMR_GetStatus(uint8_t u8Unit);
uint16_t ERMU_CTMR_GetCountValue(uint8_t u8Unit);
uint16_t ERMU_CTMR_GetCompareValue(uint8_t u8Unit);
void ERMU_CTMR_SetCompareValue(uint8_t u8Unit, uint32_t u32Value);
/* Toggle timer */
void ERMU_TTMR_Cmd(uint8_t u8Unit, en_functional_state_t enNewState);
uint16_t ERMU_TTMR_GetCompareValue(uint8_t u8Unit);
void ERMU_TTMR_SetCompareValue(uint8_t u8Unit, uint32_t u32Value);
/* Wait timer */
void ERMU_WTMR_Cmd(uint8_t u8Unit, en_functional_state_t enNewState);
void ERMU_WTMR_HighPriorityIntBootCmd(uint8_t u8Unit, en_functional_state_t enNewState);
void ERMU_WTMR_LowPriorityIntBootCmd(uint8_t u8Unit, en_functional_state_t enNewState);
void ERMU_WTMR_Stop(uint8_t u8Unit);
en_flag_status_t ERMU_WTMR_GetStatus(uint8_t u8Unit);
uint16_t ERMU_WTMR_GetCountValue(uint8_t u8Unit);
uint16_t ERMU_WTMR_GetCompareValue(uint8_t u8Unit);
void ERMU_WTMR_SetCompareValue(uint8_t u8Unit, uint32_t u32Value);

en_flag_status_t ERMU_GetErrorSrcStatus(uint8_t u8Group, uint32_t u32ErrSrc);
void ERMU_ClearErrorSrcStatus(uint8_t u8Group, uint32_t u32ErrSrc);
void ERMU_SetPseudoErrorTrigger(uint8_t u8Group, uint32_t u32ErrSrc);
void ERMU_ResetCmd(uint8_t u8Group, uint32_t u32ErrSrc, en_functional_state_t enNewState);
void ERMU_HighPriorityIntCmd(uint8_t u8Group, uint32_t u32ErrSrc, en_functional_state_t enNewState);
void ERMU_LowPriorityIntCmd(uint8_t u8Group, uint32_t u32ErrSrc, en_functional_state_t enNewState);

int32_t ERMU_DeInit(void);

/**
 * @}
 */

#endif /* LL_ERMU_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __HC32_LL_ERMU_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
