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
#include "mvSysHwConfig.h"
#include "mvBoardEnvSpec.h"
#include "mvOsAsm.h"
#include "pci/mvPciRegs.h"
#include "pex/mvPexRegs.h"
#include "mvCtrlEnvSpec.h"
#include "mvCtrlEnvAsm.h"
#include "sys/mvCpuIfRegs.h"


jumpStart:

.section	".reset_vector_sect",#alloc, #execinstr
#if defined(MV_88F6082) || defined(MV_88F6183) || defined(DB_88F5181_OLD) || defined(DB_FPGA) || \
	defined(MV88F6281) || defined(MV88F6192) || defined(MV88F6180)  || defined(MV_88F6183L) || \
    defined(MV88F6190)
#if defined(__BE)
	/* disable I-Cache */
	.word 0x100f11ee /* mrc     15, 0, r0, cr1, cr0, {0} */
	.word 0x010ac0e3 /* bic     r0, r0, #4096   ; 0x1000 */
	.word 0x0700c0e3 /* bic     r0, r0, #7      ; 0x7 */
	.word 0x020080e3 /* orr     r0, r0, #2      ; 0x2 */
	.word 0x100f01ee /* mcr     15, 0, r0, cr1, cr0, {0} */
	/* disable L2 prefetch */
	.word 0x110f3fee /* mrc p15, 1, r0, c15, c1 */
	.word 0x010480e3 /* orr r0, r0, #(1<<24) */
	.word 0x110f2fee /* mcr p15, 1, r0, c15, c1 */
        /* convert CPU to big endian */
        .word 0x100f11ee /* mrc p15, 0, r0, c1, c0 */
        .word 0x800080e3 /* orr r0, r0, #0x80 */
        .word 0x100f01ee /* mcr p15, 0, r0, c1, c0 */
	nop;nop;nop;nop;
	nop;nop;nop;nop;
        
#endif
#endif        

        /* Check if we booted from DRAM. If we did someone already      */
        /* initialize the DRAM controller                               */ 
        
	adr	r4, jumpStart		/* r4 <- current position of code   */
	ldr	r5, =~0xff
	and	r4, r4, r5
        ldr     r5, __start		/* r5 <- linker results for _start   */
        ldr     r2, _jumpStart		/* r2 <- linker results reset vector */
	sub	r8, r2, r5		/* r8 <- (reset vector address - start address) */
	sub	r8, r4, r8		/* r8 <- absolute address to jump to */
					/* r8 <- (current code address -      */
	ldr	sp, =0
	ldr	lr, =0 
	ldr	r5, =CFG_RESET_ADDRESS 	/* test if we run from flash or RAM */
	cmp     r4, r5                  /* don't reloc during debug         */
	beq     romBoot
        
        mov     r5, #1
        ldr     r4, =dramBoot
        str     r5, [r4]                /* We started executing from DRAM */
romBoot:

#if defined(MV78200)
/* Setting the PEX header device ID for MV78200 */
    	MV_DV_REG_READ_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
	ldr   r1, =MV_78200_DEV_ID    
    	ldr   r2, =0xffff
    	and   r6, r6, r2
    	orr   r6, r6, r1, LSL #PXDAVI_DEV_ID_OFFS
    	MV_DV_REG_WRITE_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
#endif

#if defined(MV_88F1181)

	/* set gpp out en */
        ldr r2, = 0xf33
        MV_DV_REG_WRITE_ASM(r1, r1, 0x10104)

	/* turn on debug led to 2 */
	ldr r2, = 0x8
        MV_DV_REG_WRITE_ASM(r1, r1, 0x10100)
         
         ldr pc, = 0xfff90000
                  
#else

#if defined(DB_FPGA)
	b device_cont
#endif /* DB_FPGA */

	/* Read device ID  */
	MV_DV_CTRL_MODEL_GET_ASM(r6, r1);
	ldr   r1, =0x5281
        cmp   r6, r1
	beq   device_5281
	
	/* TC90 acts as Orion 2 C0 */
	ldr   r1, =0x1281
        cmp   r6, r1
	beq   device_5281_C0


	/* 6183 & 6183L */
	ldr   r1, =0x6183
        cmp   r6, r1
	bne   device_cont

#if defined(MV_88F6183L)
/* Setting the PEX header device ID for 6183L */
    MV_DV_REG_READ_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
	ldr   r1, =MV_6183L_DEV_ID    
    ldr   r2, =0xffff
    and   r6, r6, r2
    orr   r6, r6, r1, LSL #PXDAVI_DEV_ID_OFFS
    MV_DV_REG_WRITE_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
#endif /* MV_88F6183L */


        /* Read device revision */
	MV_DV_CTRL_REV_GET_ASM(r6, r1);
	cmp   r6, #0x3  /* 6183 == B0 */
        bne   device_cont

#if !defined(MV78XX0)
        MV_DV_REG_READ_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)
        ldr    r1, =0x10000
        orr     r6, r6, r1
        MV_DV_REG_WRITE_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)
#endif

	b device_cont
	
device_5281:

        /* Read device revision */
	MV_DV_CTRL_REV_GET_ASM(r6, r1);
	cmp   r6, #0x0  /* Orion 2 == A0 */
        beq   device_5281_A0

	cmp   r6, #0x1  /* Orion 2 == B0 */
        beq   device_5281_B0

	cmp   r6, #0x2  /* Orion 2 == C0 */
        beq   device_5281_C0

	cmp   r6, #0x4  /* Orion 2 == d0 */
        beq   device_5281_D0

	cmp   r6, #0x5  /* Orion 2 == d1 */
        beq   device_5281_D1

	cmp   r6, #0x6  /* Orion 2 == d2 */
        beq   device_5281_D2

	b device_cont

device_5281_D0:
device_5281_D1:
device_5281_D2:
#if !defined(MV78XX0)
        MV_DV_REG_READ_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)

        ldr    r1, =0xFFFF8080
        and    r6, r6, r1
        ldr    r1, =0x1902
        orr     r6, r6, r1

        MV_DV_REG_WRITE_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)
#endif
	b device_cont
         
device_5281_C0:
#if !defined(MV78XX0)
        MV_DV_REG_READ_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)

        ldr    r1, =0xFFFF8080
        and    r6, r6, r1
        ldr    r1, =0x8308
        orr     r6, r6, r1

        MV_DV_REG_WRITE_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)
#endif
	b device_cont

device_5281_B0:
#if !defined(MV78XX0)
        MV_DV_REG_READ_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)

        ldr    r1, =0xFFFF8080
        and    r6, r6, r1
        ldr    r1, =0x821b
        orr     r6, r6, r1

        MV_DV_REG_WRITE_ASM (r6, r1, CPU_FTDLL_CONFIG_REG)
#endif
	b device_cont
device_5281_A0:	
device_cont:

#if defined(MV88F6190)
/* Setting the PEX header device ID for 6190 */
    MV_DV_REG_READ_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
	ldr   r1, =MV_6190_DEV_ID    
    ldr   r2, =0xffff
    and   r6, r6, r2
    orr   r6, r6, r1, LSL #PXDAVI_DEV_ID_OFFS
    MV_DV_REG_WRITE_ASM(r6, r1, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
#endif /* MV_88F6190 */

        mov lr, r8
        mov pc, lr

#endif         


__start:
        .word  _start
        
.globl dramBoot
dramBoot:
        .word   0

_jumpStart:
        .word   .reset_vector_sect
        
.section	".dummy",#alloc, #execinstr
	.long 0xffffffff
	.long 0xffffffff
	.long 0xffffffff
	.long 0xffffffff


