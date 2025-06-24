/**
 *******************************************************************************
 * @file  hc32_ll_efm.c
 * @brief This file provides firmware functions to manage the Embedded Flash
 *        Memory unit (EFM).
 @verbatim
   Change Logs:
   Date             Author          Notes
   2024-09-13       CDT             First version
   2024-10-17       CDT             Add const before buffer pointer to cater top-level calls
                                    Bug Fixed # judge the EFM_FLAG_OPTEND whether set o not before clear EFM_FLAG_OPTEND
   2024-11-08       CDT             Remap the sector number parameter of EFM_SingleSectorOperateCmd based on SWAP and OTP status
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
#include "hc32_ll_efm.h"
#include "hc32_ll_utility.h"

/**
 * @addtogroup LL_Driver
 * @{
 */

/**
 * @defgroup LL_EFM EFM
 * @brief Embedded Flash Management Driver Library
 * @{
 */

#if (LL_EFM_ENABLE == DDL_ON)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup EFM_Local_Types EFM Local Types
 * @{
 */
typedef struct {
    uint32_t *pu32Src;
    uint32_t *pu32Dest;
    uint32_t u32UnitTotal;
    uint32_t u32ProgramMode;
    en_flag_status_t enSwap;
    en_flag_status_t enOtp;
} stc_program_param_t;

/**
 * @}
 */
/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup EFM_Local_Macros EFM Local Macros
 * @{
 */
#ifndef __EFM_FUNC
#define __EFM_FUNC                      __RAM_FUNC
#endif

/**
 * @defgroup EFM_Timeout_Definition EFM timeout definition
 * @{
 */
#define EFM_TIMEOUT                     (HCLK_VALUE / 20000UL)  /* EFM wait read timeout */
#define EFM_PGM_TIMEOUT                 (HCLK_VALUE / 5000UL)  /* EFM Program timeout max 53us * 4 */
#define EFM_ERASE_TIMEOUT               (HCLK_VALUE / 50UL)     /* EFM Erase timeout max 20ms */
#define EFM_SEQ_PGM_TIMEOUT             (HCLK_VALUE / 15625UL)  /* EFM Sequence Program timeout max 16us * 4 */
/**
 * @}
 */

#define REMCR_REG(x)                    (*(__IO uint32_t *)((uint32_t)(&CM_EFM->MMF_REMCR0) + (4UL * (x))))

#define EFM_FLAG0_POS                   (0U)
#define EFM_FLAG1_POS                   (16U)
#define EFM_OTP_END_SECTOR_NUM          (15U)
#define EFM_FLAG_OFFSET(enSwapState, enOtpStat, Addr)                       \
(                                                                           \
    (((Addr) < EFM_END_ADDR) && ((((enSwapState) == RESET)          &&      \
    ((Addr) >= EFM_FLASH_1_START_ADDR)) || (((enSwapState) == SET)  &&      \
    ((enOtpStat) == RESET) && ((Addr) < EFM_FLASH_1_START_ADDR))    ||      \
    (((enSwapState) == SET) && ((enOtpStat) == SET)                 &&  \
    ((Addr) > EFM_OTP_END_ADDR1)                                    &&      \
    ((Addr) <= EFM_SWAP_FLASH1_END_ADDR)))) ? EFM_FLAG1_POS : EFM_FLAG0_POS)

#define EFM_FLASH1_START_SECTOR_NUM     (128U)
#define FNWPRT_REG                      (CM_EFM->F0NWPRT0)

#define REG_LEN                         (32U)
#define EFM_SWAP_FLASH1_END_SECTOR_NUM  (EFM_FLASH1_START_SECTOR_NUM + EFM_OTP_END_SECTOR_NUM)
#define EFM_SWAP_FLASH1_END_ADDR        (EFM_FLASH_1_START_ADDR + EFM_OTP_END_ADDR1)
#define EFM_OTP_UNLOCK_KEY1             (0x10325476UL)
#define EFM_OTP_UNLOCK_KEY2             (0xEFCDAB89UL)

/**
 * @defgroup EFM_SectorOperate EFM sector operate define
 * @{
 */
#define EFM_FNWPRT_REG_OFFSET           (2UL)
#define EFM_FNWPRT_REG_NUM              (8UL)
#define EFM_FNWPRT_REG_END_ADDR         (__IO uint32_t *)(uint32_t)(&CM_EFM->F1NWPRT3)
/**
 * @}
 */

/**
 * @defgroup EFM_protect EFM protect define
 * @{
 */
#define EFM_SECURITY_LEN                (12UL)
#define EFM_PROTECT1_KEY                (0xAF180402UL)
#define EFM_PROTECT2_KEY                (0xA85173AEUL)

#define EFM_PROTECT3_KEY                (0x42545048UL)
#define EFM_PROTECT1_ADDR               (0x00000430UL)
#define EFM_PROTECT2_ADDR               (0x00000434UL)
#define EFM_PROTECT3_ADDR1              (0x00000420UL)
#define EFM_PROTECT3_ADDR2              (0x00000424UL)
#define EFM_PROTECT3_ADDR3              (0x00000428UL)
#define EFM_SECURITY_ADDR               (0x03004000UL)

#define EFM_SWAP_ON_PROTECT_SECTOR_NUM  EFM_FLASH1_START_SECTOR_NUM
/**
 * @}
 */

/**
 * @defgroup EFM_ECC_REG EFM ECC register define
 * @{
 */
#define EFM_ECC_CKCR_PROTECT_CODE                           (0x3BU << EFM_CKPR_CKPRKW_POS)
#define EFM_ECC_BIT_POS_DELTA_EIEN                          (EFM_EIEN_F1_EIEN_POS - EFM_EIEN_F0_EIEN_POS)
#define EFM_ECC_ERR_INJECT_REG_IDX_ECC_VALUE                (4UL)
#define EFM_ECC_ERR_INJECT_REG_IDX_ADDR                     (4UL)
#define EFM_ECC_ERR_RECORD_COUNT                            (2UL)
#define EFM_ECC_ERR_RECORD_MASK_RESCUE_SECTOR               (0x00200000UL)
#define EFM_ECC_ERR_RECORD_MASK_SPECIAL_FUNC_SECTOR         (0x00400000UL)
#define EFM_ECC_ERR_RECORD_MASK_ADDRESS_OFFSET              (0x000FFFF0UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Check_Parameters_Validity EFM Check Parameters Validity
 * @{
 */
/* Parameter validity check for efm chip . */
#define IS_EFM_CHIP(x)                                                          \
(   ((x) == EFM_CHIP0)                  ||                                      \
    ((x) == EFM_CHIP1)                  ||                                      \
    ((x) == EFM_CHIP_ALL))

/* Parameter validity check for flash latency. */
#define IS_EFM_WAIT_CYCLE(x)            ((x) <= EFM_WAIT_CYCLE15)

/* Parameter validity check for operate mode. */
#define IS_EFM_OP_MD(x)                                                         \
(   ((x) == EFM_MD_PGM_SINGLE)          ||                                      \
    ((x) == EFM_MD_PGM_READBACK)        ||                                      \
    ((x) == EFM_MD_PGM_SEQ)             ||                                      \
    ((x) == EFM_MD_ERASE_SECTOR)        ||                                      \
    ((x) == EFM_MD_ERASE_ONE_CHIP)      ||                                      \
    ((x) == EFM_MD_ERASE_ALL_CHIP)      ||                                      \
    ((x) == EFM_MD_READONLY))

/* Parameter validity check for efm status. */
#define IS_EFM_STATUS(x)                                                        \
(   ((x) == EFM_FLASH0_ACT_FLASH1_ACT)  ||                                      \
    ((x) == EFM_FLASH0_STP_FLASH1_ACT)  ||                                      \
    ((x) == EFM_FLASH0_ACT_FLASH1_STP)  ||                                      \
    ((x) == EFM_FLASH0_STP_FLASH1_STP))

/* Parameter validity check for PGM. */
#define IS_ALIGNED_PGM_ADDR(addr)        ((addr) % EFM_PGM_UNIT_BYTES == 0U)

/* Parameter validity check for read accelerator cmd. */
#define IS_RD_ACCL_CMD(type)                                              \
(   ((type) >= EFM_RD_ACCL_CMD_ICACHE)  &&                                      \
    ((type) <= EFM_RD_ACCL_CMD_ALL))

/* Parameter validity check for flash interrupt select. */
#define IS_EFM_INT_SEL(x)               (((x) | EFM_INT_ALL) == EFM_INT_ALL)

/* Parameter validity check for flash flag. */
#define IS_EFM_FLAG(x)                  (((x) | EFM_FLAG_ALL) == EFM_FLAG_ALL)

/* Parameter validity check for flash clear flag. */
#define IS_EFM_CLRFLAG(x)               (((x) | EFM_FLAG_ALL) == EFM_FLAG_ALL)

/* Parameter validity check for bus status while flash program or erase. */
#define IS_EFM_BUS_STATUS(x)                                                    \
(   ((x) == EFM_BUS_HOLD)               ||                                      \
    ((x) == EFM_BUS_RELEASE))

/* Parameter validity check for efm address. */
#define IS_EFM_ADDR(x)                                                          \
(   ((x) <= EFM_END_ADDR)               ||                                      \
    (((x) >= EFM_OTP_START_ADDR) && ((x) <= EFM_OTP_END_ADDR)) ||               \
    (((x) >= EFM_SECURITY_START_ADDR) && ((x) <= EFM_SECURITY_END_ADDR)))

/* Parameter validity check for efm erase address. */
#define IS_EFM_ERASE_ADDR(x)                                                    \
(   ((x) <= EFM_END_ADDR)               ||                                      \
    (((x) >= EFM_OTP_START_ADDR) && ((x) <= EFM_OTP_END_ADDR)) ||               \
    (((x) >= EFM_SECURITY_START_ADDR) && ((x) <= EFM_SECURITY_END_ADDR)))

/* Parameter validity check for efm erase mode . */
#define IS_EFM_ERASE_MD(x)                                                      \
(   ((x) == EFM_MD_ERASE_ONE_CHIP)      ||                                      \
    ((x) == EFM_MD_ERASE_FULL))

/* Parameter validity check for EFM lock status. */
#define IS_EFM_REG_UNLOCK()             (CM_EFM->FAPRT == 0x00000001UL)

/* Parameter validity check for EFM_FWMC register lock status. */
#define IS_EFM_FWMC_UNLOCK()            (bCM_EFM->FWMC_b.KEY1LOCK == 0U)

/* Parameter validity check for OTP lock status. */
#define IS_EFM_OTP_UNLOCK()             (bCM_EFM->FWMC_b.KEY2LOCK == 0U)

/* Parameter validity check for sector protected register locking. */
#define IS_EFM_SECTOR_PROTECT_REG_LOCK(x)           ((x) <= 0xFFU)

/* Parameter validity check for EFM sector number */
#define IS_EFM_SECTOR_NUM(x)            ((x) >= 1U && (x) <= EFM_SECTOR_COUNT_ALL_CHIPS)
#define IS_EFM_SECTOR_IDX(x)            ((x) < EFM_SECTOR_COUNT_ALL_CHIPS)

/* Parameter validity check for EFM remap lock status. */
#define IS_EFM_REMAP_UNLOCK()           (CM_EFM->MMF_REMPRT == 0x00000001UL)

/* Parameter validity check for EFM remap index */
#define IS_EFM_REMAP_IDX(x)                                                     \
(   ((x) == EFM_REMAP_IDX0)             ||                                      \
    ((x) == EFM_REMAP_IDX1))

/* Parameter validity check for EFM remap size */
#define IS_EFM_REMAP_SIZE(x)                                                    \
(   ((x) >= EFM_REMAP_4K)               &&                                      \
    ((x) <= EFM_REMAP_SIZE_MAX))

/* Parameter validity check for EFM remap address */
#define IS_EFM_REMAP_ADDR(x)                                                    \
(   ((x) <= EFM_REMAP_ROM_END_ADDR)     ||                                      \
    (((x) >= EFM_REMAP_RAM_START_ADDR)  &&                                      \
    ((x) <= EFM_REMAP_RAM_END_ADDR)))

/* Parameter validity check for EFM remap state */
#define IS_EFM_REMAP_STATE(x)                                                   \
(   ((x) == EFM_REMAP_OFF)             ||                                       \
    ((x) == EFM_REMAP_ON))

/* Parameter validity check for EFM security code length */
#define IS_EFM_SECURITY_CODE_LEN(x)    ((x) <= EFM_SECURITY_LEN)

/* Parameter validity check for EFM ECC */
#define IS_EFM_ECC_MD(x)                    ((x) <= EFM_ECC_MD3)
#define IS_EFM_ECC_EXP(x)                   ((x) <= EFM_ECC_EXP_TYPE_RESET)
#define IS_EFM_ECC_ERR_FLAG(x)              (((x) | EFM_CHECK_FLAG_ECC_ALL) == EFM_CHECK_FLAG_ECC_ALL)
#define IS_EFM_ECC_ERR_REC(idx)                                                 \
(   ((idx) >= EFM_ECC_ERR_REC_ID0)     &&                                       \
    ((idx) <= EFM_ECC_ERR_REC_ALL))

#define IS_EFM_ECC_ERR_INJECT_BIT_ECC_DATA(x)   ((x) <= EFM_ECC_BIT_MASK_9BIT_ECC_DATA)
#define IS_EFM_ECC_ERR_INJECT_BIT_ADDR(x)       (((x) | EFM_ECC_BIT_MASK_20BIT_ADDR) == EFM_ECC_BIT_MASK_20BIT_ADDR)

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
/**
 * @addtogroup EFM_Local_Functions EFM Local Functions
 * @{
 */
static int32_t SectorErase_Implement(uint32_t u32Addr);

__EFM_FUNC static int32_t Program_Implement(uint32_t u32Addr, const uint8_t *pu8Buf, uint32_t u32Len, uint32_t u32NeedReadBack);
__EFM_FUNC static int32_t Program_CheckCond(uint32_t u32Addr, uint32_t u32ByteLen);
__EFM_FUNC static int32_t Program_Write(stc_program_param_t *pstcParam);
__EFM_FUNC static void Program_PadLastUnit(uint8_t *pPaddingBuf, uint8_t *pu8RemainByteStart, uint32_t u32RemainByteLen);

__EFM_FUNC static int32_t WaitFlashReady(uint32_t u32Addr, uint32_t u32Len, en_flag_status_t enSwap, en_flag_status_t enOtp);
__EFM_FUNC static uint8_t GetFlagOffset(en_flag_status_t enSwap, en_flag_status_t enOtp, uint32_t u32Addr);
__EFM_FUNC static int32_t WaitAndClearOPTEndFlag(uint8_t u8FlagOffset);
__EFM_FUNC static int32_t WaitStatus(uint32_t u32Flag, en_flag_status_t enStatus);
__EFM_FUNC static en_flag_status_t GetSwapStatus(void);
__EFM_FUNC static en_flag_status_t GetOTPStatus(void);

static void ECC_ClearSoftwareErrorRecord(void);
/**
 * @}
 */
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
/**
 * @defgroup EFM_Local_Variables EFM Local Variables
 * @{
 */
static stc_efm_ecc_err_record_t m_astcEccErrRecord[4UL] = {0x00000000UL};

/**
 * @}
 */
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

/**
 * @defgroup EFM_Global_Functions EFM Global Functions
 * @{
 */

/**
 * @brief  Enable or disable EFM.
 * @param  [in] u32Flash        Specifies the FLASH. @ref EFM_Chip_Sel
 * @param  [in] enNewState      An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void EFM_Cmd(uint32_t u32Flash, en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_CHIP(u32Flash));

    if (ENABLE == enNewState) {
        CLR_REG32_BIT(CM_EFM->FSTP, u32Flash);
    } else {
        SET_REG32_BIT(CM_EFM->FSTP, u32Flash);
    }
}

/**
 * @brief  Set the efm read wait cycles.
 * @param  [in] u32WaitCycle            Specifies the efm read wait cycles.
 *    @arg  This parameter can be of a value of @ref EFM_Wait_Cycle
 * @retval int32_t:
 *         - LL_OK: Program successfully.
 *         - LL_ERR_TIMEOUT: EFM is not ready.
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
int32_t EFM_SetWaitCycle(uint32_t u32WaitCycle)
{
    uint32_t u32Timeout = 0UL;

    /* Param valid check */
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_WAIT_CYCLE(u32WaitCycle));

    MODIFY_REG32(CM_EFM->FRMC, EFM_FRMC_FLWT, u32WaitCycle);
    while (u32WaitCycle != READ_REG32_BIT(CM_EFM->FRMC, EFM_FRMC_FLWT)) {
        u32Timeout++;
        if (u32Timeout > EFM_TIMEOUT) {
            return LL_ERR_TIMEOUT;
        }
    }
    return LL_OK;
}

/**
 * @brief  Reset cache RAM or cache ram release reset.
 * @param  [in] enNewState           An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void EFM_CacheRamReset(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(bCM_EFM->FRMC_b.CRST, enNewState);
}

/**
 * @brief  Enable or disable the flash prefetch.
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_PrefetchCmd(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(bCM_EFM->FRMC_b.PREFETE, enNewState);
}

/**
 * @brief  Enable or disable the flash data cache.
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_DCacheCmd(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(bCM_EFM->FRMC_b.DCACHE, enNewState);
}

/**
 * @brief  Enable or disable the flash instruction cache.
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_ICacheCmd(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(bCM_EFM->FRMC_b.ICACHE, enNewState);
}

/**
 * @brief  Read accelerator command
 * @param  [in] u32CmdType                Read accelerator command, one or any combination
 *                                        of @ref EFM_Read_Accelerator_Command
 *   @arg  EFM_RD_ACCL_CMD_PREFETCH
 *   @arg  EFM_RD_ACCL_CMD_DCACHE
 *   @arg  EFM_RD_ACCL_CMD_ICACHE
 *   @arg  EFM_RD_ACCL_CMD_ACTIVATION
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note  Call EFM_REG_Unlock() to unlock EFM register first.
 */
void EFM_ReadAcceleratorCmd(uint32_t u32CmdType, en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_RD_ACCL_CMD(u32CmdType));

    if ((u32CmdType & EFM_RD_ACCL_CMD_ACTIVATION) != 0UL) {
        if (enNewState == ENABLE) {
            CLR_REG32_BIT(CM_EFM->FRMC, u32CmdType);
        } else {
            SET_REG32_BIT(CM_EFM->FRMC, u32CmdType);
        }
        u32CmdType &= ~EFM_RD_ACCL_CMD_ACTIVATION;
    }

    if (enNewState == ENABLE) {
        SET_REG32_BIT(CM_EFM->FRMC, u32CmdType);
    } else {
        CLR_REG32_BIT(CM_EFM->FRMC, u32CmdType);
    }
}

/**
 * @brief  Enable or disable the Read of low-voltage mode.
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_LowVoltageReadCmd(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(bCM_EFM->FRMC_b.LVM, enNewState);
}

/**
 * @brief  Enable or disable the EFM swap function.
 * @param  [in] enNewState              An @ref en_functional_state_t enumeration value.
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
int32_t EFM_SwapCmd(en_functional_state_t enNewState)
{
    int32_t i32Ret;
    uint32_t u32CmdCode[4] = {EFM_SWAP_DATA, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());

    i32Ret = SectorErase_Implement(EFM_SWAP_ADDR);
    if (i32Ret != LL_OK) {
        return LL_ERR;
    }
    if (enNewState == DISABLE) {
        return i32Ret;
    }

    i32Ret = Program_Implement(EFM_SWAP_ADDR,
                               (uint8_t *)&u32CmdCode,
                               EFM_PGM_UNIT_BYTES,
                               EFM_MD_PGM_SINGLE);
    return i32Ret;
}

/**
 * @brief  Checks whether the swap function enable or disable.
 * @param  None
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t EFM_GetSwapStatus(void)
{
    if (RESET == EFM_GetOTPStatus()) {
        return (1UL == READ_REG32(bCM_EFM->FSWP_b.FSWP)) ? SET : RESET;
    } else {
        return ((*(uint32_t *)EFM_SWAP_ADDR) == EFM_SWAP_DATA) ? SET : RESET;
    }
}

/**
 * @brief  Set the FLASH erase program mode .
 * @param  [in] u32Mode                   Specifies the FLASH erase program mode.
 *    @arg  This parameter can be of a value of @ref EFM_OperateMode_Sel
 * @retval int32_t:
 *         - LL_OK: Set mode successfully.
 *         - LL_ERR_NOT_RDY: EFM is not ready.
 */
int32_t EFM_SetOperateMode(uint32_t u32Mode)
{
    int32_t i32Ret = LL_OK;
    DDL_ASSERT(IS_EFM_OP_MD(u32Mode));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());
    if (LL_ERR == WaitStatus(EFM_FLAG_FLASH_RDY, SET)) {
        i32Ret = LL_ERR_NOT_RDY;
    }

    if (i32Ret == LL_OK) {
        /* Set the program or erase mode. */
        MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, u32Mode);
    }
    return i32Ret;
}

/**
 * @brief  Enable or Disable EFM interrupt.
 * @param  [in] u32EfmInt               Specifies the FLASH interrupt source and status. @ref EFM_Interrupt_Sel
 *   @arg  EFM_INT_OPTEND:              End of EFM Operation Interrupt source
 *   @arg  EFM_INT_PEERR:               Program/erase error Interrupt source
 *   @arg  EFM_INT_COLERR:              Read collide error Interrupt source
 * @param  [in] enNewState              An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_IntCmd(uint32_t u32EfmInt, en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_INT_SEL(u32EfmInt));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (ENABLE == enNewState) {
        SET_REG32_BIT(CM_EFM->FITE, u32EfmInt);
    } else {
        CLR_REG32_BIT(CM_EFM->FITE, u32EfmInt);
    }
}

/**
 * @brief  Check any of the specified flag is set or not.
 * @param  [in] u32Flag                    Specifies the FLASH flag to check.
 *   @arg  This parameter can be of a value of @ref EFM_Flag_Sel
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t EFM_GetAnyStatus(uint32_t u32Flag)
{
    DDL_ASSERT(IS_EFM_FLAG(u32Flag));

    return ((0UL == READ_REG32_BIT(CM_EFM->FSR, u32Flag)) ? RESET : SET);
}

/**
 * @brief  Check all the specified flag is set or not.
 * @param  [in] u32Flag                    Specifies the FLASH flag to check.
 *   @arg  This parameter can be of a value of @ref EFM_Flag_Sel
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t EFM_GetStatus(uint32_t u32Flag)
{
    DDL_ASSERT(IS_EFM_FLAG(u32Flag));

    return ((u32Flag == READ_REG32_BIT(CM_EFM->FSR, u32Flag)) ? SET : RESET);
}

/**
 * @brief  Clear the flash flag.
 * @param  [in] u32Flag                  Specifies the FLASH flag to clear.
 *   @arg  This parameter can be of a value of @ref EFM_Flag_Sel
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_ClearStatus(uint32_t u32Flag)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_CLRFLAG(u32Flag));

    WRITE_REG32(CM_EFM->FSCLR, u32Flag);
}

/**
 * @brief  Set bus status while flash program or erase.
 * @param  [in] u32Status                  Specifies the new bus status while flash program or erase.
 *  This parameter can be one of the following values:
 *   @arg  EFM_BUS_HOLD:                   Bus busy while flash program or erase.
 *   @arg  EFM_BUS_RELEASE:                Bus release while flash program or erase.
 * @retval None
 */
void EFM_SetBusStatus(uint32_t u32Status)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_BUS_STATUS(u32Status));
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());

    WRITE_REG32(bCM_EFM->FWMC_b.BUSHLDCTL, u32Status);
}

/**
 * @brief  EFM read byte.
 * @param  [in] u32Addr               The specified address to read.
 * @param  [in] pu8ReadBuf            The specified read buffer.
 * @param  [in] u32ByteLen            The specified length to read.
 * @retval int32_t:
 *         - LL_OK: Read successfully
 *         - LL_ERR_INVD_PARAM: Invalid parameter
 *         - LL_ERR_NOT_RDY: EFM is not ready.
 */
int32_t EFM_ReadByte(uint32_t u32Addr, uint8_t *pu8ReadBuf, uint32_t u32ByteLen)
{
    int32_t i32Ret = LL_ERR_INVD_PARAM;
    __IO uint8_t *pu8Buf = (uint8_t *)u32Addr;
    uint32_t u32Len = u32ByteLen;

    DDL_ASSERT(IS_EFM_ADDR(u32Addr));
    DDL_ASSERT(IS_EFM_ADDR(u32Addr + u32ByteLen - 1UL));

    if (NULL != pu8ReadBuf) {
        i32Ret = WaitFlashReady(u32Addr, u32ByteLen, GetSwapStatus(), GetOTPStatus());
        if (i32Ret != LL_OK) {
            return i32Ret;
        }
        while (0UL != u32Len) {
            *(pu8ReadBuf++) = *(pu8Buf++);
            u32Len--;
        }

    }

    return i32Ret;
}

/**
 * @brief  EFM Blank Read
 * @param  [in]  u32Chip     The chip to be blank read @ref EFM_Chip_Sel
 * @retval  Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     OK,chip is(are) blank
 *         - LL_ERR_NOT_RDY:            Error, EFM register is not unlock or chip is(are) not ready
 *         - LL_ERR_INVD_PARAM:         Error, Parameter invalid
 *         - LL_ERR:                    Error, chip is(are) not blank
 * @note   1)__EFM_FUNC  is used to ensure this program is executed on a memory location distinct
 *         from the efm chip being blank read, or alternatively, on a different memory type such as RAM.
 *         __EFM_FUNC default value is __RAM_FUNC.
 *         __EFM_FUNC also could be attributed to FLASH0 or FLASH1 which is determined by users.
 *         If want to attribute to other place, please define in hc32f4xx_conf.h.
 *         2)Call EFM_REG_Unlock() to unlock EFM register first.
 */
__NOINLINE __EFM_FUNC int32_t EFM_BlankRead(uint32_t u32Chip)
{
    uint32_t u32Ready, u32ReadMode, u32Blank;
    uint32_t u32ReadAddr0 = EFM_START_ADDR, u32ReadAddr1 = EFM_FLASH_1_START_ADDR;
    int32_t  i32Ret;
    uint32_t u32ReadAcclCmd = READ_REG32_BIT(CM_EFM->FRMC, EFM_RD_ACCL_CMD_ALL);

    CLR_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    switch (u32Chip) {
        case EFM_CHIP0:
            u32Ready = EFM_FLAG_RDY;
            u32ReadMode = EFM_FRMC2_BRM0;
            u32Blank = EFM_FSR_BRMER0;
            break;
        case EFM_CHIP1:
            u32Ready = EFM_FLAG_RDY1;
            u32ReadMode = EFM_FRMC2_BRM1;
            u32Blank = EFM_FSR_BRMER1;
            break;
        default:
            u32Ready = EFM_FLAG_RDY | EFM_FLAG_RDY1;
            u32ReadMode = EFM_FRMC2_BRM0 | EFM_FRMC2_BRM1;
            u32Blank = EFM_FSR_BRMER0 | EFM_FSR_BRMER1;
            break;
    }

    /* Is okay to do bank read? */
    if ((u32Ready != (CM_EFM->FSR & u32Ready))) {
        return LL_ERR_NOT_RDY;
    }
    if (IS_EFM_REG_UNLOCK() == 0UL) {
        return LL_ERR_NOT_RDY;
    }
    if (!IS_EFM_CHIP(u32Chip)) {
        return LL_ERR_INVD_PARAM;
    }
    /* update check address if needed */
    if (GetSwapStatus() == SET) {
        if (GetOTPStatus() == RESET) {
            u32ReadAddr1 = EFM_START_ADDR;
            u32ReadAddr0 = EFM_FLASH_1_START_ADDR;
        }
    }

    /* Do bank read */
    SET_REG32_BIT(CM_EFM->FSCLR, u32Blank);
    SET_REG32_BIT(CM_EFM->FRMC2, u32ReadMode);
    switch (u32Chip) {
        case EFM_CHIP0:
            RW_MEM32(u32ReadAddr0);
            break;
        case EFM_CHIP1:
            RW_MEM32(u32ReadAddr1);
            break;
        default:
            RW_MEM32(u32ReadAddr0);
            RW_MEM32(u32ReadAddr1);
            break;
    }
    i32Ret = (READ_REG32_BIT(CM_EFM->FSR, u32Blank) == 0UL) ? LL_OK : LL_ERR;
    SET_REG32_BIT(CM_EFM->FSCLR, u32Blank);
    CLR_REG32_BIT(CM_EFM->FRMC2, u32ReadMode);
    MODIFY_REG32(CM_EFM->FRMC, EFM_CACHE_ALL, u32ReadAcclCmd);

    return i32Ret;
}

/**
 * @brief  Program EFM using single mode
 * @param  [in] u32Addr                 Starting address for efm programming
 * @param  [in] pu8DataSrc              Pointer to the data source to be programmed
 * @param  [in] u32ByteLen              Length of the data in bytes
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR_NOT_RDY:            Fail, EFM is not ready for Program
 *         - LL_ERR_INVD_PARAM:         Fail, Parameter invalid
 *         - LL_ERR:                    Fail, Error occurred during programming
 * @note  Call EFM_REG_Unlock() to unlock EFM register first
 */
int32_t EFM_Program(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen)
{
    int32_t i32Ret;

    i32Ret = Program_CheckCond(u32Addr, u32ByteLen);
    if (i32Ret == LL_OK) {
        i32Ret = Program_Implement(u32Addr, pu8DataSrc, u32ByteLen, EFM_MD_PGM_SINGLE);
    }
    return i32Ret;

}

/**
 * @brief  Program EFM using single and read-back mode
 * @param  [in] u32Addr                 Starting address for efm programming
 * @param  [in] pu8DataSrc              Pointer to the data source to be programmed
 * @param  [in] u32ByteLen              Length of the data in bytes
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR_NOT_RDY:            Fail, EFM is not ready for Program
 *         - LL_ERR_INVD_PARAM:         Fail, Parameter invalid
 *         - LL_ERR:                    Fail, Error occurred during programming
 * @note  Call EFM_REG_Unlock() to unlock EFM register first
 */
int32_t EFM_ProgramReadBack(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen)
{
    int32_t i32Ret;

    i32Ret = Program_CheckCond(u32Addr, u32ByteLen);
    if (i32Ret == LL_OK) {
        i32Ret = Program_Implement(u32Addr, pu8DataSrc, u32ByteLen, EFM_MD_PGM_READBACK);
    }
    return i32Ret;
}

/**
 * @brief  Program EFM using sequence mode
 * @param  [in] u32Addr                 Starting address for efm programming
 * @param  [in] pu8DataSrc              Pointer to the data source to be programmed
 * @param  [in] u32ByteLen              Length of the data in bytes
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR_NOT_RDY:            Fail, EFM is not ready for Program
 *         - LL_ERR_INVD_PARAM:         Fail, Parameter invalid
 *         - LL_ERR:                    Fail, Error occurred during programming
 * @note   1)__EFM_FUNC  is used to ensure this program is executed on a memory location distinct
 *         from the efm chip being programmed, or alternatively, on a different memory type such as RAM.
 *         __EFM_FUNC default value is __RAM_FUNC.
 *         __EFM_FUNC also could be attributed to FLASH0 or FLASH1 which is determined by users.
 *         If want to attribute to other place, please define in hc32f4xx_conf.h.
 *         2)Call EFM_REG_Unlock() to unlock EFM register first
 */
__NOINLINE __EFM_FUNC int32_t EFM_SequenceProgram(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen)
{
    int32_t i32Ret;

    i32Ret = Program_CheckCond(u32Addr, u32ByteLen);
    if (i32Ret == LL_OK) {
        i32Ret = Program_Implement(u32Addr, pu8DataSrc, u32ByteLen, EFM_MD_PGM_SEQ);
    }

    return i32Ret;
}
/**
 * @}
 */

/**
 * @defgroup EFM_Local_Functions EFM Local Functions
 * @{
 */

/**
 * @brief  Check condition for program
 * @param  [in] u32Addr                 Starting address for efm programming
 * @param  [in] u32ByteLen              Length of the data in bytes to be programmed
 * @retval An @ref Generic_Error_Codes enumeration type value.
 */
__EFM_FUNC static int32_t Program_CheckCond(uint32_t u32Addr, uint32_t u32ByteLen)
{
    if (!IS_EFM_REG_UNLOCK()) {
        return LL_ERR_NOT_RDY;
    }
    if (!IS_EFM_FWMC_UNLOCK()) {
        return LL_ERR_NOT_RDY;
    }
    if (!IS_ALIGNED_PGM_ADDR(u32Addr)) {
        return LL_ERR_INVD_PARAM;
    }
    if ((!IS_EFM_ADDR(u32Addr)) || (!IS_EFM_ADDR(u32Addr + u32ByteLen - 1UL))) {
        return LL_ERR_INVD_PARAM;
    }

    return LL_OK;
}

/**
 * @brief  EFM program implement
 * @param  [in] u32Addr                 Starting address for efm programming
 * @param  [in] pu8Data                 Pointer to the data array to be programmed
 * @param  [in] u32ByteLen              Length of the data in bytes
 * @param  [in] u32ProgramMode          @ref EFM_OperateMode_Sel
 * @retval Result of the programming operation @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
__EFM_FUNC static int32_t Program_Implement(uint32_t u32Addr, const uint8_t *pu8Data, uint32_t u32ByteLen, uint32_t u32ProgramMode)
{
    int32_t i32Ret = LL_OK;
    uint32_t u32RemainByteLen = u32ByteLen % EFM_PGM_UNIT_BYTES;
    uint8_t u8PaddingBuf[16U];
    int32_t i32Ready;
    stc_program_param_t stcProgramParam;
    uint32_t u32ReadAcclCmd = READ_REG32_BIT(CM_EFM->FRMC, EFM_RD_ACCL_CMD_ALL);

    /*Prepare Program*/
    CLR_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    SET_REG32_BIT(CM_EFM->FSCLR, EFM_FLAG_WRITE);
    stcProgramParam.pu32Src = (uint32_t *)(uint32_t)pu8Data;
    stcProgramParam.pu32Dest = (uint32_t *)u32Addr;
    stcProgramParam.u32UnitTotal = u32ByteLen / EFM_PGM_UNIT_BYTES;
    stcProgramParam.u32ProgramMode = u32ProgramMode;
    stcProgramParam.enSwap = GetSwapStatus();
    stcProgramParam.enOtp = GetOTPStatus();

    /*Program*/
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, u32ProgramMode);
    if (stcProgramParam.u32UnitTotal > 0U) {
        i32Ret = Program_Write(&stcProgramParam);
    }
    if ((u32RemainByteLen > 0U) && (i32Ret == LL_OK)) {
        Program_PadLastUnit(u8PaddingBuf, (uint8_t *)stcProgramParam.pu32Src, u32RemainByteLen);
        stcProgramParam.pu32Src = (uint32_t *)(uint32_t)u8PaddingBuf;
        stcProgramParam.u32UnitTotal = 1U;
        i32Ret = Program_Write(&stcProgramParam);
    }
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_READONLY);

    /*Handle program done*/
    i32Ready = WaitFlashReady(u32Addr, u32ByteLen, stcProgramParam.enSwap, stcProgramParam.enOtp);
    i32Ret = (i32Ret == LL_OK) ? i32Ready : i32Ret;
    MODIFY_REG32(CM_EFM->FRMC, EFM_CACHE_ALL, u32ReadAcclCmd);

    return i32Ret;
}

/**
 * @brief  Write data to flash
 * @param  [in] pstcParam               parameter for programming @ref stc_program_param_t
 * @retval An @ref Generic_Error_Codes enumeration type value.
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
__EFM_FUNC static int32_t Program_Write(stc_program_param_t *pstcParam)
{
    uint8_t u8Offset;
    uint32_t u32WaitFlagInit = EFM_FLAG_OPTEND, u32WaitFlag, u32ClrFlag, u32MismatchFlag;
    uint32_t u32CheckMismatch = (uint32_t)(pstcParam->u32ProgramMode == EFM_MD_PGM_READBACK);
    uint32_t u32Count;

    if (pstcParam->u32ProgramMode != EFM_MD_PGM_SEQ) {
        u32WaitFlagInit |= EFM_FLAG_RDY;
    }
    while (pstcParam->u32UnitTotal-- > 0UL) {
        u8Offset = EFM_FLAG_OFFSET(pstcParam->enSwap, pstcParam->enOtp, ((uint32_t)pstcParam->pu32Dest));
        u32WaitFlag = u32WaitFlagInit << u8Offset;
        u32ClrFlag = EFM_FLAG_OPTEND << u8Offset;

        *pstcParam->pu32Dest++ = *pstcParam->pu32Src++;
        *pstcParam->pu32Dest++ = *pstcParam->pu32Src++;
        *pstcParam->pu32Dest++ = *pstcParam->pu32Src++;
        *pstcParam->pu32Dest++ = *pstcParam->pu32Src++;

        /* Wait and clear flag */
        u32Count = 0UL;
        while ((CM_EFM->FSR & u32WaitFlag) != u32WaitFlag) {
            if (++u32Count > EFM_TIMEOUT) {
                return  LL_ERR;
            }
        }
        u32Count = 0UL;
        while ((CM_EFM->FSR & u32ClrFlag) == u32ClrFlag) {
            CM_EFM->FSCLR = u32ClrFlag;
            if (++u32Count > EFM_TIMEOUT) {
                return  LL_ERR;
            }
        }
        /* Check mismatch flag if needed */
        if (u32CheckMismatch == 1UL) {
            u32MismatchFlag = EFM_FLAG_PGMISMTCH << u8Offset;
            if ((CM_EFM->FSR & u32MismatchFlag) == u32MismatchFlag) {
                return  LL_ERR;
            }
        }
    }

    return LL_OK;
}

/**
 * @brief  Pad last program unit
 * @param  [in] pPaddingBuf             Padding data buffer
 * @param  [in] pu8RemainByteStart      Start address of remaining bytes.
 * @param  [in] u32RemainByteLen        Total number of remaining bytes.
 * @retval None
 */
__EFM_FUNC static void Program_PadLastUnit(uint8_t *pPaddingBuf, uint8_t *pu8RemainByteStart, uint32_t u32RemainByteLen)
{
    uint32_t i;

    for (i = 0U; i < EFM_PGM_UNIT_BYTES; i++) {
        pPaddingBuf[i] = EFM_PGM_PAD_BYTE;
    }
    for (i = 0U; i < u32RemainByteLen; i++) {
        pPaddingBuf[i] = pu8RemainByteStart[i];
    }
}

/**
 * @brief  Wait flash ready
 * @param  [in] u32Addr                 Starting address of read/write operation
 * @param  [in] u32ByteLen              Byte Length of of read/write operation
 * @param  [in] enSwap                  Swap is enabled or not
 * @param  [in] enOtp                   Otp is enabled or not
 * @retval An @ref Generic_Error_Codes enumeration type value.
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
__EFM_FUNC static int32_t WaitFlashReady(uint32_t u32Addr, uint32_t u32ByteLen, en_flag_status_t enSwap, en_flag_status_t enOtp)
{
    int32_t i32Ret;
    uint32_t u32EndAddr = u32Addr + u32ByteLen - 1UL;
    uint32_t u32ReadyStartAddr;
    uint32_t u32ReadyEndAddr;
    uint32_t u32ReadyToBeWait;
    uint8_t u8Offset;

    /* Get ready flag of start address and end address */
    u8Offset = GetFlagOffset(enSwap, enOtp, u32Addr);
    u32ReadyStartAddr = EFM_FLAG_RDY << u8Offset;
    u8Offset = GetFlagOffset(enSwap, enOtp, u32EndAddr);
    u32ReadyEndAddr = EFM_FLAG_RDY << u8Offset;

    /* Set ready flag to be wait*/
    u32ReadyToBeWait = u32ReadyStartAddr | u32ReadyEndAddr;
    if (u32ReadyToBeWait == EFM_FLAG_RDY) {
        if ((u32Addr < EFM_START_ADDR + EFM_SECTOR_SIZE) && (u32EndAddr >= EFM_FLASH_1_START_ADDR + EFM_SECTOR_SIZE)) {
            u32ReadyToBeWait |= EFM_FLAG_RDY1;
        }
    }

    /* Wait ready flag */
    i32Ret = WaitStatus(u32ReadyToBeWait, SET);

    return i32Ret;
}

/**
 * @brief  Get flag offset of specified flash address
 * @param  [in] enSwap                  Swap state
 * @param  [in] enOtp                   Opt state
 * @param  [in] u32Addr                 Flash address
 * @retval uint8_t offset
 */
__EFM_FUNC static uint8_t GetFlagOffset(en_flag_status_t enSwap, en_flag_status_t enOtp, uint32_t u32Addr)
{
    return EFM_FLAG_OFFSET(enSwap, enOtp, u32Addr);
}

/**
 * @brief  Wait and clear OPT End flag
 * @param  [in] u8FlagOffset            Flag offset
 * @retval An @ref Generic_Error_Codes enumeration type value.
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
__EFM_FUNC static int32_t WaitAndClearOPTEndFlag(uint8_t u8FlagOffset)
{
    int32_t iRet;
    uint32_t u32OptEnd = EFM_FLAG_OPTEND << u8FlagOffset;

    iRet = WaitStatus(u32OptEnd, SET);
    if (iRet == LL_OK) {
        CM_EFM->FSCLR = u32OptEnd;
        iRet = WaitStatus(u32OptEnd, RESET);
    }

    return iRet;
}

/**
 * @brief  Wait until Flag is(are all) Set or RESET
 * @param  [in] u32Flag                 One or any combination of @ref EFM_Flag_Sel
 * @param  [in] enStatus                SET or RESET @ref en_flag_status_t
 * @retval An @ref Generic_Error_Codes enumeration type value.
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
__EFM_FUNC static int32_t WaitStatus(uint32_t u32Flag, en_flag_status_t enStatus)
{
    int32_t i32Ret = LL_ERR;
    uint32_t u32Count = 0UL;
    uint32_t u32IsFlagSet;

    while (u32Count++ < EFM_ERASE_TIMEOUT && i32Ret == LL_ERR) {
        u32IsFlagSet = (uint32_t)((CM_EFM->FSR & u32Flag) == u32Flag);

        if ((uint32_t)(enStatus == SET) == u32IsFlagSet) {
            i32Ret = LL_OK;
        }
    }

    return i32Ret;
}

/**
 * @brief  Get otp status
 * @param  None
 * @retval An @ref en_flag_status_t enumeration type value.
 */
__EFM_FUNC static en_flag_status_t GetOTPStatus(void)
{
    en_flag_status_t enStatus = SET;
    uint32_t i;

    for (i = 0U; i < EFM_PGM_UNIT_WORDS; i++) {
        if ((RW_MEM32(EFM_OTP_ENABLE_ADDR + i * 4UL)) == 0xFFFFFFFFUL) {
            enStatus = RESET;
            break;
        }
    }

    return enStatus;
}

/**
 * @brief  Get swap status
 * @param  None
 * @retval An @ref en_flag_status_t enumeration type value.
 */
__EFM_FUNC static en_flag_status_t GetSwapStatus(void)
{
    if (GetOTPStatus() == RESET) {
        return (1UL == READ_REG32(bCM_EFM->FSWP_b.FSWP)) ? SET : RESET;
    } else {
        return ((*(uint32_t *)EFM_SWAP_ADDR) == EFM_SWAP_DATA) ? SET : RESET;
    }
}

/**
 * @}
 */

/**
 * @addtogroup EFM_Global_Functions EFM Global Functions
 * @{
 */

/**
 * @brief  Erase efm sector
 *
 * @param  [in] u32Addr  The address in the specified sector
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
static int32_t SectorErase_Implement(uint32_t u32Addr)
{
    int32_t i32Ret;
    uint8_t u8Offset = GetFlagOffset(GetSwapStatus(), GetOTPStatus(), u32Addr);
    uint32_t u32ReadAcclCmd = READ_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);

    CLR_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    EFM_ClearStatus(EFM_FLAG_WRITE);

    /* Erase sector */
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_ERASE_SECTOR);
    RW_MEM32(u32Addr) = 0UL;
    i32Ret = WaitAndClearOPTEndFlag(u8Offset);
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_READONLY);

    MODIFY_REG32(CM_EFM->FRMC, EFM_CACHE_ALL, u32ReadAcclCmd);

    return i32Ret;
}

/**
 * @brief  Erase efm sector
 *
 * @param  [in] u32Addr  The address in the specified sector
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_SectorErase(uint32_t u32Addr)
{
    DDL_ASSERT(IS_EFM_ERASE_ADDR(u32Addr));
    DDL_ASSERT(IS_ADDR_ALIGN_WORD(u32Addr));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());

    return SectorErase_Implement(u32Addr);
}

/**
 * @brief  EFM chip erase.
 * @param  [in] u32Chip      One or any combination of @ref EFM_Chip_Sel
 *    @arg  EFM_CHIP0
 *    @arg  EFM_CHIP1
 *    @arg  EFM_CHIP_ALL
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR_NOT_RDY:            Fail, EFM is not ready for erase
 *         - LL_ERR_INVD_PARAM:         Fail, Parameter invalid
 *         - LL_ERR:                    Fail, Error occurred during erasing
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 *         __EFM_FUNC default value is __RAM_FUNC.
 *         __EFM_FUNC also could be attributed to FLASH0 or FLASH1 which is determined by users.
 *        If want to attribute to other place, please define in hc32f4xx_conf.h.
 */
__NOINLINE __EFM_FUNC int32_t EFM_ChipErase(uint32_t u32Chip)
{
    uint32_t u32ReadAcclCmd = READ_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    uint32_t u32Addr = EFM_START_ADDR;
    int32_t i32Ret;
    uint8_t u8Offset = EFM_FLAG0_POS;

    if (!IS_EFM_REG_UNLOCK()) {
        return LL_ERR_NOT_RDY;
    }
    if (!IS_EFM_FWMC_UNLOCK()) {
        return LL_ERR_NOT_RDY;
    }
    if (!IS_EFM_CHIP(u32Chip)) {
        return LL_ERR_INVD_PARAM;
    }

    /* Prepare */
    CLR_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    SET_REG32_BIT(CM_EFM->FSCLR, EFM_FLAG_WRITE);

    if (EFM_CHIP1 == u32Chip) {
        u32Addr = EFM_FLASH_1_START_ADDR;
    }
    if (EFM_CHIP_ALL != u32Chip) {
        u8Offset = GetFlagOffset(GetSwapStatus(), GetOTPStatus(), u32Addr);
    }

    /* Erase chip */
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, (EFM_CHIP_ALL == u32Chip) ? \
                 EFM_MD_ERASE_ALL_CHIP : EFM_MD_ERASE_ONE_CHIP);
    RW_MEM32(u32Addr) = 0UL;
    if (EFM_CHIP_ALL == u32Chip) {
        i32Ret = WaitAndClearOPTEndFlag(EFM_FLAG0_POS);
        if (i32Ret == LL_OK) {
            i32Ret = WaitAndClearOPTEndFlag(EFM_FLAG1_POS);
        }
    } else {
        i32Ret = WaitAndClearOPTEndFlag(u8Offset);
    }
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_READONLY);

    /* Disable Swap */
    if ((i32Ret  == LL_OK) && (EFM_CHIP_ALL == u32Chip)) {
        MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_ERASE_SECTOR);
        RW_MEM32(EFM_SWAP_ADDR) = 0UL;
        i32Ret = WaitAndClearOPTEndFlag(EFM_FLAG0_POS);
        MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_READONLY);
    }

    MODIFY_REG32(CM_EFM->FRMC, EFM_CACHE_ALL, u32ReadAcclCmd);
    return i32Ret;
}

/**
 * @brief  FWMC register write enable or disable.
 * @param  [in] enNewState                An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void EFM_FWMC_Cmd(en_functional_state_t enNewState)
{
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (ENABLE == enNewState) {
        WRITE_REG32(CM_EFM->KEY1, 0x01234567UL);
        WRITE_REG32(CM_EFM->KEY1, 0xFEDCBA98UL);
    } else {
        SET_REG32_BIT(CM_EFM->FWMC, EFM_FWMC_KEY1LOCK);
    }
}

/**
 * @brief  Get chip ID.
 * @param  None
 * @retval Returns the value of the Chip ID
 */
uint32_t EFM_GetCID(void)
{
    return READ_REG32(CM_EFM->CHIPID);
}
/**
 * @brief  EFM OTP operate unlock.
 * @param  None
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_OTP_WP_Unlock(void)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(CM_EFM->KEY2, EFM_OTP_UNLOCK_KEY1);
    WRITE_REG32(CM_EFM->KEY2, EFM_OTP_UNLOCK_KEY2);
}

/**
 * @brief  EFM OTP write protect lock.
 * @param  None
 * @retval None
 */
void EFM_OTP_WP_Lock(void)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    SET_REG32_BIT(CM_EFM->FWMC, EFM_FWMC_KEY2LOCK);
}

/**
 * @brief  Enable OTP Function
 * @param  None
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_OTP_Enable(void)
{
    int32_t i32Ret = LL_OK;
    uint32_t u32EnableCode[4] = {0UL};

    DDL_ASSERT(IS_EFM_OTP_UNLOCK());

    if (EFM_GetOTPStatus() == SET) {
        return i32Ret;
    }
    i32Ret = Program_Implement(EFM_OTP_ENABLE_ADDR,
                               (uint8_t *)u32EnableCode,
                               EFM_PGM_UNIT_BYTES,
                               EFM_MD_PGM_SINGLE);
    return i32Ret;
}

/**
 * @brief  Set block OTP by the lock address of block
 * @param  [in]  u32Addr   The lock address of block @ref EFM_OTP_BLOCK_LOCKADDR
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_OTP_Lock(uint32_t u32Addr)
{
    int32_t i32Ret;
    uint32_t u32LockCode[4] = {0UL};

    DDL_ASSERT(IS_EFM_OTP_UNLOCK());
    DDL_ASSERT((u32Addr >= EFM_OTP_LOCK_ADDR_START) && (u32Addr < EFM_OTP_LOCK_ADDR_END));

    i32Ret = Program_Implement(u32Addr,
                               (uint8_t *)u32LockCode,
                               EFM_PGM_UNIT_BYTES,
                               EFM_MD_PGM_SINGLE);
    return i32Ret;
}

/**
 * @brief  Set block OTP by a range of block index
 * @param  [in]  u32BlockStartIdx       Start block index,range from 0 to @ref EFM_OTP_BLOCK_IDX_MAX
 * @param  [in]  u16Count               Number of block(s) starting from u32BlockStartIdx
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_OTP_LockBlock(uint32_t u32BlockStartIdx, uint16_t u16Count)
{
    int32_t i32Ret = LL_OK;
    uint32_t i, j;
    uint32_t u32ReadAcclCmd = READ_REG32_BIT(CM_EFM->FRMC, EFM_RD_ACCL_CMD_ALL);
    uint32_t *pu32LockAddr;
    uint32_t u32EndIdx = u32BlockStartIdx + u16Count - 1U;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());
    DDL_ASSERT((EFM_FLAG_RDY) == READ_REG32_BIT(CM_EFM->FSR, (EFM_FLAG_RDY)));
    DDL_ASSERT((EFM_FLAG_RDY1) == READ_REG32_BIT(CM_EFM->FSR, (EFM_FLAG_RDY1)));
    DDL_ASSERT(IS_EFM_OTP_UNLOCK());
    DDL_ASSERT(u32EndIdx <= EFM_OTP_BLOCK_IDX_MAX);
    DDL_ASSERT(u32BlockStartIdx <= u32EndIdx);

    /*Prepare Program*/
    CLR_REG32_BIT(CM_EFM->FRMC, EFM_CACHE_ALL);
    SET_REG32_BIT(CM_EFM->FSCLR, EFM_FLAG_WRITE);

    /*Program lock code*/
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_PGM_SINGLE);
    for (i = u32BlockStartIdx; i <= u32EndIdx; i++) {
        pu32LockAddr = (uint32_t *)EFM_OTP_BLOCK_LOCKADDR(i);
        for (j = 0U; j < EFM_PGM_UNIT_WORDS; j++) {
            *pu32LockAddr++ = 0UL;
        }
        if (i32Ret != WaitAndClearOPTEndFlag(EFM_FLAG0_POS)) {
            break;
        }
    }
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_PEMOD, EFM_MD_READONLY);

    /* Recover read accelerator cmd */
    MODIFY_REG32(CM_EFM->FRMC, EFM_CACHE_ALL, u32ReadAcclCmd);

    return i32Ret;
}

/**
 * @brief  Get otp enabled or disabled status
 * @param  None
 * @retval en_flag_status_t:
 *         - SET:                       OTP enabled
 *         - RESET:                     OTP disable
 */
en_flag_status_t EFM_GetOTPStatus(void)
{
    return GetOTPStatus();
}

/**
 * @brief  Sector protected register lock.
 * @param  [in] u32RegLock                      Specifies sector protected register locking.
 *  @arg   EFM_WRLOCK0 ~ EFM_WRLOCK7
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 */
void EFM_SectorProtectRegLock(uint32_t u32RegLock)
{
    DDL_ASSERT(IS_EFM_SECTOR_PROTECT_REG_LOCK(u32RegLock));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    SET_REG32_BIT(CM_EFM->WLOCK, u32RegLock);
}

/**
 * @brief  Set sector lock or unlock (Single).
 * @param  [in] u8SectorNum     Specifies sector for unlock.
 *                              This parameter can be set 0~255.
 *                              see the macro definition for @ref EFM_Sector_Count
 * @param  [in] enNewState      An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 *         If you want to unlock sequence sectors,Please call EFM_SequenceSectorOperateCmd function
 */
void EFM_SingleSectorOperateCmd(uint8_t u8SectorNum, en_functional_state_t enNewState)
{
    __IO uint32_t *EFM_FxNWPRTy;
    uint8_t u8RegIndex;
    uint8_t u8BitPos;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (SET == EFM_GetSwapStatus()) {
        if (0xFFFFFFFFUL != RW_MEM32(EFM_OTP_ENABLE_ADDR)) {
            if (u8SectorNum > EFM_SWAP_FLASH1_END_SECTOR_NUM) {
                u8SectorNum -= EFM_FLASH1_START_SECTOR_NUM;
            } else if ((u8SectorNum > EFM_OTP_END_SECTOR_NUM) && (u8SectorNum < EFM_FLASH1_START_SECTOR_NUM)) {
                u8SectorNum += EFM_FLASH1_START_SECTOR_NUM;
            } else {
                /* rsvd */
            }
        } else {
            if (u8SectorNum >= EFM_FLASH1_START_SECTOR_NUM) {
                u8SectorNum -= EFM_FLASH1_START_SECTOR_NUM;
            } else {
                u8SectorNum += EFM_FLASH1_START_SECTOR_NUM;
            }
        }
    }

    u8BitPos = u8SectorNum % REG_LEN;
    u8RegIndex = u8SectorNum / REG_LEN;
    EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u8RegIndex << EFM_FNWPRT_REG_OFFSET));
    MODIFY_REG32(*EFM_FxNWPRTy, 1UL << u8BitPos, (uint32_t)enNewState << u8BitPos);
}

/**
 * @brief  Set sector lock or unlock (Sequence).
 * @param  [in] u32StartSectorNum   Specifies start sector to unlock.
 *                                  This parameter can be set 0~255.
 *                                  see the macro definition for @ref EFM_Sector_Count
 * @param  [in] u16Count            Specifies count of sectors to unlock.
 *                                  This parameter can be set 1~256.
 *                                  see the macro definition for @ref EFM_Sector_Count
 * @param  [in] enNewState          An @ref en_functional_state_t enumeration value.
 * @retval None
 * @note   Call EFM_REG_Unlock() unlock EFM register first.
 *         If you want to unlock only one sector,Please call EFM_SingleSectorOperateCmd function
 */
void EFM_SequenceSectorOperateCmd(uint32_t u32StartSectorNum, uint16_t u16Count, en_functional_state_t enNewState)
{
    __IO uint32_t *EFM_FxNWPRTy;
    uint32_t u32EndSectorNum;
    uint16_t u16StartRegIndex;
    uint16_t u16StartBitPos;
    uint16_t u16EndRegIndex;
    uint16_t u16EndBitPos;
    uint32_t u32RegValue;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_SECTOR_IDX(u32StartSectorNum));
    DDL_ASSERT(IS_EFM_SECTOR_NUM(u32StartSectorNum + u16Count));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if (SET == EFM_GetSwapStatus()) {
        if (0xFFFFFFFFUL != RW_MEM32(EFM_OTP_ENABLE_ADDR)) {
            if (u32StartSectorNum > EFM_SWAP_FLASH1_END_SECTOR_NUM) {
                u32StartSectorNum -= EFM_FLASH1_START_SECTOR_NUM;
            } else if ((u32StartSectorNum > EFM_OTP_END_SECTOR_NUM) && (u32StartSectorNum < EFM_FLASH1_START_SECTOR_NUM)) {
                u32StartSectorNum += EFM_FLASH1_START_SECTOR_NUM;
            } else {
                /* rsvd */
            }
        } else {
            if (u32StartSectorNum >= EFM_FLASH1_START_SECTOR_NUM) {
                u32StartSectorNum -= EFM_FLASH1_START_SECTOR_NUM;
            } else {
                u32StartSectorNum += EFM_FLASH1_START_SECTOR_NUM;
            }
        }
    }

    u16StartRegIndex = (uint16_t)(u32StartSectorNum / REG_LEN);            /* Register offset for the start sector */
    u16StartBitPos = (uint16_t)(u32StartSectorNum % REG_LEN);              /* Bit offset for the start sector */
    u32EndSectorNum = (uint16_t)(u32StartSectorNum + u16Count - 1U);       /* Calculate the end sector */
    u16EndRegIndex = (uint16_t)(u32EndSectorNum / REG_LEN);                /* Register offset for the end sector */
    u16EndBitPos = (uint16_t)(u32EndSectorNum % REG_LEN);                  /* Bit offset for the end sector */

    if ((u16StartBitPos == 0U) && (u16EndBitPos == 31U)) {
        while (u16StartRegIndex <= u16EndRegIndex) {
            EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
            if (EFM_FxNWPRTy > EFM_FNWPRT_REG_END_ADDR) {
                EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + (((uint32_t)u16StartRegIndex - EFM_FNWPRT_REG_NUM) << EFM_FNWPRT_REG_OFFSET));
            }
            if (enNewState == ENABLE) {
                WRITE_REG32(*EFM_FxNWPRTy, 0xFFFFFFFFUL);
            } else {
                WRITE_REG32(*EFM_FxNWPRTy, 0x0UL);
            }
            u16StartRegIndex += 1U;
        }
    } else {
        EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
        if (u16StartRegIndex == u16EndRegIndex) {
            u32RegValue = ((1UL << u16EndBitPos) - (1UL << u16StartBitPos)) | (1UL << u16EndBitPos);
            if (enNewState == ENABLE) {
                SET_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
            } else {
                CLR_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
            }
        } else {
            u32RegValue = ((1UL << 31U) - (1UL << u16StartBitPos)) | (1UL << 31U);
            if (enNewState == ENABLE) {
                SET_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
                while (u16StartRegIndex < (u16EndRegIndex - 1U)) {
                    u16StartRegIndex += 1U;
                    EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
                    if (EFM_FxNWPRTy > EFM_FNWPRT_REG_END_ADDR) {
                        EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + (((uint32_t)u16StartRegIndex - EFM_FNWPRT_REG_NUM) << EFM_FNWPRT_REG_OFFSET));
                    }
                    WRITE_REG32(*EFM_FxNWPRTy, 0xFFFFFFFFUL);
                }
                u16StartRegIndex += 1U;
                u32RegValue = ((1UL << u16EndBitPos) - 1UL) | (1UL << u16EndBitPos);
                EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
                if (EFM_FxNWPRTy > EFM_FNWPRT_REG_END_ADDR) {
                    EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + (((uint32_t)u16EndRegIndex - EFM_FNWPRT_REG_NUM) << EFM_FNWPRT_REG_OFFSET));
                }
                SET_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
            } else {
                CLR_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
                while (u16StartRegIndex < (u16EndRegIndex - 1U)) {
                    u16StartRegIndex += 1U;
                    EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
                    if (EFM_FxNWPRTy > EFM_FNWPRT_REG_END_ADDR) {
                        EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + (((uint32_t)u16StartRegIndex - EFM_FNWPRT_REG_NUM) << EFM_FNWPRT_REG_OFFSET));
                    }
                    WRITE_REG32(*EFM_FxNWPRTy, 0x0UL);
                }
                u32RegValue = ((1UL << u16EndBitPos) - 1UL) | (1UL << u16EndBitPos);
                EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + ((uint32_t)u16StartRegIndex << EFM_FNWPRT_REG_OFFSET));
                if (EFM_FxNWPRTy > EFM_FNWPRT_REG_END_ADDR) {
                    EFM_FxNWPRTy = (__IO uint32_t *)((uint32_t)(&FNWPRT_REG) + (((uint32_t)u16EndRegIndex - EFM_FNWPRT_REG_NUM) << EFM_FNWPRT_REG_OFFSET));
                }
                CLR_REG32_BIT(*EFM_FxNWPRTy, u32RegValue);
            }
        }
    }
}

/**
 * @brief  Get unique ID.
 * @param  [out] pstcUID     Unique ID struct
 * @retval Returns the value of the unique ID
 */
void EFM_GetUID(stc_efm_unique_id_t *pstcUID)
{
    if (NULL != pstcUID) {
        pstcUID->u32UniqueID0 = READ_REG32(CM_EFM->UQID0);
        pstcUID->u32UniqueID1 = READ_REG32(CM_EFM->UQID1);
        pstcUID->u32UniqueID2 = READ_REG32(CM_EFM->UQID2);
    }
}

/**
 * @brief  Init REMAP initial structure with default value.
 * @param  [in] pstcEfmRemapInit specifies the Parameter of REMAP.
 * @retval int32_t:
 *         - LL_OK: Initialize success
 *         - LL_ERR_INVD_PARAM: NULL pointer
 */
int32_t EFM_REMAP_StructInit(stc_efm_remap_init_t *pstcEfmRemapInit)
{
    int32_t i32Ret = LL_OK;
    if (NULL == pstcEfmRemapInit) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        pstcEfmRemapInit->u32State = EFM_REMAP_OFF;
        pstcEfmRemapInit->u32Addr = 0UL;
        pstcEfmRemapInit->u32Size = EFM_REMAP_4K;
    }
    return i32Ret;
}

/**
 * @brief  REMAP initialize.
 * @param  [in] u8RemapIdx      Specifies the remap ID.
 * @param  [in] pstcEfmRemapInit specifies the Parameter of REMAP.
 * @retval int32_t:
 *         - LL_OK: Initialize success
 *         - LL_ERR_INVD_PARAM: NULL pointer
 */
int32_t EFM_REMAP_Init(uint8_t u8RemapIdx, stc_efm_remap_init_t *pstcEfmRemapInit)
{
    int32_t i32Ret = LL_OK;
    __IO uint32_t *REMCRx;

    if (NULL == pstcEfmRemapInit) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        DDL_ASSERT(IS_EFM_REMAP_UNLOCK());
        DDL_ASSERT(IS_EFM_REMAP_IDX(u8RemapIdx));
        DDL_ASSERT(IS_EFM_REMAP_SIZE(pstcEfmRemapInit->u32Size));
        DDL_ASSERT(IS_EFM_REMAP_ADDR(pstcEfmRemapInit->u32Addr));
        DDL_ASSERT(IS_EFM_REMAP_STATE(pstcEfmRemapInit->u32State));
        if ((pstcEfmRemapInit->u32Addr % (1UL << pstcEfmRemapInit->u32Size)) != 0U) {
            i32Ret = LL_ERR_INVD_PARAM;
        } else {
            REMCRx = &REMCR_REG(u8RemapIdx);
            MODIFY_REG32(*REMCRx, EFM_MMF_REMCR_EN | EFM_MMF_REMCR_RMTADDR | EFM_MMF_REMCR_RMSIZE, \
                         pstcEfmRemapInit->u32State | pstcEfmRemapInit->u32Addr | pstcEfmRemapInit->u32Size);
        }
    }
    return i32Ret;
}

/**
 * @brief  EFM REMAP de-initialize.
 * @param  None
 * @retval None
 */
void EFM_REMAP_DeInit(void)
{
    DDL_ASSERT(IS_EFM_REMAP_UNLOCK());

    WRITE_REG32(CM_EFM->MMF_REMCR0, 0UL);
    WRITE_REG32(CM_EFM->MMF_REMCR1, 0UL);
}

/**
 * @brief  Enable or disable REMAP function.
 * @param  [in] u8RemapIdx      Specifies the remap ID.
 * @param  [in] enNewState      An @ref en_functional_state_t enumeration value.
 * @retval None
 */
void EFM_REMAP_Cmd(uint8_t u8RemapIdx, en_functional_state_t enNewState)
{
    __IO uint32_t *REMCRx;

    DDL_ASSERT(IS_EFM_REMAP_UNLOCK());
    DDL_ASSERT(IS_EFM_REMAP_IDX(u8RemapIdx));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    REMCRx = &REMCR_REG(u8RemapIdx);
    if (ENABLE == enNewState) {
        SET_REG32_BIT(*REMCRx, EFM_MMF_REMCR_EN);
    } else {
        CLR_REG32_BIT(*REMCRx, EFM_MMF_REMCR_EN);
    }
}

/**
 * @brief  Set specified REMAP target address.
 * @param  [in] u8RemapIdx      Specifies the remap ID.
 * @param  [in] u32Addr         Specifies the target address.
 * @retval None
 */
void EFM_REMAP_SetAddr(uint8_t u8RemapIdx, uint32_t u32Addr)
{
    __IO uint32_t *REMCRx;

    DDL_ASSERT(IS_EFM_REMAP_UNLOCK());
    DDL_ASSERT(IS_EFM_REMAP_IDX(u8RemapIdx));
    DDL_ASSERT(IS_EFM_REMAP_ADDR(u32Addr));

    REMCRx = &REMCR_REG(u8RemapIdx);
    MODIFY_REG32(*REMCRx, EFM_MMF_REMCR_RMTADDR, u32Addr);
}

/**
 * @brief  Set specified REMAP size.
 * @param  [in] u8RemapIdx      Specifies the remap ID.
 * @param  [in] u32Size         Specifies the remap size.
 * @retval None
 */
void EFM_REMAP_SetSize(uint8_t u8RemapIdx, uint32_t u32Size)
{
    __IO uint32_t *REMCRx;

    DDL_ASSERT(IS_EFM_REMAP_UNLOCK());
    DDL_ASSERT(IS_EFM_REMAP_IDX(u8RemapIdx));
    DDL_ASSERT(IS_EFM_REMAP_SIZE(u32Size));

    REMCRx = &REMCR_REG(u8RemapIdx);
    MODIFY_REG32(*REMCRx, EFM_MMF_REMCR_RMSIZE, u32Size);
}

/**
 * @brief  Enable efm protect.
 * @param  [in] u8Level                 One or Combine of @ref EFM_Protect_Level
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_Protect_Enable(uint8_t u8Level)
{
    int32_t i32Ret = LL_OK;
    uint32_t i;
    uint32_t u32EnableCodeBuf[4] = {0UL};

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());
    DDL_ASSERT(u8Level <= EFM_PROTECT_LEVEL_ALL);

    /* Enable security sector in main flash */
    if (SET == EFM_GetSwapStatus()) {
        (void)EFM_SingleSectorOperateCmd(EFM_SWAP_ON_PROTECT_SECTOR_NUM, ENABLE);
    } else {
        (void)EFM_SingleSectorOperateCmd(0U, ENABLE);
    }

    /* Enable one or a combination of protection Level1, Level2 */
    if (((u8Level & EFM_PROTECT_LEVEL1) > 0U) || ((u8Level & EFM_PROTECT_LEVEL2) > 0U)) {
        for (i = 0UL; i < EFM_PGM_UNIT_WORDS; i++) {
            u32EnableCodeBuf[i] = 0xFFFFFFFFUL;
        }
        if ((u8Level & EFM_PROTECT_LEVEL1) > 0U) {
            u32EnableCodeBuf[0] = EFM_PROTECT1_KEY;
        }
        if ((u8Level & EFM_PROTECT_LEVEL2) > 0U) {
            u32EnableCodeBuf[1] = EFM_PROTECT2_KEY;
        }
        i32Ret = Program_Implement(EFM_PROTECT1_ADDR,
                                   (uint8_t *)u32EnableCodeBuf,
                                   EFM_PGM_UNIT_BYTES,
                                   EFM_MD_PGM_SINGLE);
    }

    /* Enable Protect Level3 */
    if ((i32Ret == LL_OK) && ((u8Level & EFM_PROTECT_LEVEL3) > 0U)) {
        for (i = 0UL; i < 3UL; i++) {
            u32EnableCodeBuf[i] = EFM_PROTECT3_KEY;
        }
        u32EnableCodeBuf[i] = 0xFFFFFFFFUL;
        i32Ret = Program_Implement(EFM_PROTECT3_ADDR1,
                                   (uint8_t *)u32EnableCodeBuf,
                                   EFM_PGM_UNIT_BYTES,
                                   EFM_MD_PGM_SINGLE);
    }
    return (i32Ret == LL_OK) ? LL_OK : LL_ERR;
}

/**
 * @brief  Write the security code.
 * @param  [in] pu8Code       Security code.
 * @param  [in] u32Len        Byte length of the security code.
 * @retval Function execution status @ref Generic_Error_Codes
 *         - LL_OK:                     Success
 *         - LL_ERR:                    Fail
 */
int32_t EFM_WriteSecurityCode(const uint8_t *pu8Code, uint32_t u32Len)
{
    int32_t i32Ret;
    uint8_t u8CodeBuf[16];
    uint32_t i;
    DDL_ASSERT(IS_EFM_SECURITY_CODE_LEN(u32Len));
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());

    i32Ret = SectorErase_Implement(EFM_SECURITY_ADDR);
    if (i32Ret != LL_OK) {
        return i32Ret;
    }

    for (i = 0U; i < EFM_PGM_UNIT_BYTES; i++) {
        u8CodeBuf[i] = EFM_SECURITY_CODE_PAD_BYTE;
    }
    for (i = 0U; i < u32Len; i++) {
        u8CodeBuf[i] = *pu8Code++;
    }

    i32Ret = Program_Implement(EFM_SECURITY_ADDR,
                               (uint8_t *)u8CodeBuf,
                               EFM_PGM_UNIT_BYTES, EFM_MD_PGM_SINGLE);

    return i32Ret;
}

/**
 * @brief  Unlock EFM ckcr register, enable ckcr write.
 * @param  None
 * @retval None
 */
void EFM_CKCR_Unlock(void)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(CM_EFM->CKPR, EFM_ECC_CKCR_PROTECT_CODE | EFM_CKPR_CKPRC);
}

/**
 * @brief  Lock EFM ckcr register, disable ckcr write.
 * @param  None
 * @retval None
 */
void EFM_CKCR_Lock(void)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());

    WRITE_REG32(CM_EFM->CKPR, EFM_ECC_CKCR_PROTECT_CODE);
}

/**
 * @brief  Init EFM ECC config structure with default values.
 * @param  [in] pstcEccConfig           Pointer to a @ref stc_efm_ecc_config_t structure
 * @retval int32_t:
 *         - LL_OK:                     Success
 *         - LL_ERR_INVD_PARAM:         Fail, pstcEccConfig is NULL
 */
int32_t EFM_ECC_StructInit(stc_efm_ecc_config_t *pstcEccConfig)
{
    stc_efm_ecc_chip_config_t *pstcChip = &pstcEccConfig->stcChip0;
    uint32_t i;

    if (pstcEccConfig == NULL) {
        return LL_ERR_INVD_PARAM;
    }

    pstcEccConfig->enBlankEcc = DISABLE;
    pstcEccConfig->enCheckBlankEcc = DISABLE;
    for (i = 0; i < EFM_CHIP_COUNT; i++) {
        pstcChip->u32CheckMode = EFM_ECC_MD2;
        pstcChip->u32ExceptionType = EFM_ECC_EXP_TYPE_NMI;
        pstcChip->enAutoGenerate = ENABLE;
        pstcChip->enAutoCheck = ENABLE;
        pstcChip++;
    }

    return LL_OK;
}

/**
 * @brief  Config EFM ECC by provided settings.
 * @param  [in] pstcEccConfig           Pointer to a @ref stc_efm_ecc_config_t structure
 * @retval int32_t
 *           - LL_OK:                   Success
 *           - LL_ERR_INVD_PARAM:       Fail, pstcEccConfig is NULL
 * @retval None
 */
int32_t EFM_ECC_Config(const stc_efm_ecc_config_t *pstcEccConfig)
{
    uint32_t u32BlankRW, u32ErrorResponse, u32Automation;
    uint32_t u32ErrorResponseMask = (EFM_CKCR_F0ECCMOD | EFM_CKCR_F0ECCOAD);
    const stc_efm_ecc_chip_config_t *pstcChip = &pstcEccConfig->stcChip0;
    __IO uint32_t *pu32ECCCR[2U];
    uint32_t u32CKCRChipDelta = EFM_CKCR_F1ECCMOD_POS - EFM_CKCR_F0ECCMOD_POS;
    uint32_t i;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_FWMC_UNLOCK());

    if (pstcEccConfig == NULL) {
        return LL_ERR_INVD_PARAM;
    }
    DDL_ASSERT(IS_FUNCTIONAL_STATE(pstcEccConfig->enBlankEcc));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(pstcEccConfig->enCheckBlankEcc));

    pu32ECCCR[0U] = &CM_EFM->F0ECCCR;
    pu32ECCCR[1U] = &CM_EFM->F1ECCCR;

    u32BlankRW = ((uint32_t)pstcEccConfig->enBlankEcc) << EFM_FWMC_BLKECCWSEL_POS;
    u32BlankRW |= ((uint32_t)pstcEccConfig->enCheckBlankEcc) << EFM_FWMC_BLKECCRSEL_POS;
    MODIFY_REG32(CM_EFM->FWMC, EFM_FWMC_BLKECCWSEL | EFM_FWMC_BLKECCRSEL, u32BlankRW);
    for (i = 0; i < EFM_CHIP_COUNT; i++) {
        DDL_ASSERT(IS_EFM_ECC_MD(pstcChip->u32CheckMode));
        DDL_ASSERT(IS_EFM_ECC_EXP(pstcChip->u32ExceptionType));
        DDL_ASSERT(IS_FUNCTIONAL_STATE(pstcChip->enAutoGenerate));
        DDL_ASSERT(IS_FUNCTIONAL_STATE(pstcChip->enAutoCheck));

        u32Automation = ((~(uint32_t)pstcChip->enAutoGenerate) & 1U) << EFM_F0ECCCR_GDIS_POS;
        u32Automation |= ((~(uint32_t)pstcChip->enAutoCheck) & 1U) << EFM_F0ECCCR_VDIS_POS;
        u32ErrorResponse = pstcChip->u32CheckMode << EFM_CKCR_F0ECCMOD_POS;
        u32ErrorResponse |= pstcChip->u32ExceptionType << EFM_CKCR_F0ECCOAD_POS;
        u32ErrorResponse <<= (i * u32CKCRChipDelta);
        u32ErrorResponseMask <<= (i * u32CKCRChipDelta);
        WRITE_REG32(*pu32ECCCR[i], u32Automation);
        MODIFY_REG32(CM_EFM->CKCR, u32ErrorResponseMask, u32ErrorResponse);

        pstcChip++;
    }

    return LL_OK;
}

/**
 * @brief  Write ECC Data to register.
 *         this data will be programmed as ECC data during the programming operation when
 *         the chip's automatic ECC generation feature (enAutoGenerate) is DISABLED.
 * @param  [in] u32Chip                 One of @ref EFM_Chip_Sel
 * @param  [in] u32Ecc                  Ecc data @ref EFM_ECC_BIT_MASK_9BIT_ECC_DATA
 * @retval None
 */
void EFM_ECC_Write(uint32_t u32Chip, uint32_t u32Ecc)
{
    __IO uint32_t *pu32DataReg = &CM_EFM->F0ECCDR;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT((u32Chip == EFM_CHIP0) || (u32Chip == EFM_CHIP1));
    DDL_ASSERT(u32Ecc <= EFM_ECC_BIT_MASK_9BIT_ECC_DATA);

    if (u32Chip == EFM_CHIP1) {
        pu32DataReg = &CM_EFM->F1ECCDR;
    }

    WRITE_REG32(*pu32DataReg, u32Ecc);
}

/**
 * @brief  Get ECC Data from register.
 *         ECC data will be updated to register during read cycle when the chip's
 *         automatic check feature(enAutoCheck) is DISABLED
 * @param  [in] u32Chip                 One of @ref EFM_Chip_Sel
 * @retval uint32_t                     Ecc data @ref EFM_ECC_BIT_MASK_9BIT_ECC_DATA
 */
uint32_t EFM_ECC_Read(uint32_t u32Chip)
{
    __IO uint32_t *pu32DataReg = &CM_EFM->F0ECCDR;

    DDL_ASSERT((u32Chip == EFM_CHIP0) || (u32Chip == EFM_CHIP1));

    if (u32Chip == EFM_CHIP1) {
        pu32DataReg = &CM_EFM->F1ECCDR;
    }

    return READ_REG32(*pu32DataReg) >> EFM_F0ECCDR_DRD_POS;
}

/**
 * @brief  Get EFM check flag(s) is(are) set or not.
 * @param  [in]  u32Flag                One or any combination of @ref EFM_Check_Flag :
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP0_1BIT_ERR
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP0_2BIT_ERR
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP0_ALL
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP1_1BIT_ERR
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP1_2BIT_ERR
 *   @arg  EFM_CHECK_FLAG_ECC_CHIP1_ALL
 *   @arg  EFM_CHECK_FLAG_ECC_ALL
 * @retval An @ref en_flag_status_t enumeration type value.
 */
en_flag_status_t EFM_GetCheckStatus(uint32_t u32Flag)
{
    DDL_ASSERT(IS_EFM_ECC_ERR_FLAG(u32Flag));

    return ((0UL != READ_REG32_BIT(CM_EFM->CKSR, u32Flag)) ? SET : RESET);
}

/**
 * @brief  Clear EFM check flag(s)
 * @param  [in]  u32Flag                One or any combination of  @ref EFM_Check_Flag
 * @retval None
 */
void EFM_ClearCheckStatus(uint32_t u32Flag)
{
    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_ECC_ERR_FLAG(u32Flag));

    WRITE_REG32(CM_EFM->CKSR, u32Flag);
}

/**
 * @brief  Get EFM ECC Error record(s).
 * @param  [in]  u32Chip                One or any combination of @ref EFM_Chip_Sel
 * @param  [in]  u32Record              One or any combination of @ref EFM_ECC_Error_Record_ID :
 *   @arg  EFM_ECC_ERR_REC_ID0
 *   @arg  EFM_ECC_ERR_REC_ID1
 *   @arg  EFM_ECC_ERR_REC_ALL
 * @retval stc_efm_ecc_err_record_t*    Pointer to ECC error record(s) @ref stc_efm_ecc_err_record_t
 */
stc_efm_ecc_err_record_t *EFM_ECC_GetErrorRecord(uint32_t u32Chip, uint32_t u32Record)
{
    __IO uint32_t *pu32ErrorReg[2U][2U];
    uint32_t i, j, k = 0UL;
    uint32_t u32Error;

    DDL_ASSERT(IS_EFM_CHIP(u32Chip));
    DDL_ASSERT(IS_EFM_ECC_ERR_REC(u32Record));

    ECC_ClearSoftwareErrorRecord();
    pu32ErrorReg[0U][0U] = &(CM_EFM->F0ERR0);
    pu32ErrorReg[0U][1U] = &(CM_EFM->F0ERR1);
    pu32ErrorReg[1U][0U] = &(CM_EFM->F1ERR0);
    pu32ErrorReg[1U][1U] = &(CM_EFM->F1ERR1);

    for (i = 0U; i < EFM_CHIP_COUNT; i++) {
        if ((u32Chip & (1UL << i)) == 0UL) {
            continue;
        }
        for (j = 0U; j < EFM_ECC_ERR_RECORD_COUNT; j++) {
            if ((u32Record & (1UL << j)) == 0UL) {
                continue;
            }
            u32Error = READ_REG32(*pu32ErrorReg[i][j]);
            m_astcEccErrRecord[k].u32IsValid = ((u32Error & EFM_F0ERR0_VALID) != 0U) ? 1U : 0U;
            m_astcEccErrRecord[k].u32IsFatal = ((u32Error & EFM_F0ERR0_FATAL) != 0U) ? 1U : 0U;
            m_astcEccErrRecord[k].u32IsRescueSector = ((u32Error & EFM_ECC_ERR_RECORD_MASK_RESCUE_SECTOR) != 0U) ? 1U : 0U;
            m_astcEccErrRecord[k].u32IsSpecialFuncSector = ((u32Error & EFM_ECC_ERR_RECORD_MASK_SPECIAL_FUNC_SECTOR) != 0U) ? 1U : 0U;
            m_astcEccErrRecord[k].u32AddrOffset = u32Error & EFM_ECC_ERR_RECORD_MASK_ADDRESS_OFFSET;
            m_astcEccErrRecord[k].u32EfmChip = 1UL << i;
            m_astcEccErrRecord[k].u32ErrorID = 1UL << j;
            k++;
        }
    }

    return m_astcEccErrRecord;
}

/**
 * @brief  Clear EFM ECC error recorded by software.
 * @param  None
 * @retval None
 */
static void ECC_ClearSoftwareErrorRecord(void)
{
    uint32_t u32ByteLen = sizeof(m_astcEccErrRecord);
    uint8_t *pu8Buf = (uint8_t *)m_astcEccErrRecord;

    while (u32ByteLen-- > 0UL) {
        *pu8Buf++ = 0U;
    }
}

/**
 * @brief  Clear EFM ECC error record(s).
 * @param  [in]  u32Chip                One or any combination of @ref EFM_Chip_Sel
 * @param  [in]  u32Record              One or any combination of @ref EFM_ECC_Error_Record_ID :
 *   @arg  EFM_ECC_ERR_REC_ID0
 *   @arg  EFM_ECC_ERR_REC_ID1
 *   @arg  EFM_ECC_ERR_REC_ALL
 * @retval None
 */
void EFM_ECC_ClearErrorRecord(uint32_t u32Chip, uint32_t u32Record)
{
    __IO uint32_t *pu32ErrorReg[2U][2U];
    uint32_t i, j;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_CHIP(u32Chip));
    DDL_ASSERT(IS_EFM_ECC_ERR_REC(u32Record));

    pu32ErrorReg[0U][0U] = &(CM_EFM->F0ERR0);
    pu32ErrorReg[0U][1U] = &(CM_EFM->F0ERR1);
    pu32ErrorReg[1U][0U] = &(CM_EFM->F1ERR0);
    pu32ErrorReg[1U][1U] = &(CM_EFM->F1ERR1);
    for (i = 0UL; i < EFM_CHIP_COUNT; i++) {
        if ((u32Chip & (1UL << i)) == 0UL) {
            continue;
        }
        for (j = 0UL; j < EFM_ECC_ERR_RECORD_COUNT; j++) {
            if ((u32Record & (1UL << j)) == 0UL) {
                continue;
            }
            WRITE_REG32(*pu32ErrorReg[i][j], EFM_F0ERR0_FATAL);
        }
    }

    ECC_ClearSoftwareErrorRecord();
}

/**
 * @brief  EFM ECC error injection command.
 * @param  [in]  u32Chip                One or any combination of @ref EFM_Chip_Sel
 * @param  [in]  enNewState             One of @ref en_functional_state_t
 * @retval None
 */
void EFM_ECC_ErrorInjectCmd(uint32_t u32Chip, en_functional_state_t enNewState)
{
    uint32_t u32Mask = 0UL;
    uint32_t u32ChipOffset = 0UL;

    DDL_ASSERT(IS_EFM_REG_UNLOCK());
    DDL_ASSERT(IS_EFM_CHIP(u32Chip));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    while (u32Chip != 0U) {
        if ((u32Chip & 1U) == 1U) {
            u32Mask |= EFM_EIEN_F0_EIEN << u32ChipOffset;
        }
        u32Chip >>= 1U;
        u32ChipOffset += EFM_ECC_BIT_POS_DELTA_EIEN;
    }

    if (enNewState == ENABLE) {
        SET_REG32_BIT(CM_EFM->EIEN, u32Mask);
    } else {
        CLR_REG32_BIT(CM_EFM->EIEN, u32Mask);
    }
}

/**
 * @brief  Enable or disable EFM ECC error injection bit
 * @param  [in]  u32Chip                One or any combination of @ref EFM_Chip_Sel
 * @param  [in]  pstcBitSel             Error injection bit selection @ref stc_efm_ecc_err_inject_bit_t
 * @param  [in]  enNewState             One of @ref en_functional_state_t
 * @note   Bit selection is depend on bit mask @ref EFM_ECC_Bit_Mask :
 *                                      EFM_ECC_BIT_MASK_WORD:            0xFFFFFFFFUL
 *                                      EFM_ECC_BIT_MASK_9BIT_ECC_DATA:   0x1FFUL
 *                                      EFM_ECC_BIT_MASK_20BIT_ADDR:      0xFFFFF000UL
 * @retval None
 */
void EFM_ECC_ErrorInjectBitCmd(uint32_t u32Chip, const stc_efm_ecc_err_inject_bit_t *pstcBitSel, en_functional_state_t enNewState)
{
    uint32_t *pu32BitSel = (uint32_t *)(uint32_t)pstcBitSel;
    uint32_t u32RegIndex = 0UL;
    uint32_t u32ErrorBitTypeCount = sizeof(stc_efm_ecc_err_inject_bit_t) / sizeof(uint32_t);
    __IO uint32_t *pu32RegBaseAddr[2U];
    __IO uint32_t *pu32RegAddr;
    uint32_t i, j;

    if (pstcBitSel != NULL) {
        DDL_ASSERT(IS_EFM_REG_UNLOCK());
        DDL_ASSERT(IS_EFM_CHIP(u32Chip));
        DDL_ASSERT(IS_EFM_ECC_ERR_INJECT_BIT_ECC_DATA(pstcBitSel->u32ECCDataBit0_8));
        DDL_ASSERT(IS_EFM_ECC_ERR_INJECT_BIT_ADDR(pstcBitSel->u32AddrBit0_19));

        pu32RegBaseAddr[0U] = &CM_EFM->F0EIR0;
        pu32RegBaseAddr[1U] = &CM_EFM->F1EIR0;

        for (i = 0UL; i < EFM_CHIP_COUNT; i++) {
            if ((u32Chip & (1UL << i)) == 0UL) {
                continue;
            }
            for (j = 0UL; j < u32ErrorBitTypeCount; j++) {
                pu32RegAddr = pu32RegBaseAddr[i] + u32RegIndex;
                if (pu32BitSel < &pstcBitSel->u32ECCDataBit0_8) {
                    u32RegIndex++;
                } else {
                    u32RegIndex = EFM_ECC_ERR_INJECT_REG_IDX_ECC_VALUE;
                }

                if (enNewState == ENABLE) {
                    SET_REG32_BIT(*pu32RegAddr, *pu32BitSel);
                } else {
                    CLR_REG32_BIT(*pu32RegAddr, *pu32BitSel);
                }
                pu32BitSel++;
            }
        }
    }
}
/**
 * @}
 */

#endif  /* LL_EFM_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
