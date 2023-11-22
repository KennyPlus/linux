/*
 * hal_bdma.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __HAL_BDMA_H__
#define __HAL_BDMA_H__

/*=============================================================*/
// Data type definition
/*=============================================================*/

typedef enum
{
    HAL_BDMA_PROC_DONE       = 0,
    HAL_BDMA_ERROR           = -1,
    HAL_BDMA_POLLING_TIMEOUT = -2,
    HAL_BDMA_NO_INIT         = -3,
    HAL_BDMA_NO_SUPPOSE_CMDQ = -4
} HalBdmaErr_e;

typedef enum
{
    // BDMA0
    HAL_BDMA_CH0 = 0,
    HAL_BDMA_CH1,
    HAL_BDMA_CH2,
    HAL_BDMA_CH3,

    // BDMA2
    HAL_BDMA1_CH0,
    HAL_BDMA1_CH1,
    HAL_BDMA1_CH2,
    HAL_BDMA1_CH3,

    // BDMA3
    HAL_BDMA2_CH0,
    HAL_BDMA2_CH1,
    HAL_BDMA2_CH2,
    HAL_BDMA2_CH3,

    // BDMA4
    HAL_BDMA3_CH0,
    HAL_BDMA3_CH1,
    HAL_BDMA3_CH2,
    HAL_BDMA3_CH3,

    // BDMA5
    HAL_BDMA4_CH0,
    HAL_BDMA4_CH1,
    HAL_BDMA4_CH2,
    HAL_BDMA4_CH3,

    HAL_BDMA_CH_NUM
} HalBdmaCh_e;

typedef enum
{
    HAL_BDMA_MIU0_TO_MIU0 = 0x0,
    HAL_BDMA_MIU0_TO_MIU1,
    HAL_BDMA_MIU1_TO_MIU0,
    HAL_BDMA_MIU1_TO_MIU1,
    HAL_BDMA_MIU0_TO_IMI,
    HAL_BDMA_MIU1_TO_IMI,
    HAL_BDMA_IMI_TO_MIU0,
    HAL_BDMA_IMI_TO_MIU1,
    HAL_BDMA_IMI_TO_IMI,
    HAL_BDMA_MEM_TO_MIU0,
    HAL_BDMA_MEM_TO_MIU1,
    HAL_BDMA_MEM_TO_IMI,
    HAL_BDMA_SPI_TO_MIU0,
    HAL_BDMA_SPI_TO_MIU1,
    HAL_BDMA_SPI_TO_IMI,
    HAL_BDMA_MIU0_TO_SPI,
    HAL_BDMA_MSPI_TO_MIU,
    HAL_BDMA_MIU_TO_MSPI
} HalBdmaPathSel_e;

typedef enum
{
    HAL_BDMA_DATA_BYTE_1  = 0x0,
    HAL_BDMA_DATA_BYTE_2  = 0x1,
    HAL_BDMA_DATA_BYTE_4  = 0x2,
    HAL_BDMA_DATA_BYTE_8  = 0x3,
    HAL_BDMA_DATA_BYTE_16 = 0x4
} HalBdmaDataWidth_e;

typedef enum
{
    HAL_BDMA_ADDR_INC = 0x0,
    HAL_BDMA_ADDR_DEC = 0x1
} HalBdmaAddrMode_e;

/*=============================================================*/
// Structure definition
/*=============================================================*/

typedef void (*HalBdmaTxCb)(void *);

typedef struct
{
    u32 u32SrcWidth;  ///< Width of source
    u32 u32SrcOffset; ///< Line-to-line offset of source
    u32 u32DstWidth;  ///< Width of destination
    u32 u32DstOffset; ///< Line-to-line offset of destination
} HalBdmaLineOfst_t;

typedef struct
{
    bool               bIntMode;
    HalBdmaPathSel_e   ePathSel;
    HalBdmaDataWidth_e eSrcDataWidth;
    HalBdmaDataWidth_e eDstDataWidth;
    HalBdmaAddrMode_e  eDstAddrMode;
    u32                u32TxCount;
    u32                u32Pattern;
    phys_addr_t        pSrcAddr;
    phys_addr_t        pDstAddr;
    u32                bEnLineOfst;
    HalBdmaLineOfst_t *pstLineOfst;
    HalBdmaTxCb        pfTxCbFunc;
    void *             pTxCbParm;
} HalBdmaParam_t;

/*=============================================================*/
// Global function definition
/*=============================================================*/

HalBdmaErr_e HalBdma_Initialize(u8 u8DmaCh);
HalBdmaErr_e HalBdma_Transfer(u8 u8DmaCh, HalBdmaParam_t *ptBdmaParam);
HalBdmaErr_e HalBdma_WaitTransferDone(u8 u8DmaCh, HalBdmaParam_t *ptBdmaParam);
#ifdef BDMA_USE_CMDQ_ENABLE
HalBdmaErr_e HalBdma_Transfer_cmdq(u8 u8DmaCh, HalBdmaParam_t *ptBdmaParam, void *pCmdQInfs);
#endif

#endif // __HAL_BDMA_H__
