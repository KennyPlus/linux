#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef CHIP_TYPE_hi3519

/********* Here define the function supported by chip *****************/
#define  CIPHER_IRQ_NUMBER                      58
#define  CIPHER_HASH_SUPPORT
#define  CIPHER_MHASH_SUPPORT
#define  CIPHER_RSA_SUPPORT
#define  CIPHER_RNG_SUPPORT
#define  CIPHER_RNG_VERSION_1
#define  CIPHER_CCM_GCM_SUPPORT
#define  CIPHER_MULTICIPHER_SUPPORT
#define  CIPHER_EFUSE_SUPPORT
//#define  CIPHER_KLAD_SUPPORT

/********* Here define the base address of chip ***********************/
#define  REG_BASE_PHY_ADDR_CIPHER            (0x10080000)
#define  REG_BASE_PHY_ADDR_SHA               (0x10090000)
#define  REG_BASE_PHY_ADDR_RSA               (0x120B0000)
#define  REG_BASE_PHY_ADDR_RNG 		         (0x120C0200)
#define  REG_BASE_PHY_ADDR_EFUSE 		     (0x12070800)

/********* Here define the clcok and reset signal in CRG ***************/
#define  REG_CRG_CLK_PHY_ADDR_CIPHER         (0x1201007c)
#define  REG_CRG_CLK_PHY_ADDR_SHA            (0x1201007c)
#define  REG_CRG_CLK_PHY_ADDR_RSA     		 (0x120100e0)
#define  REG_CRG_CLK_PHY_ADDR_RNG    		 (0x120100e0)
#define  CRG_RST_BIT_CIPHER              0
#define  CRG_CLK_BIT_CIPHER              1
#define  CRG_RST_BIT_SHA                 2
#define  CRG_CLK_BIT_SHA                 3
#define  CRG_RST_BIT_RSA                 6
#define  CRG_CLK_BIT_RSA                 7
#define  CRG_RST_BIT_RNG                 4
#define  CRG_CLK_BIT_RNG                 5
#define  CIPHER_READL                    readl
#define  CIPHER_WRITEL                   writel

#else
    #error You need to define a correct chip type!
#endif
#endif

