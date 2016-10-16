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

#define MV_ASMLANGUAGE
#include "mvOsAsm.h"
#include <config.h>
#include <version.h>
#include "mvBoardEnvSpec.h"
#include "mvCtrlEnvSpec.h"
#include "mvCpuIfConfig.h"
#include "pex/mvPexRegs.h"
#include "ddr1_2/mvDramIfRegs.h"
#include "pci/mvPciRegs.h"
#include "mvCtrlEnvAsm.h"
#if defined(MV_INC_BOARD_SPI_FLASH)
#include "spi/mvSpiSpec.h"
#endif


#define MAGIC_BYTE0	((MV_BOARD_I2C_MAGIC >> 24 )& 0xff)
#define MAGIC_BYTE1	((MV_BOARD_I2C_MAGIC >> 16 )& 0xff)
#define MAGIC_BYTE2	((MV_BOARD_I2C_MAGIC >> 8)& 0xff)
#define MAGIC_BYTE3	((MV_BOARD_I2C_MAGIC >> 0)& 0xff)

                  
/*
  get_board_id - get board id from twsi eeprom
  input : r5 - board id data start offset in the eeprom
        
*/
.globl lowlevel_init

/************************************************/
/*              lowlevel_init                   *
/************************************************/

lowlevel_init:

         /* if we are DDRI board then we
         should open the CS2 for only 1 MB 
         to turn the 7segement and not to overlap with the
         internal registers and we will do it always
         */
#if !defined(MV_88F6183) && !defined(MV_88F6183L)                  
#if defined(MV_88F6082)                  
	ldr	r4, =0x000f1d11
#else
	ldr	r4, =0x000f1b11
#endif

        MV_DV_REG_WRITE_ASM(r4, r1, 0x20060)
#endif
                  
	/* change reg base to 0xf1000000 */
	ldr     r4, =CFG_MV_REGS
        MV_DV_REG_WRITE_ASM(r4, r1, 0x20080)
	
#ifndef DB_FPGA
	/* Clear device ID in PCI in MV_88F5082 */
	/* Read device ID  */
	MV_CTRL_MODEL_GET_ASM(r4, r1);
        ldr     r1, =MV_5082_DEV_ID
        cmp     r4, r1
        bne     next

	/* Read bus number */				
	MV_REG_READ_ASM(r1, r4, PCI_P2P_CONFIG_REG(0));
	mov 	r4, r1;				
	and 	r1, r1, #PPCR_BUS_NUM_MASK;		
	/* Read dev number */				
	and 	r4, r4, #PPCR_DEV_NUM_MASK;		
	orr	r1, r4, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS);
	/* Set PCI config enable BIT31 */		
	orr 	r1, r1, #PCAR_CONFIG_EN;			
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r1, r4, PCI_CONFIG_ADDR_REG(0));
	/* Set PCI configuration data to 0xffffffff */
	ldr	r1, =0xffffffff
	MV_REG_WRITE_ASM(r1, r4, PCI_CONFIG_DATA_REG(0));	

#endif /* DB_FPGA */
        /* save Link Registers */
next:	mov	r2, lr

#ifndef DB_FPGA
	bl      _i2cInit

        /* Initialize BUS-L to DDR configuration parameters     */
        /* Must be done prior to DDR operation                  */
        bl      _mvCpuIfPreInit
#endif /* DB_FPGA */


#if !defined(MV_INC_BOARD_DDIM)



#if defined(DB_CUSTOMER)
        /* Insert here customer boards specific */
	/* init MPP */
	ldr	r6, =RD_88F5181L_CUS1_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181L_CUS1_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181L_CUS1_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181L_CUS1_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =RD_88F5181L_CUS1_GPP_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	ldr	r6, =RD_88F5181L_CUS1_GPP_VAL
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F6183GP) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, 0x1480) 

#if defined(MV_INCLUDE_PCI)
	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
#endif

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F6183AP) 
#if defined(MV_INC_BOARD_SPI_FLASH)
	/* configure the Prescale of SPI clk Tclk = 166MHz */
	MV_REG_READ_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
	and	r6, r6, #~MV_SPI_CLK_PRESCALE_MASK
	orr	r6, r6, #0x14
	MV_REG_WRITE_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
#endif

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, 0x1480) 

#if defined(MV_INCLUDE_PCI)
	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
#endif

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5082) 
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x8002C) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

#if defined(MV_INCLUDE_PCI)
	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
#endif

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5082_3)
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x8002C) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5182) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

#if defined(MV_INCLUDE_PCI)
	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
#endif

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5182_3)
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88W8660)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88W8660_AP82S)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b


	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5181L_FE) 

	/* init MPP */
	ldr	r6, =RD_88F5181L_VOIP_FE_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181L_VOIP_FE_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181L_VOIP_FE_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181L_VOIP_FE_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =RD_88F5181L_VOIP_FE_GPP_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 -turn off leds0 0,15 wireless indication-*/
	ldr	r6, =RD_88F5181L_VOIP_FE_GPP_IO
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5181L_GE) 

	/* init MPP */
	ldr	r6, =RD_88F5181L_VOIP_GE_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181L_VOIP_GE_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181L_VOIP_GE_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181L_VOIP_GE_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF0330
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 -turn off leds 10 wireless indication-*/
	ldr	r6, =0xC
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5181_GTWFE) 

	/* init MPP */
	ldr	r6, =RD_88F5181_GTW_FE_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181_GTW_FE_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181_GTW_FE_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181_GTW_FE_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF0330
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 -turn off leds 10 wireless indication-*/
	ldr	r6, =0xC
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5181_GTWGE) 

	/* init MPP */
	ldr	r6, =RD_88F5181_GTW_GE_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181_GTW_GE_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181_GTW_GE_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181_GTW_GE_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF0330
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 -turn off leds 10 wireless indication-*/
	ldr	r6, =0xC
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(RD_88F5181L_FXO_GE) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(MV_POS_NAS) 

        /* init MPP */
        ldr     r6, =RD_88F5181_POS_NAS_MPP0_7
        MV_REG_WRITE_ASM (r6, r1, 0x10000)
        ldr     r6, =RD_88F5181_POS_NAS_MPP8_15
        MV_REG_WRITE_ASM (r6, r1, 0x10004)
        ldr     r6, =RD_88F5181_POS_NAS_MPP16_23
        MV_REG_WRITE_ASM (r6, r1, 0x10050)
        ldr     r6, =RD_88F5181_POS_NAS_MPP_DEV
        MV_REG_WRITE_ASM (r6, r1, 0x10008)

        /* init GPP , Out enable */
        ldr     r6, =0x00000BDF
        MV_REG_WRITE_ASM (r6, r1, 0x10104)

        /* turn Led to 1 */
        ldr     r6, =0x0000e000
        MV_REG_WRITE_ASM (r6, r1, 0x10100)

        /*DDR SDRAM Initialization Control */
        ldr     r6, =0x00000001
        MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

        /* PCI ARBITER */
        ldr     r6, =0x80000030
        MV_REG_WRITE_ASM (r6, r1, 0x31d00)

        bl      _mvDramIfStaticInit

        b       done

#elif defined(MV_VOIP)

	/* init MPP */
	ldr	r6, =RD_88F5181_VOIP_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F5181_VOIP_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_88F5181_VOIP_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =RD_88F5181_VOIP_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0x000005F5
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x0000D000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	bl	_mvDramIfStaticInit

	b	done

#elif defined(DB_PRPMC)

	/* init MPP */
	ldr	r6, =DB_88F5181_DDR1_PRPMC_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181_DDR1_PRPMC_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181_DDR1_PRPMC_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181_DDR1_PRPMC_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF8FFC
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x00006000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	bl	_mvDramIfStaticInit

	b	done

#elif defined(DB_MNG)

	/* init MPP */
	ldr	r6, =DB_88F5181_DDR1_MNG_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181_DDR1_MNG_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181_DDR1_MNG_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181_DDR1_MNG_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =DB_88F5181_DDR1_MNG_GPP_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x00006000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Init PCI arbiter according to Monarch bit 4 in I2c register 0x27  */
	/* If this bit is set disable internal arbiter.			     */
        mov     r4, #0x27 
        mov     r7, #1

        bl      _i2cRead                /* result in r7 */

        tst	r7, #0x10

        bne	no_pci_arbiter
 
	/* Activate PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

no_pci_arbiter:
	bl	_mvDramIfStaticInit

	b	done

#elif defined(DB_FPGA)

#if defined(MV_MEM_OVER_PCI_WA)
	/* Disable PCI ARBITER */
	ldr	r6, =0x00000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
	MV_REG_READ_ASM (r6, r1, 0x30c00)
	orr	r6, r6, #0x20000000
	bic	r6, r6, #0x10
	MV_REG_WRITE_ASM (r6, r1, 0x30c00)
	
	/* Set config bus to 1 and device to 0 */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	mov 	r1, r6;
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	and 	r1, r1, #PPCR_DEV_NUM_MASK;
	orr	r6, r1, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */
	/* Set PCI config enable BIT31 and config reg 64 */
	orr 	r6, r6, #PCAR_CONFIG_EN;
	orr 	r6, r6, #0x64;
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	ldr	r1, =0xffff
	bic	r7, r7, r1
	orr	r7, r7, #0x100
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));

	/* Set config PCI ME/IOE/MEME */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	mov 	r1, r6;
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	and 	r1, r1, #PPCR_DEV_NUM_MASK;
	orr	r6, r1, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */
	/* Set PCI config enable BIT31 and config reg 4 */
	orr 	r6, r6, #(PCAR_CONFIG_EN | 0x4);
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	orr	r7, r7, #0x7
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	
	MV_REG_READ_ASM (r6, r1, 0x20104)
	and r6, r6, #~0x1;
	MV_REG_WRITE_ASM (r6, r1, 0x20104)

	/* Read PCI agent device ID  */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	ldr 	r1, =(0x8 << PCAR_DEVICE_NUM_OFFS);
	orr	r6, r6, r1
	/* Set PCI config enable BIT31 */
	orr 	r6, r6, #PCAR_CONFIG_EN;
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));

wait_for_pci:
	/* Read the Data returned in the PCI Data register */
	MV_REG_READ_ASM(r6, r1, PCI_CONFIG_DATA_REG(0));
        mov     r6, r6, LSR #PDVIR_DEV_ID_OFFS         /* toReg = toReg >> 16 */

	/* if Orion NAS default board is DB, rd must have special compilation flag */
        ldr     r1, =MV_5181_DEV_ID
        cmp     r1, r6
        beq     dram_config
        ldr     r1, =0x6450
        cmp     r1, r6
        beq     dram_config
	b wait_for_pci

dram_config:
	/* Set PCI agent ME/IOE/MEME */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	ldr 	r1, =(0x8 << PCAR_DEVICE_NUM_OFFS);
	orr	r6, r6, r1
	/* Set PCI config enable BIT31 */
	orr 	r6, r6, #(PCAR_CONFIG_EN | 0x4);
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	orr	r7, r7, #0x7
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));

	ldr	r6, =0x0
	/* Set PCI window base address to address 0 */
	MV_REG_WRITE_ASM (r6, r1, 0x20014)
	/* Set remap to 128MB */
	ldr	r6, =BIT27
	MV_REG_WRITE_ASM (r6, r1, 0x20018)
	
	/* Set PCI window size 128MB */
	MV_REG_READ_ASM (r6, r1, 0x20010)
	ldr	r1, =0xffff
	and	r6, r6, r1
	ldr	r1, =0x7ff
	orr	r6, r1, LSL #16
	MV_REG_WRITE_ASM (r6, r1, 0x20010)
	
	/* Set PCI I/O window base address to address f2000000 */
	ldr	r6, =0xf5131
	MV_REG_WRITE_ASM (r6, r1, 0x20030)
	/* Set base to 0xf2000000 */
	ldr	r6, =0xf2000000
	MV_REG_WRITE_ASM (r6, r1, 0x20034)
	/* Set remap to 0xf1000000 */
	ldr	r6, =0xf1000000
	MV_REG_WRITE_ASM (r6, r1, 0x20038)
	
#else

	/* Disable PCI ARBITER */
	ldr	r6, =0x00000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)
	MV_REG_READ_ASM (r6, r1, 0x30c00)
	orr	r6, r6, #0x20000000
	bic	r6, r6, #0x10
	MV_REG_WRITE_ASM (r6, r1, 0x30c00)

	/* Enable access to internal registers in PCI IO tran' */
	MV_REG_READ_ASM (r6, r1, 0x30c3c)
	bic	r6, r6, #0x400
	MV_REG_WRITE_ASM (r6, r1, 0x30c3c)


	/* Set config bus to 1 and device to 0 */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	mov 	r1, r6;
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	and 	r1, r1, #PPCR_DEV_NUM_MASK;
	orr	r6, r1, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */
	/* Set PCI config enable BIT31 and config reg 64 */
	orr 	r6, r6, #PCAR_CONFIG_EN;
	orr 	r6, r6, #0x64;
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	ldr	r1, =0xffff
	bic	r7, r7, r1
	orr	r7, r7, #0x100
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));

	/* Set config PCI ME/IOE/MEME */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	mov 	r1, r6;
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	and 	r1, r1, #PPCR_DEV_NUM_MASK;
	orr	r6, r1, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */
	/* Set PCI config enable BIT31 and config reg 4 */
	orr 	r6, r6, #(PCAR_CONFIG_EN | 0x4);
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	orr	r7, r7, #0x7
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	
	MV_REG_READ_ASM (r6, r1, 0x20104)
	and r6, r6, #~0x1;
	MV_REG_WRITE_ASM (r6, r1, 0x20104)

	/* Read PCI agent device ID  */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	ldr 	r1, =(0x8 << PCAR_DEVICE_NUM_OFFS);
	orr	r6, r6, r1
	/* Set PCI config enable BIT31 */
	orr 	r6, r6, #PCAR_CONFIG_EN;
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));

wait_for_pci:
	/* Read the Data returned in the PCI Data register */
	MV_REG_READ_ASM(r6, r1, PCI_CONFIG_DATA_REG(0));
        mov     r6, r6, LSR #PDVIR_DEV_ID_OFFS         /* toReg = toReg >> 16 */

        ldr     r1, =0x6450
        cmp     r1, r6
        beq     con_config
	b wait_for_pci

con_config:
	/* Set PCI agent ME/IOE/MEME */
	MV_REG_READ_ASM(r6, r1, PCI_P2P_CONFIG_REG(0));
	and 	r6, r6, #PPCR_BUS_NUM_MASK;
	/* Read dev number */
	ldr 	r1, =(0x8 << PCAR_DEVICE_NUM_OFFS);
	orr	r6, r6, r1
	/* Set PCI config enable BIT31 */
	orr 	r6, r6, #(PCAR_CONFIG_EN | 0x4);
	/* Write the address to the PCI configuration address register */
	MV_REG_WRITE_ASM(r6, r1, PCI_CONFIG_ADDR_REG(0));
	MV_REG_READ_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));
	/* Write the Data to the PCI Data register */
	orr	r7, r7, #0x7
	MV_REG_WRITE_ASM(r7, r1, PCI_CONFIG_DATA_REG(0));

	/* Set PCI I/O window base address to address f2000000 */
	ldr	r6, =0xf5131
	MV_REG_WRITE_ASM (r6, r1, 0x20030)
	/* Set base to 0xf2000000 */
	ldr	r6, =0xf2000000
	MV_REG_WRITE_ASM (r6, r1, 0x20034)
	/* Set remap to 0xf1000000 */
	ldr	r6, =0xf1000000
	MV_REG_WRITE_ASM (r6, r1, 0x20038)
	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	bl	_mvDramIfStaticInit
#endif
	b	done
#elif defined(DB_PEX_PCI)
	/* init MPP */
	ldr	r6, =DB_88F5181_DDR1_PEXPCI_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181_DDR1_PEXPCI_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181_DDR1_PEXPCI_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181_DDR1_PEXPCI_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF0FFF
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x00001000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	bl	_mvDramIfStaticInit

	b	done
         
#elif defined(RD_88F6082NAS)
	/* Set SATA LED to blink and do not invert indication */
	ldr	r6, =0x9
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 
	
	/* init MPP */
	ldr	r6, =RD_88F6082_NAS_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F6082_NAS_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	
	/* init GPP , disable HDD power */
	MV_REG_READ_ASM (r6, r1, 0x10100)
	bic	r6, r6, #(1 << 11)
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 

	/* init GPP , Out enable */
	ldr	r6, =RD_88F6082_NAS_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	b	done

#elif defined(RD_88F6082DAS_PLUS)
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 
	
	/* init MPP */
	ldr	r6, =RD_88F6082_DAS_P_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F6082_DAS_P_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	
	/* init GPP , disable HDD power */
	MV_REG_READ_ASM (r6, r1, 0x10100)
	bic	r6, r6, #(1 << 2)
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 

	/* init GPP , Out enable */
	ldr	r6, =RD_88F6082_DAS_P_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	b	done

#elif defined(RD_88F6082MICRO_DAS_NAS)
	
	/* Invert SATA indecation polarety and set LED to blink */
	ldr	r6, =0x9
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

     	/*DDR SDRAM set pad drive */
	MV_REG_READ_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	orr	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	orr	r6, r6, #0x3000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	bic	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	MV_REG_READ_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	orr	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	orr	r6, r6, #0x3000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	bic	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 

#ifdef MV_INC_DRAM_MFG_TEST
	/* DDR MFG test */
	bl _mvDramMfgTest
#endif

	b	done

#elif defined(RD_88F6082GE_SATA)
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 
	
	/* init MPP */
	ldr	r6, =RD_88F6082_GE_SATA_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_88F6082_GE_SATA_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	
	/* init GPP , disable HDD power */
	MV_REG_READ_ASM (r6, r1, 0x10100)
	and	r6, r6, #(1 << 10)
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 

	/* init GPP , Out enable */
	ldr	r6, =RD_88F6082_GE_SATA_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	b	done

#elif defined(DB_88F6082BP)
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

     	/*DDR SDRAM set pad drive */
	MV_REG_READ_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	orr	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	orr	r6, r6, #0x3000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	bic	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_ADDR_CTRL_PADS_CAL_REG) 
	MV_REG_READ_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	orr	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	orr	r6, r6, #0x3000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 
	bic	r6, r6, #0x80000000
	MV_REG_WRITE_ASM (r6, r1, SDRAM_DATA_PADS_CAL_REG) 

	b	done

#elif defined(DB_88F6082LBP)
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x6002C) 

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	b	done

#elif defined(RD_88F6082_DX243)
	
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG) 
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	b	done

#endif

#else /* #if !defined(MV_INC_BOARD_DDIM) */

/* First jump to the new board release without the board ID on the TWSI */
#if defined(MV_88F5082)        
        b	db_88f5082_ddr2
#elif defined(MV_88F1281)        
        b	db_88f1281_ddr2
#elif defined(MV_88F5182)        
        b	db_88f5182_ddr2
#elif defined(DB_88F5181)        
        b	db_88f5x81_ddrx
#elif defined(RD_DB_88F5181L)        
        b	db_88f5181L_ddr2
#elif defined(MV_88W8660)        
        b	db_88w8660_ddr2 
#elif defined(MV_88F5180N)        
        b	db_88f5180n_ddrx        
#elif defined(MV_88F6082)        
        b	db_88f6082_sa        
#elif defined(MV_88F6183) ||defined(MV_88F6183L)        
        b	db_88f6183_bp
#else        
        cmp    r7, #DB_88F5181_5281_DDR1     
        beq	db_88f5181_5281_ddr1
        cmp    r7, #DB_88F5181_5281_DDR2     
        beq	db_88f5181_5281_ddr2
#endif        
         

#if defined(MV_88F5082)

db_88f5082_ddr2:
	/* Set SATA LED to blink */
	ldr	r6, =0x1
	MV_REG_WRITE_ASM (r6, r1, 0x8002C) 

	/* init MPP */
	ldr	r6, =DB_88F5082_DDR2_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5082_DDR2_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5082_DDR2_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
         
         /* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
        
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	/* Set dram width to 16bit */
	MV_REG_READ_ASM (r6, r1, 0x1400)
	bic	r6, r6, #0xc000
	orr	r6, r6, #0x4000
	MV_REG_WRITE_ASM (r6, r1, 0x1400)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
         
	b	done


#elif defined(MV_88F5182)

db_88f5182_ddr2:

	/* init MPP */
	ldr	r6, =DB_88F5182_DDR2_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5182_DDR2_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5182_DDR2_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
         
         /* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
        
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

	/* Set dram width to 16bit */
	MV_REG_READ_ASM (r6, r1, 0x1400)
	bic	r6, r6, #0xc000
	orr	r6, r6, #0x4000
	MV_REG_WRITE_ASM (r6, r1, 0x1400)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
         
	b	done

#elif defined(MV_88F1281)

db_88f1281_ddr2:

	/* init MPP */
	ldr	r6, =DB_88F1281_DDR2_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F1281_DDR2_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
         /* init GPP , Out enable */
	ldr	r6, =DB_88F1281_DDR2_OE 
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x4
	MV_REG_WRITE_ASM (r6, r1, 0x10100)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
	b	done

#elif defined(RD_DB_88F5181L)        

db_88f5181L_ddr2:

	/* init MPP */
	ldr	r6, =DB_88F5181L_DDR2_2XTDM_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181L_DDR2_2XTDM_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181L_DDR2_2XTDM_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181L_DDR2_2XTDM_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008)
         
         /* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
        
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)


     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
        ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
        ldr   r6, [r6]
         
	b	done


#elif defined(MV_88F6183) || defined(MV_88F6183L)        
db_88f6183_bp:
	
#if defined(MV_INC_BOARD_SPI_FLASH)
	/* configure the Prescale of SPI clk Tclk = 166MHz */
	MV_REG_READ_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
	and	r6, r6, #~MV_SPI_CLK_PRESCALE_MASK
	orr	r6, r6, #0x14
	MV_REG_WRITE_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
#endif

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, 0x1480)
         
1:	MV_REG_READ_ASM (r6, r1, 0x1480)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
	b	done

#elif defined(MV_88W8660)        

db_88w8660_ddr2:
        /* init MPP */
	ldr	r6, =DB_88W8660_DDR2_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88W8660_DDR2_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
         
	/* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
        
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)


     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
         
	b	done

#elif defined(MV_88F5180N)
db_88f5180n_ddrx:

	/* init MPP */
	ldr	r6, =DB_88F5180N_DDRX_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5180N_DDRX_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5180N_DDRX_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5180N_DDRX_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008)
         
         /* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
        
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)


     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
         
	b	done

#elif defined(MV_88F6082)
db_88f6082_sa:
	/* init MPP */
	ldr	r6, =DB_88F6082_SA_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F6082_SA_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	
	/* init GPP , Out enable */
	ldr	r6, =DB_88F6082_SA_OE
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
         
	b	done
#else

db_88f5181_5281_ddr1: /*old board*/

	/* init MPP */
	ldr	r6, =DB_88F5181_5281_DDR1_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181_5281_DDR1_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181_5281_DDR1_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181_5281_DDR1_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008)
         
         
         /* turn 7segment to val 1 - Set DevCS2- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x10464)
	ldr	r6, =0x000F1B11
	MV_REG_WRITE_ASM (r6, r1, 0x20060)
	ldr	r6, =0xF0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20064)
        
         
         ldr	r6, =0xF0000010 /* DevCS2 Default base address +0x10 */
         ldr   r6, [r6]
         
        /* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)


     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         #if 0
         ldr	r6, =0xF0000020 /* DevCS2 Default base address +0x20 */
         ldr   r6, [r6]
         #endif
         
	b	done
         
db_88f5181_5281_ddr2: /*old board*/

	/* init MPP */
	ldr	r6, =DB_88F5181_5281_DDR2_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5181_5281_DDR2_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5181_5281_DDR2_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5181_5281_DDR2_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008)
         
	/* init GPP , Out enable */
	ldr	r6, =0xFFFF3F3F 
	MV_REG_WRITE_ASM (r6, r1, 0x10104) 
	
	/* turn Led to 1 */
	ldr	r6, =0x00004000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	
         

	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)


     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit 
         
	/* turn Led to 2 */
	ldr	r6, =0x00008000
	MV_REG_WRITE_ASM (r6, r1, 0x10100) 	
         
	b	done

db_88f5x81_ddrx:

	/* init MPP */
	ldr	r6, =DB_88F5X81_DDRX_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_88F5X81_DDRX_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =DB_88F5X81_DDRX_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10050) 
	ldr	r6, =DB_88F5X81_DDRX_MPP_DEV
	MV_REG_WRITE_ASM (r6, r1, 0x10008)
         
         /* turn 7segment to val 1 - Set DevCS0- window 6 is the default*/
	ldr	r6, =0x8FCFFFFF
	MV_REG_WRITE_ASM (r6, r1, 0x1045C)
	ldr	r6, =0x000F1E11
	MV_REG_WRITE_ASM (r6, r1, 0x20050)
	ldr	r6, =0xE0000000
	MV_REG_WRITE_ASM (r6, r1, 0x20054)
         
         ldr	r6, =0xE0000010 /* DevCS0 Default base address +0x10 */
         ldr   r6, [r6]
         
	/* PCI ARBITER */
	ldr	r6, =0x80000030
	MV_REG_WRITE_ASM (r6, r1, 0x31d00)

     	/*DDR SDRAM Initialization Control */
	ldr	r6, =0x00000001
	MV_REG_WRITE_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
1:	MV_REG_READ_ASM (r6, r1, DDR_SDRAM_INIT_CTRL_REG)
         cmp    r6, #0
         bne 1b

	/* Load r3 with the size to open SDRAM bank 0 */
	mov	r3, #0x04000000   /* Open DRAM bank 0 to 64MB */

        /* Call DRAM basic initialization to allow C runtime stack */
	bl	_mvDramIfBasicInit
         
         ldr	r6, =0xE0000020 /* DevCS0 Default base address +0x20 */
         ldr   r6, [r6]
        
	b	done
#endif


#endif /*#if !defined(MV_INC_BOARD_DDIM)*/
done:
	mov	lr, r2	


	/* All done by Integrator's boot monitor! */
	mov pc, lr
         

       .globl _mvDramMfgTest

_mvDramMfgTest:

        mov     r11, lr     		/* Save link register */

        ldr    r3,=0x55555555
        ldr    r7,=0x0
        ldr    r8,=0x0

mem_test:
        /* memory test for the U-Boot space */
        
        ldr    r4,= 0 
        ldr    r5,= 0x400000
        
fill_mem:
        str   r3, [r4]
        add   r4, r4, #4
        cmp   r4, r5
        bne   fill_mem
        
        ldr    r4,= 0 
        
check_mem:
        ldr   r6, [r4]
        cmp   r6, r3
        bne   fail_mem
        add   r4, r4, #4
        cmp   r4, r5
        bne   check_mem
        
        cmp    r8, #0x0
        bne    next_test
        ldr    r3,=0xAAAAAAAA
        add   r8, r8, #1
        b    mem_test

next_test:
	/* Undo walking 1 or 0 */
	b pass_mem
        cmp    r8, #0x1
        bne    next_test1
	/* walking 1 */
	mov   r3, r8, LSL r7
        add   r7, r7, #1
        cmp   r7, #0x32
        bne    mem_test

	add   r8, r8, #1
        eor   r7, r7, r7
	ldr   r3,=0x7FFFFFFF

next_test1:
        cmp    r8, #0x2
        bne    pass_mem
	/* walking 0 */
	mov   r3, r3, ROR r7
        add   r7, r7, #1
        cmp   r7, #0x32
        bne    mem_test

pass_mem:
	mov	lr, r11	
	mov	pc, lr

fail_mem:
        /* Error Message */
	bl _mvDramMfgFailMes
fail_mem1:
        b fail_mem1

     .globl _mvDramMfgFailMes

_mvDramMfgFailMes:

        mov     r11, lr     		/* Save link register */

        ldr     r2, =(INTER_REGS_BASE + 0x12000);

        mov   r1, #10
        strb  r1, [r2]        
        mov   r1, #13
        strb  r1, [r2]
        mov   r1, #'M'
        strb  r1, [r2]        
        mov   r1, #'F'
        strb  r1, [r2]
        mov   r1, #'G'
        strb  r1, [r2]
        mov   r1, #' '
        strb  r1, [r2]
        mov   r1, #'D'
        strb  r1, [r2]
        mov   r1, #'R'
        strb  r1, [r2]        
        mov   r1, #'A'
        strb  r1, [r2]
        mov   r1, #'M'
        strb  r1, [r2]
        mov   r1, #' '
        strb  r1, [r2]
        mov   r1, #'F'
        strb  r1, [r2]        
        mov   r1, #'A'
        strb  r1, [r2]
        mov   r1, #'I'
        strb  r1, [r2]
        mov   r1, #'L'
        strb  r1, [r2]
        mov   r1, #'!'
        strb  r1, [r2]
	mov	lr, r11	
	mov	pc, lr

