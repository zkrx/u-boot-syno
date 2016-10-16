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

#ifndef __INCmvSysHwConfigh
#define __INCmvSysHwConfigh


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

#define _1G         0x40000000
#define _2G         0x80000000

#if defined(DB_CUSTOMER)
#include "mvCustomerSysHwConfig.h"
#elif defined(MV_88F1181)
#include "mv88F1X81SysHwConfig.h"
#elif defined(MV_88F1281)
#include "mv88F1281SysHwConfig.h"
#elif defined(MV_88F5182)
#include "mv88F5182SysHwConfig.h"
#elif defined(MV_88F5082)
#include "mv88F5082SysHwConfig.h"
#elif defined(MV_88F5181L)
#include "mv88F5181LSysHwConfig.h"
#elif defined(MV_88W8660)
#include "mv88w8660SysHwConfig.h"
#elif defined(MV_88F5181)
#include "mv88F5X81SysHwConfig.h"
#elif defined(MV_88F5180N)
#include "mv88F5180NSysHwConfig.h"
#elif defined(MV_88F6082)
#include "mv88F6082SysHwConfig.h"
#elif defined(MV_88F6183)
#include "mv88F6183SysHwConfig.h"
#elif defined(MV_88F6183L)
#include "mv88F6183LSysHwConfig.h"
#endif

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

/* We use the following registers to store DRAM interface pre configuration   */
/* auto-detection results													  */
/* IMPORTANT: We are using mask register for that purpose. Before writing     */
/* to units mask register, make sure main maks register is set to disable     */
/* all interrupts.                                                            */
#define DRAM_BUF_REG0	0x1011c	/* sdram bank 0 size	        */  
#define DRAM_BUF_REG1	0x20318	/* sdram config			        */
#define DRAM_BUF_REG2   0x20114	/* sdram mode 			        */
#define DRAM_BUF_REG3	0x20320	/* dunit control low 	        */          
#define DRAM_BUF_REG4	0x20404	/* sdram address control        */
#define DRAM_BUF_REG5	0x2040c	/* sdram timing control low     */

#if defined(MV_INCLUDE_PEX)
#define DRAM_BUF_REG6	0x40108	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x40114	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x41910	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x41a08	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x41a30	/* sdram Extended Mode		    */
#elif defined(MV_INCLUDE_IDMA)
#define DRAM_BUF_REG6	0x60810	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x60814	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x60818	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x6081c	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x60820	/* sdram Extended Mode		*/
#elif defined(DB_FPGA)
#define DRAM_BUF_REG6	0x10108	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x10118	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x10118	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x10118	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x1010c	/* sdram Extended Mode		*/
#endif

/* Following the pre-configuration registers default values restored after    */
/* auto-detection is done                                                     */
#define DRAM_BUF_REG0_DV    	0           /* GPIO Interrupt Level Mask Reg      */
#define DRAM_BUF_REG1_DV	0           /* ARM Timer 1 reload register        */
#define DRAM_BUF_REG2_DV    	0           /* AHB to MBUS Bridge int Mask Reg    */
#define DRAM_BUF_REG3_DV	0           /* ARM Watchdog Timer Register        */
#define DRAM_BUF_REG4_DV	0           /* Host to ARM Doorbel Mask Register  */
#define DRAM_BUF_REG5_DV	0           /* ARM To Host Doorbel Mask Register  */
#define DRAM_BUF_REG6_DV	0           /* PCI Exp Uncorrectable Err Mask Reg */
#define DRAM_BUF_REG7_DV	0           /* PCI Exp Correctable Err Mask Reg   */
#define DRAM_BUF_REG8_DV	0           /* PCI Express interrupt Mask Reg     */
#define DRAM_BUF_REG9_DV	0           /* PCI Express Spare Register         */
#if defined(DB_FPGA)
#define DRAM_BUF_REG10_DV	0x0
#else
#define DRAM_BUF_REG10_DV	0x012C0004  /* PCI Exp Acknowledge Timers (x4) Reg*/
#endif


#define MV_ETH_TX_Q_NUM		1
#define MV_ETH_RX_Q_NUM		1
#define ETH_NUM_OF_RX_DESCR     64
#define ETH_NUM_OF_TX_DESCR     ETH_NUM_OF_RX_DESCR*2

/* port's default queueus */
#define ETH_DEF_TXQ         0
#define ETH_DEF_RXQ         0

#define MV_DISABLE_PEX_DEVICE_BAR 

#endif /* __INCmvSysHwConfigh */
