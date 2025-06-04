/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */


/******************************************************************************
 * @file     core_ck610.h
 * @brief    CSI CK610 Core Peripheral Access Layer Header File
 * @version  V1.0
 * @date     02. June 2017
 ******************************************************************************/

#ifndef __CORE_CK610_H_GENERIC
#define __CORE_CK610_H_GENERIC

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *                 CSI definitions
 ******************************************************************************/
/**
  \ingroup Ck610
  @{
 */

/*  CSI CK610 definitions */
#define __CK610_CSI_VERSION_MAIN  (0x01U)                                      /*!< [31:16] CSI HAL main version */
#define __CK610_CSI_VERSION_SUB   (0x1EU)                                      /*!< [15:0]  CSI HAL sub version */
#define __CK610_CSI_VERSION       ((__CK610_CSI_VERSION_MAIN << 16U) | \
                                   __CK610_CSI_VERSION_SUB           )         /*!< CSI HAL version number */

#define __CK610                   (0x01U)                                      /*!< CK610 Core */

/** __FPU_USED indicates whether an FPU is used or not.
    This core does not support an FPU at all
*/
#define __FPU_USED       1U

#if defined ( __GNUC__ )
#if defined (__VFP_FP__) && !defined(__SOFTFP__)
#error "Compiler generates FPU instructions for a device without an FPU (check __FPU_PRESENT)"
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CORE_CK610_H_GENERIC */

#ifndef __CSI_GENERIC

#ifndef __CORE_CK610_H_DEPENDANT
#define __CORE_CK610_H_DEPENDANT

#ifdef __cplusplus
extern "C" {
#endif

/* check device defines and use defaults */
#ifndef __CK610_REV
#define __CK610_REV               0x0000U
#endif

#ifndef __GSR_GCR_PRESENT
#define __GSR_GCR_PRESENT         0U
#endif

#ifndef __MPU_PRESENT
#define __MPU_PRESENT             0U
#endif

#ifndef __ICACHE_PRESENT
#define __ICACHE_PRESENT          1U
#endif

#ifndef __DCACHE_PRESENT
#define __DCACHE_PRESENT          1U
#endif

#include <csi_gcc.h>

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CSI_glob_defs CSI Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
#define     __I      volatile             /*!< Defines 'read only' permissions */
#else
#define     __I      volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O      volatile             /*!< Defines 'write only' permissions */
#define     __IO     volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const       /*! Defines 'read only' structure member permissions */
#define     __OM     volatile             /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile             /*! Defines 'read / write' structure member permissions */

/*@} end of group CK610 */

/*******************************************************************************
 *                 Register Abstraction
  Core Register contain:
  - Core Register
  - Core MGU Register
  - Core MMU Register
 ******************************************************************************/
/**
  \defgroup CSI_core_register Defines and Type Definitions
  \brief Type definitions and defines for CK610 processor based devices.
*/

/**
  \brief  Access Processor Status Register(PSR)struct definition.
 */
typedef union {
    struct {
        uint32_t C: 1;                       /*!< bit:      0  Conditional code/Carry flag */
        uint32_t AF: 1;                      /*!< bit:      1  Alternate register valid control bit */
        uint32_t _reserved0: 2;              /*!< bit:  2.. 3  Reserved */
        uint32_t FE: 1;                      /*!< bit:      4  Fast interrupt enable control bit */
        uint32_t _reserved1: 1;              /*!< bit:      5  Reserved */
        uint32_t IE: 1;                      /*!< bit:      6  Interrupt effective control bit */
        uint32_t IC: 1;                      /*!< bit:      7  Interrupt control bit */
        uint32_t EE: 1;                      /*!< bit:      8  Abnormally effective control bit */
        uint32_t MM: 1;                      /*!< bit:      9  Unsymmetrical masking bit */
        uint32_t _reserved2: 2;              /*!< bit: 10..11  Reserved */
        uint32_t TE: 1;                      /*!< bit:     12  Trace transmission control bit */
        uint32_t TP: 1;                      /*!< bit:     13  Pending trace exception set bit */
        uint32_t TM: 2;                      /*!< bit: 14..15  Tracing mode bit */
        uint32_t VEC: 8;                     /*!< bit: 16..23  Abnormal event vector value */
        uint32_t CPID: 4;                    /*!< bit: 24..27  Number of processor currently running */
        uint32_t _reserved3: 3;              /*!< bit: 28..30  Reserved */
        uint32_t S: 1;                       /*!< bit:     31  Superuser mode set bit */
    } b;                                     /*!< Structure    Access by bit */
    uint32_t w;                              /*!< Type         Access by whole register */
} PSR_Type;

/* PSR Register Definitions */
#define PSR_S_Pos                          31U                                            /*!< PSR: S Position */
#define PSR_S_Msk                          (0x1UL << PSR_S_Pos)                           /*!< PSR: S Mask */

#define PSR_CPID_Pos                       24U                                            /*!< PSR: CPID Position */
#define PSR_CPID_Msk                       (0xFUL << PSR_CPID_Pos)                        /*!< PSR: CPID Mask */

#define PSR_VEC_Pos                        16U                                            /*!< PSR: VEC Position */
#define PSR_VEC_Msk                        (0xFFUL << PSR_VEC_Pos)                        /*!< PSR: VEC Mask */

#define PSR_TM_Pos                         14U                                            /*!< PSR: TM Position */
#define PSR_TM_Msk                         (0x3UL << PSR_TM_Pos)                          /*!< PSR: TM Mask */

#define PSR_TP_Pos                         13U                                            /*!< PSR: TP Position */
#define PSR_TP_Msk                         (0x1UL << PSR_TM_Pos)                          /*!< PSR: TP Mask */

#define PSR_TE_Pos                         12U                                            /*!< PSR: TE Position */
#define PSR_TE_Msk                         (0x1UL << PSR_TE_Pos)                          /*!< PSR: TE Mask */

#define PSR_MM_Pos                         9U                                             /*!< PSR: MM Position */
#define PSR_MM_Msk                         (0x1UL << PSR_MM_Pos)                          /*!< PSR: MM Mask */

#define PSR_EE_Pos                         8U                                             /*!< PSR: EE Position */
#define PSR_EE_Msk                         (0x1UL << PSR_EE_Pos)                          /*!< PSR: EE Mask */

#define PSR_IC_Pos                         7U                                             /*!< PSR: IC Position */
#define PSR_IC_Msk                         (0x1UL << PSR_IC_Pos)                          /*!< PSR: IC Mask */

#define PSR_IE_Pos                         6U                                             /*!< PSR: IE Position */
#define PSR_IE_Msk                         (0x1UL << PSR_IE_Pos)                          /*!< PSR: IE Mask */

#define PSR_FE_Pos                         4U                                             /*!< PSR: FE Position */
#define PSR_FE_Msk                         (0x1UL << PSR_FE_Pos)                          /*!< PSR: FE Mask */

#define PSR_AF_Pos                         1U                                             /*!< PSR: AF Position */
#define PSR_AF_Msk                         (0x1UL << PSR_AF_Pos)                          /*!< PSR: AF Mask */

#define PSR_C_Pos                          0U                                             /*!< PSR: C Position */
#define PSR_C_Msk                          (0x1UL << PSR_C_Pos)                           /*!< PSR: C Mask */

/**
  \brief Consortium definition for accessing Cache Configuration Registers(CCR, CR<18, 0>).
 */
typedef union {
    struct {
        uint32_t MP: 2;                      /*!< bit:  0..1   Memory protection settings */
        uint32_t IE: 1;                      /*!< bit:      2  Endian mode */
        uint32_t DE: 1;                      /*!< bit:      3  Endian mode */
        uint32_t WB: 1;                      /*!< bit:      4  Endian mode */
        uint32_t RS: 1;                      /*!< bit:      5  Endian mode */
        uint32_t Z:  1;                      /*!< bit:      6  Endian mode */
        uint32_t BE: 1;                      /*!< bit:      7  Endian mode */
        uint32_t SCK: 3;                     /*!< bit:  8..10  The clock ratio of the system and the processor */
        uint32_t _reserved0: 21;             /*!< bit:  11..31 Reserved */
    } b;                                     /*!< Structure    Access by bit */
    uint32_t w;                              /*!< Type         Access by whole register */
} CCR_Type;

/* CCR Register Definitions */
#define CCR_SCK_Pos                        8U                                               /*!< CCR: SCK Position */
#define CCR_SCK_Msk                        (0x7UL << CCR_SCK_Pos)                           /*!< CCR: SCK Mask */

#define CCR_BE_Pos                         7U                                               /*!< CCR: BE Position */
#define CCR_BE_Msk                         (0x1UL << CCR_BE_Pos)                            /*!< CCR: BE Mask */

#define CCR_Z_Pos                          6U                                               /*!< CCR: Z Position */
#define CCR_Z_Msk                          (0x1UL << CCR_Z_Pos)                             /*!< CCR: Z Mask */

#define CCR_RS_Pos                         5U                                               /*!< CCR: RS Position */
#define CCR_RS_Msk                         (0x1UL << CCR_RS_Pos)                            /*!< CCR: RS Mask */

#define CCR_WB_Pos                         4U                                               /*!< CCR: WB Position */
#define CCR_WB_Msk                         (0x1UL << CCR_WB_Pos)                            /*!< CCR: WB Mask */

#define CCR_DE_Pos                         3U                                               /*!< CCR: DE Position */
#define CCR_DE_Msk                         (0x1UL << CCR_DE_Pos)                            /*!< CCR: DE Mask */

#define CCR_IE_Pos                         2U                                               /*!< CCR: IE Position */
#define CCR_IE_Msk                         (0x1UL << CCR_IE_Pos)                            /*!< CCR: IE Mask */

#define CCR_MP_Pos                         0U                                               /*!< CCR: MP Position */
#define CCR_MP_Msk                         (0x3UL << CCR_MP_Pos)                            /*!< CCR: MP Mask */


/**
  \brief  Consortium definition for accessing high ease access permission configutation registers(CAPR, CR<19,0>)
 */
typedef union {
    struct {
        uint32_t C0: 1;                      /*!< bit:      0  Cacheable setting */
        uint32_t C1: 1;                      /*!< bit:      1  Cacheable setting */
        uint32_t C2: 1;                      /*!< bit:      2  Cacheable setting */
        uint32_t C3: 1;                      /*!< bit:      3  Cacheable setting */
        uint32_t _reserved0: 4;              /*!< bit:  4.. 7  Reserved */
        uint32_t AP0: 2;                     /*!< bit:  8.. 9  access permissions settings bit */
        uint32_t AP1: 2;                     /*!< bit: 10..11  access permissions settings bit */
        uint32_t AP2: 2;                     /*!< bit: 12..13  access permissions settings bit */
        uint32_t AP3: 2;                     /*!< bit: 14..15  access permissions settings bit */
        uint32_t _reserved1: 16;             /*!< bit: 16..31  Reserved */
    } b;                                   /*!< Structure    Access by bit */
    uint32_t w;                            /*!< Type         Access by whole register */
} CAPR_Type;

/* CAPR Register Definitions */
#define CAPR_AP3_Pos                       14U                                            /*!< CAPR: AP3 Position */
#define CAPR_AP3_Msk                       (0x3UL << CAPR_AP3_Pos)                        /*!< CAPR: AP3 Mask */

#define CAPR_AP2_Pos                       12U                                            /*!< CAPR: AP2 Position */
#define CAPR_AP2_Msk                       (0x3UL << CAPR_AP2_Pos)                        /*!< CAPR: AP2 Mask */

#define CAPR_AP1_Pos                       10U                                            /*!< CAPR: AP1 Position */
#define CAPR_AP1_Msk                       (0x3UL << CAPR_AP1_Pos)                        /*!< CAPR: AP1 Mask */

#define CAPR_AP0_Pos                       8U                                             /*!< CAPR: AP0 Position */
#define CAPR_AP0_Msk                       (0x3UL << CAPR_AP0_Pos)                        /*!< CAPR: AP0 Mask */

#define CAPR_X3_Pos                        3U                                             /*!< CAPR: X3 Position */
#define CAPR_X3_Msk                        (0x1UL << CAPR_X3_Pos)                         /*!< CAPR: X3 Mask */

#define CAPR_X2_Pos                        2U                                             /*!< CAPR: X2 Position */
#define CAPR_X2_Msk                        (0x1UL << CAPR_X2_Pos)                         /*!< CAPR: X2 Mask */

#define CAPR_X1_Pos                        1U                                             /*!< CAPR: X1 Position */
#define CAPR_X1_Msk                        (0x1UL << CAPR_X1_Pos)                         /*!< CAPR: X1 Mask */

#define CAPR_X0_Pos                        0U                                             /*!< CAPR: X0 Position */
#define CAPR_X0_Msk                        (0x1UL << CAPR_X0_Pos)                         /*!< CAPR: X0 Mask */


/**
  \brief  Consortium definition for accessing control register(PACR, CR<20,0>).
 */
typedef union {
    struct {
        uint32_t E: 1;                       /*!< bit:      0  Effective setting of protected area */
        uint32_t size: 5;                    /*!< bit:  1.. 5  Size of protected area */
        uint32_t _reserved0: 6;              /*!< bit:  6.. 11 Reserved */
        uint32_t base_addr: 20;              /*!< bit:  10..31 The high position of the address of a protected area */
    } b;                                     /*!< Structure    Access by bit */
    uint32_t w;                              /*!< Type         Access by whole register */
} PACR_Type;

    /* PACR Register Definitions */
#define PACR_BASE_ADDR_Pos                 12U                                            /*!< PACR: base_addr Position */
#define PACR_BASE_ADDR_Msk                 (0xFFFFFUL << PACR_BASE_ADDR_Pos)              /*!< PACR: base_addr Mask */

#define PACR_SIZE_Pos                      1U                                             /*!< PACR: Size Position */
#define PACR_SIZE_Msk                      (0x1FUL << PACR_SIZE_Pos)                      /*!< PACR: Size Mask */

#define PACR_E_Pos                         0U                                             /*!< PACR: E Position */
#define PACR_E_Msk                         (0x1UL << PACR_E_Pos)                          /*!< PACR: E Mask */

/**
  \brief  Consortium definition for accessing protection area selection register(PRSR,CR<21,0>).
 */
typedef union {
    struct {
        uint32_t RID: 2;                     /*!< bit:  0.. 1  Protected area index value */
        uint32_t _reserved0: 30;             /*!< bit:  2..31  Reserved */
    } b;                                     /*!< Structure    Access by bit */
    uint32_t w;                              /*!< Type         Access by whole register */
} PRSR_Type;

/* PRSR Register Definitions */
#define PRSR_RID_Pos                       0U                                            /*!< PRSR: RID Position */
#define PRSR_RID_Msk                       (0x3UL << PRSR_RID_Pos)                       /*!< PRSR: RID Mask */

/**
  \brief  Consortium definition for accessing mmu index register(MIR,CP15_CR0).
 */
typedef union {
    struct {
        uint32_t Index: 10;
        uint32_t _reserved: 20;
        uint32_t TF: 1;
        uint32_t P: 1;
    } b;
    uint32_t w;
} MIR_Type;

/* MIR Register Definitions */
#define MIR_P_Pos                          31                                            /*!< PRSR: P(TLBP instruction) Position */
#define MIR_P_Msk                          (0x1UL << MIR_P_Pos)                          /*!< PRSR: P(TLBP instruction) Mask */

#define MIR_TF_Pos                         30                                            /*!< PRSR: Tfatal Position */
#define MIR_TF_Msk                         (0x1UL << MIR_TF_Pos)                         /*!< PRSR: Tfatal Mask */

#define MIR_Index_Pos                      0                                             /*!< PRSR: Index Position */
#define MIR_Index_Msk                      (0x3ffUL << MIR_Index_Pos)                    /*!< PRSR: Index Mask */


/**
  \brief  Consortium definition for accessing mmu entry of high physical address register(MEL, CP15_CR2 and CP15_CR3).
 */
typedef union {
    struct {
        uint32_t G: 1;
        uint32_t V: 1;
        uint32_t D: 1;
        uint32_t C: 3;
        uint32_t PFN: 20;
        uint32_t _reserved: 6;
    } b;
    uint32_t w;
} MEL_Type;

/* MEL Register Definitions */
#define MEL_PFN_Pos                        6                                             /*!< MEL: PFN Position */
#define MEL_PFN_Msk                        (0xFFFFFUL << MEL_PFN_Pos)                    /*!< MEL: PFN Mask */

#define MEL_C_Pos                          3                                             /*!< MEL: C Position */
#define MEL_C_Msk                          (0x7UL << MEL_C_Pos)                          /*!< MEL: C Mask */

#define MEL_D_Pos                          2                                             /*!< MEL: D Position */
#define MEL_D_Msk                          (0x1UL << MIR_D_Pos)                          /*!< MEL: D Mask */

#define MEL_V_Pos                          1                                             /*!< MEL: V Position */
#define MEL_V_Msk                          (0x1UL << MIR_V_Pos)                          /*!< MEL: V Mask */

#define MEL_G_Pos                          0                                             /*!< MEL: G Position */
#define MEL_G_Msk                          (0x1UL << MIR_G_Pos)                          /*!< MEL: G Mask */


/**
  \brief  Consortium definition for accessing mmu entry of high physical address register(MEH, CP15_CR4).
 */
typedef union {
    struct {
        uint32_t ASID :8;
        uint32_t _reserved :4;
        uint32_t VPN :20;
    } b;
    uint32_t w;
} MEH_Type;

/* MEH Register Definitions */
#define MEH_VPN_Pos                        12                                             /*!< MEH: VPN Position */
#define MEH_VPN_Msk                        (0xFFFFFUL << MEH_VPN_Pos)                     /*!< MEH: VPN Mask */

#define MEH_ASID_Pos                       0                                              /*!< MEH: ASID Position */
#define MEH_ASID_Msk                       (0xFFUL << MEH_ASID_Pos)                       /*!< MEH: ASID Mask */


/**
  \brief  Consortium definition for accessing mmu entry of high physical address register(MPR, CP15_CR6).
 */
typedef union {
    struct {
        uint32_t _reserved0: 13;
        uint32_t page_mask: 12;
        uint32_t _reserved1: 7;
    } b;
    uint32_t w;
} MPR_Type;

/* MPR Register Definitions */
#define MPR_PAGE_MASK_Pos                        13                                             /*!< MPR: PAGE_MASK Position */
#define MPR_PAGE_MASK_Msk                        (0xFFFUL << MPR_PAGE_MASK_Pos)                 /*!< MPR: PAGE_MASK Mask */


/**
  \brief  Consortium definition for accessing mmu entry of high physical address register(MCIR, CP15_CR8).
 */
typedef union {
    struct {
        uint32_t ASID: 8;
        uint32_t _reserved: 17;
        uint32_t TLBINV_INDEX: 1;
        uint32_t TLBINV_ALL: 1;
        uint32_t TLBINV: 1;
        uint32_t TLBWR: 1;
        uint32_t TLBWI: 1;
        uint32_t TLBR: 1;
        uint32_t TLBP: 1;
    } b;
    uint32_t w;
} MCIR_Type;

/* MCIR Register Definitions */
#define MCIR_TLBP_Pos                        31                                               /*!< MCIR: TLBP Position */
#define MCIR_TLBP_Msk                        (0x1UL << MCIR_TLBP_Pos)                         /*!< MCIR: TLBP Mask */

#define MCIR_TLBR_Pos                        30                                               /*!< MCIR: TLBR Position */
#define MCIR_TLBR_Msk                        (0x1UL << MCIR_TLBR_Pos)                         /*!< MCIR: TLBR Mask */

#define MCIR_TLBWI_Pos                       29                                               /*!< MCIR: TLBWI Position */
#define MCIR_TLBWI_Msk                       (0x1UL << MCIR_TLBWI_Pos)                        /*!< MCIR: TLBWI Mask */

#define MCIR_TLBWR_Pos                       28                                               /*!< MCIR: TLBWR Position */
#define MCIR_TLBWR_Msk                       (0x1UL << MCIR_TLBWR_Pos)                        /*!< MCIR: TLBWR Mask */

#define MCIR_TLBINV_Pos                      27                                               /*!< MCIR: TLBINV Position */
#define MCIR_TLBINV_Msk                      (0x1UL << MCIR_TLBINV_Pos)                       /*!< MCIR: TLBINV Mask */

#define MCIR_TLBINV_ALL_Pos                  26                                               /*!< MCIR: TLBINV_ALL Position */
#define MCIR_TLBINV_ALL_Msk                  (0x1UL << MCIR_TLBINV_ALL_Pos)                   /*!< MCIR: TLBINV_ALL Mask */

#define MCIR_TLBINV_INDEX_Pos                25                                               /*!< MCIR: TLBINV_INDEX Position */
#define MCIR_TLBINV_INDEX_Msk                (0x1UL << MCIR_TLBINV_INDEX_Pos)                 /*!< MCIR: TLBINV_INDEX Mask */

#define MCIR_ASID_Pos                        0                                                /*!< MCIR: ASID Position */
#define MCIR_ASID_Msk                        (0xFFUL << MCIR_ASID_Pos)                        /*!< MCIR: ASID Mask */


/*@} end of group CSI_CORE */


/**
  \ingroup    CSI_core_register
  \defgroup   CSI_CACHE
  \brief      Type definitions for the cache Registers
  @{
 */

#define SSEG0_BASE_ADDR         0x80000000
#define CACHE_RANGE_MAX_SIZE    0x80000

#define INS_CACHE               (1 << 0)
#define DATA_CACHE              (1 << 1)
#define CACHE_INV               (1 << 4)
#define CACHE_CLR               (1 << 5)
#define CACHE_OMS               (1 << 6)
#define CACHE_ITS               (1 << 7)
#define CACHE_LICF              (1 << 31)

#define    L1_CACHE_SHIFT       4      /* 16 Bytes */
#define    L1_CACHE_BYTES       (1 << L1_CACHE_SHIFT)

/**
  \brief   Mask and shift a bit field value for use in a register bit range.
  \param[in] field  Name of the register bit field.
  \param[in] value  Value of the bit field.
  \return           Masked and shifted value.
*/
#define _VAL2FLD(field, value)    ((value << field ## _Pos) & field ## _Msk)

/**
  \brief     Mask and shift a register value to extract a bit filed value.
  \param[in] field  Name of the register bit field.
  \param[in] value  Value of register.
  \return           Masked and shifted bit field value.
*/
#define _FLD2VAL(field, value)    ((value & field ## _Msk) >> field ## _Pos)

/*@} end of group CSI_core_bitfield */


/*******************************************************************************
 *                Hardware Abstraction Layer
  Core Function Interface contains:
  - Core VIC Functions
  - Core CORET Functions
  - Core Register Access Functions
 ******************************************************************************/
/* ##########################  Cache functions  #################################### */
/**
  \ingroup  CSI_Core_FunctionInterface
  \defgroup CSI_Core_CacheFunctions Cache Functions
  \brief    Functions that configure Instruction and Data cache.
  @{
 */

/**
  \brief   Enable I-Cache
  \details Turns on I-Cache
  */
__STATIC_INLINE void csi_icache_enable (void)
{
   __set_CCR(__get_CCR() | 0x00000004);
}


/**
  \brief   Disable I-Cache
  \details Turns off I-Cache
  */
__STATIC_INLINE void csi_icache_disable (void)
{
   __set_CCR(__get_CCR() & 0xFFFFFFFB);
}


/**
  \brief   Invalidate I-Cache
  \details Invalidates I-Cache
  */
__STATIC_INLINE void csi_icache_invalid (void)
{
    __set_CFR(0x11);
    __set_CFR(INS_CACHE | CACHE_INV);
}


/**
  \brief   Enable D-Cache
  \details Turns on D-Cache
  \note    I-Cache also turns on.
  */
__STATIC_INLINE void csi_dcache_enable (void)
{
   __set_CCR(__get_CCR() | 0x00000008);
}


/**
  \brief   Disable D-Cache
  \details Turns off D-Cache
  \note    I-Cache also turns off.
  */
__STATIC_INLINE void csi_dcache_disable (void)
{
   __set_CCR(__get_CCR() & 0xFFFFFFF7);
}


/**
  \brief   Invalidate D-Cache
  \details Invalidates D-Cache
  \note    I-Cache also invalid
  */
__STATIC_INLINE void csi_dcache_invalid (void)
{
    __set_CFR(DATA_CACHE | CACHE_INV);
}


/**
  \brief   Clean D-Cache
  \details Cleans D-Cache
  \note    I-Cache also cleans
  */
__STATIC_INLINE void csi_dcache_clean (void)
{
    __set_CFR(DATA_CACHE | CACHE_CLR);
}


/**
  \brief   Clean & Invalidate D-Cache
  \details Cleans and Invalidates D-Cache
  \note    I-Cache also flush.
  */
__STATIC_INLINE void csi_dcache_clean_invalid (void)
{
    __set_CFR(DATA_CACHE | CACHE_CLR | CACHE_INV);
}

__STATIC_INLINE  void set_cache_range (uint32_t start, uint32_t end, uint32_t value)
{
    if (!(start & SSEG0_BASE_ADDR) || (end - start) &~(CACHE_RANGE_MAX_SIZE - 1)) {
        __set_CFR(value);
    }

    if (value & INS_CACHE) {
        csi_icache_disable();
    }
    uint32_t i;
    for (i = start; i < end; i += L1_CACHE_BYTES) {
        __set_CIR(i);
        __set_CFR(CACHE_OMS | value);
    }

    if (end & (L1_CACHE_BYTES-1)) {
        __set_CIR(end);
        __set_CFR(CACHE_OMS | value);
    }

    if (value & INS_CACHE) {
        csi_icache_enable();
    }

}

/**
  \brief   D-Cache Invalidate by address
  \details Invalidates D-Cache for the given address
  \param[in]   addr    address (aligned to 16-byte boundary)
  \param[in]   dsize   size of memory block (in number of bytes)
*/
__STATIC_INLINE void csi_dcache_invalid_range (uint32_t *addr, int32_t dsize)
{
    set_cache_range((uint32_t)addr, (uint32_t)addr + dsize, (DATA_CACHE | CACHE_INV));
}


/**
  \brief   D-Cache Clean by address
  \details Cleans D-Cache for the given address
  \param[in]   addr    address (aligned to 16-byte boundary)
  \param[in]   dsize   size of memory block (in number of bytes)
*/
__STATIC_INLINE void csi_dcache_clean_range (uint32_t *addr, int32_t dsize)
{
    set_cache_range((uint32_t)addr, (uint32_t)addr + dsize, (DATA_CACHE | CACHE_CLR));
}


/**
  \brief   D-Cache Clean and Invalidate by address
  \details Cleans and invalidates D_Cache for the given address
  \param[in]   addr    address (aligned to 16-byte boundary)
  \param[in]   dsize   size of memory block (in number of bytes)
*/
__STATIC_INLINE void csi_dcache_clean_invalid_range (uint32_t *addr, int32_t dsize)
{
    set_cache_range((uint32_t)addr, (uint32_t)addr + dsize, (DATA_CACHE | CACHE_CLR | CACHE_INV));
}

/*@} end of CSI_Core_CacheFunctions */


/* ##########################  MMU functions  #################################### */
/**
  \ingroup  CSI_Core_FunctionInterface
  \defgroup CSI_Core_MMUFunctions MMU Functions
  \brief    Functions that configure MMU.
  @{
 */

typedef struct {
    uint32_t global: 1;         /* tlb page global access. */
    uint32_t valid: 1;          /* tlb page valid */
    uint32_t writeable: 1;      /* tlb page writeable */
    uint32_t cacheable: 1;      /* tlb page cacheable*/
} page_attr_t;

typedef enum {
    PAGE_SIZE_4KB   = 0x000,
    PAGE_SIZE_16KB  = 0x003,
    PAGE_SIZE_64KB  = 0x00F,
    PAGE_SIZE_256KB = 0x03F,
    PAGE_SIZE_1MB   = 0x0FF,
    PAGE_SIZE_4MB   = 0x3FF,
    PAGE_SIZE_16MB  = 0xFFF
} page_size_e;

/**
  \brief  enable mmu
  \details
  */
__STATIC_INLINE void csi_mmu_enable(void)
{
    __set_CCR(__get_CCR() | (1u << CCR_MP_Pos));
}

/**
  \brief  disable mmu
  \details
  */
__STATIC_INLINE void csi_mmu_disable(void)
{
    __set_CCR(__get_CCR() & (~(1u << CCR_MP_Pos)));
}

/**
  \brief  create page with feature.
  \details
  \param [in]  vaddr     virtual address.
  \param [in]  paddr     physical address.
  \param [in]  asid      address sapce id (default: 0).
  \param [in]  attr      \ref page_attr_t. tlb page attribute.
  */
__STATIC_INLINE void csi_mmu_set_tlb(uint32_t vaddr, uint32_t paddr, uint32_t asid, page_attr_t attr)
{
    MPR_Type pgmask;
    MEL_Type mel;
    MEH_Type meh;
    uint32_t vaddr_bit = 0;
    uint32_t page_feature = 0;

    page_feature |= attr.global << MEL_G_Pos | attr.valid << MEL_V_Pos |
                    attr.writeable << MEL_D_Pos | (attr.cacheable | 0x2) << MEL_C_Pos;

    pgmask.w = __FF1(__get_MPR());
    vaddr_bit = (pgmask.w == 32 ? 12 : (31 - pgmask.w));

    meh.b.ASID = asid;
    meh.b.VPN  = (vaddr & ((~pgmask.w | 0xFE000000) & 0xFFFFE000)) >> MEH_VPN_Pos;
    __set_MEH(meh.w);

    __set_MCIR(1u << MCIR_TLBP_Pos);

    mel.w = (((paddr >> 6) & ~(pgmask.b.page_mask << 6)) | page_feature);

    if (vaddr & (1 << vaddr_bit)) {
        __set_MEL1(mel.w);
    } else {
        __set_MEL0(mel.w);
    }

    if (__get_MIR() & (1 << MIR_P_Pos)) {
       __set_MCIR(1u << MCIR_TLBWR_Pos);
    } else {
       __set_MCIR(1u << MCIR_TLBWI_Pos);
    }
}


/**
  \brief  enble mmu
  \details
  \param [in]  size  tlb page size.
  */
__STATIC_INLINE void csi_mmu_set_pagesize(page_size_e size)
{
    MPR_Type mpr;
    mpr.w = __get_MPR();
    mpr.b.page_mask = size;
    __set_MPR(mpr.w);
}


/**
  \brief  read MEH, MEL0, MEL1 by tlb index.
  \details
  \param [in]    index  tlb index(0, 1, 2, ...)
  \param [out]   meh    pointer to variable for retrieving MEH.
  \param [out]   mel0   pointer to variable for retrieving MEL0.
  \param [out]   mel1   pointer to variable for retrieving MEL1.
  */
__STATIC_INLINE void csi_mmu_read_by_index(uint32_t index, uint32_t *meh, uint32_t *mel0, uint32_t *mel1)
{
    MIR_Type mir;

    if (meh == NULL || mel0 == NULL || mel1 == NULL) {
        return;
    }

    mir.b.Index = index;
    __set_MIR(mir.w);
    __set_MCIR(1u << MCIR_TLBR_Pos);

    *meh = __get_MEH();
    *mel0 = __get_MEL0();
    *mel1 = __get_MEL1();
}


/**
  \brief  flush all mmu tlb.
  \details
  */
__STATIC_INLINE void csi_mmu_invalid_tlb_all(void)
{
    __set_MCIR(1u << MCIR_TLBINV_ALL_Pos);
}


/**
  \brief  flush mmu tlb by index.
  \details
  */
__STATIC_INLINE void csi_mmu_invalid_tlb_by_index(uint32_t index)
{
    MIR_Type mir;

    mir.b.Index = index;
    __set_MIR(mir.w);
    __set_MCIR(1u << MCIR_TLBINV_INDEX_Pos);
}


/**
  \brief  flush mmu tlb by virtual address.
  \details
  */
__STATIC_INLINE void csi_mmu_invalid_tlb_by_vaddr(uint32_t vaddr, uint32_t asid)
{
    __set_MEH(vaddr | (asid & MEH_ASID_Msk));
    __set_MCIR(__get_MCIR() | (1 << MCIR_TLBP_Pos));

    if (__get_MIR() & (1 << MIR_P_Pos)) {
        return;
    } else {
        __set_MCIR(__get_MCIR() | (1 << MCIR_TLBINV_INDEX_Pos));
    }
}

/*@} end of CSI_Core_MMUFunctions */

/* ##########################  MPU functions  #################################### */
/**
  \ingroup  CSI_Core_FunctionInterface
  \defgroup CSI_Core_MPUFunctions MPU Functions
  \brief    Functions that configure MPU.
  @{
 */

typedef enum {
    REGION_SIZE_4KB      = 0xB,
    REGION_SIZE_8KB      = 0xC,
    REGION_SIZE_16KB     = 0xD,
    REGION_SIZE_32KB     = 0xE,
    REGION_SIZE_64KB     = 0xF,
    REGION_SIZE_128KB    = 0x10,
    REGION_SIZE_256KB    = 0x11,
    REGION_SIZE_512KB    = 0x12,
    REGION_SIZE_1MB      = 0x13,
    REGION_SIZE_2MB      = 0x14,
    REGION_SIZE_4MB      = 0x15,
    REGION_SIZE_8MB      = 0x16,
    REGION_SIZE_16MB     = 0x17,
    REGION_SIZE_32MB     = 0x18,
    REGION_SIZE_64MB     = 0x19,
    REGION_SIZE_128MB    = 0x1A,
    REGION_SIZE_256MB    = 0x1B,
    REGION_SIZE_512MB    = 0x1C,
    REGION_SIZE_1GB      = 0x1D,
    REGION_SIZE_2GB      = 0x1E,
    REGION_SIZE_4GB      = 0x1F
} region_size_e;

typedef enum {
    AP_BOTH_INACCESSIBLE = 0,
    AP_SUPER_RW_USER_INACCESSIBLE,
    AP_SUPER_RW_USER_RDONLY,
    AP_BOTH_RW
} access_permission_e;

typedef struct {
    access_permission_e ap: 2;    /* super user and normal user access.*/
    uint32_t c: 1;                /* cacheable */
} mpu_region_attr_t;
/**
  \brief  enable mpu
  \details
  */
__STATIC_INLINE void csi_mpu_enable(void)
{
    __set_CCR(__get_CCR() | CCR_MP_Msk);
}

/**
  \brief  disable mpu
  \details
  */
__STATIC_INLINE void csi_mpu_disable(void)
{
    __set_CCR(__get_CCR() & (~CCR_MP_Msk));
}

/**
  \brief  configure memory protected region.
  \details
  \param [in]  idx        memory protected region (0, 1, 2, 3.).
  \param [in]  base_addr  base address must be aligned with page size.
  \param [in]  size       \ref region_size_e. memory protected region size.
  \param [in]  attr       \ref region_size_t. memory protected region attribute.
  \param [in]  enable     enable or disable memory protected region.
  */
__STATIC_INLINE void csi_mpu_config_region(uint32_t idx, uint32_t base_addr, region_size_e size,
                                           mpu_region_attr_t attr, uint32_t enable)
{
    if (idx > 3) {
        return;
    }

    CAPR_Type capr;
    PACR_Type pacr;
    PRSR_Type prsr;

    capr.w = __get_CAPR();
    pacr.w = __get_PACR();
    prsr.w = __get_PRSR();

    pacr.b.base_addr = (base_addr >> PACR_BASE_ADDR_Pos) & (0xFFFFF);

    prsr.b.RID = idx;
    __set_PRSR(prsr.w);

    if (size != REGION_SIZE_4KB) {
        pacr.w &= ~(((1u << (size -11)) - 1) << 12);
    }

    pacr.b.size = size;

    capr.w = (0xFFFFFFFE & capr.w) | (attr.c << idx);
    capr.w = ((~((0x3) << (2*idx + 8))) & capr.w) | (attr.ap << (2*idx + 8));
    __set_CAPR(capr.w);

    pacr.b.E = enable;
    __set_PACR(pacr.w);
}

/**
  \brief  enable mpu region by idx.
  \details
  \param [in]  idx        memory protected region (0, 1, 2, 3.).
  */
__STATIC_INLINE void csi_mpu_enable_region(uint32_t idx)
{
    if (idx > 3) {
        return;
    }

    __set_PRSR((__get_PRSR() & (~PRSR_RID_Msk)) | idx);
    __set_PACR(__get_PACR() | PACR_E_Msk);
}

/**
  \brief  disable mpu region by idx.
  \details
  \param [in]  idx        memory protected region (0, 1, 2, 3.).
  */
__STATIC_INLINE void csi_mpu_disable_region(uint32_t idx)
{
    if (idx > 3) {
        return;
    }

    __set_PRSR((__get_PRSR() & (~PRSR_RID_Msk)) | idx);
    __set_PACR(__get_PACR() & (~PACR_E_Msk));
}

/*@} end of CSI_Core_MMUFunctions */

/*@} */

#ifdef __cplusplus
}
#endif

#endif /* __CORE_CK610_H_DEPENDANT */

#endif /* __CSI_GENERIC */
