#ifndef __HISI_PCIE_H__
#define __HISI_PCIE_H__

#define MISC_CTRL_BASE		0x12030000
#define PCIE_MEM_BASE		0x28000000
#define PCIE_EP_CONF_BASE	0x20000000
#define PCIE_DBI_BASE		0x12160000
#define PERI_CRG_BASE		0x12010000

#define PERI_CRG43      0xAC
#define PERI_CRG44		0xB0
#define PCIE_X2_SRST_REQ	2

#define PCIE_X2_AUX_CKEN	7
#define PCIE_X2_PIPE_CKEN	6
#define PCIE_X2_SYS_CKEN	5
#define PCIE_X2_BUS_CKEN	4
#define PCIE_PAD_OE_MASK	(0x7 << 8)

#define PCIE_SYS_CTRL0		0x1000
#define PCIE_DEVICE_TYPE	28
#define PCIE_WM_EP		0x0
#define PCIE_WM_LEGACY		0x1
#define PCIE_WM_RC		0x4

#define PCIE_SYS_CTRL7		0x101C
#define PCIE_APP_LTSSM_ENBALE	11
#define PCIE_ACCESS_ENABLE	13

#define PCIE_SYS_STATE0		0x1100
#define PCIE_XMLH_LINK_UP	15
#define PCIE_RDLH_LINK_UP	5

#define PCIE_IRQ_INTA		89
#define PCIE_IRQ_INTB		90
#define PCIE_IRQ_INTC		91
#define PCIE_IRQ_INTD		92
#define PCIE_IRQ_EDMA		93
#define PCIE_IRQ_MSI		94
#define PCIE_IRQ_LINK_DOWN	95

#define PCIE_INTA_PIN		1
#define PCIE_INTB_PIN		2
#define PCIE_INTC_PIN		3
#define PCIE_INTD_PIN		4

#define MISC_CTRL33		0x84
#define MISC_CTRL34		0x88
#define DEEMPHASIS_REG		0xa0
#define PCIE_CLKREQ_FILTER_BYPASS	0x600
#define DEEMPHASIS_VAL			0x42
#define COM_PHY_TEST_VAL1		((0x1 << 3) | (0x1 << 8))
#define COM_PHY_TEST_VAL2		((0x1 << 16) | (0x1 << 3) | (0x1 << 8))

#endif
