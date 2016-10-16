/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/
/*******************************************************************************
* mvSysHwCfg.h - Marvell system HW configuration file
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __INCmvSysHwConfigh
#define __INCmvSysHwConfigh
/****************************************/
/* Soc supporetd Units definitions	*/
/****************************************/

#define MV_INCLUDE_PEX
#define MV_INCLUDE_IDMA
#define MV_INCLUDE_XOR
#define MV_INCLUDE_SPI
#define MV_INCLUDE_SATA
#define MV_INCLUDE_USB
#define MV_INCLUDE_GIG_ETH
#define MV_INCLUDE_TWSI
#define MV_INCLUDE_CLK_PWR_CNTRL
#define MV_INCLUDE_INTEG_SATA

/*********************************************/
/* Board Specific defines : On-Board devices */
/*********************************************/

/* DRAM ddim detection support */
#define MV_INC_BOARD_DDIM
/* On-Board NOR Flash support */
#define MV_INC_BOARD_NOR_FLASH
/* On-Board NAND Flash support */
#define MV_INC_BOARD_NAND_FLASH
/* On-Board SPI Flash support */
#define MV_INC_BOARD_SPI_FLASH
/* On-Board RTC */
#define MV_INC_BOARD_RTC
/* On-Board PCI\PEX SATA driver*/
#define MV_INC_BOARD_PCI_SATA

#if defined(DB_MV78XX0)
#define CFG_DRAM_BANKS		4
#elif defined(RD_MV78XX0_AMC)
#define MV_STATIC_DRAM_ON_BOARD
#define CFG_DRAM_BANKS		1
#define CFG_NO_FLASH
#elif defined(RD_MV78XX0_MASA)
#define CFG_DRAM_BANKS		2
#define CFG_NO_FLASH
#elif defined(RD_MV78XX0_H3C)
#define MV_STATIC_DRAM_ON_BOARD
#define CFG_DRAM_BANKS		1
#define AMD_FLASH_16BIT_IN_8BIT_MODE
#endif

/************************************************/
/* U-Boot Specific				*/
/************************************************/
#define MV_INCLUDE_MONT_EXT

#if defined(MV_INCLUDE_MONT_EXT)
#define MV_INCLUDE_MONT_MMU
#if defined(MV_INC_BOARD_NOR_FLASH)
#define MV_INCLUDE_MONT_FFS
#endif
#endif


#define MV_CACHEABLE(address) ((address) | 0x80000000)

/* includes */
#define _1K         0x00000400
#define _4K         0x00001000
#define _8K         0x00002000
#define _16K        0x00004000
#define _32K        0x00008000
#define _64K        0x00010000
#define _128K       0x00020000
#define _256K       0x00040000
#define _512K       0x00080000

#define _1M         0x00100000
#define _2M         0x00200000
#define _4M         0x00400000
#define _8M         0x00800000
#define _16M        0x01000000
#define _32M        0x02000000
#define _64M        0x04000000
#define _128M       0x08000000
#define _256M       0x10000000
#define _512M       0x20000000

/* 
 *  System memory mapping 
 */

/* SDRAM: actual mapping is auto detected */
#define SDRAM_CS0_BASE  0x00000000
#define SDRAM_CS0_SIZE  _8M

#define SDRAM_CS1_BASE  0x00800000
#define SDRAM_CS1_SIZE  _8M

#define SDRAM_CS2_BASE  0x01000000
#define SDRAM_CS2_SIZE  _8M

#define SDRAM_CS3_BASE  0x01800000
#define SDRAM_CS3_SIZE  _8M

/* PCI/PEX: IO and memory size */
#define PCIx_IO_SIZE    _1M
#define PCIx_MEM0_SIZE  _32M

/* PEX Work arround */
/* the target we will use for the workarround */
#define PEX_CONFIG_RW_WA_TARGET PEX0_MEM
/*a flag that indicates if we are going to use the 
size and base of the target we using for the workarround
window */

/* PCI0: IO and memory space */
#define PCI0_IO_BASE    0xf0000000
#define PCI0_IO_SIZE    _1M
#define PCI0_MEM0_BASE  0xc0000000
#define PCI0_MEM0_ME_BASE  0x90000000
#define PCI0_MEM0_SIZE  _64M

/* PCI1: IO and memory space */
#define PCI1_IO_BASE    0xf0100000
#define PCI1_IO_SIZE    _1M
#define PCI1_MEM0_BASE  0xc4000000
#define PCI1_MEM0_ME_BASE  0x94000000
#define PCI1_MEM0_SIZE  _64M

/* PCI2: IO and memory space */
#define PCI2_IO_BASE    0xf0200000
#define PCI2_IO_SIZE    _1M
#define PCI2_MEM0_BASE  0xd0000000
#define PCI2_MEM0_SIZE  _64M

/* PCI3: IO and memory space */
#define PCI3_IO_BASE    0xf0300000
#define PCI3_IO_SIZE    _1M
#define PCI3_MEM0_BASE  0xcc000000
#define PCI3_MEM0_ME_BASE  0x98000000
#define PCI3_MEM0_SIZE  _64M

/* PCI4: IO and memory space */
#define PCI4_IO_BASE    0xf0400000
#define PCI4_IO_SIZE    _1M
#define PCI4_MEM0_BASE  0xc8000000
#define PCI4_MEM0_ME_BASE  0x9c000000
#define PCI4_MEM0_SIZE  _64M

/* PCI5: IO and memory space */
#define PCI5_IO_BASE    0
#define PCI5_IO_SIZE    0
#define PCI5_MEM0_BASE  0xd4000000
#define PCI5_MEM0_SIZE  _64M

/* PCI6: IO and memory space */
#define PCI6_IO_BASE    0
#define PCI6_IO_SIZE    0
#define PCI6_MEM0_BASE  0xd8000000
#define PCI6_MEM0_SIZE  _64M

/* PCI7: IO and memory space */
#define PCI7_IO_BASE    0
#define PCI7_IO_SIZE    0
#define PCI7_MEM0_BASE  0xdc000000
#define PCI7_MEM0_SIZE  _64M

/* Crypto memory space */
#define CRYPTO_BASE  0xd8000000
#define CRYPTO_SIZE  _1M

/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xf1000000 /*0x14000000*/
/* #define IDMA_INTER_REGS_BASE	0xf1060000 0x14000000 */

/* Device: CS0 - 32MB 32bit FLASH, CS1 - 7seg, CS2 - NAND, CS3 - SPI */
#define DEVICE_CS0_BASE 0xf8000000
#define DEVICE_CS0_SIZE _32M

#define DEVICE_CS1_BASE 0xfe000000
#define DEVICE_CS1_SIZE _1M

#define DEVICE_CS2_BASE 0xfc000000
#define DEVICE_CS2_SIZE _1M

#define DEVICE_CS3_BASE 0xfd000000
#define DEVICE_CS3_SIZE _8M

#define DEVICE_SPI_BASE DEVICE_CS3_BASE
#define DEVICE_SPI_SIZE DEVICE_CS3_SIZE

#if defined(MV_BOOTSIZE_256K)

#define BOOTDEV_CS_SIZE _256K

#elif defined(MV_BOOTSIZE_512K)

#define BOOTDEV_CS_SIZE _512K

#elif defined(MV_BOOTSIZE_4M)

#define BOOTDEV_CS_SIZE _4M

#elif defined(MV_BOOTSIZE_8M)

#define BOOTDEV_CS_SIZE _8M

#elif defined(MV_BOOTSIZE_16M)

#define BOOTDEV_CS_SIZE _16M

#elif defined(MV_BOOTSIZE_32M)

#define BOOTDEV_CS_SIZE _32M

#elif defined(MV_BOOTSIZE_64M)

#define BOOTDEV_CS_SIZE _64M

#elif defined(MV_NAND_BOOT)

#define BOOTDEV_CS_SIZE _512K

#else

#define Error MV_BOOTSIZE undefined

#endif                                               

#define BOOTDEV_CS_BASE	((0xFFFFFFFF - BOOTDEV_CS_SIZE) + 1)

#if !defined(MV_NAND_BOOT)
#define CFG_NAND_BASE   DEVICE_CS2_BASE
#else
#define CFG_NAND_BASE   BOOTDEV_CS_BASE
#endif

#define BOOT_FLASH_INDEX					1
#define MAIN_FLASH_INDEX					0

/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE

/* These addresses defines the place where global parameters will be placed	*/
/* in case running from ROM. We Use SYS_MEM_TOP. See bootInit.c file		*/
#define DRAM_DETECT_FLAG_ADDR 	0x03000000
#define DRAM_CONFIG_ROM_ADDR 	0x03000004

#define MV_CPU_IF_ADDR_WIN_MAP_TBL	{				\
/*     	base low      base high      size        	WinNum		enable/disable */ \
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE }, 0xFFFFFFFF,	EN },/*  0 */ \
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE }, 0xFFFFFFFF,	EN },/*  1 */ \
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE }, 0xFFFFFFFF,	EN },/*  2 */ \
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE }, 0xFFFFFFFF,	EN },/*  3 */ \
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE}, 10,		EN },/*  4 */ \
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE}, 11,		EN },/*  5 */ \
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 12,		EN },/*  6 */ \
	{{DEVICE_CS3_BASE,    0,      DEVICE_CS3_SIZE}, 9,		DIS },/*  7 */ \
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 13,		EN },/*  8 */ \
	{{DEVICE_SPI_BASE,    0,      DEVICE_SPI_SIZE}, 9,		EN },/*  7 */ \
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   }, 0,		EN },/*  9 */ \
	{{PCI0_MEM0_BASE ,    0,      PCI0_MEM0_SIZE }, 1,		EN },/* 10 */ \
	{{PCI1_IO_BASE   ,    0,      PCI1_IO_SIZE   }, 2,		EN },/* 11 */ \
	{{PCI1_MEM0_BASE ,    0,      PCI1_MEM0_SIZE }, 3,		EN },/* 12 */ \
	{{PCI2_IO_BASE   ,    0,      PCI2_IO_SIZE   }, 4,		DIS},/* 13 */ \
	{{PCI2_MEM0_BASE ,    0,      PCI2_MEM0_SIZE }, 5,		DIS},/* 14 */ \
	{{PCI3_IO_BASE   ,    0,      PCI3_IO_SIZE   }, 4,		EN },/* 15 */ \
	{{PCI3_MEM0_BASE ,    0,      PCI3_MEM0_SIZE }, 5,		EN },/* 16 */ \
	{{PCI4_IO_BASE   ,    0,      PCI4_IO_SIZE   }, 6,		EN },/* 17 */ \
	{{PCI4_MEM0_BASE ,    0,      PCI4_MEM0_SIZE }, 7,		EN },/* 18 */ \
	{{PCI5_IO_BASE   ,    0,      PCI5_IO_SIZE   }, 5,		DIS},/* 19 */ \
	{{PCI5_MEM0_BASE ,    0,      PCI5_MEM0_SIZE }, 5,		DIS},/* 20 */ \
	{{PCI6_IO_BASE   ,    0,      PCI6_IO_SIZE   }, 6,		DIS},/* 21 */ \
	{{PCI6_MEM0_BASE ,    0,      PCI6_MEM0_SIZE }, 6,		DIS},/* 22 */ \
	{{PCI7_IO_BASE   ,    0,      PCI7_IO_SIZE   }, 7,		DIS},/* 23 */ \
	{{PCI7_MEM0_BASE ,    0,      PCI7_MEM0_SIZE }, 7,		DIS},/* 24 */ \
	{{CRYPTO_BASE ,	      0,      CRYPTO_SIZE    }, 8,		EN },/* 25 */ \
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE}, 14,		EN },/* 26 */ \
    	/* Table terminator */ \
   	{{TBL_TERM, TBL_TERM, TBL_TERM}, TBL_TERM, TBL_TERM} \
};

#define MV_CPU_IF_ADDR_WIN_MON_EXT_MAP_TBL	{				\
/*     	base low      base high      size        	WinNum		enable/disable */ \
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE }, 0xFFFFFFFF,	EN },/*  0 */ \
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE }, 0xFFFFFFFF,	EN },/*  1 */ \
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE }, 0xFFFFFFFF,	EN },/*  2 */ \
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE }, 0xFFFFFFFF,	EN },/*  3 */ \
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE}, 10,		EN },/*  4 */ \
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE}, 11,		EN },/*  5 */ \
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 12,		EN },/*  6 */ \
	{{DEVICE_CS3_BASE,    0,      DEVICE_CS3_SIZE}, 9,		DIS },/*  7 */ \
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 13,		EN },/*  8 */ \
	{{DEVICE_SPI_BASE,    0,      DEVICE_SPI_SIZE}, 9,		EN },/*  7 */ \
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   }, 0,		EN },/*  9 */ \
	{{PCI0_MEM0_ME_BASE,  0,      PCI0_MEM0_SIZE }, 1,		EN },/* 10 */ \
	{{PCI1_IO_BASE   ,    0,      PCI1_IO_SIZE   }, 2,		EN },/* 11 */ \
	{{PCI1_MEM0_ME_BASE,  0,      PCI1_MEM0_SIZE }, 3,		EN },/* 12 */ \
	{{PCI2_IO_BASE   ,    0,      PCI2_IO_SIZE   }, 4,		DIS},/* 13 */ \
	{{PCI2_MEM0_BASE ,    0,      PCI2_MEM0_SIZE }, 5,		DIS},/* 14 */ \
	{{PCI3_IO_BASE   ,    0,      PCI3_IO_SIZE   }, 4,		EN },/* 15 */ \
	{{PCI3_MEM0_ME_BASE,  0,      PCI3_MEM0_SIZE }, 5,		EN },/* 16 */ \
	{{PCI4_IO_BASE   ,    0,      PCI4_IO_SIZE   }, 6,		EN },/* 17 */ \
	{{PCI4_MEM0_ME_BASE,  0,      PCI4_MEM0_SIZE }, 7,		EN },/* 18 */ \
	{{PCI5_IO_BASE   ,    0,      PCI5_IO_SIZE   }, 5,		DIS},/* 19 */ \
	{{PCI5_MEM0_BASE ,    0,      PCI5_MEM0_SIZE }, 5,		DIS},/* 20 */ \
	{{PCI6_IO_BASE   ,    0,      PCI6_IO_SIZE   }, 6,		DIS},/* 21 */ \
	{{PCI6_MEM0_BASE ,    0,      PCI6_MEM0_SIZE }, 6,		DIS},/* 22 */ \
	{{PCI7_IO_BASE   ,    0,      PCI7_IO_SIZE   }, 7,		DIS},/* 23 */ \
	{{PCI7_MEM0_BASE ,    0,      PCI7_MEM0_SIZE }, 7,		DIS},/* 24 */ \
	{{CRYPTO_BASE ,	      0,      CRYPTO_SIZE    }, 8,		EN },/* 25 */ \
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE}, 14,		EN },/* 26 */ \
    	/* Table terminator */ \
   	{{TBL_TERM, TBL_TERM, TBL_TERM}, TBL_TERM, TBL_TERM} \
};

/* We use the following registers to store DRAM interface pre configuration   	*/
/* auto-detection results							*/
#define DRAM_BUF_REG0	0x60810	/* sdram bank 0 size	        */
#define DRAM_BUF_REG1	0x60814	/* sdram config			*/
#define DRAM_BUF_REG2   0x60818	/* sdram mode 			*/
#define DRAM_BUF_REG3	0x6081c	/* dunit control low 	        */          
#define DRAM_BUF_REG4	0x60820	/* sdram address control        */
#define DRAM_BUF_REG5	0x60824	/* sdram timing control low     */
#define DRAM_BUF_REG6	0x60828	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x6082c	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x60830	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x60834	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x60838	/* sdram Extended Mode		*/
#define DRAM_BUF_REG11	0x6083c	/* sdram Ddr2 Timing Low 	*/
#define DRAM_BUF_REG12	0x60870	/* sdram Ddr2 Timing High	*/
#define DRAM_BUF_REG13	0x60874	/* dunit control high		*/
#define DRAM_BUF_REG14	0x60878	/* if == '1' second dimm exist  */

/* HW cache coherency configuration */
#define DRAM_COHERENCY	    NO_COHERENCY

/* PCI stuff */
/* Local bus and device number of PCI0/1*/
#define PCI_HOST_BUS_NUM(pciIf) 0
#define PCI_HOST_DEV_NUM(pciIf) 0
#define PEX_HOST_BUS_NUM(pexIf) (1 * pexIf)
#define PEX_HOST_DEV_NUM(pexIf) 0


#define PCI_ARBITER_CTRL    /* Use/unuse the Marvell integrated PCI arbiter	*/
#undef	PCI_ARBITER_BOARD	/* Use/unuse the PCI arbiter on board			*/

/* Check macro validity */
#if defined(PCI_ARBITER_CTRL) && defined (PCI_ARBITER_BOARD)
	#error "Please select either integrated PCI arbiter or board arbiter"
#endif

/* Board clock detection */
#undef TCLK_AUTO_DETECT    /* Use Tclk auto detection 		*/
#undef SYSCLK_AUTO_DETECT	/* Use SysClk auto detection 	*/

/* Memory uncached, HW or SW cache coherency is not needed */
#define MV_UNCACHED             0   
/* Memory cached, HW cache coherency supported in WriteThrough mode */
#define MV_CACHE_COHER_HW_WT    1
/* Memory cached, HW cache coherency supported in WriteBack mode */
#define MV_CACHE_COHER_HW_WB    2
/* Memory cached, No HW cache coherency, Cache coherency must be in SW */
#define MV_CACHE_COHER_SW       3


#if INTEG_SRAM_COHER == MV_CACHE_COHER_HW_WB
#   define INTEG_SRAM_CONFIG_STR      "MV_CACHE_COHER_HW_WB"
#elif INTEG_SRAM_COHER == MV_CACHE_COHER_HW_WT
#   define INTEG_SRAM_CONFIG_STR      "MV_CACHE_COHER_HW_WT"
#elif INTEG_SRAM_COHER == MV_CACHE_COHER_SW
#   define INTEG_SRAM_CONFIG_STR      "MV_CACHE_COHER_SW"
#elif INTEG_SRAM_COHER == MV_UNCACHED
#   define INTEG_SRAM_CONFIG_STR      "MV_UNCACHED"
#else
#   error "Unexpected INTEG_SRAM_COHER value"
#endif /* INTEG_SRAM_COHER */

/************* Ethernet driver configuration ********************/

/*#define ETH_JUMBO_SUPPORT*/
#define ETH_DEF_TXQ    		0
#define ETH_DEF_RXQ    		0
#define MV_ETH_TX_Q_NUM		    1
#define MV_ETH_RX_Q_NUM		    1
#define ETH_NUM_OF_RX_DESCR     64
#define ETH_NUM_OF_TX_DESCR     ETH_NUM_OF_RX_DESCR*2

#define ETH_DESCR_IN_SDRAM
#undef  ETH_DESCR_IN_SRAM

#if defined(ETH_DESCR_IN_SRAM)
#   define ETH_DESCR_CONFIG_STR "ETH_DESCR_IN_SRAM"
#elif defined(ETH_DESCR_IN_SDRAM)
#   define ETH_DESCR_CONFIG_STR "ETH_DESCR_IN_SDRAM"
#else 
#   error "ETH_DESCR_IN_SRAM or ETH_DESCR_IN_SDRAM must be defined"
#endif /* ETH_DESCR_IN_SRAM or ETH_DESCR_IN_SDRAM*/

#if (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WB)
#   define ETH_SDRAM_CONFIG_STR      "MV_CACHE_COHER_HW_WB"
#elif (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WT)
#   define ETH_SDRAM_CONFIG_STR      "MV_CACHE_COHER_HW_WT"
#elif (ETHER_DRAM_COHER == MV_CACHE_COHER_SW)
#   define ETH_SDRAM_CONFIG_STR      "MV_CACHE_COHER_SW"
#elif (ETHER_DRAM_COHER == MV_UNCACHED)
#   define ETH_SDRAM_CONFIG_STR      "MV_UNCACHED"
#else
#   error "Unexpected ETHER_DRAM_COHER value"

#endif /* ETHER_DRAM_COHER */

/*********** Idma default configuration ***********/
#define UBOOT_CNTRL_DMA_DV     (ICCLR_DST_BURST_LIM_8BYTE | \
				ICCLR_SRC_INC | \
				ICCLR_DST_INC | \
				ICCLR_SRC_BURST_LIM_8BYTE | \
				ICCLR_NON_CHAIN_MODE | \
				ICCLR_BLOCK_MODE )
                
#define MV_BRIDGE_REORDER_WA(param)		\
{						\
	volatile MV_U8 dummy = (MV_U8)param;\
}

			   
#endif /* __INCmvSysHwConfigh */
