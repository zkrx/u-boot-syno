/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

********************************************************************************
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

#ifndef __INCmv88F6082SysHwConfigh
#define __INCmv88F6082SysHwConfigh

/****************************************/
/* Soc supporetd Units definitions		*/
/****************************************/

#define MV_INCLUDE_PEX
#define MV_INCLUDE_CESA
#define MV_INCLUDE_GIG_ETH
#define MV_INCLUDE_INTEG_SATA
#define MV_INCLUDE_USB
#define MV_INCLUDE_TWSI
#define MV_INCLUDE_NAND
#define MV_INCLUDE_UART
#define MV_INCLUDE_SPI
#define MV_INCLUDE_INTEG_MFLASH
/*#define MV_INCLUDE_INTEG_MFLASH_SPI*/

/*********************************************/
/* Board Specific defines : On-Board devices */
/*********************************************/

/* DRAM ddim detection support */
#define MV_INC_BOARD_DDIM
/* On-Board NAND Flash support */
#define MV_INC_BOARD_NAND_FLASH
/* On-Board SPI Flash support */
#define MV_INC_BOARD_SPI_FLASH
/* On-Board RTC */
#define MV_INC_BOARD_RTC

/* PEX-PCI\PCI-PCI Bridge*/
#define PCI0_IF_PTP		0		/* no Bridge on pciIf0*/
#define PCI1_IF_PTP		0		/* no Bridge on pciIf1*/

#if defined(MV_INCLUDE_INTEG_MFLASH_SPI) && defined (MV_INC_BOARD_SPI_FLASH)
#error "Cannot Use MFlash in SPI mode while configuring and external SPI flash"
#endif

/*********************************************/
/* U-Boot Specific							*/
/*********************************************/
#define MV_INCLUDE_MONT_EXT

/* Tailgate WA for Nanya memory */
/* #define	MV_MEM_OVER_PCI_WA */

#if defined(MV_INCLUDE_MONT_EXT)
#define MV_INCLUDE_MONT_MMU
#define MV_INCLUDE_MONT_MPU
#if defined(MV_INC_BOARD_NOR_FLASH)
#define MV_INCLUDE_MONT_FFS
#endif
#define MV_INCLUDE_MONT_LOAD_NET
#endif


/*********************************************/
/* DB boards specifics 						 */
/*********************************************/

#if defined(DB_88F6082BP) || defined(DB_88F6082LBP)

#undef MV_INC_BOARD_DDIM

#endif

/*********************************************/
/* RD boards specifics 						 */
/*********************************************/

#if defined(RD_88F6082NAS)
#undef MV_INC_BOARD_DDIM
#undef MV_INCLUDE_IDMA
#undef MV_INCLUDE_PCI
#undef MV_INC_BOARD_SPI_FLASH
#endif

#if defined (RD_88F6082GE_SATA) || defined (RD_88F6082SA) || defined (RD_88F6082DAS_PLUS) \
	|| defined (RD_88F6082MICRO_DAS_NAS)
#undef MV_INC_BOARD_NAND_FLASH
#undef MV_INC_BOARD_DDIM
#undef MV_INCLUDE_IDMA
#undef MV_INCLUDE_PCI
#endif

#if defined (RD_88F6082MICRO_DAS_NAS)
#define MV_INC_DRAM_MFG_TEST
#endif

#if defined (RD_88F6082_DX243)
#undef MV_INC_BOARD_DDIM
#undef MV_INCLUDE_IDMA
#undef MV_INCLUDE_PCI
#define MV_INCLUDE_PEX
#undef MV_INCLUDE_CESA
#undef MV_INCLUDE_INTEG_SATA
#undef MV_INCLUDE_USB
#undef MV_INC_BOARD_NAND_FLASH
#endif

/* 
 *  System memory mapping 
 */

/* SDRAM: actual mapping is auto detected */
#define SDRAM_CS0_BASE  0x00000000
#define SDRAM_CS0_SIZE  _256M

#define SDRAM_CS1_BASE  0x10000000
#define SDRAM_CS1_SIZE  _256M

/* PEX */
#define PEX0_MEM_BASE 0x90000000
#define PEX0_MEM_SIZE _128M

#define PEX0_IO_BASE 0xf0000000
#define PEX0_IO_SIZE _1M

/* PEX Work arround */
/* the target we will use for the workarround */
#define PEX_CONFIG_RW_WA_TARGET PEX0_MEM
/*a flag that indicates if we are going to use the 
size and base of the target we using for the workarround
window */
#define PEX_CONFIG_RW_WA_USE_ORIGINAL_WIN_VALUES 1
/* if the above flag is 0 then the following values
will be used for the workarround window base and size,
otherwise the following defines will be ignored */
#define PEX_CONFIG_RW_WA_BASE 0x50000000
#define PEX_CONFIG_RW_WA_SIZE _16M

/* Device: CS0 - NAND, CS1 - MFlash, CS2 - SPI, CS3 - Boot ROM, CS4 - Boot device */
#define DEVICE_CS0_BASE 0xf8000000
#define DEVICE_CS0_SIZE _1M

#define DEVICE_MFLASH_BASE  0xf8100000
#define DEVICE_CS1_BASE	    DEVICE_MFLASH_BASE
#define DEVICE_CS1_SIZE _1M

#define DEVICE_CS2_BASE 0xf9000000
#define DEVICE_CS2_SIZE _16M

#define DEVICE_CS3_BASE 0xfa000000
#define DEVICE_CS3_SIZE _1M

#define DEVICE_CS4_BASE BOOTDEV_CS_BASE
#define DEVICE_CS4_SIZE BOOTDEV_CS_SIZE

#define CFG_NAND_BASE 	DEVICE_CS0_BASE

/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xF1000000


#define CRYPT_ENG_BASE	0xFB000000
#define CRYPT_ENG_SIZE	_64K


#if defined (MV_INCLUDE_PCI) && defined (MV_INCLUDE_PEX)

#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#define PCI_IF1_MEM0_BASE 	PCI0_MEM_BASE
#define PCI_IF1_MEM0_SIZE 	PCI0_MEM_SIZE
#define PCI_IF1_IO_BASE 	PCI0_IO_BASE
#define PCI_IF1_IO_SIZE 	PCI0_IO_SIZE

#elif defined (MV_INCLUDE_PCI)

#define PCI_IF0_MEM0_BASE 	PCI0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PCI0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PCI0_IO_BASE
#define PCI_IF0_IO_SIZE 	PCI0_IO_SIZE

#elif defined (MV_INCLUDE_PEX)

#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#endif


/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE

#define PCI_ARBITER_CTRL    /* Use/unuse the Marvell integrated PCI arbiter	*/
#undef	PCI_ARBITER_BOARD	/* Use/unuse the PCI arbiter on board			*/

/* Check macro validity */
#if defined(PCI_ARBITER_CTRL) && defined (PCI_ARBITER_BOARD)
	#error "Please select either integrated PCI arbiter or board arbiter"
#endif

/* Board clock detection */
#define TCLK_AUTO_DETECT    /* Use Tclk auto detection 		*/
#define SYSCLK_AUTO_DETECT	/* Use SysClk auto detection 	*/
#define PCLCK_AUTO_DETECT  /* Use PClk auto detection */


/************* Ethernet driver configuration ********************/

/*#define ETH_JUMBO_SUPPORT*/
/* HW cache coherency configuration */
#define DMA_RAM_COHER	    NO_COHERENCY
#define ETHER_DRAM_COHER    MV_UNCACHED 
#define INTEG_SRAM_COHER    MV_UNCACHED  /* Where integrated SRAM available */

#define ETH_DESCR_IN_SDRAM
#undef  ETH_DESCR_IN_SRAM

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


/* CPU address decode table. Note that table entry number must match its    */
/* winNum enumerator. For example, table entry '4' must describe Deivce CS0 */
/* winNum which is represent by DEVICE_CS0 enumerator (4).                  */
#define MV_CPU_IF_ADDR_WIN_MAP_TBL	{				\
/*     base low      base high     size       	   WinNum     enable */ \
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE} ,0xFFFFFFFF,DIS}, \
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE} ,0xFFFFFFFF,DIS}, \
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE } ,0x0       ,EN},	\
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE  } ,0x2       ,EN},	\
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE},0x8       ,EN},	\
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE},0x1	 ,EN},	\
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE},0x3	 ,EN},	\
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE},0x5       ,EN},	\
	{{DEVICE_CS3_BASE,    0,      DEVICE_CS3_SIZE},0x6       ,EN},	\
	{{DEVICE_CS4_BASE,    0,      DEVICE_CS4_SIZE},0x4       ,EN},	\
	{{CRYPT_ENG_BASE,     0,      CRYPT_ENG_SIZE} ,0x7  	 ,EN},	\
    /* Table terminator */\
    {{TBL_TERM, TBL_TERM, TBL_TERM}, TBL_TERM,TBL_TERM}			\
};

#endif /* __INCmv88F6082SysHwConfigh */
