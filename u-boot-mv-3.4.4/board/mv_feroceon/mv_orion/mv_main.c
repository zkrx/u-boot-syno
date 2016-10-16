/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
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

#include <common.h>
#include "mvTypes.h"
#include "mvBoardEnvLib.h"
#include "mvCpuIf.h"
#include "mvCtrlEnvLib.h"
#include "mv_mon_init.h"
#include "rtc/ext_rtc/mvDS133x.h"
#include "mvDebug.h"
#include "cpu/mvCpuArm.h"
#include "device/mvDevice.h"
#include "twsi/mvTwsi.h"
#include "ethfp/mvEth.h"
#include "pex/mvPex.h"
#include "eth-phy/mvEthPhy.h"
#include "ethswitch/mvSwitch.h"
#include "gpp/mvGpp.h"
#include "sys/mvSysUsb.h"

#if defined(MV_INCLUDE_XOR)
#include "xor/mvXor.h"
#endif
#if defined(MV_INCLUDE_IDMA)
#include "sys/mvSysIdma.h"
#include "idma/mvIdma.h"
#endif
#if defined(MV_INCLUDE_USB)
#include "usb/mvUsb.h"
#endif

#include "cpu/mvCpu.h"

#ifdef CONFIG_PCI
#   include <pci.h>
#endif
#include "pci/mvPciRegs.h"

#include <asm/arch/vfpinstr.h>
#include <asm/arch/vfp.h>

/* CPU address decode table. */
MV_CPU_DEC_WIN mvCpuAddrWinMap[] = MV_CPU_IF_ADDR_WIN_MAP_TBL;

static void mvStatusDebugLeds(char num);
#if defined(RD_88F6082NAS) || defined(RD_88F6082GE_SATA) || defined(RD_88F6082DAS_PLUS) || defined(RD_88F5182_3)
static void mvHddPowerCtrl(void);
#endif
#if defined(RD_88F6082MICRO_DAS_NAS)
static void bootDetection(void);
static void mv6082uDasNasSysPower(MV_BOOL enable);
#endif
void mv_cpu_init(void);
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
static int mv_set_power_scheme(void);
#endif

#ifdef	CFG_FLASH_CFI_DRIVER
MV_VOID mvUpdateNorFlashBaseAddrBank(MV_VOID);
int mv_board_num_flash_banks;
extern flash_info_t	flash_info[]; /* info for FLASH chips */
extern unsigned long flash_add_base_addr (uint flash_index, ulong flash_base_addr);
#endif	/* CFG_FLASH_CFI_DRIVER */

#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
extern MV_VOID mvBoardEgigaPhySwitchInit(void);
#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
/* Define for SDK 2.0 */
int __aeabi_unwind_cpp_pr0(int a,int b,int c) {return 0;}
int __aeabi_unwind_cpp_pr1(int a,int b,int c) {return 0;}
#endif

/* Define for SDK 2.0 */
int raise(void) {return 0;}

/* Define only for open Linux support */
#define RD_KUROBOX_PRO			0xFE
#define RD_KUROBOX_PRO_OPEN_LINUX_ID	1509

void print_mvBanner(void)
{
	printf("\n");
	printf("         __  __                      _ _\n");
	printf("        |  \\/  | __ _ _ ____   _____| | |\n");
	printf("        | |\\/| |/ _` | '__\\ \\ / / _ \\ | |\n");
	printf("        | |  | | (_| | |   \\ V /  __/ | |\n");
	printf("        |_|  |_|\\__,_|_|    \\_/ \\___|_|_|\n");
	printf(" _   _     ____              _\n");
	printf("| | | |   | __ )  ___   ___ | |_ \n");
	printf("| | | |___|  _ \\ / _ \\ / _ \\| __| \n");
	printf("| |_| |___| |_) | (_) | (_) | |_ \n");
	printf(" \\___/    |____/ \\___/ \\___/ \\__| ");
#ifdef MV_INCLUDE_MONT_EXT
	if(!enaMonExt())
		printf(" ** LOADER **"); 
	else
		printf(" ** MONITOR **");
#else

	printf(" ** Forcing LOADER mode only **"); 
#endif /* MV_INCLUDE_MONT_EXT */


	return;
}

void print_dev_id(void){
	static char boardName[30];

	mvBoardNameGet(boardName);
	
#if defined(MV_CPU_BE)
	printf("\n ** MARVELL BOARD: %s BE ",boardName);
#else
	printf("\n ** MARVELL BOARD: %s LE ",boardName);
#endif

    return;
}


extern MV_BOOL mvSataPhyShutdown(MV_U8 port);
MV_BOOL mvSataPhyPowerOn(MV_U8 port);

#define CPU_MAIN_IRQ_MASK 0x20204
#define CPU_MAIN_FIQ_MASK 0x20208
#define CPU_END_POIN_MASK 0x2020c
void maskAllInt(void)
{
        /* mask all external interrupt sources */
        MV_REG_WRITE(CPU_MAIN_IRQ_MASK, 0);
        MV_REG_WRITE(CPU_MAIN_FIQ_MASK, 0);
        MV_REG_WRITE(CPU_END_POIN_MASK, 0);
}

/* init for the Master*/
void misc_init_r_dec_win(void)
{
    /* update all the windows BARS */
#if defined(MV_INCLUDE_IDMA)
    mvDmaInit();
#endif

#if defined(MV_INCLUDE_USB)
	{
		char *env;

		env = getenv("usb0Mode");
		if((!env) || (strcmp(env,"device") == 0) || (strcmp(env,"Device") == 0) )
		{
			printf("USB 0: device mode\n");	
			mvUsbInit(0, MV_FALSE);
		}
		else
		{
			printf("USB 0: host mode\n");	
			mvUsbInit(0, MV_TRUE);
		}
			
		
		if (mvCtrlUsbMaxGet() == 2)
		{
			env = getenv("usb1Mode");
			if((!env) || (strcmp(env,"device") == 0) || (strcmp(env,"Device") == 0) )
			{
				printf("USB 1: device mode\n");	
				mvUsbInit(1, MV_FALSE);
			}
			else
			{
				printf("USB 1: host mode\n");	
				mvUsbInit(1, MV_TRUE);
			}
				

		}

	}
#endif/* #if defined(MV_INCLUDE_USB) */


#if defined(MV_INCLUDE_XOR)
	mvXorInit();
#endif

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	mv_set_power_scheme();
#endif

    return;
}


/*
 * Miscellaneous platform dependent initialisations
 */

extern	MV_STATUS mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data);
extern	MV_STATUS mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data);

/* golabal mac address for yukon EC */
unsigned char yuk_enetaddr[6];
extern int interrupt_init (void);
extern void i2c_init(int speed, int slaveaddr);


int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
#if defined(MV_INCLUDE_TWSI)
	MV_TWSI_ADDR slave;
#endif
	unsigned int i;

	maskAllInt();
	/* must initialize the int in order for udelay to work */
	interrupt_init();

#if defined(MV_INCLUDE_TWSI)
	slave.type = ADDR7_BIT;
	slave.address = 0;
	mvTwsiInit(CFG_I2C_SPEED, CFG_TCLK, &slave, 0);
#endif

 
	/* Init the Board environment module (device bank params init) */
	mvBoardEnvInit();
   	
	/* Init the Controlloer environment module (MPP init) */
	mvCtrlEnvInit();

#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
	/* Init the PHY or Switch of the board */
	mvBoardEgigaPhySwitchInit();
#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */

	mvStatusDebugLeds(3);

        /* Init the Controller CPU interface */
	mvCpuIfInit(mvCpuAddrWinMap);
                                                                                                                                     
        /* arch number of Integrator Board */
        gd->bd->bi_arch_number = 526;
 
        /* adress of boot parameters */
        gd->bd->bi_boot_params = 0x00000100;

	/* relocate the exception vectors */
	/* U-Boot is running from DRAM at this stage */
	for(i = 0; i < 0x100; i+=4)
	{
		*(unsigned int *)(0x0 + i) = *(unsigned int*)(TEXT_BASE + i);

	}

	/* Enable i cache for DB_FPGA only */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	__asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop");
	i |= BIT12;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));
	__asm__ __volatile__("nop;nop;nop;nop;nop;nop;nop");

	/* i cache can work without MMU,
	   d cache can not work without MMU:
	 i cache already enable in start.S */
	/* Change reset vector to address 0x0 */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	i &= ~BIT13;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));
	
	/* Update NOR flash base address bank for CFI driver */
#ifdef	CFG_FLASH_CFI_DRIVER
	mvUpdateNorFlashBaseAddrBank();
#endif	/* CFG_FLASH_CFI_DRIVER */

	mvStatusDebugLeds(4);
	
	return 0;
}

void misc_init_r_env(void){
        char *env;
        char tmp_buf[10];
        unsigned int malloc_len, board_id;
        DECLARE_GLOBAL_DATA_PTR;
	
#if defined(RD_88F5181L_FE) || defined(RD_88F5181L_GE) || \
    defined(RD_88W8660) || defined(RD_88F5181L_FXO_GE) || \
	defined(RD_88F5181_GTWGE) || defined(RD_88F5181_GTWFE) || defined(RD_88W8660_AP82S) ||\
	defined(DB_88W8660) || defined(DB_88F5181L)

#if defined(MV_NAND_BOOT)
	env = getenv("bootargs");
	if(!env)
		setenv("bootargs","console=ttyS0,115200 mtdparts=nand_mtd:1m(uboot)ro,2m(uImage),29m(rootfs)");
#else
        unsigned int flashSize =0 , secSize =0, ubootSize =0;
	char buff[256];

#if defined(MV_BOOTSIZE_4M)
	flashSize = _4M;
#elif defined(MV_BOOTSIZE_8M)
	flashSize = _8M;
#elif defined(MV_BOOTSIZE_16M)
	flashSize = _16M;
#elif defined(MV_BOOTSIZE_32M)
	flashSize = _32M;
#elif defined(MV_BOOTSIZE_64M)
	flashSize = _64M;
#endif

#if defined(MV_SEC_64K)
	secSize = _64K;
#if defined(MV_TINY_IMAGE)
	ubootSize = _256K;
#else
	ubootSize = _512K;
#endif
#elif defined(MV_SEC_128K)
	secSize = _128K;
#if defined(MV_TINY_IMAGE)
	ubootSize = _128K * 3;
#else
	ubootSize = _128K * 5;
#endif
#elif defined(MV_SEC_256K)
	secSize = _256K;
#if defined(MV_TINY_IMAGE)
	ubootSize = _256K * 3;
#else
	ubootSize = _256K * 3;
#endif
#endif

	if ((0 == flashSize) || (0 == secSize) || (0 == ubootSize))
	{
		env = getenv("bootargs");
		if(!env)
			setenv("bootargs","console=ttyS0,115200");
	}
	else
	{
		sprintf(buff,"console=ttyS0,115200 mtdparts=cfi_flash:0x%x(root),0x%x(uboot)ro",
			flashSize - ubootSize, ubootSize);
		env = getenv("bootargs");
		if(!env)
			setenv("bootargs",buff);
	}
#endif /* defined(MV_NAND_BOOT) */
#else
	env = getenv("bootargs");
	if(!env)
		setenv("bootargs","console=ttyS0,115200");
#endif

	/* Check if to use the LED's for debug or to use single led for init and Linux heartbeat */
#if defined(MV_INCLUDE_TDM) || defined(MV_INC_BOARD_QD_SWITCH)
	env = getenv("enaDebugLed");
	if(!env || ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
		setenv("enaDebugLed","no");
	else
		setenv("enaDebugLed","yes");
#else
	env = getenv("enaDebugLed");
	if(!env || ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		setenv("enaDebugLed","yes");
	else
		setenv("enaDebugLed","no");
#endif
		
        /* Linux open port support */
	env = getenv("mainlineLinux");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		setenv("mainlineLinux","yes");
	else
		setenv("mainlineLinux","no");

	env = getenv("mainlineLinux");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
	{
	    /* arch number for open port Linux */
	    env = getenv("arcNumber");
	    if(!env )
	    {
		/* arch number according to board ID */
		board_id = mvBoardIdGet();	
		switch(board_id){
		    case(DB_88F5X81_DDR2):
			sprintf(tmp_buf,"%d", DB_88F5X81_DDR2_OPEN_LINUX_ID);
			board_id = DB_88F5X81_DDR2_OPEN_LINUX_ID; 
			break;
		    case(RD_88F5182_2XSATA):
			sprintf(tmp_buf,"%d", RD_88F5182_2XSATA_OPEN_LINUX_ID);
			board_id = RD_88F5182_2XSATA_OPEN_LINUX_ID; 
			break;
		    case(RD_KUROBOX_PRO):
			sprintf(tmp_buf,"%d", RD_KUROBOX_PRO_OPEN_LINUX_ID);
			board_id = RD_KUROBOX_PRO_OPEN_LINUX_ID; 
			break;
		    default:
			sprintf(tmp_buf,"%d", DB_88F5X81_DDR2_OPEN_LINUX_ID);
			board_id = DB_88F5X81_DDR2_OPEN_LINUX_ID; 
			break;
		}
		gd->bd->bi_arch_number = board_id;
		setenv("arcNumber", tmp_buf);
	    }
	    else
	    {
		gd->bd->bi_arch_number = simple_strtoul(env, NULL, 10);
	    }
	}

        /* update the CASset env parameter */
        env = getenv("CASset");
        if(!env )
        {
#ifdef MV_MIN_CAL
                setenv("CASset","min");
#else
                setenv("CASset","max");
#endif
        }
        /* Monitor extension */
#ifdef MV_INCLUDE_MONT_EXT
        env = getenv("enaMonExt");
        if(/* !env || */ ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
                setenv("enaMonExt","yes");
        else
#endif
                setenv("enaMonExt","no");

        env = getenv("enaFlashBuf");
        if( ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
                setenv("enaFlashBuf","no");
        else
                setenv("enaFlashBuf","yes");

	/* CPU streaming */
        env = getenv("enaCpuStream");
        if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
                setenv("enaCpuStream","no");
        else
                setenv("enaCpuStream","yes");

	if(mvCtrlModelGet() == MV_5281_DEV_ID) /* Orion 2 */
	{
		/* VFP */
		env = getenv("enaVFP");
		if (mvCtrlRevGet() == MV_5281_C0_REV)  /* Orion 2 == C0 */
		{
			if( env && ( ((strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) )))
				setenv("enaVFP","yes");
			else
				setenv("enaVFP","no");
		}
		else
		{
			if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
				setenv("enaVFP","no");
			else
				setenv("enaVFP","yes");

		}
		
		/* Orion 2 D2 VFP sub arch */	
		if (mvCtrlRevGet() == MV_5281_D2_REV)  /* Orion 2 == D2 */
		{
			env = getenv("enaVFP");
			if( env && ( ((strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) )))
			{
				env = getenv("vfpSubArch2");
				if( !env || ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
					setenv("vfpSubArch2","no");
				else
					setenv("vfpSubArch2","yes");
			}
		}

		/* Write allocation for all 5281 device set to no */
		env = getenv("enaWrAllo");
		if( !env || ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaWrAllo","no");
		else
			setenv("enaWrAllo","yes");

		if (mvCtrlRevGet() >= MV_5281_B0_REV) /* Orion 2 >= B0 */
		{

			/* ICache Prefetch */
			env = getenv("enaICPref");
			if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
				setenv("enaICPref","no");
			else
				setenv("enaICPref","yes");
		
		
			/* DCache Prefetch */
			env = getenv("enaDCPref");
			if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
				setenv("enaDCPref","no");
			else
				setenv("enaDCPref","yes");
		}
	}

	if (mvCtrlModelGet() == MV_1281_DEV_ID) /* TC90 */
	{
		/* VFP */
		env = getenv("enaVFP");
		if( env && ( ((strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) )))
			setenv("enaVFP","yes");
		else
			setenv("enaVFP","no");

		/* Write allocation */
		env = getenv("enaWrAllo");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaWrAllo","no");
		else
		{	
			setenv("enaWrAllo","yes");
		}

		/* ICache Prefetch */
		env = getenv("enaICPref");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaICPref","no");
		else
			setenv("enaICPref","yes");
		
		
		/* DCache Prefetch */
		env = getenv("enaDCPref");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaDCPref","no");
		else
			setenv("enaDCPref","yes");
	}

	if (mvCtrlModelGet() == MV_6183_DEV_ID || 
        mvCtrlModelGet() == MV_6183L_DEV_ID) /* MV88F6183 & MV88F6183L*/
	{

		/* Pex mode */
		env = getenv("pexMode");
		if( env && ( ((strcmp(env,"EP") == 0) || (strcmp(env,"ep") == 0) )))
			setenv("pexMode","EP");
		else
		{	
			setenv("pexMode","RC");
		}

		/* Write allocation */
		env = getenv("enaWrAllo");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaWrAllo","no");
		else
		{	
			setenv("enaWrAllo","yes");
		}

		/* ICache Prefetch */
		env = getenv("enaICPref");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaICPref","no");
		else
			setenv("enaICPref","yes");
		
		
		/* DCache Prefetch */
		env = getenv("enaDCPref");
		if( env && ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("enaDCPref","no");
		else
			setenv("enaDCPref","yes");
	}

	if (mvCtrlModelGet() == MV_6082_DEV_ID) /* MV88F6082 */
	{

		/* Pex mode */
		env = getenv("pexMode");
		if( env && ( ((strcmp(env,"EP") == 0) || (strcmp(env,"ep") == 0) )))
			setenv("pexMode","EP");
		else
		{	
			setenv("pexMode","RC");
		}
	}

        env = getenv("sata_dma_mode");
        if( env && ((strcmp(env,"No") == 0) || (strcmp(env,"no") == 0) ) )
                setenv("sata_dma_mode","no");
        else
                setenv("sata_dma_mode","yes");


        /* Malloc length */
        env = getenv("MALLOC_len");
        malloc_len =  simple_strtoul(env, NULL, 10) << 20;
        if(malloc_len == 0){
                sprintf(tmp_buf,"%d",CFG_MALLOC_LEN>>20);
                setenv("MALLOC_len",tmp_buf);}
         
        /* primary network interface */
        env = getenv("ethprime");

	if(!env)
		setenv("ethprime",ENV_ETH_PRIME);
 

	/* netbsd boot arguments */
        env = getenv("netbsd_en");
	if( !env || ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
			setenv("netbsd_en","no");
	else
	{

	    setenv("netbsd_en","yes");
	    env = getenv("netbsd_gw");
	    if(!env)
                setenv("netbsd_gw","192.168.0.254");

	    env = getenv("netbsd_mask");
	    if(!env)
                setenv("netbsd_mask","255.255.255.0");

	    env = getenv("netbsd_fs");
	    if(!env)
                setenv("netbsd_fs","nfs");

	    env = getenv("netbsd_server");
	    if(!env)
                setenv("netbsd_server","192.168.0.1");

	    env = getenv("netbsd_ip");
	    if(!env)
	    {
		env = getenv("ipaddr");
               	setenv("netbsd_ip",env);
	    }

	    env = getenv("netbsd_rootdev");
	    if(!env)
                setenv("netbsd_rootdev","mgi0");

	    env = getenv("netbsd_add");
	    if(!env)
                setenv("netbsd_add","0x800000");

	    env = getenv("netbsd_get");
	    if(!env)
                setenv("netbsd_get","tftpboot $(netbsd_add) $(image_name)");

	    env = getenv("netbsd_set_args");
	    if(!env)
	    	setenv("netbsd_set_args","setenv bootargs nfsroot=$(netbsd_server):$(rootpath) fs=$(netbsd_fs) \
ip=$(netbsd_ip) serverip=$(netbsd_server) mask=$(netbsd_mask) gw=$(netbsd_gw) rootdev=$(netbsd_rootdev) \
ethaddr=$(ethaddr)");

	    env = getenv("netbsd_boot");
	    if(!env)
	    	setenv("netbsd_boot","bootm $(netbsd_add) $(bootargs)");

	    env = getenv("netbsd_bootcmd");
	    if(!env)
	    	setenv("netbsd_bootcmd","run netbsd_get ; run netbsd_set_args ; run netbsd_boot");
	}

        /* linux boot arguments */
        env = getenv("bootargs_root");
        if(!env)
                setenv("bootargs_root","root=/dev/nfs rw");
 
	/* For open Linux we set boot args differently */
	env = getenv("mainlineLinux");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
	{
        	env = getenv("bootargs_end");
        	if(!env)
                setenv("bootargs_end",":::orion:eth0:none");
	}
	else
	{
        	env = getenv("bootargs_end");
        	if(!env)
                setenv("bootargs_end",CFG_BOOTARGS_END);
	}
	
 
        env = getenv("image_name");
        if(!env)
                setenv("image_name","uImage");
 

#if (CONFIG_BOOTDELAY >= 0)
        env = getenv("bootcmd");
        if(!env)
#if defined(MV_INCLUDE_TDM) && defined(MV_INC_BOARD_QD_SWITCH)
	    setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig) $(mvPhoneConfig);  bootm 0x400000; ");
#elif defined(MV_INC_BOARD_QD_SWITCH)
            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig);  bootm 0x400000; ");
#elif defined(MV_INCLUDE_TDM)
            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvPhoneConfig);  bootm 0x400000; ");
#elif defined(RD_88F6082MICRO_DAS_NAS)
            setenv("bootcmd","setenv bootargs $(bootargs) temp=$(minTemp),$(maxTemp); \
bootext2 0:1,2 0x1000000 /boot/$(image_name) /dev/sda;");
#else

            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
ip=$(ipaddr):$(serverip)$(bootargs_end);  bootm 0x400000; ");
#endif
#endif /* (CONFIG_BOOTDELAY >= 0) */

        env = getenv("standalone");
        if(!env)
#if defined(MV_INCLUDE_TDM) && defined(MV_INC_BOARD_QD_SWITCH)
	    setenv("standalone","fsload 0x400000 $(image_name);setenv bootargs $(bootargs) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig) $(mvPhoneConfig); bootm 0x400000;");
#elif defined(MV_INC_BOARD_QD_SWITCH)
            setenv("standalone","fsload 0x400000 $(image_name);setenv bootargs $(bootargs) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig); bootm 0x400000;");
#elif defined(MV_INCLUDE_TDM)
            setenv("standalone","fsload 0x400000 $(image_name);setenv bootargs $(bootargs) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvPhoneConfig); bootm 0x400000;");
#else
            setenv("standalone","fsload 0x400000 $(image_name);setenv bootargs $(bootargs) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end); bootm 0x400000;");
#endif
                 
#if defined(DB_PRPMC) || defined(DB_MNG)
		env = getenv("vx_boot"); 
		if(!env)
				setenv("vx_boot","tftpboot $(default_load_addr) $(image_name);setenv bootargs sgi(0,0) host:VxWorks h=$(serverip) \
e=$(ipaddr):FFFF0000 u=anonymous pw=target; setenv bootaddr 0x700; bootvx $(default_load_addr);");

		env = getenv("dhcp_boot");
		if(!env)
				setenv("dhcp_boot","dhcp;setenv bootargs $(bootargs) $(bootargs_root) nfsroot=$(serverip):$(rootpath) \
ip=$(ipaddr):$(serverip)$(bootargs_end);  bootm $(default_load_addr);");
#endif /* DB_PRPMC || DB_MNG */

       /* Set boodelay to 3 sec, if Monitor extension are disabled */
        if(!enaMonExt()){
#if defined(RD_88F6082MICRO_DAS_NAS)
                setenv("bootdelay","1");
#else
                setenv("bootdelay","3");
#endif
		setenv("disaMvPnp","no");
	}

	/* Disable PNP config of Marvel memory controller devices. */
        env = getenv("disaMvPnp");
        if(!env)
                setenv("disaMvPnp","no");

 


#if (defined(MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH)) \
	&& !defined(DB_PRPMC) && !defined(DB_88F1281) && !defined(DB_MNG)

	/* MAC addresses */
        env = getenv("ethaddr");
        if(!env)
                setenv("ethaddr",ETHADDR);
        
#if !defined(MV_INCLUDE_TDM) && !defined(MV_INC_BOARD_QD_SWITCH)
/* ETH1ADDR not define in GWAP boards */
        env = getenv("eth1addr");
        if(!env)
                setenv("eth1addr",ETH1ADDR);
#endif

#if defined(MV_INCLUDE_TDM)
        /* Set mvPhoneConfig env parameter */
        env = getenv("mvPhoneConfig");
        if(!env )
            setenv("mvPhoneConfig","mv_phone_config=dev0:fxs,dev1:fxs");
#endif

#if defined(MV_INC_BOARD_QD_SWITCH)
        /* Set mvNetConfig env parameter */
        env = getenv("mvNetConfig");
        if(!env )
		    setenv("mvNetConfig","mv_net_config=(eth0,00:aa:bb:cc:dd:ee,0)(eth1,00:11:22:33:44:55,1:2:3:4)");
#endif
#endif /*  (MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH) */

#if defined(MV_INCLUDE_PCI)
	env = getenv("pciMode");
	if(!env)
	{
			setenv("pciMode",ENV_PCI_MODE);
	}
	
	env = getenv("pciMode");
	if ( env && ( (strcmp(env,"device") == 0) || (strcmp(env,"Device") == 0) ))
			MV_REG_WRITE(PCI_ARBITER_CTRL_REG(0),(MV_REG_READ(PCI_ARBITER_CTRL_REG(0) & BIT31)));

#endif /* (MV_INCLUDE_PCI) */

#if defined(MV_INCLUDE_USB)

	/* USB Host */
	env = getenv("usb0Mode");

	if(!env)
	{
		setenv("usb0Mode",ENV_USB0_MODE);
	}

	if (mvCtrlUsbMaxGet() == 2)
	{
		env = getenv("usb1Mode");

		if(!env)
		{
			setenv("usb1Mode",ENV_USB1_MODE);
		}

	}

		
#endif  /* (MV_INCLUDE_USB) */

#if defined(YUK_ETHADDR)

	env = getenv("yuk_ethaddr");
	if(!env)
			setenv("yuk_ethaddr",YUK_ETHADDR);

	{
		int i;

		char *tmp = getenv ("yuk_ethaddr");
		char *end;

		for (i=0; i<6; i++) {
			yuk_enetaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
			if (tmp)
				tmp = (*end) ? end+1 : end;

		}

	}


#endif /* defined(YUK_ETHADDR) */

#if defined(RD_88F6082NAS) || defined(RD_88F6082GE_SATA) || defined(RD_88F6082DAS_PLUS) || defined(RD_88F5182_3)
	mvHddPowerCtrl();
#endif

#if defined(RD_88F6082MICRO_DAS_NAS)
        env = getenv("serialNo");
        if(!env)
                setenv("serialNo","AS100352999999");

        env = getenv("fanPowerCtrl");
        if(!env)
                setenv("fanPowerCtrl","Yes");

        env = getenv("minTemp");
        if(!env)
                setenv("minTemp","45");

        env = getenv("maxTemp");
        if(!env)
                setenv("maxTemp","50");
	
	env = getenv("enaPost");
        if(!env)
		setenv("enaPost","No");

	bootDetection();
#endif
        return;
}

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	/* Check if to use the LED's for debug or to use single led for init and Linux heartbeat */
	mvStatusDebugLeds(0);
	    
	return 0;
}
#endif

int misc_init_r (void)
{
	char name[15];
	char *env;
	volatile unsigned int temp;
	
	mvStatusDebugLeds(5);

	/* set as 946 */
	env = getenv("cpuName");
	if(!env)
			setenv("cpuName","926");
	else
	{
		if (strcmp(env,"946") == 0)
		{
			/* set as 946 */
			unsigned int regVal;

			regVal = MV_REG_READ(CPU_CONFIG_REG);
			regVal |= CCR_MMU_DISABLED;
			MV_REG_WRITE(CPU_CONFIG_REG, regVal);
		}

	}

	mvCpuNameGet(name);
	printf("\nCPU : %s\n",  name);

	/* init special env variables */
	misc_init_r_env();

	mv_cpu_init();

#if defined(MV_INCLUDE_MONT_EXT)
	if(enaMonExt()){
			printf("\n      Marvell monitor extension:\n");
			mon_extension_after_relloc();
	}
    printf("\n");		
#endif /* MV_INCLUDE_MONT_EXT */

	/* init the units decode windows */
	misc_init_r_dec_win();

#ifdef CONFIG_PCI
#if !defined(MV_MEM_OVER_PCI_WA)
        pci_init();
#endif
#endif

	mvStatusDebugLeds(6);

	if((mvCtrlModelGet() == MV_5281_DEV_ID) || /* Orion 2 */
	(mvCtrlModelGet() == MV_1281_DEV_ID)) /* TC90 */
	{

		__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
		if (temp & 0x10000000)
		{
			printf("CPU: Write allocate enabled\n");
		}
		else
		{
			printf("CPU: Write allocate Disabled\n");
		}
	}

	mvStatusDebugLeds(7);
	
	return 0;
}

MV_U32 mvTclkGet(void)
{
        DECLARE_GLOBAL_DATA_PTR;
                                                                                                                                               
        /* get it only on first time */
        if(gd->tclk == 0)
                gd->tclk = mvBoardTclkGet();
                                                                                                                                               
        return gd->tclk;
}
                                                                                                                                               
MV_U32 mvSysClkGet(void)
{
        DECLARE_GLOBAL_DATA_PTR;
                                                                                                                                               
        /* get it only on first time */
        if(gd->bus_clk == 0)
                gd->bus_clk = mvBoardSysClkGet();
                                                                                                                                               
        return gd->bus_clk;
}
 
#ifndef MV_TINY_IMAGE
/* exported for EEMBC */
MV_U32 mvGetRtcSec(void)
{
        MV_RTC_TIME time;
        mvRtcDS133xTimeGet(&time);
	return (time.minutes * 60) + time.seconds;
}
#endif

void reset_cpu(void)
{
	mvBoardReset();
}


void mv_cpu_init(void)
{
	char *env;
	char name[20];
	volatile unsigned int temp;

	/*CPU streaming */
    if( (mvCtrlModelGet() == MV_5181_DEV_ID) ||
		(mvCtrlModelGet() == MV_5182_DEV_ID) ||
		(mvCtrlModelGet() == MV_5180_DEV_ID) ||
		(mvCtrlModelGet() == MV_8660_DEV_ID) ||
		(mvCtrlModelGet() == MV_5082_DEV_ID) ) /* orion 1 */
	{ 
		
			__asm__ __volatile__("mrc    p15, 0, %0, c1, c0, 0" : "=r" (temp):: "memory");

			env = getenv("enaCpuStream");
			if(!env || (strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0) )
			{
				mvCtrlModelRevNameGet(name);
				printf("%s streaming disabled \n",name);
				temp &= ~BIT28;
			}
			else
			{
				printf("Orion 1 streaming enabled \n");
				 temp |= BIT28;
			}

			__asm__ __volatile__("mcr    p15, 0, %0, c1, c0, 0": :"r" (temp));
			
	}
	else if((mvCtrlModelGet() == MV_5281_DEV_ID) || /* Orion 2 */
		(mvCtrlModelGet() == MV_6183_DEV_ID) || /* Orion 6183 */
        (mvCtrlModelGet() == MV_6183L_DEV_ID) || /* Orion 6183L */
		(mvCtrlModelGet() == MV_1281_DEV_ID)) /* TC90 */
	{ 
		__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");

		env = getenv("enaCpuStream");
		if(!env || (strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0) )
		{
			printf("Streaming disabled \n");
			temp &= ~BIT29;
		}
		else
		{
			printf("Streaming enabled \n");
			temp |= BIT29;
		}
		
		__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
	}


	if (mvCtrlModelGet() == MV_5281_DEV_ID) /* Orion 2 */
	{
		
		env = getenv("mainlineLinux");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		{
		    if (mvCtrlRevGet() == MV_5281_D2_REV) /* Orion 2 D2 */
		    {
			/* Change CPU ID to Marvell */	
			MV_REG_BIT_SET(CPU_FTDLL_CONFIG_REG, CFCR_FTDLL_OVERWRITE_ENABLE);
			MV_REG_BIT_SET(CPU_FTDLL_CONFIG_REG, CFCR_MRVL_CPU_ID);
			MV_REG_BIT_RESET(CPU_FTDLL_CONFIG_REG, CFCR_FTDLL_OVERWRITE_ENABLE);

			/* Change VFP ID to Marvell */	
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp |= BIT31;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		    }
		}

		if (mvCtrlRevGet() == MV_5281_A0_REV) /* Orion 2 A0 */
		{
#ifndef DB_FPGA
			env = getenv("enaVFP");
			if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
			{
				/* init VFP to Run Fast Mode */
				printf("VFP initialized to Run Fast Mode.\n");
					temp = fmrx(FPSCR);
					temp |= (FPSCR_DEFAULT_NAN | FPSCR_FLUSHTOZERO);
					fmxr(FPSCR, temp);
			}
			else
#endif /* DB_FPGA */
			{
				printf("VFP not initialized\n");
			}

		} 
		else if (mvCtrlRevGet() >= MV_5281_B0_REV)/* Orion 2 >= B0 */
		{
#ifndef DB_FPGA
			env = getenv("enaVFP");
			if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
			{
				/* Orion 2 D2 has VFP sub arch */
		 		if (mvCtrlRevGet() >= MV_5281_D2_REV)/* Orion 2 >= D2 */
				{
					env = getenv("vfpSubArch2");
					if(env && ((strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0)))
					{
					    MV_REG_BIT_SET(CPU_FTDLL_CONFIG_REG, CFCR_FTDLL_OVERWRITE_ENABLE);
					    MV_REG_BIT_SET(CPU_FTDLL_CONFIG_REG, CFCR_VFP_SUB_ARC_NUM_2);
					    MV_REG_BIT_RESET(CPU_FTDLL_CONFIG_REG, CFCR_FTDLL_OVERWRITE_ENABLE);
					}
				}

				/* init and Enable VFP to Run Fast Mode */
				printf("VFP initialized to Run Fast Mode.\n");
				/* Enable */
				temp = FPEXC_ENABLE;
				fmxr(FPEXC, temp);
				/* Run Fast Mode */
				temp = fmrx(FPSCR);
				temp |= (FPSCR_DEFAULT_NAN | FPSCR_FLUSHTOZERO);
				fmxr(FPSCR, temp);
			}
			else
#endif /* DB_FPGA */
			{
				printf("VFP not initialized\n");
				/* Disable */
				temp = fmrx(FPEXC);
				temp &= ~FPEXC_ENABLE;
				fmxr(FPEXC, temp);
			}
							
			/* DCache Pref  */
			env = getenv("enaDCPref");
			if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
			{
				MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
			}
			if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            		{
				MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
			}

			/* ICache Pref  */
			env = getenv("enaICPref");
            		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
			{
				MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);
			}
			if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            		{
				MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);

			}

		}

		/* write allocate */
		env = getenv("enaWrAllo");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp |= BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
		else
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp &= ~BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
	}

	if (mvCtrlModelGet() == MV_6183_DEV_ID ||
        mvCtrlModelGet() == MV_6183L_DEV_ID) /* MV88F6183 & MV88F6183L*/
	{
		/* DCache Pref  */
		env = getenv("enaDCPref");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		{
			MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
		}
		if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            	{
			MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
		}

		/* ICache Pref  */
		env = getenv("enaICPref");
            	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		{
			MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);
		}
		if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            	{
			MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);

		}

		/* write allocate */
		env = getenv("enaWrAllo");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp |= BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
		else
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp &= ~BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
	}

	if (mvCtrlModelGet() == MV_1281_DEV_ID) /* TC90 */
	{
		env = getenv("enaVFP");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
		{
			/* init and Enable VFP to Run Fast Mode */
			printf("VFP initialized to Run Fast Mode.\n");
			/* Enable */
			temp = FPEXC_ENABLE;
			fmxr(FPEXC, temp);

			/* Run Fast Mode */
			temp = fmrx(FPSCR);
			temp |= (FPSCR_DEFAULT_NAN | FPSCR_FLUSHTOZERO);

			fmxr(FPSCR, temp);
		}
		else
		{
			printf("VFP not initialized\n");
			/* Disable */
			temp = fmrx(FPEXC);
			temp &= ~FPEXC_ENABLE;
			fmxr(FPEXC, temp);
		}

		/* DCache Pref  */
		env = getenv("enaDCPref");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		{
			MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
		}
		if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            	{
			MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_DCACH_PREF_BUF_ENABLE);
		}

		/* ICache Pref  */
		env = getenv("enaICPref");
            	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		{
			MV_REG_BIT_SET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);
		}
		if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
            	{
			MV_REG_BIT_RESET( CPU_CONFIG_REG , CCR_ICACH_PREF_BUF_ENABLE);
		}

		/* write allocate */
		env = getenv("enaWrAllo");
		if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp |= BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
		else
		{
			__asm__ __volatile__("mrc    p15, 0, %0, c14, c0, 0" : "=r" (temp):: "memory");
			temp &= ~BIT28;
			__asm__ __volatile__("mcr    p15, 0, %0, c14, c0, 0" :: "r" (temp));
		}
	}
}

/* Set unit in power off mode acording to the env powerOffUnit */
/* Env powerOffUnit can has the following value: giga0, giga1, */
/* pex0, pex1, usb0, usb1, sata0, sata1, cesa, none	       */
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
static int mv_set_power_scheme(void)
{
	char *env;
	env = getenv("powerOffUnit");
	if(!env || (strcmp(env,"none") == 0))
	{
		setenv("powerOffUnit", "none");
		return MV_OK;
	}

	env = getenv("powerOffUnit");
#if defined(MV_INCLUDE_PEX)
	if (strcmp(env,"pex0") == 0)
	{
		printf("Warning: Pex0 is Powered Off\n");
		mvPexPowerDown(0);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 0, MV_FALSE);
	}
	else
	if (strcmp(env,"pex1") == 0)
	{
		printf("Warning: Pex1 is Powered Off\n");
		mvPexPowerDown(1);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 1, MV_FALSE);
	}
	else 
#endif /* #if defined(MV_INCLUDE_PEX) */
#if defined(MV_INCLUDE_INTEG_SATA)
	if (strcmp(env,"sata0") == 0)
	{
		printf("Warning: SATA0 is Powered Off\n");
		mvSataPhyShutdown(0);
		mvCtrlPwrClckSet(SATA_UNIT_ID, 0, MV_FALSE);
	}
	else 
	if (strcmp(env,"sata1") == 0)
	{
		printf("Warning: SATA1 is Powered Off\n");
		mvSataPhyShutdown(1);
		mvCtrlPwrClckSet(SATA_UNIT_ID, 1, MV_FALSE);
	}
	else 
#endif/*#if defined(MV_INCLUDE_SATA) */
#if defined(MV_INCLUDE_GIG_ETH)
	if (strcmp(env,"giga0") == 0)
	{
		printf("Warning: Giga0 and Giga1 is Powered Off\n");
		mvEthPortPowerDown(0);
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 0, MV_FALSE);
		mvEthPortPowerDown(1);
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 1, MV_FALSE);
	}
	else 
	if (strcmp(env,"giga1") == 0)
	{
		printf("Warning: Giga1 is Powered Off\n");
		mvEthPortPowerDown(1);
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 1, MV_FALSE);
	}
	else
#endif /* #if defined(MV_INCLUDE_GIG_ETH) */
#if defined(MV_INCLUDE_CESA)
	if (strcmp(env,"cesa") == 0)
	{
		printf("Warning: CESA is Powered Off\n");
		mvCtrlPwrClckSet(CESA_UNIT_ID, 1, MV_FALSE);
	}
	else
#endif /* if defined(MV_INCLUDE_CESA) */
#if defined(MV_INCLUDE_USB)
	if (strcmp(env,"usb0") == 0)
	{
		printf("Warning: USB0 is Powered Off\n");
		mvUsbPowerDown(0);
		mvCtrlPwrClckSet(USB_UNIT_ID, 0, MV_FALSE);
	}
	else 
	if (strcmp(env,"usb1") == 0)
	{
		printf("Warning: USB1 is Powered Off\n");
		mvUsbPowerDown(1);
		mvCtrlPwrClckSet(USB_UNIT_ID, 1, MV_FALSE);
	}
#endif/*#if defined(MV_INCLUDE_USB)*/

	return MV_OK;
}

#endif /* defined(MV_INCLUDE_CLK_PWR_CNTRL) */

/*******************************************************************************
* mvUpdateNorFlashBaseAddrBank - 
*
* DESCRIPTION:
*       This function update the CFI driver base address bank with on board NOR
*       devices base address.
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*       None
*
*******************************************************************************/
#ifdef	CFG_FLASH_CFI_DRIVER
MV_VOID mvUpdateNorFlashBaseAddrBank(MV_VOID)
{
    
    MV_U32 devBaseAddr;
    MV_U32 devNum = 0;
    int i;

    /* Update NOR flash base address bank for CFI flash init driver */
    for (i = 0 ; i < CFG_MAX_FLASH_BANKS_DETECT; i++)
    {
	devBaseAddr = mvBoardGetDeviceBaseAddr(i,BOARD_DEV_NOR_FLASH);
	if (devBaseAddr != 0xFFFFFFFF)
	{
	    flash_add_base_addr (devNum, devBaseAddr);
	    devNum++;
	}
    }
    mv_board_num_flash_banks = devNum;

    /* Update SPI flash count for CFI flash init driver */
    /* Assumption only 1 SPI flash on board */
    for (i = 0 ; i < CFG_MAX_FLASH_BANKS_DETECT; i++)
    {
    	devBaseAddr = mvBoardGetDeviceBaseAddr(i,BOARD_DEV_SPI_FLASH);
    	if (devBaseAddr != 0xFFFFFFFF)
		mv_board_num_flash_banks += 1;
    }

    if (mvCtrlModelGet() == MV_6082_DEV_ID)
	/* Add internal MFlash to NOR devices */
	mv_board_num_flash_banks += 1;

}
#endif	/* CFG_FLASH_CFI_DRIVER */

/*******************************************************************************
* mvStatusDebugLeds - 
*
* DESCRIPTION:
*       This function update the 7 segment or the debug/status LED
*       
*
* INPUT:
*	num - number to display over the 7 segment
* OUTPUT:
*
* RETURN:
*       None
*
*******************************************************************************/
static void mvStatusDebugLeds(char num)
{
	char* env;

	env = getenv("enaDebugLed");
	if(!env || ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
	{
		mvBoardStatusLed(MV_TRUE);
	}
	else
		mvBoardDebug7Seg(num);
}

/*******************************************************************************
* mvHddPowerCtrl - 
*
* DESCRIPTION:
*       This function set HDD power on/off acording to env or wait for button push
* INPUT:
*	None
* OUTPUT:
*	None
* RETURN:
*       None
*
*******************************************************************************/
#if defined(RD_88F6082NAS) || defined(RD_88F6082GE_SATA) || defined(RD_88F6082DAS_PLUS) || defined(RD_88F5182_3)
static void mvHddPowerCtrl(void)
{

    	MV_32 hddPowerBit = mvBoarGpioPinNumGet(BOARD_DEV_HDD0_POWER);
    	MV_32 extHddPowerBit = mvBoarGpioPinNumGet(BOARD_DEV_HDD1_POWER);
    	MV_32 hddBottenBit = mvBoarGpioPinNumGet(BOARD_DEV_POWER_BUTTON);
	char* env;


	if ((RD_88F6082_NAS == mvBoardIdGet()) ||
		(RD_88F5182_2XSATA3 == mvBoardIdGet()))
	{
		env = getenv("hddPowerCtrl");
 		if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
                	setenv("hddPowerCtrl","no");
        	else
                	setenv("hddPowerCtrl","yes");

		/* #include "mvGpp.h" */
		/* GPIO / Leds / and Power button handling */
			
		env = getenv("hddPowerCtrl");
 		if(env && ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
		{
			/* Set HDD input indication pin as input */
			MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0),(1<<hddBottenBit));
			/* Every value is Zero */
			MV_REG_WRITE(GPP_DATA_OUT_REG(0),0);
	
			/* Clear Interrupt cause */
			MV_REG_WRITE(GPP_INT_CAUSE_REG(0),0);
	
			/* Enable interrupt as edge */
			MV_REG_BIT_SET(GPP_INT_MASK_REG(0),(1 << hddBottenBit) );
	
			printf("\nPlease Press HDD power button to continue ... \n");
			do
			{
				mvOsDelay(200);
				if ( MV_REG_READ(GPP_INT_CAUSE_REG(0)) & (1 << hddBottenBit)) break;
			}while (1);
			


			/* Clear Interrupt cause */
			MV_REG_BIT_RESET(GPP_INT_CAUSE_REG(0),(1 << hddBottenBit));

			/* Disable interrupt */
			MV_REG_BIT_RESET(GPP_INT_MASK_REG(0),(1 << hddBottenBit) );
		

			/* HDD power on */
			MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<hddPowerBit));
			MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<hddPowerBit));
	
			/* extra HDD power on */
			if (extHddPowerBit != MV_ERROR)
			{
				/* Insert delay between HDD0 power on and HDD1 power on to prevent power noise */
				udelay(1000000);
				/* HDD power on */
				MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<extHddPowerBit));
				MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<extHddPowerBit));
			}
		}
		else
		{
			/* HDD power on */
			MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<hddPowerBit));
			MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<hddPowerBit));
			/* extra HDD power on */
			if (extHddPowerBit != MV_ERROR)
			{
				/* Insert delay between HDD0 power on and HDD1 power on to prevent power noise */
				udelay(1000000);
				/* HDD power on */
				MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<extHddPowerBit));
				MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<extHddPowerBit));
			}
		}

		if (RD_88F6082_NAS == mvBoardIdGet()) 
		{
			/* Set power indication pin as output */
			MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<hddBottenBit));
			/* Ligth power LED */
			MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<hddBottenBit));

			/* First init of PHY devices in RD-88F6082-NAS */
			mvEthE1112PhyBasicInit(0);
			mvEthE1112PhyBasicInit(1);
		}
	}
}
#endif

/*******************************************************************************
* bootDetection - 
*
* DESCRIPTION:
*       This function detect power up boot or soft reset reboot
*       
*
* INPUT:
*	None
* OUTPUT:
*	None
* RETURN:
*       None
*
*******************************************************************************/
#if defined(RD_88F6082MICRO_DAS_NAS)
static void bootDetection(void)
{
    	MV_32 powerBottenBit = mvBoarGpioPinNumGet(BOARD_DEV_POWER_BUTTON);
    	MV_32 restorBottenBit = mvBoarGpioPinNumGet(BOARD_DEV_RESTOR_BUTTON);
    	MV_32 hddPowerBit = mvBoarGpioPinNumGet(BOARD_DEV_HDD0_POWER);
    	MV_32 powerLedBit = mvBoarGpioPinNumGet(BOARD_DEV_POWER_ON_LED);
    	MV_32 powerFanBit = mvBoarGpioPinNumGet(BOARD_DEV_FAN_POWER);
	MV_32 waitCount = 0;
	char buff[256];
	char* env;

	MV_U32* pMesHead = (MV_U32*)0x400000;
	MV_U32* pMesBody = (MV_U32*)0x400004;

	MV_U32 mesHead = *(pMesHead);
	MV_U32 mesBody = *(pMesBody);

	/* Clear old message */
	*(pMesHead) = 0x0;
	*(pMesBody) = 0x0;

	/* Check for reboot option */
	if (0xdeadbeaf == mesHead)
	{
		if (0x0b0b0b0b == mesBody)
		{
			/* We are in stanby mode */
			/* Call system power down */
			mv6082uDasNasSysPower(MV_FALSE);
			
			/* Set power input indication pin as input */
			MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0),((1<<powerBottenBit)|(1<<restorBottenBit)));
	
			/* Clear out reg */
			MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),0);

			/* Clear Interrupt cause */
			MV_REG_WRITE(GPP_INT_CAUSE_REG(0),0);
	
			/* Enable interrupt as edge */
			MV_REG_BIT_SET(GPP_INT_MASK_REG(0),(1 << powerBottenBit) );
	
			printf("\nPlease Press power button to continue ... \n");
			do
			{	
				/* Check power on button */
				if ( MV_REG_READ(GPP_INT_CAUSE_REG(0)) & (1 << powerBottenBit)) break;

				/* Check if reset button is press for more then 5 Sec */
				if (!( MV_REG_READ(GPP_DATA_IN_REG(0)) & (1 << restorBottenBit)))
				{
					waitCount++;
					mvOsDelay(1000);
				}
				else
					waitCount = 0;

				if (waitCount > 5) break;

			}while (1);


			/* Clear Interrupt cause */
			MV_REG_BIT_RESET(GPP_INT_CAUSE_REG(0),(1 << powerBottenBit));

			/* Disable interrupt */
			MV_REG_BIT_RESET(GPP_INT_MASK_REG(0),(1 << powerBottenBit) );
		

			/* Call system power up */
			mv6082uDasNasSysPower(MV_TRUE);
		}
	}
	else
	{
	/* U-boot is running after power-up */
		/* Check restor botten push */
		/* Set restor input indication pin as input */
		MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(0),(1<<restorBottenBit));
		mvOsDelay(200);	
		if (!( MV_REG_READ(GPP_DATA_IN_REG(0)) & (1 << restorBottenBit)))
		{
		/* Restor botten is push on power up, send Linux the message of reset_defualt */
                	printf("Enter reset default command\n");
			env = getenv("bootargs");
        		if(env)
			{
				sprintf(buff,env);
				strcat(buff, " reset_defualt");
				setenv("bootargs",buff);
			}
		}
	}
	
	/* Power up HDD, Fan and LED's */
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<powerLedBit));
	MV_REG_BIT_SET(GPP_BLINK_EN_REG(0),(1<<powerLedBit));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<powerLedBit));
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<hddPowerBit));
	MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<hddPowerBit));

        env = getenv("fanPowerCtrl");
        if(!env|| ( (strcmp(env,"Yes") == 0) || (strcmp(env,"yes") == 0) ) )
	{
	    MV_REG_BIT_SET(GPP_DATA_OUT_REG(0),(1<<powerFanBit));
	    MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0),(1<<powerFanBit));
	}

	    
}

/*******************************************************************************
* mv6082uDasNasSysPower - 
*
* DESCRIPTION:
*       This function power up/down the device units
*       
*
* INPUT:
*	enable -
	MV_FALSE - power down
	MV_TRUE - power up
* OUTPUT:
*	None
* RETURN:
*       None
*
*******************************************************************************/
static MV_VOID mv6082uDasNasSysPower(MV_BOOL enable)
{

	if (enable == MV_FALSE)
	{
		mvEthE1112PhyPowerDown(0); 				/* power doen the PHY */
		mvEthPortPowerDown(1);					/* set the port in power down mode */
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 1, MV_FALSE);		/* disable the unit clock */
		mvEthPortPowerDown(0);					/* set the port in power down mode */
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 0, MV_FALSE);		/* disable the unit clock */
			
		mvUsbPowerDown(0);
		mvCtrlPwrClckSet(USB_UNIT_ID, 0, MV_FALSE);
	}
	else
	{

		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 0, MV_TRUE);
		mvEthPortPowerUp(0);						
		mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 1, MV_TRUE);
		mvEthPortPowerUp(1);						
		mvEthE1112PhyPowerUp(0); 				/* power up the PHY */

		mvCtrlPwrClckSet(USB_UNIT_ID, 0, MV_TRUE);
		mvUsbPowerUp(0);
	}
}
#endif
