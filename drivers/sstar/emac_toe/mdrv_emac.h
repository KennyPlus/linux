/*
 * mdrv_emac.h- Sigmastar
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
#ifndef __DRV_EMAC_H_
#define __DRV_EMAC_H_

#define EMAC_DBG(fmt, args...) //{printk("Sstar_emac: "); printk(fmt, ##args);}
#define EMAC_ERR(fmt, args...)  \
    {                           \
        printk("Sstar_emac: "); \
        printk(fmt, ##args);    \
    }
#define EMAC_INFO                      \
    {                                  \
        printk("Line:%u\n", __LINE__); \
    }
#define EMAC_TODO(fmt, args...)             \
    {                                       \
        printk("[EMAC]%d TODO:", __LINE__); \
        printk(fmt, ##args);                \
    }

#define MINOR_EMAC_NUM 1
#define MAJOR_EMAC_NUM 241

#define EXT_PHY_PATCH 1

/////////////////////////////////
// to be refined
/////////////////////////////////

#define EMAC_SG           1
#define EMAC_SG_BDMA      0
#define EMAC_SG_BUF_CACHE 1

#define EMAC_GSO 1

// #define DYNAMIC_INT_TX_TH       64
#define DYNAMIC_INT_TX 1

#define DYNAMIC_INT_RX 1
#ifdef CONFIG_SS_SWTOE
#define REDUCE_CPU_FOR_RBNA 0
#else
#define REDUCE_CPU_FOR_RBNA 1
#endif

#ifdef CONFIG_SS_SWTOE

#define MSTAR_EMAC_NAPI 0

#define EMAC_FLOW_CONTROL_RX      0
#define EMAC_FLOW_CONTROL_RX_TEST 0

#define EMAC_FLOW_CONTROL_TX           0
#define EMAC_FLOW_CONTROL_TX_TEST      0
#define EMAC_FLOW_CONTROL_TX_TEST_TIME 0x200

#else

#define MSTAR_EMAC_NAPI           1
#define EMAC_FLOW_CONTROL_RX      1
#define EMAC_FLOW_CONTROL_RX_TEST 0

#define EMAC_FLOW_CONTROL_TX           1
#define EMAC_FLOW_CONTROL_TX_TEST      0
#define EMAC_FLOW_CONTROL_TX_TEST_TIME 0x200
#endif

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define EMAC_MTU (1524)

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
// #define DRV_EMAC_MAX_DEV                        0x1

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
/*
phys_addr_t     RAM_VA_BASE;                      //= 0x00000000;     // After init, RAM_ADDR_BASE = EMAC_ABSO_MEM_BASE
phys_addr_t     RAM_PA_BASE;
phys_addr_t     RAM_VA_PA_OFFSET;
phys_addr_t     RBQP_BASE;                          //= RX_BUFFER_SIZE;//0x00004000;     // IMPORTANT: lowest 13 bits as
zero.
*/

#define ETHERNET_TEST_NO_LINK          0x00000000UL
#define ETHERNET_TEST_AUTO_NEGOTIATION 0x00000001UL
#define ETHERNET_TEST_LINK_SUCCESS     0x00000002UL
#define ETHERNET_TEST_RESET_STATE      0x00000003UL
#define ETHERNET_TEST_SPEED_100M       0x00000004UL
#define ETHERNET_TEST_DUPLEX_FULL      0x00000008UL
#define ETHERNET_TEST_INIT_FAIL        0x00000010UL

u8 MY_DEV[16]                = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
u8 MY_MAC[6]                 = {0x00UL, 0x30UL, 0x1BUL, 0xBAUL, 0x02UL, 0xDBUL};
u8 PC_MAC[6]                 = {0x00UL, 0x1AUL, 0x4BUL, 0x5CUL, 0x39UL, 0xDFUL};
u8 ETH_PAUSE_FRAME_DA_MAC[6] = {0x01UL, 0x80UL, 0xC2UL, 0x00UL, 0x00UL, 0x01UL};

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct rbf_t
{
    u32 addr;
    u32 size;
} __attribute__((packed));

#define EP_FLAG_OPEND      0X00000001UL
#define EP_FLAG_SUSPENDING 0X00000002UL

typedef struct
{
    // u8 used;
    struct sk_buff* skb;      /* holds skb until xmit interrupt completes */
    dma_addr_t      skb_phys; /* phys addr from pci_map_single */
    int             skb_len;
} skb_info;

typedef struct
{
    skb_info* skb_info_arr;
    int       read;
    int       write;
    int       rw;
    int       size[2];
} skb_queue;

#ifndef CONFIG_SS_SWTOE
typedef struct
{
    // int off_va_pa;
    // skb_queue skb_queue_rx;
    struct rbf_t* desc;
    // dma_addr_t descPhys;
    struct sk_buff** skb_arr;
    int              num_desc;
    int              size_desc_queue;
    int              idx;
} rx_desc_queue_t;
#endif

struct emac_handle
{
    struct net_device_stats stats;

    spinlock_t mutexNetIf;
    spinlock_t mutexTXQ; // spin_lock_bh�]�^�Ospin_unlock_bh�]�^
    spinlock_t mutexPhy;

    /* Transmit */
    skb_queue skb_queue_tx;

    unsigned int irqcnt;
    unsigned int tx_irqcnt;

    /* Receive */
    // spinlock_t mutexRXD;
#ifdef CONFIG_SS_SWTOE
    int                cnx_id;
    struct work_struct rx_work;
#else
    rx_desc_queue_t rx_desc_queue;
#endif

#if EXT_PHY_PATCH
    char* pu8RXBuf;
#endif

    /* Suspend and resume */
    unsigned long ep_flag;

    struct net_device* netdev;

    struct device* mstar_class_emac_device;
#if MSTAR_EMAC_NAPI
    struct napi_struct napi;
#else
    spinlock_t      mutexRXInt;
#endif
    MSYS_DMEM_INFO mem_info;

    u32 txd_num;
    u32 txq_num_sw;

    // led gpio
    int led_orange;
    int led_green;
    int led_count;
    int led_flick_speed;

    // mac address
    u8 sa[4][6];

    // BasicConfigEMAC ThisBCE;
    // UtilityVarsEMAC ThisUVE;

    // struct timer_list timer_link;

    //
    unsigned int irq_emac;
    unsigned int irq_lan;

    //
    u32 emacRIU;
    u32 emacX32;
    u32 phyRIU;

    //
    u32 pad_reg;
    u32 pad_msk;
    u32 pad_val;

    u32 phy_mode;

    // led
    u32 pad_led_reg;
    u32 pad_led_msk;
    u32 pad_led_val;

    // hal handle
    void* hal;

    ////////////////
    u32 gu32intrEnable;
    u32 irq_count[32];

    u32             skb_tx_send;
    u32             skb_tx_free;
    u64             data_done;
    struct timespec data_time_last;
    spinlock_t      emac_data_done_lock;

    u32 txPkt;
    u32 txInt;

    u32           initstate;
    u32           contiROVR;
    unsigned long oldTime;
    unsigned long PreLinkStatus;

    //
    const char* name;
    u8          bInit;
    u8          bEthCfg;

    u8 u8Minor;

#if KERNEL_PHY
    /// phy separation
    struct mii_bus* mii_bus;
    struct device*  dev; // don't know its useness
#endif

#if 0
    // not sure about its use
    u32 gu32PhyResetCount1;
    u32 gu32PhyResetCount2;
    u32 gu32PhyResetCount3;
    u32 gu32PhyResetCount4;
    u32 gu32PhyResetCount;
#endif

#if EMAC_FLOW_CONTROL_TX
    // TX pause packet (TX flow control)
    spinlock_t        mutexFlowTX;
    struct timer_list timerFlowTX;
    // int                 isPauseTX;
#endif

#if EMAC_FLOW_CONTROL_RX
    // RX pause packet (TX flow control)
    u8* pu8PausePkt;
    u8  u8PausePktSize;
    u8  isPausePkt;
#endif

#if REDUCE_CPU_FOR_RBNA
    spinlock_t        mutexIntRX;
    struct timer_list timerIntRX;
#endif

#if DYNAMIC_INT_RX
    struct timespec rx_stats_time;
    u32             rx_stats_packet;
    u8              rx_stats_enable;
#endif

#if EMAC_SG
    // char*               pTxBuf;
    // int                 TxBufIdx;
    int maxSG;
#endif // #if EMAC_SG
};

#endif
// -----------------------------------------------------------------------------
// Linux EMAC.h End
// -----------------------------------------------------------------------------
