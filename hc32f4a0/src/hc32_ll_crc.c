/**
 *******************************************************************************
 * @file  hc32_ll_crc.c
 * @brief This file provides firmware functions to manage the Cyclic Redundancy
 *        Check(CRC).
 @verbatim
   Change Logs:
   Date             Author          Notes
   2022-03-31       CDT             First version
   2022-06-30       CDT             Add waiting time after write CRC data
   2023-06-30       CDT             Reconstruct interface function relate to calculate CRC
                                    Optimize CRC_DeInit function
   2023-09-30       CDT             Delete and modify some of group/function relate to calculate CRC
                                    Modify typo
   2024-06-30       CDT             Add API CRC_GetResult() & CRC_SetInitValue()
                                    Optimized APIs CRC_WriteData8/16/32
   2024-11-08       CDT             Modify interface of AccumulateData and Calculate functions
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

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ll_crc.h"
#include "hc32_ll_utility.h"

/**
 * @addtogroup LL_Driver
 * @{
 */

/**
 * @defgroup LL_CRC CRC
 * @brief Cyclic Redundancy Check Driver Library
 * @{
 */

#if (LL_CRC_ENABLE == DDL_ON)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup CRC_Local_Macros CRC Local Macros
 * @{
 */

/**
 * @defgroup CRC_Check_Parameters_Validity CRC Check Parameters Validity
 * @{
 */
/*! Parameter validity check for CRC protocol. */
#define IS_CRC_PROTOCOL(x)                                                     \
(   ((x) == CRC_CRC16)                  ||                                     \
    ((x) == CRC_CRC32))

/*! Parameter validity check for CRC data width. */
#define IS_CRC_DATA_WIDTH(x)                                                   \
(   ((x) == CRC_DATA_WIDTH_8BIT)        ||                                     \
    ((x) == CRC_DATA_WIDTH_16BIT)       ||                                     \
    ((x) == CRC_DATA_WIDTH_32BIT))

/**
 * @}
 */

/**
 * @defgroup CRC_Registers_Reset_Value_definition CRC Registers Reset Value
 * @{
 */
#define CRC_CR_RST_VALUE                (0x0001UL)
/**
 * @}
 */

/**
 * @defgroup CRC_DATA_Register_Address CRC Data Register Address
 * @{
 */
#define CRC_DATA_ADDR                   ((uint32_t)(&CM_CRC->DAT0))
/**
 * @}
 */

/**
 * @defgroup CRC_Calculate_Clock_Count CRC Calculate Clock Count
 * @{
 */
#define CRC_CALC_CLK_COUNT              (10UL)
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
 * @defgroup CRC_Local_Functions CRC Local Functions
 * @{
 */

/**
 * @brief  Calculate the CRC value of a 8-bit data buffer.
 * @param  [in] au8Data                 Pointer to the input data buffer.
 * @param  [in] u32Len                  The length(counted in byte) of the data to be calculated.
 * @retval None
 */
static void CRC_WriteData8(const uint8_t au8Data[], uint32_t u32Len)
{
    uint32_t i;
    __IO uint8_t *reg8DR = (__IO uint8_t *)CRC_DATA_ADDR;
    for (i = 0UL; i < u32Len; i++) {
        *reg8DR = au8Data[i];
    }
}

/**
 * @brief  Calculate the CRC value of a 16-bit data buffer.
 * @param  [in] au16Data                Pointer to the input data buffer.
 * @param  [in] u32Len                  The length(counted in half-word) of the data to be calculated.
 * @retval None
 */
static void CRC_WriteData16(const uint16_t au16Data[], uint32_t u32Len)
{
    uint32_t i;
    __IO uint16_t *reg16DR = (__IO uint16_t *)CRC_DATA_ADDR;
    for (i = 0UL; i < u32Len; i++) {
        *reg16DR = au16Data[i];
    }
}

/**
 * @brief  Calculate the CRC value of a 32-bit data buffer.
 * @param  [in] au32Data                Pointer to the input data buffer.
 * @param  [in] u32Len                  The length(counted in word) of the data to be calculated.
 * @retval None
 */
static void CRC_WriteData32(const uint32_t au32Data[], uint32_t u32Len)
{
    uint32_t i;
    __IO uint32_t *reg32DR = (__IO uint32_t *)CRC_DATA_ADDR;
    for (i = 0UL; i < u32Len; i++) {
        *reg32DR = au32Data[i];
    }
}

/**
 * @}
 */

/**
 * @defgroup CRC_Global_Functions CRC Global Functions
 * @{
 */

/**
 * @brief  Set the fields of structure stc_crc_init_t to default values.
 * @param  [out] pstcCrcInit            Pointer to a @ref stc_crc_init_t structure.
 * @retval int32_t:
 *           - LL_OK:                   Initialize successfully.
 *           - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 */
int32_t CRC_StructInit(stc_crc_init_t *pstcCrcInit)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if (NULL != pstcCrcInit) {
        pstcCrcInit->u32Protocol = CRC_CRC16;
        pstcCrcInit->u32InitValue = CRC_INIT_VALUE_DEFAULT;
        i32Ret = LL_OK;
    }

    return i32Ret;
}

/**
 * @brief  Initialize the CRC.
 * @param  [in] pstcCrcInit             Pointer to a @ref stc_crc_init_t structure.
 * @retval int32_t:
 *           - LL_OK:                   Initialize successfully.
 *           - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 */
int32_t CRC_Init(const stc_crc_init_t *pstcCrcInit)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if (NULL != pstcCrcInit) {
        DDL_ASSERT(IS_CRC_PROTOCOL(pstcCrcInit->u32Protocol));

        MODIFY_REG32(CM_CRC->CR, CRC_CRC32, pstcCrcInit->u32Protocol);

        /* Set initial value */
        if (CRC_CRC16 == (pstcCrcInit->u32Protocol & CRC_CRC32)) {
            WRITE_REG16(CM_CRC->RESLT, (uint16_t)pstcCrcInit->u32InitValue);
        } else {
            WRITE_REG32(CM_CRC->RESLT, pstcCrcInit->u32InitValue);
        }
        i32Ret = LL_OK;
    }
    return i32Ret;
}

/**
 * @brief  De-initialize the CRC.
 * @param  None
 * @retval int32_t:
 *           - LL_OK:           Reset success.
 */
int32_t CRC_DeInit(void)
{
    int32_t i32Ret = LL_OK;
    WRITE_REG32(CM_CRC->CR, CRC_CR_RST_VALUE);

    return i32Ret;
}

/**
 * @brief  Get the CRC result
 * @param  None
 * @retval The CRC result.
 * @note   None.
 */
uint32_t CRC_GetResult(void)
{
    if (CRC_CRC32 == READ_REG32_BIT(CM_CRC->CR, CRC_CR_CR)) {
        return READ_REG32(CM_CRC->RESLT);
    } else {
        return READ_REG32_BIT(CM_CRC->RESLT, 0x0000FFFFUL);
    }
}

/**
 * @brief  Set the CRC initial value
 * @param  [in] u32Value        The CRC initial value.
 * @retval None
 * @note   None.
 */
void CRC_SetInitValue(uint32_t u32Value)
{
    if (CRC_CRC32 == READ_REG32_BIT(CM_CRC->CR, CRC_CR_CR)) {
        WRITE_REG32(CM_CRC->RESLT, u32Value);
    } else {
        WRITE_REG32(CM_CRC->RESLT, u32Value & 0x0000FFFFUL);
    }
}

/**
 * @brief  Get status of the CRC operation result.
 * @param  None
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t CRC_GetResultStatus(void)
{
    uint32_t u32Status;

    u32Status = READ_REG32_BIT(CM_CRC->CR, CRC_CR_FLAG);

    return (u32Status > 0UL) ? SET : RESET;
}

/**
 * @brief  Calculate the CRC16 value and start with the previously calculated CRC as initial value.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the macros group @ref CRC_DATA_Bit_Width
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be calculated.
 * @param  [in] u32Len                  The length(counted in bytes or half word or word, depending on
 *                                      the bit width) of the data to be calculated.
 * @param  [out] pu16Out                Pointer to the calculated CRC value.
 * @retval int32_t:
 *          - LL_OK:                   Initialize successfully.
 *          - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 * @note   The function fetch data in byte or half word or word depending on the data bit width(the parameter u8DataWidth).
 */
int32_t CRC_CRC16_AccumulateData(uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint16_t *pu16Out)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        DDL_ASSERT(IS_CRC_DATA_WIDTH(u8DataWidth));

        /* Write data */
        if (CRC_DATA_WIDTH_32BIT == u8DataWidth) {
            CRC_WriteData32((const uint32_t *)pvData, u32Len);
        } else if (CRC_DATA_WIDTH_16BIT == u8DataWidth) {
            CRC_WriteData16((const uint16_t *)pvData, u32Len);
        } else {
            CRC_WriteData8((const uint8_t *)pvData, u32Len);
        }

        if (pu16Out != NULL) {
            /* Get checksum */
            *pu16Out = (uint16_t)READ_REG16(CM_CRC->RESLT);
            i32Ret = LL_OK;
        }
    }

    return i32Ret;
}

/**
 * @brief  Calculate the CRC32 value and start with the previously calculated CRC as initial value.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the macros group @ref CRC_DATA_Bit_Width
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be calculated.
 * @param  [in] u32Len                  The length(counted in bytes or half word or word, depending on
 *                                      the bit width) of the data to be calculated.
 * @param  [out] pu32Out                Pointer to the calculated CRC value.
 * @retval int32_t:
 *          - LL_OK:                   Initialize successfully.
 *          - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 * @note   The function fetch data in byte or half word or word depending on the data bit width(the parameter u8DataWidth).
 */
int32_t CRC_CRC32_AccumulateData(uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint32_t *pu32Out)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        DDL_ASSERT(IS_CRC_DATA_WIDTH(u8DataWidth));

        /* Write data */
        if (CRC_DATA_WIDTH_32BIT == u8DataWidth) {
            CRC_WriteData32((const uint32_t *)pvData, u32Len);
        } else if (CRC_DATA_WIDTH_16BIT == u8DataWidth) {
            CRC_WriteData16((const uint16_t *)pvData, u32Len);
        } else {
            CRC_WriteData8((const uint8_t *)pvData, u32Len);
        }

        if (pu32Out != NULL) {
            /* Get checksum */
            *pu32Out = READ_REG32(CM_CRC->RESLT);
            i32Ret = LL_OK;
        }
    }

    return i32Ret;
}

/**
 * @brief  Calculate the CRC16 value and start with the specified initial value.
 * @param  [in] u16InitValue            The CRC initialization value which is the valid bits same as
 *                                      the bits of CRC Protocol.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the macros group @ref CRC_DATA_Bit_Width
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be computed.
 * @param  [in] u32Len                  The length(counted in bytes or half word or word, depending on
 *                                      the bit width) of the data to be computed.
 * @param  [out] pu16Out                Pointer to the calculated CRC value.
 * @retval int32_t:
 *          - LL_OK:                   Initialize successfully.
 *          - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 * @note   The function fetch data in byte or half word or word depending on the data bit width(the parameter u8DataWidth).
 */
int32_t CRC_CRC16_Calculate(uint16_t u16InitValue, uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint16_t *pu16Out)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        /* Set initial value */
        WRITE_REG16(CM_CRC->RESLT, u16InitValue);
        i32Ret = CRC_CRC16_AccumulateData(u8DataWidth, pvData, u32Len, pu16Out);
    }

    return i32Ret;
}

/**
 * @brief  Calculate the CRC32 value and start with the specified initial value.
 * @param  [in] u32InitValue            The CRC initialization value which is the valid bits same as
 *                                      the bits of CRC Protocol.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the macros group @ref CRC_DATA_Bit_Width
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be computed.
 * @param  [in] u32Len                  The length(counted in bytes or half word or word, depending on
 *                                      the bit width) of the data to be computed.
 * @param  [out] pu32Out                Pointer to the calculated CRC value.
 * @retval int32_t:
 *          - LL_OK:                   Initialize successfully.
 *          - LL_ERR_INVD_PARAM:       The pointer pstcCrcInit value is NULL.
 * @note   The function fetch data in byte or half word or word depending on the data bit width(the parameter u8DataWidth).
 */
int32_t CRC_CRC32_Calculate(uint32_t u32InitValue, uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint32_t *pu32Out)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        /* Set initial value */
        WRITE_REG32(CM_CRC->RESLT, u32InitValue);
        i32Ret = CRC_CRC32_AccumulateData(u8DataWidth, pvData, u32Len, pu32Out);
    }

    return i32Ret;
}

/**
 * @brief  Check the CRC16 calculating result with the expected value.
 * @param  [in] u16InitValue            The CRC initialization value which is the valid bits same as
 *                                      the bits of CRC Protocol.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the following values:
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be computed.
 * @param  [in] u32Len                  The length(counted in byte) of the data to be calculated.
 * @param  [in] u16ExpectValue          The expected CRC value to be checked.
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t CRC_CRC16_CheckData(uint16_t u16InitValue, uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint16_t u16ExpectValue)
{
    __IO uint32_t u32Count = CRC_CALC_CLK_COUNT;
    en_flag_status_t enStatus = RESET;
    uint32_t u32ExpectValueTemp = u16ExpectValue;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        (void)CRC_CRC16_Calculate(u16InitValue, u8DataWidth, pvData, u32Len, NULL);

        /* Writes the expected CRC value to be checked */
        CRC_WriteData16((uint16_t *)((void *)&u32ExpectValueTemp), 1UL);

        /* Delay for waiting CRC result flag */
        while (u32Count-- != 0UL) {
            __NOP();
        }

        enStatus = CRC_GetResultStatus();
    }

    return enStatus;
}

/**
 * @brief  Check the CRC32 calculating result with the expected value.
 * @param  [in] u32InitValue            The CRC initialization value which is the valid bits same as
 *                                      the bits of CRC Protocol.
 * @param  [in] u8DataWidth             Bit width of the data.
 *         This parameter can be one of the following values:
 *           @arg CRC_DATA_WIDTH_8BIT:  8  Bit
 *           @arg CRC_DATA_WIDTH_16BIT: 16 Bit
 *           @arg CRC_DATA_WIDTH_32BIT: 32 Bit
 * @param  [in] pvData                  Pointer to the buffer containing the data to be computed.
 * @param  [in] u32Len                  The length(counted in byte) of the data to be calculated.
 * @param  [in] u32ExpectValue          The expected CRC value to be checked.
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t CRC_CRC32_CheckData(uint32_t u32InitValue, uint8_t u8DataWidth, const void *pvData, uint32_t u32Len, uint32_t u32ExpectValue)
{
    __IO uint32_t u32Count = CRC_CALC_CLK_COUNT;
    en_flag_status_t enStatus = RESET;
    uint32_t u32ExpectValueTemp = u32ExpectValue;

    if ((pvData != NULL) && (u32Len != 0UL)) {
        (void)CRC_CRC32_Calculate(u32InitValue, u8DataWidth, pvData, u32Len, NULL);

        /* Writes the expected CRC value to be checked */
        CRC_WriteData32(&u32ExpectValueTemp, 1UL);

        /* Delay for waiting CRC result flag */
        while (u32Count-- != 0UL) {
            __NOP();
        }

        enStatus = CRC_GetResultStatus();
    }

    return enStatus;
}

/**
 * @brief  Get the CRC16 check result with the expected value.
 * @param  [in] u16ExpectValue          The expected CRC value to be checked.
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t CRC_CRC16_GetCheckResult(uint16_t u16ExpectValue)
{
    __IO uint32_t u32Count = CRC_CALC_CLK_COUNT;
    en_flag_status_t enStatus;
    uint32_t u32ExpectValueTemp = u16ExpectValue;

    /* Writes the expected CRC value to be checked */
    CRC_WriteData16((uint16_t *)((void *)&u32ExpectValueTemp), 1UL);

    /* Delay for waiting CRC result flag */
    while (u32Count-- != 0UL) {
        __NOP();
    }

    enStatus = CRC_GetResultStatus();

    return enStatus;
}

/**
 * @brief  Get the CRC32 check result with the expected value.
 * @param  [in] u32ExpectValue          The expected CRC value to be checked.
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t CRC_CRC32_GetCheckResult(uint32_t u32ExpectValue)
{
    __IO uint32_t u32Count = CRC_CALC_CLK_COUNT;
    en_flag_status_t enStatus;
    uint32_t u32ExpectValueTemp = u32ExpectValue;

    /* Writes the expected CRC value to be checked */
    CRC_WriteData32(&u32ExpectValueTemp, 1UL);

    /* Delay for waiting CRC result flag */
    while (u32Count-- != 0UL) {
        __NOP();
    }

    enStatus = CRC_GetResultStatus();

    return enStatus;
}

/**
 * @}
 */

#endif /* LL_CRC_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
