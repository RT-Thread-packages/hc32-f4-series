/**
 *******************************************************************************
 * @file  hc32_ll_dac.c
 * @brief This file provides firmware functions to manage the Digital-to-Analog
 *        Converter(DAC).
 @verbatim
   Change Logs:
   Date             Author          Notes
   2024-01-15       CDT             First version
   2024-06-30       CDT             Modify valid channel assert macro
                                    All parameter assert add module name
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2022-2024, Xiaohua Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by XHSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_dac.h"
#include "hc32_ll_utility.h"

/**
 * @addtogroup LL_Driver
 * @{
 */

/**
 * @defgroup LL_DAC DAC
 * @brief DAC Driver Library
 * @{
 */

#if (LL_DAC_ENABLE == DDL_ON)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup DAC_Local_Macros DAC Local Macros
 * @{
 */
#define DAC_RESOLUTION                  (DAC_RESOLUTION_12BIT)
#define DAC_DATA_REG_WIDTH              (16U)
#define DAC_DATA_RIGHT_ALIGN_MASK       ((2U << DAC_RESOLUTION) - 1U)
#define DAC_DATA_LEFT_ALIGN_MASK        (DAC_DATA_RIGHT_ALIGN_MASK << (DAC_DATA_REG_WIDTH - DAC_RESOLUTION))

#define DAC_RMU_TIMEOUT                 (100U)

/**
 * @defgroup DAC_Check_Parameters_Validity DAC Check Parameters Validity
 * @{
 */
#define IS_DAC_UNIT(x)                  (((x) == CM_DAC1) || ((x) == CM_DAC2))
#define IS_DAC_DUAL_CH_UNIT(x)          ((x) == CM_DAC1)

#define IS_DAC_CH(unit,ch)              ((CM_DAC1 == (unit) && (DAC_CH1 == (ch) || DAC_CH2 == (ch))) || \
                                         (CM_DAC2 == (unit) && (DAC_CH1 == (ch))))

#define IS_DAC_DATA_ALIGN(x)            (((x) == DAC_DATA_ALIGN_LEFT) || ((x) == DAC_DATA_ALIGN_RIGHT))

#define IS_DAC_ADCPRIO_CONFIG(x)        ((0U != (x)) && (DAC_ADP_SEL_ALL == ((x) | DAC_ADP_SEL_ALL)))

#define IS_DAC_RIGHT_ALIGNED_DATA(data) (((data) & ~DAC_DATA_RIGHT_ALIGN_MASK) == 0U)
#define IS_DAC_LEFT_ALIGNED_DATA(data)  (((data) & ~DAC_DATA_LEFT_ALIGN_MASK) == 0U)

#define IS_DAC_AMP_CTRL_ALLOWED(x,Ch)                                          \
(   (READ_REG16_BIT(DACx->DAOCR, DAC_DAOCR_DAODIS1 << (Ch)) == 0U) ||          \
    ((x) == DISABLE))

#define IS_DAC_CH_DATA_TRANS_MD(x)                                             \
(   ((x) == DAC_CH_DATA_TRANS_NORMAL)   ||                                     \
    ((x) == DAC_CH_DATA_TRANS_HRPWM))

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup DAC_Global_Functions DAC Global Functions
 * @{
 */

/**
 * @brief  DAC data register's data alignment pattern configuration
 * @param  [in] DACx       Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Align   Specify the data alignment.
 *         This parameter can be a value of @ref DAC_DATAREG_ALIGN_PATTERN
 *         - DAC_DATA_ALIGN_LEFT:  left alignment
 *         - DAC_DATA_ALIGN_RIGHT:  right alignment
 * @retval None
 */
void DAC_DataRegAlignConfig(CM_DAC_TypeDef *DACx, uint16_t u16Align)
{
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_DATA_ALIGN(u16Align));

    MODIFY_REG16(DACx->DACR, DAC_DACR_DPSEL, u16Align);
}

/**
 * @brief  DAC output function command
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch      Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @param  [in] enNewState           An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void DAC_OutputCmd(CM_DAC_TypeDef *DACx, uint16_t u16Ch, en_functional_state_t enNewState)
{
    uint16_t u16Cmd;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    u16Cmd = (uint16_t)(1UL << (DAC_DAOCR_DAODIS1_POS + u16Ch));

    if (ENABLE == enNewState) {
        CLR_REG16_BIT(DACx->DAOCR, u16Cmd);
    } else {
        SET_REG16_BIT(DACx->DAOCR, u16Cmd);
    }
}

/**
 * @brief  DAC AMP function command
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch      Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @param  [in] enNewState           An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void DAC_AMPCmd(CM_DAC_TypeDef *DACx, uint16_t u16Ch, en_functional_state_t enNewState)
{
    uint16_t u16Cmd;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_DAC_AMP_CTRL_ALLOWED(enNewState, u16Ch));
    u16Cmd = (uint16_t)(1UL << (DAC_DACR_DAAMP1_POS + u16Ch));

    if (ENABLE == enNewState) {
        SET_REG16_BIT(DACx->DACR, u16Cmd);
    } else {
        CLR_REG16_BIT(DACx->DACR, u16Cmd);
    }
}

/**
 * @brief  DAC ADC priority function command
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] enNewState           An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   please make sure ADC is in stoped status before calling DAC_ADCPrioCmd
 */
void DAC_ADCPrioCmd(CM_DAC_TypeDef *DACx, en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (ENABLE == enNewState) {
        SET_REG16_BIT(DACx->DAADPCR, DAC_DAADPCR_ADPEN);
    } else {
        CLR_REG16_BIT(DACx->DAADPCR, DAC_DAADPCR_ADPEN);
    }
}

/**
 * @brief  Enable or Disable the ADC priority for the selected ADCx
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16ADCxPrio  ADCx priority to be enabled or disabled.
 *         This parameter can be one or any combination of the following values:
 *         @arg DAC_ADP_SEL_ADC1
 *         @arg DAC_ADP_SEL_ADC2
 *         @arg DAC_ADP_SEL_ADC3
 * @param  [in] enNewState    An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   please make sure ADC is in stoped status before calling DAC_ADCPrioConfig
 */
void DAC_ADCPrioConfig(CM_DAC_TypeDef *DACx, uint16_t u16ADCxPrio, en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_ADCPRIO_CONFIG(u16ADCxPrio));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (ENABLE == enNewState) {
        SET_REG16_BIT(DACx->DAADPCR, u16ADCxPrio);
    } else {
        CLR_REG16_BIT(DACx->DAADPCR, u16ADCxPrio);
    }
}

/**
 * @brief  Start the specified DAC channel
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @retval int32_t:
 *         - LL_OK: No errors occurred
 *         - LL_ERR_INVD_MD: cannot start single channel when \n
 *                           this channel have already been started by \n
 *                           @ref DAC_StartDualCh
 */
int32_t DAC_Start(CM_DAC_TypeDef *DACx, uint16_t u16Ch)
{
    int32_t i32Ret = LL_OK;
    uint16_t u16Cmd;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));

    if ((DACx->DACR & DAC_DACR_DAE) != 0U) {
        i32Ret = LL_ERR_INVD_MD;
    } else {
        u16Cmd = (uint16_t)(1UL << (DAC_DACR_DA1E_POS + u16Ch));
        SET_REG16_BIT(DACx->DACR, u16Cmd);
    }

    return i32Ret;
}

/**
 * @brief  Stop the specified DAC channel
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @retval int32_t:
 *         - LL_OK: No errors occurred
 *         - LL_ERR_INVD_MD: cannot stop single channel when \n
 *                           this channel is started by \n
 *                           @ref DAC_StartDualCh
 */
int32_t DAC_Stop(CM_DAC_TypeDef *DACx, uint16_t u16Ch)
{
    int32_t i32Ret = LL_OK;
    uint16_t u16Cmd;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));

    if ((DACx->DACR & DAC_DACR_DAE) != 0U) {
        i32Ret = LL_ERR_INVD_MD;
    } else {
        u16Cmd = (uint16_t)(1UL << (DAC_DACR_DA1E_POS + u16Ch));
        CLR_REG16_BIT(DACx->DACR, u16Cmd);
    }

    return i32Ret;
}

/**
 * @brief  Start DAC channel 1 and channel 2
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @retval None
 */
void DAC_StartDualCh(CM_DAC_TypeDef *DACx)
{
    DDL_ASSERT(IS_DAC_DUAL_CH_UNIT(DACx));

    SET_REG16_BIT(DACx->DACR, DAC_DACR_DAE);
}

/**
 * @brief  Stop DAC channel 1 and channel 2
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @retval None
 */
void DAC_StopDualCh(CM_DAC_TypeDef *DACx)
{
    DDL_ASSERT(IS_DAC_DUAL_CH_UNIT(DACx));

    CLR_REG16_BIT(DACx->DACR, DAC_DACR_DAE);
}

/**
 * @brief  Set the specified data to the data holding register of specified DAC channel
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @param  [in] u16Data   Data to be loaded into data holding register of specified channel
 * @retval None
 */
void DAC_SetChData(CM_DAC_TypeDef *DACx, uint16_t u16Ch, uint16_t u16Data)
{
    __IO uint16_t *u16DADRx;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));

    if (READ_REG16_BIT(DACx->DACR, DAC_DACR_DPSEL) == DAC_DATA_ALIGN_LEFT) {
        DDL_ASSERT(IS_DAC_LEFT_ALIGNED_DATA(u16Data));
    } else {
        DDL_ASSERT(IS_DAC_RIGHT_ALIGNED_DATA(u16Data));
    }

    u16DADRx = (uint16_t *)((uint32_t) & (DACx->DADR1) + u16Ch * 2UL);
    WRITE_REG16(*u16DADRx, u16Data);
}

/**
 * @brief  Set the specified data to the data holding register of DAC channel 1 and channel 2
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  u16Data1:    Data to be loaded into data holding register of channel 1
 * @param  u16Data2:    Data to be loaded into data holding register of channel 2
 * @retval None
 */
void DAC_SetDualChData(CM_DAC_TypeDef *DACx, uint16_t u16Data1, uint16_t u16Data2)
{
    uint32_t u32Data;
    __IO uint32_t *u32DADRx;

    DDL_ASSERT(IS_DAC_DUAL_CH_UNIT(DACx));

    if (READ_REG16_BIT(DACx->DACR, DAC_DACR_DPSEL) == DAC_DATA_ALIGN_LEFT) {
        DDL_ASSERT(IS_DAC_LEFT_ALIGNED_DATA(u16Data1));
        DDL_ASSERT(IS_DAC_LEFT_ALIGNED_DATA(u16Data2));
    } else {
        DDL_ASSERT(IS_DAC_RIGHT_ALIGNED_DATA(u16Data1));
        DDL_ASSERT(IS_DAC_RIGHT_ALIGNED_DATA(u16Data2));
    }

    u32Data = ((uint32_t)u16Data2 << 16U) | u16Data1;
    u32DADRx = (__IO uint32_t *)(uint32_t)(&DACx->DADR1);
    WRITE_REG32(*u32DADRx, u32Data);
}

/**
 * @brief  Get convert status of specified channel in ADC priority mode
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @retval int32_t:
 *         - LL_ERR_INVD_MD: Could not get convert status when adc priority is not enabled
 *         - LL_OK: Data convert completed
 *         - LL_ERR_BUSY: Data convert is ongoing
 */
int32_t DAC_GetChConvertState(const CM_DAC_TypeDef *DACx, uint16_t u16Ch)
{
    int32_t i32Ret = LL_ERR_INVD_MD;

    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));

    if (0U != READ_REG16_BIT(DACx->DAADPCR, DAC_DAADPCR_ADPEN)) {
        i32Ret = LL_ERR_BUSY;

        if (READ_REG16_BIT(DACx->DAADPCR, (DAC_DAADPCR_DA1SF << u16Ch)) == 0U) {
            i32Ret = LL_OK;
        }
    }

    return i32Ret;
}

/**
 * @brief  Fills each pstcDacInit member with its default value
 * @param  [in] pstcDacInit   pointer to a stc_dac_init_t structure which will
 *         be initialized.
 * @retval int32_t:
 *         - LL_OK: No errors occurred.
 *         - LL_ERR_INVD_PARAM: pstcDacInit is NULL
 */
int32_t DAC_StructInit(stc_dac_init_t *pstcDacInit)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if (pstcDacInit != NULL) {
        pstcDacInit->u16Align = DAC_DATA_ALIGN_RIGHT;
        pstcDacInit->enOutput = ENABLE;
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Initialize the DAC peripheral according to the specified parameters
 *         in the stc_dac_init_t
 * @param  [in] DACx       Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @param  [in] pstcDacInit   pointer to a stc_dac_init_t structure that contains
 *         the configuration information for the specified DAC channel.
 * @retval int32_t:
 *         - LL_OK: Initialize successfully
 *         - LL_ERR_INVD_PARAM: pstcDacInit is NULL
 */
int32_t DAC_Init(CM_DAC_TypeDef *DACx, uint16_t u16Ch, const stc_dac_init_t *pstcDacInit)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if (pstcDacInit != NULL) {
        DDL_ASSERT(IS_DAC_UNIT(DACx));
        DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));
        DDL_ASSERT(IS_FUNCTIONAL_STATE(pstcDacInit->enOutput));
        DDL_ASSERT(IS_DAC_DATA_ALIGN(pstcDacInit->u16Align));

        MODIFY_REG16(DACx->DACR, DAC_DACR_DPSEL, pstcDacInit->u16Align);
        DAC_OutputCmd(DACx, u16Ch, pstcDacInit->enOutput);
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Deinitialize the DAC peripheral registers to their default reset values.
 * @param  [in] DACx       Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @retval int32_t:
 * @retval None
 */
int32_t DAC_DeInit(CM_DAC_TypeDef *DACx)
{
    int32_t i32Ret = LL_OK;
    const uint32_t au32DACx[] = {CM_DAC1_BASE, CM_DAC2_BASE};
    __IO uint8_t u8TimeOut = 0U;
    uint32_t RMU_FRST3_DACx = 0UL;
    uint8_t i;
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT((CM_PWC->FPRC & PWC_FPRC_FPRCB1) == PWC_FPRC_FPRCB1);

    for (i = 0; i < ARRAY_SZ(au32DACx); i++) {
        if ((uint32_t)DACx == au32DACx[i]) {
            RMU_FRST3_DACx = (RMU_FRST3_DAC1 << i);
            break;
        }
    }
    if (i < ARRAY_SZ(au32DACx)) {
        /* Reset DAC */
        CLR_REG32_BIT(CM_RMU->FRST3, RMU_FRST3_DACx);
        /* Ensure reset procedure is completed */
        while (RMU_FRST3_DACx != READ_REG32_BIT(CM_RMU->FRST3, RMU_FRST3_DACx)) {
            u8TimeOut++;
            if (u8TimeOut > DAC_RMU_TIMEOUT) {
                i32Ret = LL_ERR_TIMEOUT;
                break;
            }
        }
    } else {
        i32Ret = LL_ERR_INVD_PARAM;
    }
    return i32Ret;
}

/**
 * @brief  Set DAC channel data transfer mode
 * @param  [in] DACx       Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DAC or CM_DACx
 * @param  [in] u16Ch      Specify the DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @param  [in] u16Mode    Specify the data transfer mode.
 *         This parameter can be a value of @ref DAC_CH_DATA_TRANS_MD
 *         - DAC_CH_DATA_TRANS_NORMAL:  Data transfer immediately
 *         - DAC_CH_DATA_TRANS_HRPWM:   Data transfer trigger by HRPWM event
 * @retval None
 */
void DAC_SetChDataTransMode(CM_DAC_TypeDef *DACx, uint16_t u16Ch, uint16_t u16Mode)
{
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));
    DDL_ASSERT(IS_DAC_CH_DATA_TRANS_MD(u16Mode));

    if (DAC_CH1 == u16Ch) {
        MODIFY_REG16(DACx->DACR2, DAC_DACR2_LDMD1, u16Mode);
    } else {
        MODIFY_REG16(DACx->DACR2, DAC_DACR2_LDMD2, u16Mode << 8U);
    }
}

/**
 * @brief  Get DAC active data
 * @param  [in] DACx   Pointer to the DAC peripheral register.
 *         This parameter can be a value of the following:
 *         @arg CM_DACx
 * @param  [in] u16Ch  Specify DAC channel @ref DAC_CH.
 *         This parameter can be a value of the following:
 *         @arg DAC_CH1
 *         @arg DAC_CH2 (only valid for CM_DAC1)
 * @retval DAC active data
 */
uint16_t DAC_GetActiveData(CM_DAC_TypeDef *DACx, uint16_t u16Ch)
{
    uint16_t u16Data;
    DDL_ASSERT(IS_DAC_UNIT(DACx));
    DDL_ASSERT(IS_DAC_CH(DACx, u16Ch));

    if (DAC_CH1 == u16Ch) {
        u16Data = READ_REG16(DACx->DADACTR1) & 0xFFFU;
    } else {
        u16Data = READ_REG16(DACx->DADACTR2) & 0xFFFU;
    }
    return u16Data;
}

/**
 * @}
 */

#endif /* LL_DAC_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
