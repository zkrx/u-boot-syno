#define MV_ASMLANGUAGE
#include "nBootstrap.h"
#include "ddr2/mvDramIfRegs.h"
#include "ddr2/mvDramIfConfig.h"
#include "xor/mvXorRegs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

#define XOR_CHAN0         0   /* XOR channel 0 used for memory initialization */
#define XOR_UNIT0         0   /* XOR unit 0 used for memory initialization */
#define XOR_ADDR_DEC_WIN0 0   /* Enable DRAM access using XOR decode window 0 */ 

/* XOR Engine Address Decoding Register Map */                  
#define XOR_WINDOW_CTRL_REG(unit,chan)     (XOR_UNIT_BASE(unit)+(0x240 + ((chan) * 4)))
#define XOR_BASE_ADDR_REG(unit,winNum)     (XOR_UNIT_BASE(unit)+(0x250 + ((winNum) * 4)))
#define XOR_SIZE_MASK_REG(unit,winNum)     (XOR_UNIT_BASE(unit)+(0x270 + ((winNum) * 4)))

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
        bic     r1, r1, #0x00000300     /* clear bits 9:8 (--V- --RS) */
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
        
        
        /* DRAM memory initialization */
                
        /* Load SDRAM controller base address 0xd0001000             */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x1000


	/* Write to SDRAM coniguration register                         */         
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_CONFIG_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_CONFIG_REG)
        
	/* Write Dunit control low register                             */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_DUNIT_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_DUNIT_CTRL_REG);
        
        /* Write SDRAM address control register                         */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_ADDR_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_ADDR_CTRL_REG)
        
        /* Write SDRAM timing Low register                              */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_TIMING_CTRL_LOW_REG_DVAL))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_TIMING_CTRL_LOW_REG)
        
        /* Write SDRAM timing High register                             */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_TIMING_CTRL_HIGH_REG_DVAL))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_TIMING_CTRL_HIGH_REG)

        /* Write DDR2 SDRAM timing Low register                             */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_DDR2_TIMING_LO_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_DDR2_TIMING_LO_REG)

        /* Write DDR2 SDRAM timing High register                             */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_DDR2_TIMING_HI_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_DDR2_TIMING_HI_REG)
                
        /* Write SDRAM mode register                                    */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_MODE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_MODE_REG)
        
        /* Write SDRAM Extended mode register                           */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_EXTENDED_MODE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_EXTENDED_MODE_REG)

        /* Config DDR2 On Die Termination (ODT) registers               */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_SDRAM_ODT_CTRL_LOW_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_SDRAM_ODT_CTRL_LOW_REG)
        
        /* Write SDRAM DDR2 ODT control high register                   */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_SDRAM_ODT_CTRL_HIGH_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_SDRAM_ODT_CTRL_HIGH_REG)
        
        /* Write SDRAM DDR2 Dunit ODT control register                  */
        GPR_LOAD(r1, MV_32BIT_LE(DDR2_DUNIT_ODT_CTRL_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, DDR2_DUNIT_ODT_CONTROL_REG)

        /*SDRAM_DUNIT_CTRL_HI_REG */
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_DUNIT_CTRL_HI_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_DUNIT_CTRL_HI_REG)

        /* DDR SDRAM Initialization Control Register. Init enable       */
        mov   r1, #MV_32BIT_LE(DSICR_INIT_EN)
        MV_REG_WRITE_ASM (r1, r2, DDR_SDRAM_INIT_CTRL_REG)
         
ddrInitLoop:
	MV_REG_READ_ASM (r1, r2, DDR_SDRAM_INIT_CTRL_REG)
        cmp    r1, #0
        bne    ddrInitLoop
                          
        /* Continue in next NAND page */
        
        
/*****************************************************************************/
/* This code open NAND page 1 for read and relocates the CPU to read         */
/* from that page.                                                           */
/*****************************************************************************/


/* r2 - is the page number 		*/
/* r5 - nand flash base 		*/
/* r6 - is cmd read 			*/ 
/* r0 - 0 				*/
/* r9 - is cmd status			*/
/* r4 - next address to jump 	 	*/

        /* Load the NAND base address */
        GPR_LOAD(r5, NAND_FLASH_BASE)

	/* init */
	mov   r0, #0
    mov   r2, #0x1            	/* start with page 1 */
	mov   r6, #CMD_READ
	
    mov   r9, #CMD_STATUS
	orr   r4, r5, #0x200

.align 5
	b     ncl
sop:
        /* issue read command */
        strb  r6, [r5, #NAND_CMD_PORT]
                        
        /* issue address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r2, [r5, #NAND_ADDR_PORT] /* page address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        
        /* Check status */
        strb  r9, [r5, #NAND_CMD_PORT]
	b     busy_loop

.align 5
ncl:	
	b     ncl2
busy_loop:        
        ldrb  r1, [r5]
        tst   r1, #STATUS_READY 
        beq   busy_loop
        
        /* back to read mode */
        strb  r6, [r5, #NAND_CMD_PORT]
	mov   pc, r4
               
.align 5   
ncl2:
	b     sop
     

/* Next page must be in page offset - 512 bytes */
.align 9
page1:
/*****************************************************************************/
/* This code extract the booter from the rest of block 1 which is known to   */
/* be error free (no need for ECC). This booter should copy the U-boot code  */
/* to DRAM                                                                   */
/*****************************************************************************/
        /* DRAM init - Cont'd */
        
        /* Load back SDRAM controller base address 0xd0001000           */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x1000
        
        /* Open SDRAM bank 0 size register                              */ 
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_SIZE_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_SIZE_REG(0,0))
#ifdef RD_MV78XX0_MASA_2DIMM
	orr	r1, #0x5
        MV_REG_WRITE_ASM(r1, r2, SDRAM_SIZE_REG(0,1))
        GPR_LOAD(r1, MV_32BIT_LE(SDRAM_BASE1_REG_DV))
        MV_REG_WRITE_ASM(r1, r2, SDRAM_BASE_ADDR_REG(0,1))
#else
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(0,1))
#endif
        /* Close SDRAM bank 2,3                                       */ 
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(0,2))
        MV_REG_WRITE_ASM(r0, r2, SDRAM_SIZE_REG(0,3))

	/* Prepare the address where to find the nandBoot function pointer */
        mov   lr, #0x20000
	orr   lr, lr, #0x620
#if 1
        /* Load xor controller base address 0xd0060000           */
        mov    r2, #0xd0000000
        orr    r2, r2, #0x60000

        /* Disable all XOR address decode windows to avoid possible overlap */
        MV_REG_WRITE_ASM (r0, r2, (XOR_WINDOW_CTRL_REG(XOR_UNIT0,XOR_CHAN0)))

        /* Init first XOR_SIZE_MASK_REG */
        MV_REG_WRITE_ASM (r1, r2, XOR_SIZE_MASK_REG(XOR_UNIT0,XOR_ADDR_DEC_WIN0))
       
        /* Update destination & size */
        MV_REG_WRITE_ASM(r0, r2, XOR_DST_PTR_REG(XOR_UNIT0,XOR_CHAN0))
	mov	r6, #0xe00              
        MV_REG_WRITE_ASM (r6, r2, XOR_BASE_ADDR_REG(XOR_UNIT0,XOR_ADDR_DEC_WIN0))
	mov	r6, #0xf0000
	orr	r6, r6, #0xf000
	orr	r6, r6, #0x1              
        MV_REG_WRITE_ASM (r6, r2, XOR_WINDOW_CTRL_REG(XOR_UNIT0,XOR_CHAN0))        

        /* Configure XOR engine for memory init function.           */
        MV_REG_READ_ASM (r6, r2, XOR_CONFIG_REG(XOR_UNIT0,XOR_CHAN0))
        and	r6, r6, #~0x7        	/* Clear operation mode field      */
        orr     r6, r6, #0x4             /* Set operation to memory init    */
        MV_REG_WRITE_ASM(r6, r2, XOR_CONFIG_REG(XOR_UNIT0,XOR_CHAN0))
               
        /* Set initVal in the XOR Engine Initial Value Registers       */
        GPR_LOAD(r6, MV_32BIT_LE(0xfeedfeed))
        MV_REG_WRITE_ASM(r6, r2, XOR_INIT_VAL_LOW_REG(XOR_UNIT0))
        MV_REG_WRITE_ASM(r6, r2, XOR_INIT_VAL_HIGH_REG(XOR_UNIT0))

        /* Set block size using DRAM bank size  */
	and	r6, r1, #SCSR_SIZE_MASK
	mov	r1, r6, LSR #SCSR_SIZE_OFFS
        add	r1, r1, #1
	mov	r6, r5, LSL #SCSR_SIZE_OFFS

        MV_REG_WRITE_ASM(r6, r2, XOR_BLOCK_SIZE_REG(XOR_UNIT0,XOR_CHAN0))
        
        /* Clean interrupt cause*/
        MV_REG_WRITE_ASM(r0, r2, XOR_CAUSE_REG(XOR_UNIT0))

        /* Clean error interrupt cause*/
        MV_REG_READ_ASM(r6, r2, XOR_ERROR_CAUSE_REG(XOR_UNIT0))
        MV_REG_READ_ASM(r6, r2, XOR_ERROR_ADDR_REG(XOR_UNIT0))

        /* Start transfer */
        MV_REG_READ_ASM (r6, r2, XOR_ACTIVATION_REG(XOR_UNIT0,XOR_CHAN0))
        orr     r6, r6, #0x1 /* Preform start command      */
        MV_REG_WRITE_ASM(r6, r2, XOR_ACTIVATION_REG(XOR_UNIT0,XOR_CHAN0))

.align 5
        b     ncl32
sop1:
        /* Wait for engine to finish */
waitForComplete:        
        MV_REG_READ_ASM(r6, r2, XOR_CAUSE_REG(XOR_UNIT0))
        and   	r6, r6, #2
	    cmp	r6, #0
        beq     waitForComplete

	    b	cp_page_init
	
#else	
        /* Initialize UART 0 to 115200 bps */
        GPR_LOAD(r2, 0xd0012000)
        mov   r1, #0x07
        strb  r1, [r2, #0x8]        
        
        mov   r1, #0x83
        strb  r1, [r2, #0xC]        

        mov   r1, #0x5A
        strb  r1, [r2, #0x0]        
        
        mov   r1, #0x03
        strb  r1, [r2, #0xC]        
        
        mov   r1, #'N'
        strb  r1, [r2]        
        mov   r1, #'A'
        strb  r1, [r2]
        mov   r1, #'N'
        strb  r1, [r2]
        mov   r1, #'D'
        strb  r1, [r2]
        mov   r1, #' '
        strb  r1, [r2]
        mov   r1, #'b'
        strb  r1, [r2]        
        mov   r1, #'o'
        strb  r1, [r2]
        mov   r1, #'o'
        strb  r1, [r2]
        mov   r1, #'t'
        strb  r1, [r2]
        mov   r1, #'s'
        strb  r1, [r2]        
        mov   r1, #'t'
        strb  r1, [r2]
        mov   r1, #'r'
        strb  r1, [r2]
        mov   r1, #'a'
        strb  r1, [r2]
        mov   r1, #'p'
        strb  r1, [r2]
        mov   r1, #10
        strb  r1, [r2]
        mov   r1, #13
        strb  r1, [r2]
      
#endif
.align 5
ncl32:        
        b     ncl33
cp_page_init:
	/* init */
        mov   r2, #0x3            	/* start with page 3 */
        mov   r3, #0x20000
	orr   r3, r3, #0x600
        b     cp_page_loop

.align 5
ncl33:        
        b     ncl31

cp_page_loop:
	mov   r0, #0
        mov   r6, #CMD_READ
        mov   r9, #CMD_STATUS
	mov   r4, #PAGE_SIZE           	/* Byte counter */
        b     startRead

.align 5
ncl31:        
        b     ncl3        
startRead:	
        /* issue read command */
        strb  r6, [r5, #NAND_CMD_PORT]
                        
        /* issue address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        strb  r2, [r5, #NAND_ADDR_PORT] /* page address */
        strb  r0, [r5, #NAND_ADDR_PORT]
        
        /* Check status */
        strb  r9, [r5, #NAND_CMD_PORT]
        b     busy_loop1

.align 5
ncl3:        
        b     ncl4
        
busy_loop1:        
        ldrb  r1, [r5]
        tst   r1, #STATUS_READY
        beq busy_loop1
        
        /* back to read mode */
        strb  r6, [r5, #NAND_CMD_PORT]
        
        /* now perform reading */
        mov   r0, r5
        b     copy_loop1
        
.align 5
ncl4:        
        b     ncl5

copy_loop1:
        sub   r4, r4, #32 	/* 8 dwords * 4 bytes */
	ldmia r0!, {r6-r13}
        stmia r3!, {r6-r13}
        
        cmp   r4, #0 /* check if we have read a full Page */
        bne   copy_loop1
        b     nextPage

.align 5
ncl5:        
        b     ncl6
        
nextPage:        
        add   r2, r2, #1  /* increment page number */
        cmp   r2, #BOOTER_PAGE_NUM
        bne   cp_page_loop
        b     stack_setup

.align 5
ncl6:        
        b     sop1

        
	/* Set up the stack						    */
stack_setup:
	sub   sp, lr, #40		/* leave 3 words for abort-stack    */
        
        /* jump to new  code */
        mov   pc, lr
               

.align 9

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
        
        ldr   lr, =nand_boot
        mov   pc, lr
#endif /* !defined(MV_BOOTROM) */
