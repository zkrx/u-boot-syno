/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

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
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef __INCnBootstraph
#define __INCnBootstraph

/* includes */

/* defines  */
#if  defined(DB_88F6082BP) || defined(DB_88F6082LBP) || defined(DB_88F6082SA) || defined(DB_88W8660) ||\
	 defined(RD_DB_88F5181L) || defined(DB_88F5182) || defined(DB_88F5082) || defined(DB_88F5181) ||\
	 defined(DB_FPGA) || defined(DB_88F6183BP) || defined(DB_88F6183LBP) || defined(RD_88F6183GP) ||\
      defined(DB_88F5182_A) ||defined(DB_88F5181_OLD)
#define _DDR2
#endif

#if  defined(RD_88F5181_GTWFE) || defined(RD_88F5181_GTWGE) || defined(RD_88F5182) || defined(DB_MNG)\
 	|| defined(RD_88F5082) || defined(RD_88F6183GP) 
#define _DDR1
#endif

#if  defined(MV_88F5281)
#define NAND_CTRL_88F528x    /* NAND controller 88F528x */
#endif  /* defined(MV_88F5281) */

#if  defined(RD_88W8660_AP82S)
#define RD_88W8660_AP82S
#endif

#undef DEV_X8
#undef DEV_DENS_256Mb
#define DEV_DENS_512Mb

#if  defined(RD_88W8660_AP82S)

/* Shark RD */
#define SDRAM_SIZE_REG_DV		     	0x00ff0001
#define SDRAM_CONFIG_REG_DV	     		0x03144400	
#define SDRAM_MODE_REG_DV	     		0x62	
#define SDRAM_DUNIT_CTRL_REG_DV	     		0x4041000
#define SDRAM_ADDR_CTRL_REG_DV	     		0x0
#define SDRAM_TIMING_CTRL_LOW_REG_DVAL 		0x11602220
#define SDRAM_TIMING_CTRL_HIGH_REG_DVAL	     	0x40b
#define DDR2_SDRAM_ODT_CTRL_LOW_REG_DV	     	0x0
#define DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV	     	0x0	
#define DDR2_DUNIT_ODT_CTRL_REG_DV    		0x0
#define SDRAM_EXTENDED_MODE_REG_DV     		0x0
#define SDRAM_OPEN_PAGE_CTRL_REG_DV         	0x0
#define SDRAM_FTDLL_CONFIG_REG_DV           	0x0

#elif defined(RD_88W8660) 

/* Shark RD */
#define SDRAM_SIZE_REG_DV		     	0x03ff0001
#define SDRAM_CONFIG_REG_DV	     		0x03144400	
#define SDRAM_MODE_REG_DV	     		0x62	
#define SDRAM_DUNIT_CTRL_REG_DV	     		0x4041000
#define SDRAM_ADDR_CTRL_REG_DV	     		0x10
#define SDRAM_TIMING_CTRL_LOW_REG_DVAL 		0x11602220
#define SDRAM_TIMING_CTRL_HIGH_REG_DVAL	     	0x40b
#define DDR2_SDRAM_ODT_CTRL_LOW_REG_DV	     	0x0
#define DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV	     	0x0	
#define DDR2_DUNIT_ODT_CTRL_REG_DV    		0x0
#define SDRAM_EXTENDED_MODE_REG_DV     		0x0
#define SDRAM_OPEN_PAGE_CTRL_REG_DV         	0x0
#define SDRAM_FTDLL_CONFIG_REG_DV           	0x0

#elif defined(_DDR2)

/* This is Uboot 1.6.4 dump of DB-88F5x81-DDR2-BP-A CPU @ 400 DDR @ 200
with the DIMM: MT9HTF3272AY-53EB3 (256MB DDR2 533 CL4 ECC) LE mode Orion2 B0

f1001400: 7f258400 04041040 11812220 0000040e    ..%.@... "......
f1001410: 00000010 00000000 00000000 00000432    ............2...
f1001420: 00000440 00000000 00063305 00000003    @........3......
f1001430: 76543210 fedcba98 000100ff 00000000    .2Tv............
f1001440: 00000000 00000000 00000000 00000000    ................
f1001450: 00000000 00000000 00000000 00000000    ................
f1001460: 00000000 00000000 00000000 00000000    ................
f1001470: 05699aa6 00000036 00000000 00006330    ..i.6.......0c..
f1001480: 00000000 00f95001 00000102 00000001    .....P..........
f1001490: 00000000 00010000 00000002 00000a01    ................
f10014a0: 00000000 00000000 00000001 00000000    ................
f10014b0: 00000000 00000000 00000000 00000000    ................
f10014c0: 1965328a 1965328a 00000000 00000000    .2e..2e.........
f10014d0: 00000000 00000000 00000000 00000000    ................
f10014e0: 0401a590 00000000 00000000 00000001    ................
f10014f0: 00000000 00000000 00000000 00000000    ................
*/

/* registers defaults values for a DB-88xxxxx-DDR2-BP with the DIMM:  */
/* M378T3354CZ3-CD5 0541 (256MB DDR2 533 1Rx16 CL4 ECC)               */


#ifndef MV_88W8660
#ifdef  DEV_X8
#define SDRAM_CONFIG_REG_DV                 0x03258400      /* 1400 */
#else
#define SDRAM_CONFIG_REG_DV                 0x03154400     /* 1400 */
#endif /* DEV_X */
#else
#ifdef  DEV_X8
#define SDRAM_CONFIG_REG_DV                 0x03254400      /* 1400 */
#else
#define SDRAM_CONFIG_REG_DV                 0x03154400      /* 1400 */
#endif /* DEV_X */
#endif /* MV_88W8660 */

#define SDRAM_DUNIT_CTRL_REG_DV             0x04041040      /* 1404 */
#define SDRAM_TIMING_CTRL_LOW_REG_DVAL      0x11712220      /* 1408 */
#define SDRAM_TIMING_CTRL_HIGH_REG_DVAL     0x00000104      /* 140C */

#ifdef  DEV_DENS_256Mb
#define SDRAM_ADDR_CTRL_REG_DV              0x00000010      /* 1410 */
#endif
#ifdef  DEV_DENS_512Mb
#define SDRAM_ADDR_CTRL_REG_DV              0x00000020      /* 1410 */
#endif

#define SDRAM_OPEN_PAGE_CTRL_REG_DV         0x00000000      /* 1414 */
#define SDRAM_MODE_REG_DV                   0x00000432      /* 141C */
#define SDRAM_EXTENDED_MODE_REG_DV          0x00000440      /* 1420 */
#define SDRAM_FTDLL_CONFIG_REG_DV           0x00f95000      /* 1484 */
#define DDR2_SDRAM_ODT_CTRL_LOW_REG_DV      0x84210000      /* 1494 */
#define DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV     0x00000000      /* 1498 */
#define DDR2_DUNIT_ODT_CTRL_REG_DV          0x0000780f      /* 149C */

#if defined(MV_88F6183) || defined(MV_88F6183L)
#undef SDRAM_FTDLL_CONFIG_REG_DV		
#define SDRAM_FTDLL_CONFIG_REG_DV		0
#endif

#if defined(MV_88W8660) || defined(MV_88F6183) defined(MV_88F6183L)
#define SDRAM_SIZE_REG_DV                   0x03ff0001  /* 64MB */
#else
#define SDRAM_SIZE_REG_DV                   0x07ff0001  /* 128MB */
#endif


#elif defined(_DDR1)

/* This is Uboot 1.6.7 dump of DB-88F5281-DDR1-BP-A CPU @ 533 DDR @ 133
with the DIMM: MT8VDDT1664AG-265A1 (128MB DDR1 266 CL2.5 no ECC) LE mode Orion2 B0

f1001400: 07248400 04041040 11501220 00000409    ..$.@... .P.....
f1001410: 00000000 00000000 00000000 00000062    ............b...
f1001420: 00000000 00000000 00063305 00000000    .........3......
f1001430: 76543210 fedcba98 000100ff 00000000    .2Tv............
f1001440: 00000000 00000000 00000000 00000000    ................
f1001450: 00000000 00000000 00000000 00000000    ................
f1001460: 00000000 00000000 00000000 00000000    ................
f1001470: 06b4ea54 00000055 00000000 00006330    T...U.......0c..
f1001480: 00000000 00e3f001 00000102 00000001    ................
f1001490: 00000000 00000000 00000000 00000000    ................
f10014a0: 00000000 00000000 00000001 00000000    ................
f10014b0: 00000000 00000000 00000000 00000000    ................
f10014c0: 14d21a69 14d21a69 00000000 00000000    i...i...........
f10014d0: 00000000 00000000 00000000 00000000    ................
f10014e0: ba1d4858 00000000 00000000 0000000f    XH..............
f10014f0: 00000000 00000000 00000000 00000000    ................
*/
/* registers defaults values for DB-88F5281-DDR1-BP-A CPU @ 533 DDR @ 133
with the DIMM: MT8VDDT1664AG-265A1                                          */
#define SDRAM_CONFIG_REG_DV                 0x07248400      /* 1400 */
#define SDRAM_DUNIT_CTRL_REG_DV             0x04041040      /* 1404 */
#define SDRAM_TIMING_CTRL_LOW_REG_DVAL      0x11501220      /* 1408 */
#define SDRAM_TIMING_CTRL_HIGH_REG_DVAL     0x00000409      /* 140C */
#define SDRAM_ADDR_CTRL_REG_DV              0x00000000      /* 1410 */
#define SDRAM_OPEN_PAGE_CTRL_REG_DV         0x00000000      /* 1414 */
#define SDRAM_MODE_REG_DV                   0x00000062      /* 141C */
#define SDRAM_EXTENDED_MODE_REG_DV          0x00000000      /* 1420 */
#define SDRAM_FTDLL_CONFIG_REG_DV           0x00e3f001      /* 1484 */
#define DDR2_SDRAM_ODT_CTRL_LOW_REG_DV      0x00000000      /* 1494 */
#define DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV     0x00000000      /* 1498 */
#define DDR2_DUNIT_ODT_CTRL_REG_DV          0x00000000      /* 149C */
#define SDRAM_SIZE_REG_DV                   0x03ff0001  /* 64MB */
#endif
       
/* NAND Flash access */
#define NAND_CMD_PORT       (0x1 << (NFLASH_DEV_WIDTH >> 4))
#define NAND_ADDR_PORT      (0x2 << (NFLASH_DEV_WIDTH >> 4))

/* NAND Flash Chip Capability */
#ifdef MV_LARGE_PAGE
#define NUM_BLOCKS		2048
#define PAGES_PER_BLOCK     	64
#define PAGE_SIZE      		2048	 /* Bytes */
#define SPARE_SIZE        	64
#define CFG_NAND_PAGE_SIZE	(2048)		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(128 << 10)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	(64)		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	(0)		/* Location of bad block marker	*/

#define CFG_NAND_U_BOOT_OFFS	CFG_MONITOR_BASE	/* Offset to U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	CFG_MONITOR_LEN	/* Size of RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_DST	CFG_MONITOR_IMAGE_DST	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/

#else /* ! LARGE PAGE NAND */
/* NAND Flash Chip Capability */
#define NUM_BLOCKS		2048
#define PAGES_PER_BLOCK     	32
#define PAGE_SIZE      		512	 /* Bytes */
#define SPARE_SIZE        	16
#define CFG_NAND_PAGE_SIZE	(512)		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(16 << 10)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	(32)		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	(5)		/* Location of bad block marker	*/

#define CFG_NAND_U_BOOT_OFFS	CFG_MONITOR_BASE	/* Offset to U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	CFG_MONITOR_LEN	/* Size of RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_DST	CFG_MONITOR_IMAGE_DST	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/

#endif

/*  NAND Flash Command. This appears to be generic across all NAND flash chips */
#define CMD_READ            0x00        /*  Read */
#define CMD_READ1           0x01        /*  Read1 */
#define CMD_READ2           0x50        /*  Read2 */
#define CMD_START_READ      0x30        /*  Read command after write addr */
#define CMD_READID          0x90        /*  ReadID */
#define CMD_READID2         0x91        /*  Read extended ID */
#define CMD_WRITE           0x80        /*  Write phase 1 */
#define CMD_WRITE2          0x10        /*  Write phase 2 */
#define CMD_ERASE           0x60        /*  Erase phase 1 */
#define CMD_ERASE2          0xd0        /*  Erase phase 2 */
#define CMD_STATUS          0x70        /*  Status read */
#define CMD_RESET           0xff        /*  Reset */

/*  Status bit pattern */
#define STATUS_READY        0x40        /*  Ready */
#define STATUS_ERROR        0x01        /*  Error */


#define NFLASH_DEV_WIDTH    8
#ifdef MV_LARGE_PAGE
#define BOOTER_PAGE_NUM     2
#define BOOTER_BASE         0x00020000 + PAGE_SIZE
#else
#define BOOTER_PAGE_NUM     5
#define BOOTER_BASE         0x00020000 + (3 * PAGE_SIZE)
#endif /* MV_LARGE_PAGE */
#define BOOTER_END          (BOOTER_BASE + (BOOTER_PAGE_NUM * PAGE_SIZE))

#undef  INTER_REGS_BASE
#define INTER_REGS_BASE		0xd0000000

           
#if defined(MV_BOOTROM)
#if defined(MV_88F6082)
#define NAND_FLASH_BASE		0xD8000000
#endif
#if defined(MV_88F5182)
#define NAND_FLASH_BASE		0xf0000000
#endif
#else
#define NAND_FLASH_BASE		0xffff0000
#endif /* defined(MV_BOOTROM) */

#if 0
#if defined(MV_BOOTROM)
#define UBOOT_IMAGE_OFFS    	0x4000      /* Uboot starts on block 1  */
#define UBOOT_IMAGE_SIZE    	0x80000     /* 1/2 MB of Uboot          */
#define UBOOT_IMAGE_DEST    	0x200000
#define UBOOT_IMAGE_ENV_OFFS  	0x84000  /* Uboot starts on block 1  */
#define UBOOT_IMAGE_ENV_SIZE  	0x4000      
#define UBOOT_IMAGE_ENV_DEST    0x300000 - UBOOT_IMAGE_ENV_SIZE
#else
#define UBOOT_IMAGE_OFFS    	0x24000      /* Uboot starts on block 1  */
#define UBOOT_IMAGE_SIZE    	0x80000     /* 1/2 MB of Uboot          */
#define UBOOT_IMAGE_DEST    	0x200000
#define UBOOT_IMAGE_ENV_OFFS  	UBOOT_IMAGE_OFFS - 0x20000  /* Uboot starts on block 1  */
#define UBOOT_IMAGE_ENV_SIZE  	0x4000      
#define UBOOT_IMAGE_ENV_DEST    0x300000 - UBOOT_IMAGE_ENV_SIZE
#endif /* defined(MV_BOOTROM) */
#endif

#define NBOOT_UART_CHAN     0
#define NBOOT_BAUDRATE      115200
#define NBOOT_TIMER_NUM     0


/* CPU config register (0x20100) bit[15:8] value for CPU to DDR clock ratio */ 
#define CPU_2_MBUSL_DDR_CLK     0x0000   /* clock ratio 1x2 */
/* #define CPU_2_MBUSL_DDR_CLK     0x2100  *//*  clock ratio 1x3 */
/* #define CPU_2_MBUSL_DDR_CLK     0x2200  *//* clock ratio 1x4 */
           
/* Load General Purpose Register (GPR) with 32-bit constant value */
#define GPR_LOAD(reg, val)                     \
        mov     reg, $(val & 0xFF)             ;\
        orr     reg, reg, $(val & 0xFF00)      ;\
        orr     reg, reg, $(val & 0xFF0000)    ;\
        orr     reg, reg, $(val & 0xFF000000)

/* Register Read/Write */
#define MV_REG_READ_ASM(toReg, baseReg, regOffs)         \
        ldr     toReg, [baseReg, $(regOffs & 0xFFF)]
        
#define MV_REG_WRITE_ASM(fromReg, baseReg, regOffs)      \
        str     fromReg, [baseReg, $(regOffs & 0xFFF)]
        

/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define MV_BYTE_SWAP_32BIT(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))
/* Endianess macros.                                                        */
#if defined(MV_CPU_LE)
    #define MV_32BIT_LE(X)  (X)
    #define MV_32BIT_BE(X)  MV_BYTE_SWAP_32BIT(X)
#elif defined(MV_CPU_BE)
    #define MV_32BIT_LE(X)  MV_BYTE_SWAP_32BIT(X)
    #define MV_32BIT_BE(X)  (X)
#else
    #error "CPU endianess isn't defined!\n"
#endif
 
#endif /* __INCnBootstraph */
