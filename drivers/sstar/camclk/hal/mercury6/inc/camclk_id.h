/*
 * camclk_id.h- Sigmastar
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

#ifndef __CAMCLK_ID_H__
#define __CAMCLK_ID_H__
typedef enum
{
    HAL_CAMCLK_SRC_CLK_VOID,
    HAL_CAMCLK_SRC_CLK_utmi_480m,
    HAL_CAMCLK_SRC_CLK_mpll_432m,
    HAL_CAMCLK_SRC_CLK_upll_384m,
    HAL_CAMCLK_SRC_CLK_mpll_345m,
    HAL_CAMCLK_SRC_CLK_upll_320m,
    HAL_CAMCLK_SRC_CLK_mpll_288m,
    HAL_CAMCLK_SRC_CLK_utmi_240m,
    HAL_CAMCLK_SRC_CLK_mpll_216m,
    HAL_CAMCLK_SRC_CLK_utmi_192m,
    HAL_CAMCLK_SRC_CLK_mpll_172m,
    HAL_CAMCLK_SRC_CLK_utmi_160m,
    HAL_CAMCLK_SRC_CLK_mpll_123m,
    HAL_CAMCLK_SRC_CLK_mpll_86m,
    HAL_CAMCLK_SRC_CLK_mpll_288m_div2,
    HAL_CAMCLK_SRC_CLK_mpll_288m_div4,
    HAL_CAMCLK_SRC_CLK_mpll_288m_div8,
    HAL_CAMCLK_SRC_CLK_mpll_216m_div2,
    HAL_CAMCLK_SRC_CLK_mpll_216m_div4,
    HAL_CAMCLK_SRC_CLK_mpll_216m_div8,
    HAL_CAMCLK_SRC_CLK_mpll_123m_div2,
    HAL_CAMCLK_SRC_CLK_mpll_86m_div2,
    HAL_CAMCLK_SRC_CLK_mpll_86m_div4,
    HAL_CAMCLK_SRC_CLK_mpll_86m_div16,
    HAL_CAMCLK_SRC_CLK_utmi_192m_div4,
    HAL_CAMCLK_SRC_CLK_utmi_160m_div4,
    HAL_CAMCLK_SRC_CLK_utmi_160m_div5,
    HAL_CAMCLK_SRC_CLK_utmi_160m_div8,
    HAL_CAMCLK_SRC_CLK_xtali_12m,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div2,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div4,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div8,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div16,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div40,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div64,
    HAL_CAMCLK_SRC_CLK_xtali_12m_div128,
    HAL_CAMCLK_SRC_CLK_xtali_24m,
    HAL_CAMCLK_SRC_CLK_RTC_CLK_32K,
    HAL_CAMCLK_SRC_CLK_pm_riu_w_clk_in,
    HAL_CAMCLK_SRC_CLK_miupll_clk,
    HAL_CAMCLK_SRC_CLK_ddrpll_clk,
    HAL_CAMCLK_SRC_CLK_lpll_clk,
    HAL_CAMCLK_SRC_CLK_ven_pll,
    HAL_CAMCLK_SRC_CLK_ven_pll_div6,
    HAL_CAMCLK_SRC_CLK_lpll_div2,
    HAL_CAMCLK_SRC_CLK_lpll_div4,
    HAL_CAMCLK_SRC_CLK_lpll_div8,
    HAL_CAMCLK_SRC_CLK_armpll_37p125m,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_in,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_top,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_sc_gp,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_vhe_gp,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_hemcu_gp,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_mipi_if_gp,
    HAL_CAMCLK_SRC_CLK_riu_w_clk_mcu_if_gp,
    HAL_CAMCLK_SRC_CLK_fuart0_synth_out,
    HAL_CAMCLK_SRC_CLK_miu_p,
    HAL_CAMCLK_SRC_CLK_mspi0_p,
    HAL_CAMCLK_SRC_CLK_mspi1_p,
    HAL_CAMCLK_SRC_CLK_miu_vhe_gp_p,
    HAL_CAMCLK_SRC_CLK_miu_sc_gp_p,
    HAL_CAMCLK_SRC_CLK_mcu_p,
    HAL_CAMCLK_SRC_CLK_fclk1_p,
    HAL_CAMCLK_SRC_CLK_sdio_p,
    HAL_CAMCLK_SRC_CLK_tck_buf,
    HAL_CAMCLK_SRC_CLK_eth_buf,
    HAL_CAMCLK_SRC_CLK_rmii_buf,
    HAL_CAMCLK_SRC_CLK_emac_testrx125_in_lan,
    HAL_CAMCLK_SRC_CLK_gop0,
    HAL_CAMCLK_SRC_CLK_rtc_32k,
    HAL_CAMCLK_SRC_CLK_fro,
    HAL_CAMCLK_SRC_CLK_fro_div2,
    HAL_CAMCLK_SRC_CLK_fro_div8,
    HAL_CAMCLK_SRC_CLK_fro_div16,
    HAL_CAMCLK_SRC_CLK_cpupll_clk,
    HAL_CAMCLK_SRC_CLK_utmi,
    HAL_CAMCLK_SRC_CLK_bach,
    HAL_CAMCLK_SRC_CLK_miu,
    HAL_CAMCLK_SRC_CLK_miu_boot,
    HAL_CAMCLK_SRC_CLK_ddr_syn,
    HAL_CAMCLK_SRC_CLK_miu_rec,
    HAL_CAMCLK_SRC_CLK_mcu,
    HAL_CAMCLK_SRC_CLK_riubrdg,
    HAL_CAMCLK_SRC_CLK_bdma,
    HAL_CAMCLK_SRC_CLK_spi_arb,
    HAL_CAMCLK_SRC_CLK_spi_flash,
    HAL_CAMCLK_SRC_CLK_pwm,
    HAL_CAMCLK_SRC_CLK_uart0,
    HAL_CAMCLK_SRC_CLK_uart1,
    HAL_CAMCLK_SRC_CLK_fuart0_synth_in,
    HAL_CAMCLK_SRC_CLK_fuart,
    HAL_CAMCLK_SRC_CLK_mspi0,
    HAL_CAMCLK_SRC_CLK_mspi1,
    HAL_CAMCLK_SRC_CLK_mspi,
    HAL_CAMCLK_SRC_CLK_miic0,
    HAL_CAMCLK_SRC_CLK_miic1,
    HAL_CAMCLK_SRC_CLK_miic2,
    HAL_CAMCLK_SRC_CLK_bist,
    HAL_CAMCLK_SRC_CLK_pwr_ctl,
    HAL_CAMCLK_SRC_CLK_xtali,
    HAL_CAMCLK_SRC_CLK_live,
    HAL_CAMCLK_SRC_CLK_sr00_mclk,
    HAL_CAMCLK_SRC_CLK_sr01_mclk,
    HAL_CAMCLK_SRC_CLK_sr1_mclk,
    HAL_CAMCLK_SRC_CLK_bist_pm,
    HAL_CAMCLK_SRC_CLK_bist_ipu_gp,
    HAL_CAMCLK_SRC_CLK_ipu,
    HAL_CAMCLK_SRC_CLK_ipuff,
    HAL_CAMCLK_SRC_CLK_bist_usb30_gp,
    HAL_CAMCLK_SRC_CLK_csi_mac_lptx_top_i_m00,
    HAL_CAMCLK_SRC_CLK_csi_mac_top_i_m00,
    HAL_CAMCLK_SRC_CLK_ns_top_i_m00,
    HAL_CAMCLK_SRC_CLK_csi_mac_lptx_top_i_m01,
    HAL_CAMCLK_SRC_CLK_csi_mac_top_i_m01,
    HAL_CAMCLK_SRC_CLK_ns_top_i_m01,
    HAL_CAMCLK_SRC_CLK_csi_mac_lptx_top_i_m1,
    HAL_CAMCLK_SRC_CLK_csi_mac_top_i_m1,
    HAL_CAMCLK_SRC_CLK_ns_top_i_m1,
    HAL_CAMCLK_SRC_CLK_mipi1_tx_csi,
    HAL_CAMCLK_SRC_CLK_bist_vhe_gp,
    HAL_CAMCLK_SRC_CLK_vhe,
    HAL_CAMCLK_SRC_CLK_mfe,
    HAL_CAMCLK_SRC_CLK_xtali_sc_gp,
    HAL_CAMCLK_SRC_CLK_bist_sc_gp,
    HAL_CAMCLK_SRC_CLK_emac_ahb,
    HAL_CAMCLK_SRC_CLK_jpe,
    HAL_CAMCLK_SRC_CLK_aesdma,
    HAL_CAMCLK_SRC_CLK_sdio,
    HAL_CAMCLK_SRC_CLK_sd,
    HAL_CAMCLK_SRC_CLK_ecc,
    HAL_CAMCLK_SRC_CLK_isp,
    HAL_CAMCLK_SRC_CLK_fclk1,
    HAL_CAMCLK_SRC_CLK_odclk,
    HAL_CAMCLK_SRC_CLK_dip,
    HAL_CAMCLK_SRC_CLK_emac_tx,
    HAL_CAMCLK_SRC_CLK_emac_rx,
    HAL_CAMCLK_SRC_CLK_emac_tx_ref,
    HAL_CAMCLK_SRC_CLK_emac_rx_ref,
    HAL_CAMCLK_SRC_CLK_ive,
    HAL_CAMCLK_SRC_CLK_ldcfeye,
    HAL_CAMCLK_SRC_CLK_live_pm,
    HAL_CAMCLK_SRC_CLK_mcu_pm_p1,
    HAL_CAMCLK_SRC_CLK_spi_pm,
    HAL_CAMCLK_SRC_CLK_miic_pm,
    HAL_CAMCLK_SRC_CLK_pm_sleep,
    HAL_CAMCLK_SRC_CLK_rtc,
    HAL_CAMCLK_SRC_CLK_sar,
    HAL_CAMCLK_SRC_CLK_pir,
    HAL_CAMCLK_SRC_CLK_pm_uart,
    HAL_CAMCLK_SRC_Id_MAX
} HalCamClkSrcId_e;
#endif
