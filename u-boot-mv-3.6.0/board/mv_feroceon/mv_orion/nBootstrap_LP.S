#define MV_ASMLANGUAGE
#include "mvDramIfRegs.h"
#include "mvDramIfConfig.h"
#include "nBootstrap.h"


/* #define NAND_DEBUG */
#if !defined(MV_BOOTROM)
.globl nbootStart
nbootStart:
        /*
         * set the cpu to SVC32 mode, I and F disabled.
         */
        mov     r1, #0xd3 
        msr     cpsr,r1 

        /*
         * flush v4 I/D caches
         */
        mcr     p15, 0, r1, c7, c7, 0   /* invalidate v3/v4 cache */
        /*
         * disable MMU stuff and caches
         */
        mrc     p15, 0, r1, c1, c0, 0
        bic     r1, r1, #0x00002300     /* clear bits 13, 9:8 (--V- --RS) */
        bic     r1, r1, #0x00000007     /* clear bits 2:0 (-CAM) */
        orr     r1, r1, #0x00001000     /* set bit 12 (I) I-Cache */
        /* MUST BE PLACED AT END OF CACHE LINE!!!!!!!!!!!!!!! */
        mcr     p15, 0, r1, c1, c0, 0
	
	/* Add nop commands for cache flush operations */
	nop
	nop
	nop
	nop
	nop
	/* here. MUST BE IN THE SAME CACHE LINE */
       
        mov    r0, #0   /* We use r0 as always '0' */
        
#ifdef NAND_CTRL_88F528x
                
        /* Load CPU controller base address 0xD0020000                     */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x20000
        
        MV_REG_READ_ASM (r1, r2, 0x20120)
        bic    r1, r1, #MV_32BIT_LE(0x7F00)
        orr    r1, r1, #MV_32BIT_LE(0x8200)
        bic    r1, r1, #MV_32BIT_LE(0x007F)
        orr    r1, r1, #MV_32BIT_LE(0x001b)
        MV_REG_WRITE_ASM(r1, r2, 0x20120)

        /* Set CPU to Mbus-L DDR Interface Tick Driver and Tick Sample */
        MV_REG_READ_ASM (r1, r2, 0x20100)
        bic    r1, r1, #MV_32BIT_LE(0xFF00)
        orr    r1, r1, #MV_32BIT_LE(CPU_2_MBUSL_DDR_CLK)
        MV_REG_WRITE_ASM(r1, r2, 0x20100)
        
#endif /*  NAND_CTRL_88F528x */

	/* lock I-Cache */
	mrc   p15, 0, r8, c9, c0, 1
	orr   r8, r8, #0xf
	mcr   p15, 0, r8, c9, c0, 1

	/* Start load code into I-Cache */
	mov   r2, #0x500
	mov   r8, pc
.align 5
	bic   r8, #0x1f
	add   r8, r8, #32
load_loop:
	mcr   p15, 0, r8, c7, c13, 1
        add   r8, r8, #32 	/* 8 dwords * 4 bytes */
        sub   r2, r2, #32 	/* 8 dwords * 4 bytes */
        cmp   r2, #0 /* check if we have read a full Page */
        bne   load_loop

#ifdef NAND_DEBUG
        /* GPP initialization */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x10000
        mov    r1, #0xf0ffffff
        MV_REG_WRITE_ASM(r1, r2, 0x104)
        mov    r1, #0x1000000
        MV_REG_WRITE_ASM(r1, r2, 0x100)
#endif
	
        /* DRAM memory initialization */
        /* Load SDRAM controller base address 0xd0001000             */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x1000

	/* Write to SDRAM coniguration register                         */         
#ifndef MV_88W8660
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_CONFIG_REG_DV))
#else
        GPR_LOAD(r1, MV_32BIT_LE((SDRAM_CONFIG_REG_DV & ~(0x40))))
#endif /* MV_88W8660 */
        MV_REG_WRITE_ASM(r1, r2, SDRAM_CONFIG_REG)
        
	/* Write Dunit control low register                             */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_DUNIT_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_DUNIT_CTRL_REG)
        
        /* Write SDRAM address control register                         */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_ADDR_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_ADDR_CTRL_REG)
        
        /* Write SDRAM timing Low register                              */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_TIMING_CTRL_LOW_REG_DVAL))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_TIMING_CTRL_LOW_REG)
        
        /* Write SDRAM timing High register                             */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_TIMING_CTRL_HIGH_REG_DVAL))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_TIMING_CTRL_HIGH_REG)
                
        /* Write SDRAM mode register                                    */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_MODE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_MODE_REG)
        
        /* Write SDRAM Extended mode register                           */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_EXTENDED_MODE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_EXTENDED_MODE_REG)

        /* Config DDR2 registers pad calibration    */
        MV_REG_READ_ASM (r1, r2, SDRAM_CONFIG_REG)
        tst     r1, #SDRAM_DTYPE_DDR2
        beq     ddr1PadCal

        /* Config DDR2 On Die Termination (ODT) registers               */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_SDRAM_ODT_CTRL_LOW_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_SDRAM_ODT_CTRL_LOW_REG)
        
        /* Write SDRAM DDR2 ODT control high register                   */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_SDRAM_ODT_CTRL_HIGH_REG)
        
        /* Write SDRAM DDR2 Dunit ODT control register                  */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_DUNIT_ODT_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_DUNIT_ODT_CONTROL_REG)

        mov   r3, #MV_32BIT_LE(DDR2_ADDR_CTRL_PAD_STRENGTH_TYPICAL_DV)
        mov   r4, #MV_32BIT_LE(DDR2_DATA_PAD_STRENGTH_TYPICAL_DV)
        b next

ddr1PadCal:
        mov   r3, #MV_32BIT_LE(DDR1_ADDR_CTRL_PAD_STRENGTH_TYPICAL_DV)
        mov   r4, #MV_32BIT_LE(DDR1_DATA_PAD_STRENGTH_TYPICAL_DV)

next:
        /* Implement Guideline (GL# MEM-3) Drive Strength Value         */
        /* Relevant for: 88F5181-A1/B0/B1 and 88F5281-A0/B0             */
        /* DDR SDRAM Address/Control Pads Calibration                   */
        MV_REG_READ_ASM (r1, r2, SDRAM_ADDR_CTRL_PADS_CAL_REG)
        
        orr   r5, r1, #MV_32BIT_LE(SDRAM_WR_EN) /* Make register writeable */
        
        MV_REG_WRITE_ASM (r5, r2, SDRAM_ADDR_CTRL_PADS_CAL_REG)
        
        orr   r1, r3, r1                /* Set default value for DDR    */
        
        MV_REG_WRITE_ASM (r1, r2, SDRAM_ADDR_CTRL_PADS_CAL_REG)

        
        /* DDR SDRAM Data Pads Calibration                         	*/
        MV_REG_READ_ASM (r1, r2, SDRAM_DATA_PADS_CAL_REG)
        
        orr   r5, r1, #MV_32BIT_LE(SDRAM_WR_EN) /* Make register writeable */
        
        MV_REG_WRITE_ASM (r5, r2, SDRAM_DATA_PADS_CAL_REG)
        
        orr   r1, r4, r1                /* Set default value for DDR    */        
        
        MV_REG_WRITE_ASM (r1, r2, SDRAM_DATA_PADS_CAL_REG)

#ifndef MV_88W8660
        /* Write Dunit FTDLL Configuration Register                     */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_FTDLL_CONFIG_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_FTDLL_CONFIG_REG)
#endif /* MV_88W8660 */

        /* DDR SDRAM Initialization Control Register. Init enable       */
        mov   r1, #MV_32BIT_LE(DSICR_INIT_EN)
        MV_REG_WRITE_ASM (r1, r2, DDR_SDRAM_INIT_CTRL_REG)
         
#ifdef NAND_DEBUG
	/* GPP initialization */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x10000
        mov    r1, #0x2000000
        MV_REG_WRITE_ASM(r1, r2, 0x100)
#endif
        
ddrInitLoop:
	MV_REG_READ_ASM (r1, r2, DDR_SDRAM_INIT_CTRL_REG)
        cmp    r1, #0
        bne    ddrInitLoop
                
        /* Load back SDRAM controller base address 0xd0001000           */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x1000
        
        /* Open SDRAM bank 0 size register                              */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_SIZE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_SIZE_REG(0))
        
        /* Close SDRAM bank 1,2,3                                       */ 
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(1))
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(2))
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(3))

	/* Prepare the address where to find the nandBoot function pointer */
        mov    lr, #BOOTER_BASE

#ifdef DEBUG
	/* GPP initialization */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x10000
        mov    r1, #0x2000000
        MV_REG_WRITE_ASM(r1, r2, 0x100)
#endif
        
	/* init */
        mov   r2, #0x1            	/* start with page 1 */
        mov   r3, #BOOTER_BASE    	/* start of DRAM buffer */
        GPR_LOAD(r5, NAND_FLASH_BASE)

cp_page_loop:
	mov   r0, #0
        mov   r6, #CMD_READ
        mov   r7, #CMD_START_READ
        mov   r9, #CMD_RESET
	mov   r4, #PAGE_SIZE           	/* Byte counter */
	
        
startRead:	
        /* issue reset command */
        strb  r9, [r5, #NAND_CMD_PORT]

        mov   r9, #0x1000000
loop_delay1:
	sub   r9, r9, #1
	cmp   r9, #0
	bne loop_delay1

        /* issue read command */
        strb  r6, [r5, #NAND_CMD_PORT]
                        
        /* issue address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r2, [r5, #NAND_ADDR_PORT] /* page address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r7, [r5, #NAND_CMD_PORT]
        
#ifdef NAND_DEBUG
        /* GPP initialization */
        mov    r6, #0xd0000000
        orr    r6, r6, #0x10000
        mov    r1, #0x3000000
        MV_REG_WRITE_ASM(r1, r6, 0x100)
#endif
	/* Delay of at least 25uSec (NAND flash tR) */
        mov   r9, #0x1000000
loop_delay3:
	sub   r9, r9, #1
	cmp   r9, #0
	bne loop_delay3

#ifdef NAND_DEBUG
        /* GPP initialization */
        mov    r6, #0xd0000000
        orr    r6, r6, #0x10000
        mov    r1, #0x4000000
        MV_REG_WRITE_ASM(r1, r6, 0x100)
#endif

        /* now perform reading */
        mov   r0, r5

copy_loop1:
        sub   r4, r4, #16 	/* 4 dwords * 4 bytes */
	ldmia r0!, {r6-r9}
        stmia r3!, {r6-r9}
        
        cmp   r4, #0 /* check if we have read a full Page */
        bne   copy_loop1
        
nextPage:        
        add   r2, r2, #1  /* increment page number */
        cmp   r2, #BOOTER_PAGE_NUM
        bne   cp_page_loop

#ifdef NAND_DEBUG
        /* GPP initialization */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x10000
        mov    r1, #0x5000000
        MV_REG_WRITE_ASM(r1, r2, 0x100)
#endif

	/* Set up the stack						    */
stack_setup:
	mov   r0, #BOOTER_BASE		
	sub   sp, r0, #12		/* leave 3 words for abort-stack    */
        /* jump to new  code */
        mov   pc, lr
               
.align 10
/* This is known to be address (BOOTER_BASE + 2 * PAGE_SIZE) */
_nandBootPtr:
        .word nand_boot

#else /* MV_BOOTROM */

.globl nbootStart
nbootStart:
	/* Enable I-Cache */
        mrc     p15, 0, r1, c1, c0, 0
        orr     r1, r1, #0x00001000     /* set bit 12 (I) I-Cache */
        /* MUST BE PLACED AT END OF CACHE LINE!!!!!!!!!!!!!!! */
        mcr     p15, 0, r1, c1, c0, 0

	/* Set up the stack						    */
	mov   r0, #BOOTER_BASE		
	sub   sp, r0, #12		/* leave 3 words for abort-stack    */
        
        /* jump to new  code */
        
        ldr   lr, =nandBoot
        mov   pc, lr
#endif /* !defined(MV_BOOTROM) */
