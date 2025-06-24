/**
 *******************************************************************************
 * @file  hc32_ll_efm.h
 * @brief This file contains all the functions prototypes of the EFM driver
 *        library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2024-09-13       CDT             First version
   2024-10-17       CDT             Add const before buffer pointer to cater top-level calls
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
#ifndef __HC32_LL_EFM_H__
#define __HC32_LL_EFM_H__

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
 * @addtogroup LL_EFM
 * @{
 */

#if (LL_EFM_ENABLE == DDL_ON)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 * @defgroup EFM_Global_Types EFM Global Types
 * @{
 */
/**
 * @brief EFM unique ID definition
 */
typedef struct {
    uint32_t            u32UniqueID0;      /*!< unique ID 0.       */
    uint32_t            u32UniqueID1;      /*!< unique ID 1.       */
    uint32_t            u32UniqueID2;      /*!< unique ID 2.       */
} stc_efm_unique_id_t;

typedef struct {
    uint32_t u32State;
    uint32_t u32Addr;
    uint32_t u32Size;
} stc_efm_remap_init_t;

/**
 * @brief EFM chip ECC config definition
 */
typedef struct {
    uint32_t u32CheckMode;                   /*!< ECC mode @ref EFM_ECC_Mode
                                                Default: EFM_ECC_MD2 */
    uint32_t u32ExceptionType;               /*!< ECC exception type @ref EFM_ECC_Exception_Type
                                                Default: EFM_ECC_EXP_TYPE_NMI */
    en_functional_state_t enAutoGenerate;    /*!< ECC auto generation when programming.
                                                 - ENABLE:  Using auto generated ECC.
                                                 - DISABLE: Using ECC from ECC data register.
                                                Default: ENABLE. */
    en_functional_state_t enAutoCheck;       /*!< ECC auto check.
                                                 - ENABLE: Performs ECC checks automatically.
                                                 - DISABLE: Requires manual ECC checks.
                                                Default: ENABLE. */
} stc_efm_ecc_chip_config_t;

/**
 * @brief EFM ECC config definition
 */
typedef struct {
    en_functional_state_t enBlankEcc;       /*!< Writing blank ECC during blank data(128 bits 1) writes.
                                                 - ENABLE:  ECC is blank(0x1FF).
                                                 - DISABLE: ECC is from calculation or ECC data register.
                                                 Default: DISABLE. @see EFM_ECC_Write  */
    en_functional_state_t enCheckBlankEcc;  /*!< ECC checking when reading blank data and blank ecc(137 bits 1).
                                                 - ENABLE:  Check
                                                 - DISABLE: Do not check
                                                 Default: DISABLE. */
    stc_efm_ecc_chip_config_t stcChip0;     /*!< EFM Chip 0 ECC config @ref stc_efm_ecc_chip_config_t */
    stc_efm_ecc_chip_config_t stcChip1;     /*!< EFM Chip 1 ECC config @ref stc_efm_ecc_chip_config_t */
} stc_efm_ecc_config_t;

/**
 * @brief EFM ECC error record definition
 */
typedef struct {
    uint32_t u32IsValid: 1;                  /*!< Record is valid or not:
                                                 1: valid, 0: not valid */
    uint32_t u32IsFatal: 1;                  /*!< Error is fatal or not:
                                                 1: fatal, 0: not fatal */
    uint32_t u32AddrOffset: 20;              /*!< Offset address of a single flash chip */
    uint32_t u32IsRescueSector: 1;           /*!< Address is belongs to rescue sector or not:
                                                 1: rescue sector, 0: not rescue sector */
    uint32_t u32IsSpecialFuncSector: 1;      /*!< Address is in range [0x3000000:0x300FFFF] or not:
                                                 1: in range, 0: out of range */
    uint32_t u32EfmChip: 2;                  /*!< EFM_CHIP0 or EFM_CHIP1 */
    uint32_t u32ErrorID: 2;                  /*!< EFM_ECC_ERR_REC_ID0 or EFM_ECC_ERR_REC_ID1 */
    uint32_t u32Reserved0: 4;
} stc_efm_ecc_err_record_t;

/**
 * @brief EFM ECC error injection bits definition
 * @note  Bit Mask for data @ref EFM_ECC_BIT_MASK_WORD
 *        Bit Mask for ECC data @ref EFM_ECC_BIT_MASK_9BIT_ECC_DATA
 *        Bit Mask for data address @ref EFM_ECC_BIT_MASK_20BIT_ADDR
 */
typedef struct {
    uint32_t u32DataBit0_31;                /*!< data bits 0~31 */
    uint32_t u32DataBit32_63;               /*!< data bits 32~63 */
    uint32_t u32DataBit64_95;               /*!< data bits 64~95 */
    uint32_t u32DataBit96_127;              /*!< data bits 96~127 */
    uint32_t u32ECCDataBit0_8;              /*!< ECC value bits 0~8 */
    uint32_t u32AddrBit0_19;                /*!< data address bits 0~19 */
} stc_efm_ecc_err_inject_bit_t;

/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup EFM_Global_Macros EFM Global Macros
 * @{
 */
/**
 * @defgroup EFM_Address EFM Address Area
 * @{
 */
#define EFM_START_ADDR                  (0x00000000UL)    /*!< Flash start address */

#define EFM_END_ADDR                    (0x001FFFFFUL)    /*!< Flash end address */
#define EFM_FLASH_1_START_ADDR          (0x00100000UL)
#define EFM_OTP_START_ADDR1             (0x00000000UL)    /*!< OTP start address */
#define EFM_OTP_END_ADDR1               (0x0001FFFFUL)
#define EFM_OTP_START_ADDR              (0x03000000UL)
#define EFM_OTP_END_ADDR                (0x030017FFUL)    /*!< OTP end address */
#define EFM_OTP_LOCK_ADDR_START         (0x03001800UL)    /*!< OTP lock address */
#define EFM_OTP_LOCK_ADDR_END           (0x03001F6FUL)    /*!< OTP lock address */
#define EFM_OTP_ENABLE_ADDR             (0x03001FF0UL)    /*!< OTP Enable address */
#define EFM_SECURITY_START_ADDR         (0x03004000UL)    /*!< Flash security start address */
#define EFM_SECURITY_END_ADDR           (0x0300400BUL)    /*!< Flash security end address */

/**
 * @}
 */

/**
 * @defgroup EFM_Chip_Sel EFM Chip Selection
 * @{
 */
#define EFM_CHIP0                       (EFM_FSTP_F0STP)
#define EFM_CHIP1                       (EFM_FSTP_F1STP)
#define EFM_CHIP_ALL                    (EFM_FSTP_F0STP | EFM_FSTP_F1STP)

/**
 * @}
 */

/**
 * @defgroup EFM_Chip_Count EFM Chip Count
 * @{
 */
#define EFM_CHIP_COUNT                  (2UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Sector_Count EFM Sector Count
 * @{
 */
#define EFM_SECTOR_COUNT_SINGLE_CHIP    (128U)
#define EFM_SECTOR_COUNT_ALL_CHIPS      (256U)
/**
 * @}
 */

/**
 * @defgroup EFM_Bus_Status EFM Bus Status
 * @{
 */
#define EFM_BUS_HOLD                    (0x0UL)     /*!< Bus busy while flash program or erase */
#define EFM_BUS_RELEASE                 (0x1UL)     /*!< Bus release while flash program or erase */
/**
 * @}
 */

/**
 * @defgroup EFM_Wait_Cycle EFM Wait Cycle
 * @{
 */

#define EFM_WAIT_CYCLE0                 (0U << EFM_FRMC_FLWT_POS)      /*!< Don't insert read wait cycle */
#define EFM_WAIT_CYCLE1                 (1U << EFM_FRMC_FLWT_POS)      /*!< Insert 1 read wait cycle     */

#define EFM_WAIT_CYCLE2                 (2U << EFM_FRMC_FLWT_POS)      /*!< Insert 2 read wait cycles    */
#define EFM_WAIT_CYCLE3                 (3U << EFM_FRMC_FLWT_POS)      /*!< Insert 3 read wait cycles    */
#define EFM_WAIT_CYCLE4                 (4U << EFM_FRMC_FLWT_POS)      /*!< Insert 4 read wait cycles    */
#define EFM_WAIT_CYCLE5                 (5U << EFM_FRMC_FLWT_POS)      /*!< Insert 5 read wait cycles    */
#define EFM_WAIT_CYCLE6                 (6U << EFM_FRMC_FLWT_POS)      /*!< Insert 6 read wait cycles    */
#define EFM_WAIT_CYCLE7                 (7U << EFM_FRMC_FLWT_POS)      /*!< Insert 7 read wait cycles    */
#define EFM_WAIT_CYCLE8                 (8U << EFM_FRMC_FLWT_POS)      /*!< Insert 8 read wait cycles    */
#define EFM_WAIT_CYCLE9                 (9U << EFM_FRMC_FLWT_POS)      /*!< Insert 9 read wait cycles    */
#define EFM_WAIT_CYCLE10                (10U << EFM_FRMC_FLWT_POS)     /*!< Insert 10 read wait cycles   */
#define EFM_WAIT_CYCLE11                (11U << EFM_FRMC_FLWT_POS)     /*!< Insert 11 read wait cycles   */
#define EFM_WAIT_CYCLE12                (12U << EFM_FRMC_FLWT_POS)     /*!< Insert 12 read wait cycles   */
#define EFM_WAIT_CYCLE13                (13U << EFM_FRMC_FLWT_POS)     /*!< Insert 13 read wait cycles   */
#define EFM_WAIT_CYCLE14                (14U << EFM_FRMC_FLWT_POS)     /*!< Insert 14 read wait cycles   */
#define EFM_WAIT_CYCLE15                (15U << EFM_FRMC_FLWT_POS)     /*!< Insert 15 read wait cycles   */
/**
 * @}
 */

/**
 * @defgroup EFM_Read_Accelerator_Command EFM Read Accelerator Command
 * @{
 */
#define EFM_RD_ACCL_CMD_ICACHE          (EFM_FRMC_ICACHE)   /* ICACHE command */
#define EFM_RD_ACCL_CMD_DCACHE          (EFM_FRMC_DCACHE)   /* DCACHE command */
#define EFM_RD_ACCL_CMD_PREFETCH        (EFM_FRMC_PREFETE)  /* PREFETCH command */
#define EFM_RD_ACCL_CMD_ACTIVATION      (EFM_FRMC_CRST)     /* Activation(BUFFER,CACHE,PREFETCH) command */
#define EFM_RD_ACCL_CMD_ALL             (EFM_FRMC_PREFETE | EFM_FRMC_DCACHE | EFM_FRMC_ICACHE | EFM_FRMC_CRST)
/**
 * @}
 */

/**
 * @defgroup EFM_Swap_Address EFM Swap Address
 * @{
 */
#define EFM_SWAP_ADDR                   (0x03002000UL)
#define EFM_SWAP_DATA                   (0x005A5A5AUL)
/**
 * @}
 */

/**
 * @defgroup EFM_WriteLock_Sel EFM Write Protect Lock Selection
 * @{
 */
#define EFM_WRLOCK0                     (EFM_WLOCK_WLOCK_0)     /*!< F0NWPRT0 controlled sector lock   */
#define EFM_WRLOCK1                     (EFM_WLOCK_WLOCK_1)     /*!< F0NWPRT1 controlled sector lock   */
#define EFM_WRLOCK2                     (EFM_WLOCK_WLOCK_2)     /*!< F0NWPRT2 controlled sector lock   */
#define EFM_WRLOCK3                     (EFM_WLOCK_WLOCK_3)     /*!< F0NWPRT3 controlled sector lock   */
#define EFM_WRLOCK4                     (EFM_WLOCK_WLOCK_4)     /*!< F1NWPRT0 controlled sector lock   */
#define EFM_WRLOCK5                     (EFM_WLOCK_WLOCK_5)     /*!< F1NWPRT1 controlled sector lock   */
#define EFM_WRLOCK6                     (EFM_WLOCK_WLOCK_6)     /*!< F1NWPRT2 controlled sector lock   */
#define EFM_WRLOCK7                     (EFM_WLOCK_WLOCK_7)     /*!< F1NWPRT3 controlled sector lock   */
/**
 * @}
 */

/**
 * @defgroup EFM_OperateMode_Sel EFM Operate Mode Selection
 * @{
 */
#define EFM_MD_READONLY                 (0x0UL << EFM_FWMC_PEMOD_POS)   /*!< Read only mode               */
#define EFM_MD_PGM_SINGLE               (0x1UL << EFM_FWMC_PEMOD_POS)   /*!< Program single mode          */
#define EFM_MD_PGM_READBACK             (0x2UL << EFM_FWMC_PEMOD_POS)   /*!< Program and read back mode   */
#define EFM_MD_PGM_SEQ                  (0x3UL << EFM_FWMC_PEMOD_POS)   /*!< Program sequence mode        */
#define EFM_MD_ERASE_SECTOR             (0x4UL << EFM_FWMC_PEMOD_POS)   /*!< Sector erase mode            */

#define EFM_MD_ERASE_ONE_CHIP           (0x5UL << EFM_FWMC_PEMOD_POS)   /*!< A flash Chip erase mode      */
#define EFM_MD_ERASE_ALL_CHIP           (0x6UL << EFM_FWMC_PEMOD_POS)   /*!< All chip erase mode    */

/**
 * @}
 */

/**
 * @defgroup EFM_PGM_Definition EFM PGM definition
 * @{
 */
#define EFM_PGM_UNIT_BYTES              (16UL)
#define EFM_PGM_UNIT_WORDS              (EFM_PGM_UNIT_BYTES / 4U)

#ifndef EFM_PGM_PAD_BYTE
#define EFM_PGM_PAD_BYTE                (0xFFU)
#endif
#ifndef EFM_SECURITY_CODE_PAD_BYTE
#define EFM_SECURITY_CODE_PAD_BYTE      (0xFFU)
#endif
/**
 * @}
 */

/**
 * @defgroup EFM_Flag_Sel  EFM Flag Selection
 * @{
 */
#define EFM_FLAG_OTPWERR                (EFM_FSR_OTPWERR0)      /*!< EFM Flash0 otp Programming/erase error flag.       */
#define EFM_FLAG_PEPRTERR               (EFM_FSR_PRTWERR0)      /*!< EFM Flash0 write protect address error flag.       */
#define EFM_FLAG_PGSZERR                (EFM_FSR_PGSZERR0)      /*!< EFM Flash0 programming size error flag.            */
#define EFM_FLAG_PGMISMTCH              (EFM_FSR_MISMTCH0)      /*!< EFM Flash0 programming missing match error flag.   */
#define EFM_FLAG_OPTEND                 (EFM_FSR_OPTEND0)       /*!< EFM Flash0 end of operation flag.                  */
#define EFM_FLAG_COLERR                 (EFM_FSR_COLERR0)       /*!< EFM Flash0 read collide error flag.                */
#define EFM_FLAG_RDY                    (EFM_FSR_RDY0)          /*!< EFM Flash0 ready flag.                             */
#define EFM_FLAG_PEPRTERR1              (EFM_FSR_PRTWERR1)      /*!< EFM Flash1 write protect address error flag.       */
#define EFM_FLAG_PGSZERR1               (EFM_FSR_PGSZERR1)      /*!< EFM Flash1 programming size error flag.            */
#define EFM_FLAG_PGMISMTCH1             (EFM_FSR_MISMTCH1)      /*!< EFM Flash1 programming missing match error flag.   */
#define EFM_FLAG_OPTEND1                (EFM_FSR_OPTEND1)       /*!< EFM Flash1 end of operation flag.                  */
#define EFM_FLAG_COLERR1                (EFM_FSR_COLERR1)       /*!< EFM Flash1 read collide error flag.                */
#define EFM_FLAG_RDY1                   (EFM_FSR_RDY1)          /*!< EFM Flash1 ready flag.                             */
#define EFM_FLAG_ECC_OVF0               (EFM_FSR_ECEROF0)       /*!< EFM Flash0 ECC error record overflow.                             */
#define EFM_FLAG_BLANK_RD_ERR0          (EFM_FSR_BRMER0)        /*!< EFM Flash0 is not blank.                             */
#define EFM_FLAG_ECC_OVF1               (EFM_FSR_ECEROF1)       /*!< EFM Flash1 ECC error record overflow.                             */
#define EFM_FLAG_BLANK_RD_ERR1          (EFM_FSR_BRMER1)        /*!< EFM Flash1 is not blank.                             */

#define EFM_FLAG_FLASH_RDY              (EFM_FLAG_RDY | EFM_FLAG_RDY1)

#define EFM_FLAG_WRITE_ERR0             (EFM_FLAG_PEPRTERR | EFM_FLAG_PGSZERR | EFM_FLAG_PGMISMTCH | \
                                         EFM_FLAG_COLERR | EFM_FLAG_OTPWERR)
#define EFM_FLAG_WRITE_ERR1             (EFM_FLAG_PEPRTERR1 | EFM_FLAG_PGSZERR1 | EFM_FLAG_PGMISMTCH1 | \
                                         EFM_FLAG_COLERR1)
#define EFM_FLAG_WRITE0                 (EFM_FLAG_WRITE_ERR0 | EFM_FLAG_OPTEND)
#define EFM_FLAG_WRITE1                 (EFM_FLAG_WRITE_ERR1 | EFM_FLAG_OPTEND1)
#define EFM_FLAG_WRITE                  (EFM_FLAG_WRITE0 | EFM_FLAG_WRITE1)

#define EFM_FLAG_ECC                    (EFM_FLAG_ECC_OVF0 | EFM_FLAG_ECC_OVF1)
#define EFM_FLAG_BLANK_RD               (EFM_FLAG_BLANK_RD_ERR0 | EFM_FLAG_BLANK_RD_ERR1)
#define EFM_FLAG_ALL                    (EFM_FLAG_FLASH_RDY | EFM_FLAG_WRITE | EFM_FLAG_ECC | EFM_FLAG_BLANK_RD)

/**
 * @}
 */

/**
 * @defgroup EFM_Interrupt_Sel EFM Interrupt Selection
 * @{
 */
#define EFM_INT_PEERR                   (EFM_FITE_PEERRITE)     /*!< Program/erase error Interrupt source    */
#define EFM_INT_OPTEND                  (EFM_FITE_OPTENDITE)    /*!< End of EFM operation Interrupt source   */
#define EFM_INT_COLERR                  (EFM_FITE_COLERRITE)    /*!< Read collide error Interrupt source     */

#define EFM_INT_ALL                     (EFM_FITE_PEERRITE | EFM_FITE_OPTENDITE | EFM_FITE_COLERRITE)
/**
 * @}
 */

/**
 * @defgroup EFM_Cache_Mask EFM Cache Bit Mask
 * @{
 */
#define EFM_CACHE_ALL                   (EFM_FRMC_CRST | EFM_FRMC_PREFETE | EFM_FRMC_DCACHE | EFM_FRMC_ICACHE)

/**
 * @}
 */

/**
 * @defgroup EFM_Keys EFM Keys
 * @{
 */
#define EFM_REG_UNLOCK_KEY1             (0x0123UL)
#define EFM_REG_UNLOCK_KEY2             (0x3210UL)
#define EFM_REG_LOCK_KEY                (0x0000UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Sector_Size EFM Sector Size
 * @{
 */
#define EFM_SECTOR_SIZE                 (0x2000UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Sector_Address EFM Sector Address
 * @{
 */
#define EFM_SECTOR_ADDR(x)          (uint32_t)(EFM_SECTOR_SIZE * (x))
/**
 * @}
 */

/**
 * @defgroup EFM_OTP_Base_Address EFM Otp Base Address
 * @{
 */
#define EFM_OTP_BASE1_ADDR          (0x00000000UL)
#define EFM_OTP_BASE1_SIZE          (8*1024UL)
#define EFM_OTP_BASE1_OFFSET        (0UL)
#define EFM_OTP_BASE2_ADDR          (0x03000000UL)
#define EFM_OTP_BASE2_SIZE          (16UL)
#define EFM_OTP_BASE2_OFFSET        (16UL)
#define EFM_OTP_BASE3_ADDR          (0x03000600UL)
#define EFM_OTP_BASE3_SIZE          (512UL)
#define EFM_OTP_BASE3_OFFSET        (112UL)
#define EFM_OTP_BASE4_ADDR          (0x03000800UL)
#define EFM_OTP_BASE4_SIZE          (2*1024UL)
#define EFM_OTP_BASE4_OFFSET        (113UL)
#define EFM_OTP_BASE5_ADDR          (0x03001000UL)
#define EFM_OTP_BASE5_SIZE          (256UL)
#define EFM_OTP_BASE5_OFFSET        (114UL)
#define EFM_OTP_BASE6_ADDR          (0x03001400UL)
#define EFM_OTP_BASE6_SIZE          (1*1024UL)
#define EFM_OTP_BASE6_OFFSET        (118UL)
#define EFM_OTP_LOCK_ADDR           (0x03001800UL)

/**
 * @}
 */

/**
 * @defgroup EFM_OTP_Block_Index_Definition EFM Otp Block Index Definition
 * @{
 */
#define EFM_OTP_BLOCK_IDX_MAX        (118UL)
#define EFM_OTP_BLOCK_IDX_INVALID    (0xFFFFFFFFUL)
#define EFM_OTP_BLOCK_IDX(Addr)        \
(\
    ((Addr <= EFM_OTP_BASE2_ADDR) ? \
    (Addr / EFM_OTP_BASE1_SIZE): (Addr < EFM_OTP_BASE3_OFFSET) ? \
    (Addr / EFM_OTP_BASE2_SIZE): (Addr < EFM_OTP_BASE4_OFFSET) ? \
    (Addr / EFM_OTP_BASE3_SIZE): (Addr < EFM_OTP_BASE5_OFFSET) ? \
    (Addr / EFM_OTP_BASE4_SIZE): (Addr < EFM_OTP_BASE6_OFFSET) ? \
    (Addr / EFM_OTP_BASE5_SIZE): (Addr < EFM_OTP_LOCK_ADDR) ?\
    (Addr / EFM_OTP_BASE6_SIZE): EFM_OTP_BLOCK_IDX_INVALID \
)\
)
/**
 * @}
 */

/**
 * @defgroup EFM_OTP_Block_Base_Address_Definition EFM Otp Block Base Address Definition
 * @{
 */
#define EFM_OTP_BLOCK_BASE_ADDR_INVALID   (0xFFFFFFFFUL)
#define EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx, BaseAddr, BaseOffset, BaseSize) \
    (BaseAddr + (((BlockIdx) - BaseOffset) * BaseSize))

#define EFM_OTP_BLOCK_BASE_ADDR(BlockIdx)             \
(\
    (BlockIdx < EFM_OTP_BASE2_OFFSET) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE1_ADDR,EFM_OTP_BASE1_OFFSET,EFM_OTP_BASE1_SIZE): \
    (BlockIdx < EFM_OTP_BASE3_OFFSET) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE2_ADDR,EFM_OTP_BASE2_OFFSET,EFM_OTP_BASE2_SIZE): \
    (BlockIdx < EFM_OTP_BASE4_OFFSET) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE3_ADDR,EFM_OTP_BASE3_OFFSET,EFM_OTP_BASE3_SIZE): \
    (BlockIdx < EFM_OTP_BASE5_OFFSET) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE4_ADDR,EFM_OTP_BASE4_OFFSET,EFM_OTP_BASE4_SIZE): \
    (BlockIdx < EFM_OTP_BASE6_OFFSET) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE5_ADDR,EFM_OTP_BASE5_OFFSET,EFM_OTP_BASE5_SIZE): \
    (BlockIdx <= EFM_OTP_BLOCK_IDX_MAX) ? \
    EFM_OTP_CALC_BLOCK_BASE_ADDR(BlockIdx,EFM_OTP_BASE6_ADDR,EFM_OTP_BASE6_OFFSET,EFM_OTP_BASE6_SIZE): \
    EFM_OTP_BLOCK_BASE_ADDR_INVALID\
)
/**
 * @}
 */

/**
 * @defgroup EFM_OTP_Lock_Address EFM Otp Lock_address
 *          x: otp block index, range from 0 to @ref EFM_OTP_BLOCK_IDX_MAX
 * @{
 */
#define EFM_OTP_BLOCK_LOCKADDR(x)    (EFM_OTP_LOCK_ADDR + 0x10UL * (x))   /*!< OTP block x  lock address */
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_Reg_Write_Protection Write Protection Keys For EFM Remap Registers
 * @{
 */
#define EFM_REMAP_REG_LOCK_KEY      (0x0000UL)
#define EFM_REMAP_REG_UNLOCK_KEY1   (0x0123UL)
#define EFM_REMAP_REG_UNLOCK_KEY2   (0x3210UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_State EFM remap function state
 * @{
 */
#define EFM_REMAP_OFF               (0UL)
#define EFM_REMAP_ON                (EFM_MMF_REMCR_EN)
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_Size EFM remap size definition
 * @note refer to chip user manual for details size spec.
 * @{
 */
#define EFM_REMAP_4K                (12UL)
#define EFM_REMAP_8K                (13UL)
#define EFM_REMAP_16K               (14UL)
#define EFM_REMAP_32K               (15UL)
#define EFM_REMAP_64K               (16UL)
#define EFM_REMAP_128K              (17UL)
#define EFM_REMAP_256K              (18UL)
#define EFM_REMAP_512K              (19UL)
#define EFM_REMAP_SIZE_MAX          EFM_REMAP_512K
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_Index EFM remap index
 * @{
 */
#define EFM_REMAP_IDX0              (0U)
#define EFM_REMAP_IDX1              (1U)
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_Base_Addr EFM remap base address
 * @{
 */
#define EFM_REMAP_BASE_ADDR0        (0x2000000UL)
#define EFM_REMAP_BASE_ADDR1        (0x2080000UL)
/**
 * @}
 */

/**
 * @defgroup EFM_Remap_Region EFM remap ROM/RAM region
 * @{
 */
#define EFM_REMAP_ROM_END_ADDR      EFM_END_ADDR

#define EFM_REMAP_RAM_START_ADDR    (0x1FFE0000UL)
#define EFM_REMAP_RAM_END_ADDR      (0x1FFFFFFFUL)
/**
 * @}
 */

/**
 * @defgroup EFM_Protect_Level EFM protect level
 * @{
 */
#define EFM_PROTECT_LEVEL1          (1UL << 0UL)
#define EFM_PROTECT_LEVEL2          (1UL << 1UL)
#define EFM_PROTECT_LEVEL3          (1UL << 2UL)
#define EFM_PROTECT_LEVEL_ALL       (EFM_PROTECT_LEVEL1 | EFM_PROTECT_LEVEL2 | EFM_PROTECT_LEVEL3)

/**
 * @}
 */

/**
 * @defgroup EFM_MCU_Status EFM protect level
 * @{
 */
#define EFM_MCU_PROTECT1_FREE       (0U)
#define EFM_MCU_PROTECT1_LOCK       (1U)
#define EFM_MCU_PROTECT1_UNLOCK     (2U)
#define EFM_MCU_PROTECT2_LOCK       (4U)
/**
 * @}
 */

/**
 * @defgroup EFM_Check_Flag EFM Check Flag
 * @{
 */
#define EFM_CHECK_FLAG_ECC_CHIP0_1BIT_ERR                   (EFM_CKSR_F0_1ERR)
#define EFM_CHECK_FLAG_ECC_CHIP0_2BIT_ERR                   (EFM_CKSR_F0_2ERR)
#define EFM_CHECK_FLAG_ECC_CHIP0_ALL                        (EFM_CKSR_F0_1ERR | EFM_CKSR_F0_2ERR)
#define EFM_CHECK_FLAG_ECC_CHIP1_1BIT_ERR                   (EFM_CKSR_F1_1ERR)
#define EFM_CHECK_FLAG_ECC_CHIP1_2BIT_ERR                   (EFM_CKSR_F1_2ERR)
#define EFM_CHECK_FLAG_ECC_CHIP1_ALL                        (EFM_CKSR_F1_1ERR | EFM_CKSR_F1_2ERR)
#define EFM_CHECK_FLAG_ECC_ALL                              (EFM_CHECK_FLAG_ECC_CHIP0_ALL | EFM_CHECK_FLAG_ECC_CHIP1_ALL)
/**
 * @}
 */

/**
 * @defgroup EFM_ECC_Mode EFM ECC Mode
 *  @{
 */

/**
 * @note
 @verbatim
 * ------------------------------------------------------------------------
 *  ECC Mode | Error type  | Correct or | Set error flag  | Generate NMI |
 *           |             | detect?    | ?               | or reset ?   |
 * ------------------------------------------------------------------------
 *  INVD       1-bit-error    no           no               no
 *             2-bit-error    no           no               no
 * ------------------------------------------------------------------------
 *  MD1        1-bit-error    correct      no               no
 *             2-bit-error    detect       yes              yes
 * ------------------------------------------------------------------------
 *  MD2        1-bit-error    correct      yes              no
 *             2-bit-error    detect       yes              yes
 * ------------------------------------------------------------------------
 *  MD3        1-bit-error    correct      yes              yes
 *             2-bit-error    detects      yes              yes
 * ------------------------------------------------------------------------
 @endverbatim
 */
#define EFM_ECC_MD_INVD                                     (0UL)
#define EFM_ECC_MD1                                         (1UL)
#define EFM_ECC_MD2                                         (2UL)
#define EFM_ECC_MD3                                         (3UL)
/**
 * @}
 */

/**
 * @defgroup EFM_ECC_Exception_Type EFM ECC Exception Type
 * @{
 */
#define EFM_ECC_EXP_TYPE_NMI                                (0UL)
#define EFM_ECC_EXP_TYPE_RESET                              (1UL)
/**
 * @}
 */

/**
 * @defgroup EFM_ECC_Error_Record_ID EFM ECC Error Record ID
 * @{
 */
#define EFM_ECC_ERR_REC_ID0                                 (1UL<<0UL)
#define EFM_ECC_ERR_REC_ID1                                 (1UL<<1UL)
#define EFM_ECC_ERR_REC_ALL                                 (EFM_ECC_ERR_REC_ID0 | EFM_ECC_ERR_REC_ID1)
/**
 * @}
 */

/**
 * @defgroup EFM_ECC_Bit_Mask EFM ECC Bit Mask
 * @{
 */
#define EFM_ECC_BIT_MASK_WORD                               (0xFFFFFFFFUL)
#define EFM_ECC_BIT_MASK_9BIT_ECC_DATA                      (0x1FFUL)
#define EFM_ECC_BIT_MASK_20BIT_ADDR                         (0xFFFFF000UL)
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
 * @addtogroup EFM_Global_Functions
 * @{
 */

/**
 * @brief  EFM Protect Unlock.
 * @param  None
 * @retval None
 */

__STATIC_INLINE void EFM_REG_Unlock(void)
{
    WRITE_REG32(CM_EFM->FAPRT, EFM_REG_UNLOCK_KEY1);
    WRITE_REG32(CM_EFM->FAPRT, EFM_REG_UNLOCK_KEY2);
}

/**
 * @brief  EFM Protect Lock.
 * @param  None
 * @retval None
 */
__STATIC_INLINE void EFM_REG_Lock(void)
{
    WRITE_REG32(CM_EFM->FAPRT, EFM_REG_LOCK_KEY);
}

/**
 * @brief  EFM remap Unlock.
 * @param  None
 * @retval None
 */
__STATIC_INLINE void EFM_REMAP_Unlock(void)
{
    WRITE_REG32(CM_EFM->MMF_REMPRT, EFM_REMAP_REG_UNLOCK_KEY1);
    WRITE_REG32(CM_EFM->MMF_REMPRT, EFM_REMAP_REG_UNLOCK_KEY2);
}

/**
 * @brief  EFM remap Lock.
 * @param  None
 * @retval None
 */
__STATIC_INLINE void EFM_REMAP_Lock(void)
{
    WRITE_REG32(CM_EFM->MMF_REMPRT, EFM_REMAP_REG_LOCK_KEY);
}

void EFM_Cmd(uint32_t u32Flash, en_functional_state_t enNewState);
void EFM_FWMC_Cmd(en_functional_state_t enNewState);
void EFM_SetBusStatus(uint32_t u32Status);
void EFM_IntCmd(uint32_t u32EfmInt, en_functional_state_t enNewState);
void EFM_ClearStatus(uint32_t u32Flag);
int32_t EFM_SetWaitCycle(uint32_t u32WaitCycle);
int32_t EFM_SetOperateMode(uint32_t u32Mode);
int32_t EFM_ReadByte(uint32_t u32Addr, uint8_t *pu8ReadBuf, uint32_t u32ByteLen);
int32_t EFM_BlankRead(uint32_t u32Chip);
int32_t EFM_Program(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen);
int32_t EFM_SequenceProgram(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen);
int32_t EFM_ProgramReadBack(uint32_t u32Addr, const uint8_t *pu8DataSrc, uint32_t u32ByteLen);
int32_t EFM_ChipErase(uint32_t u32Chip);

int32_t EFM_SectorErase(uint32_t u32Addr);

en_flag_status_t EFM_GetAnyStatus(uint32_t u32Flag);
en_flag_status_t EFM_GetStatus(uint32_t u32Flag);
void EFM_GetUID(stc_efm_unique_id_t *pstcUID);

void EFM_CacheRamReset(en_functional_state_t enNewState);
void EFM_PrefetchCmd(en_functional_state_t enNewState);
void EFM_DCacheCmd(en_functional_state_t enNewState);
void EFM_ICacheCmd(en_functional_state_t enNewState);
void EFM_ReadAcceleratorCmd(uint32_t u32CmdType, en_functional_state_t enNewState);
void EFM_LowVoltageReadCmd(en_functional_state_t enNewState);
int32_t EFM_SwapCmd(en_functional_state_t enNewState);
en_flag_status_t EFM_GetSwapStatus(void);
int32_t EFM_OTP_Lock(uint32_t u32Addr);

int32_t EFM_REMAP_StructInit(stc_efm_remap_init_t *pstcEfmRemapInit);
int32_t EFM_REMAP_Init(uint8_t u8RemapIdx, stc_efm_remap_init_t *pstcEfmRemapInit);
void EFM_REMAP_DeInit(void);
void EFM_REMAP_Cmd(uint8_t u8RemapIdx, en_functional_state_t enNewState);
void EFM_REMAP_SetAddr(uint8_t u8RemapIdx, uint32_t u32Addr);
void EFM_REMAP_SetSize(uint8_t u8RemapIdx, uint32_t u32Size);

uint32_t EFM_GetCID(void);
void EFM_OTP_WP_Unlock(void);
void EFM_OTP_WP_Lock(void);
int32_t EFM_OTP_Enable(void);
en_flag_status_t EFM_GetOTPStatus(void);
int32_t EFM_OTP_LockBlock(uint32_t u32BlockStartIdx, uint16_t u16Count);
void EFM_SectorProtectRegLock(uint32_t u32RegLock);
void EFM_SingleSectorOperateCmd(uint8_t u8SectorNum, en_functional_state_t enNewState);
void EFM_SequenceSectorOperateCmd(uint32_t u32StartSectorNum, uint16_t u16Count, en_functional_state_t enNewState);

int32_t EFM_Protect_Enable(uint8_t u8Level);
int32_t EFM_WriteSecurityCode(const uint8_t *pu8Code, uint32_t u32Len);

void EFM_CKCR_Unlock(void);
void EFM_CKCR_Lock(void);
en_flag_status_t EFM_GetCheckStatus(uint32_t u32Flag);
void EFM_ClearCheckStatus(uint32_t u32Flag);

int32_t EFM_ECC_StructInit(stc_efm_ecc_config_t *pstcEccConfig);
int32_t EFM_ECC_Config(const stc_efm_ecc_config_t *pstcEccConfig);

void EFM_ECC_Write(uint32_t u32Chip, uint32_t u32Ecc);
uint32_t EFM_ECC_Read(uint32_t u32Chip);

stc_efm_ecc_err_record_t *EFM_ECC_GetErrorRecord(uint32_t u32Chip, uint32_t u32Record);
void EFM_ECC_ClearErrorRecord(uint32_t u32Chip, uint32_t u32Record);
void EFM_ECC_ErrorInjectCmd(uint32_t u32Chip, en_functional_state_t enNewState);
void EFM_ECC_ErrorInjectBitCmd(uint32_t u32Chip, const stc_efm_ecc_err_inject_bit_t *pstcBitSel, en_functional_state_t enNewState);

/**
 * @}
 */

#endif /* LL_EFM_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __HC32_LL_EFM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
