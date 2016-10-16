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
#include "pci/mvPciRegs.h"
#include "pex/mvPexRegs.h"
#include "ddr2/mvDramIfRegs.h"
#include "mvCtrlEnvAsm.h"
#include "mvAhbToMbusRegs.h"
#if defined(MV_INC_BOARD_SPI_FLASH)
#include "spi/mvSpiSpec.h"
#endif

.globl lowlevel_init

/************************************************/
/*              lowlevel_init                   *
/************************************************/

lowlevel_init:
	mov	r2, lr
#if defined(MV78200)
	mov	r0, #0
	mrc	p15, 1, r0, c15, c1, 0
        /* Check if we are CPU0 or CPU1 */
	and	r0, r0, #0x4000
	cmp	r0, #0x4000
	bne	cpu0_reg_offset
	b	done_cpu1
#endif

	/* change CPU0 reg base to 0xf1000000 */
cpu0_reg_offset:
	ldr     r4, =CFG_MV_REGS
    MV_DV_REG_WRITE_ASM(r4, r1, (AHB_TO_MBUS_WIN_INTEREG_REG(0)))
	ldr     r4, =CFG_MV_REGS
	MV_REG_WRITE_ASM (r4, r1, (AHB_TO_MBUS_WIN_INTEREG_REG(1)))

#if defined(MV_INC_BOARD_SPI_FLASH)
	/* configure the Prescale of SPI clk Tclk = 200MHz */
	MV_REG_READ_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
	and	r6, r6, #~MV_SPI_CLK_PRESCALE_MASK
	orr	r6, r6, #0x15
	orr	r6, r6, #0x4000
	MV_REG_WRITE_ASM (r6, r1, MV_SPI_IF_CONFIG_REG)
#endif
#if 0
	/* Set CPU_2_AHB_TICK_DRV and CCR_CPU_2_AHB_TICK_SMPL tiks */
	MV_REG_READ_ASM (r6, r1, CPU_RESET_SAMPLE_L_REG) 
	and	r6, r6 , #MSAR_SYSCLK2CPU_MASK0
	mov	r6, r6, LSR #MSAR_SYSCLK2CPU_OFFS0
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (0 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x2
	beq	set_cpu_config
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (1 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x4
	beq	set_cpu_config
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (2 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x6
	beq	set_cpu_config
	ldr	r4, =0
set_cpu_config:
	/* Don't change R4 !!! use for Second CPU as well */
	MV_REG_READ_ASM (r6, r1, CPU_CONFIG_REG(MASTER_CPU)) 
	orr	r6, r6, r4
	MV_REG_WRITE_ASM (r6, r1, CPU_CONFIG_REG(MASTER_CPU)) 


	MV_REG_READ_ASM (r6, r1, CPU_RESET_SAMPLE_L_REG) 
	and	r6, r6 , #MSAR_SYSCLK2CPU_MASK1
	mov	r6, r6, LSR #MSAR_SYSCLK2CPU_OFFS1
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (0 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x2
	beq	set_cpu1_config
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (1 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x4
	beq	set_cpu1_config
	ldr	r4, = ((1 << CCR_CPU_2_AHB_TICK_SMPL_OFFS) | (2 << CCR_CPU_2_AHB_TICK_DRV_OFFS))
	cmp	r6, #0x6
	beq	set_cpu1_config
	ldr	r4, =0
set_cpu1_config:
	MV_REG_READ_ASM (r6, r1, CPU_CONFIG_REG(SLAVE_CPU)) 
	orr	r6, r6, r4
	MV_REG_WRITE_ASM (r6, r1, CPU_CONFIG_REG(SLAVE_CPU)) 
#endif

	/* init MPP */
	ldr	r6, =DB_78XX0_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =DB_78XX0_MPP8_15
	/* Clear SATA_ACT for load indication */
	and	r6, r6 ,#0xffffff
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
#if defined(DB_MV78XX0) && defined(MV_NAND_BOOT)
	ldr	r6, =DB_78XX0_MPP16_23_NAND
#else 
	ldr	r6, =DB_78XX0_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
#endif
	/* init GPP , Out enable */
	MV_REG_READ_ASM(r6, r1, 0x10104);    
	bic	r6, #(0x3 << 14)
	MV_REG_WRITE_ASM(r6, r1, 0x10104);    
	ldr	r6, =(0x1 << 15)
	MV_REG_WRITE_ASM(r6, r1, 0x10120);    
	ldr	r6, =(0x1 << 14)
	MV_REG_WRITE_ASM(r6, r1, 0x10124);    

#if defined(RD_MV78XX0_AMC)
	/* init MPP */
	ldr	r6, =RD_AMC_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_AMC_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_AMC_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	/* init GPP , Out enable */
#endif

#if defined(RD_MV78XX0_MASA)
	/* init MPP */
	ldr	r6, =RD_MASA_MPP0_7
	MV_REG_WRITE_ASM (r6, r1, 0x10000) 
	ldr	r6, =RD_MASA_MPP8_15
	MV_REG_WRITE_ASM (r6, r1, 0x10004) 
	ldr	r6, =RD_MASA_MPP16_23
	MV_REG_WRITE_ASM (r6, r1, 0x10008) 
	/* init GPP , Out enable, blink */
	ldr	r6, = (1 << RD_MASA_LED_GPP_PIN(0));
	MV_REG_WRITE_ASM(r6, r1, 0x10108);    
	MV_REG_WRITE_ASM(r6, r1, 0x10128);    
	MV_REG_WRITE_ASM(r6, r1, 0x10124);    
#endif

#if defined(MV_STATIC_DRAM_ON_BOARD) 

	bl	_mvDramIfStaticInit 
#else
	bl	_mvDramIfBasicInit

	/* SATA LEDs */
	ldr	r6, =(0x1 << 14)
	MV_REG_WRITE_ASM(r6, r1, 0x10120);    
	ldr	r6, =(0x1 << 15)
	MV_REG_WRITE_ASM(r6, r1, 0x10124);    
#endif


done:

	mov	lr, r2	
done_cpu1:
	mov 	pc, lr

