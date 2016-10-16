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
#include "mvCtrlEnvLib.h"
#include <common.h>
#include "mvTypes.h"
#include "mvBoardEnvLib.h"
#include "mvCpuIf.h"
#include "mvCtrlEnvLib.h"
#include "mv_mon_init.h"
#include "mvDebug.h"
#include "device/mvDevice.h"
#include "twsi/mvTwsi.h"
#include "eth/mvEth.h"
#include "pex/mvPex.h"
#include "eth-phy/mvEthPhy.h"
#include "ethSwitch/mvSwitch.h"
#include "gpp/mvGpp.h"
#include "sys/mvSysUsb.h"

#ifdef MV_INCLUDE_RTC
#include "rtc/integ_rtc/mvRtc.h"
#elif defined CONFIG_RTC_DS1338_DS1339
#include "rtc/ext_rtc/mvDS133x.h"
#endif

#if defined(MV_INCLUDE_XOR)
#include "xor/mvXor.h"
extern MV_STATUS mvXorInit (MV_VOID);
#endif
#if defined(MV_INCLUDE_IDMA)
#include "sys/mvSysIdma.h"
#include "idma/mvIdma.h"
extern MV_STATUS mvDmaInit (MV_VOID);
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

#include "net.h"
#include <command.h>

/* #define MV_DEBUG */
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#if defined(DUAL_OS_78200) && defined(MV_NAND_BOOT)
#error "Dual OS boot from NAND is not supported: use LNX_SHARED=1 configuration"
#endif

/* CPU address decode table. */
MV_CPU_DEC_WIN mvCpuAddrWinMap[] = MV_CPU_IF_ADDR_WIN_MAP_TBL;
#if defined(MV78XX0) && defined(MV_INCLUDE_MONT_EXT)
MV_CPU_DEC_WIN mvCpuAddrWinMonExtMap[] = MV_CPU_IF_ADDR_WIN_MON_EXT_MAP_TBL;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_RCVR)
static void recoveryDetection(void);
void recoveryHandle(void);
static u32 rcvrflag = 0;
#endif
void mv_cpu_init(void);
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
int mv_set_power_scheme(void);
#endif

#ifdef	CFG_FLASH_CFI_DRIVER
MV_VOID mvUpdateNorFlashBaseAddrBank(MV_VOID);
int mv_board_num_flash_banks;
extern flash_info_t	flash_info[]; /* info for FLASH chips */
extern unsigned long flash_add_base_addr (uint flash_index, ulong flash_base_addr);
#endif	/* CFG_FLASH_CFI_DRIVER */

#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
extern MV_VOID mvBoardEgigaPhySwitchInit(void);
#endif 

extern unsigned int whoAmI(void);

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
/* Define for SDK 2.0 */
int __aeabi_unwind_cpp_pr0(int a,int b,int c) {return 0;}
int __aeabi_unwind_cpp_pr1(int a,int b,int c) {return 0;}
#endif

#if 0
MV_VOID mvMppModuleTypePrint(MV_VOID);
#endif

/* Define for SDK 2.0 */
int raise(void) {return 0;}

void print_mvBanner(void)
{
#ifdef CONFIG_SILENT_CONSOLE
	DECLARE_GLOBAL_DATA_PTR;
	gd->flags |= GD_FLG_SILENT;
#endif
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
#if !defined(MV_NAND_BOOT)
#if defined(MV_INCLUDE_MONT_EXT)
	if(!enaMonExt())
		printf(" ** LOADER **"); 
	else
		printf(" ** MONITOR **");
#else

	printf(" ** Forcing LOADER mode only **"); 
#endif /* MV_INCLUDE_MONT_EXT */
#endif
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


void maskAllInt(void)
{
        /* mask all external interrupt sources */
        MV_REG_WRITE(CPU_AHB_MBUS_MASK_INT_REG(whoAmI()), 0);
        MV_REG_WRITE(CPU_INT_MASK_ERROR_REG(whoAmI()), 0);
        MV_REG_WRITE(CPU_INT_MASK_LOW_REG(whoAmI()), 0);
        MV_REG_WRITE(CPU_INT_MASK_HIGH_REG(whoAmI()), 0);
}

/* init for the Master*/
void misc_init_r_dec_win(void)
{
#if defined(MV_INCLUDE_USB)
	{
		char *env;

      	env = getenv("usb0Mode");
      	if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
      	{
     		setenv("usb0Mode","host");
      		printf("USB 0: host mode\n");
      		mvUsbInit(0, MV_TRUE);
       	}
      	else
     	{
		printf("USB 0: device mode\n");
      		mvUsbInit(0, MV_FALSE);
    	}

      	env = getenv("usb1Mode");
      	if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
      	{
     		setenv("usb1Mode","host");
      		printf("USB 1: host mode\n");
      		mvUsbInit(1, MV_TRUE);
       	}
      	else
     	{
		printf("USB 1: device mode\n");
      		mvUsbInit(1, MV_FALSE);
    	}

      	env = getenv("usb2Mode");
      	if((!env) || (strcmp(env,"device") == 0) || (strcmp(env,"Device") == 0) )
      	{
     		setenv("usb2Mode","device");
      		printf("USB 2: device mode\n");
      		mvUsbInit(2, MV_FALSE);
       	}
      	else
     	{
     		printf("USB 2: host mode\n");
      		mvUsbInit(2, MV_TRUE);
    	}


	}
#endif/* #if defined(MV_INCLUDE_USB) */

#if defined(MV_INCLUDE_XOR)
	mvXorInit();
#endif
#if defined(MV_INCLUDE_IDMA)
	mvDmaInit();
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

int cpuMapInit (void)
{
#ifndef MV_TINY_IMAGE
	/* For Mon ext mode only we reset the PEX mem base from 0xc0000000 to 
 		0x90000000 in order to enable working with CIV page table */
#if defined(MV78XX0)
	if (enaMonExt())
		mvCpuIfInit(mvCpuAddrWinMonExtMap); 
#endif
#endif
	return 0;
}


int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
#if defined(MV_INCLUDE_TWSI)
	MV_TWSI_ADDR slave;
#endif
	unsigned int i;

	maskAllInt();

    if (whoAmI() == MASTER_CPU)
    {
	/* must initialize the int in order for udelay to work */
	interrupt_init();

#if defined(MV_INCLUDE_TWSI)
	slave.type = ADDR7_BIT;
	slave.address = 0;
	for (i = 0; i < MV_TWSI_MAX_CHAN; i++)
		mvTwsiInit(i, CFG_I2C_SPEED, CFG_TCLK, &slave, 0);
#endif
 
	/* Init the Board environment module (device bank params init) */
	mvBoardEnvInit();
    }
   	
	/* Init the Controlloer environment module (MPP init) */
	mvCtrlEnvInit();

    if (whoAmI() == MASTER_CPU)
	mvBoardDebug7Seg(3);

        /* Init the Controller CPU interface */
	mvCpuIfInit(mvCpuAddrWinMap);
#ifdef RD_MV78XX0_AMC
	/* Set NAND on BootCS */
	MV_AHB_TO_MBUS_DEC_WIN addrDecWin;
	addrDecWin.target = DEV_BOOCS;
	addrDecWin.addrWin.baseLow = DEVICE_CS2_BASE;
	addrDecWin.addrWin.baseHigh = 0;
	addrDecWin.addrWin.size = _1M;
	addrDecWin.enable = MV_TRUE;
	mvAhbToMbusWinSet(0, 12, &addrDecWin);
#endif

	/* relocate the exception vectors */
	/* U-Boot is running from DRAM at this stage */
	for(i = 0; i < 0x100; i+=4)
	{
		*(unsigned int *)(0x0 + i) = *(unsigned int*)(TEXT_BASE + i);
	}
	
#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
	/* Init the PHY or Switch of the board */
    if (whoAmI() == MASTER_CPU)
	mvBoardEgigaPhySwitchInit();
#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */

    if (whoAmI() == MASTER_CPU)
	mvBoardDebug7Seg(4);
	
    /* arch number of Integrator Board */
    gd->bd->bi_arch_number = 528;
 
    /* adress of boot parameters */
    gd->bd->bi_boot_params = 0x00000100;

    /* Update NOR flash base address bank for CFI driver */
#ifdef	CFG_FLASH_CFI_DRIVER
    mvUpdateNorFlashBaseAddrBank();
#endif	/* CFG_FLASH_CFI_DRIVER */

	return 0;
}

void misc_init_r_env(void){
        char *env;
        char tmp_buf[10];
        unsigned int malloc_len;
        DECLARE_GLOBAL_DATA_PTR;
	
        unsigned int flashSize =0 , secSize =0, ubootSize =0;
	char buff[256];

#if defined(MV_BOOTSIZE_1M)
	flashSize = _1M;
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
	/* Dual CPU Firmware load address */
        env = getenv("fw_image_base");
        if(!env)
        	setenv("fw_image_base", "0x0");

	/* Dual CPU Firmware size */
        env = getenv("fw_image_size");
        if(!env)
        	setenv("fw_image_size", "0x0");
#ifndef MV632X
#ifndef MV_STATIC_DRAM_ON_BOARD 
#ifdef CONFIG_MV_ECC
        /* update the enaECC env parameter */
        env = getenv("enaECC");
        if(env && ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
                setenv("enaECC","yes");
        else
#endif
                setenv("enaECC","no");
#endif
#endif
	if ((0 == flashSize) || (0 == secSize) || (0 == ubootSize))
	{
		env = getenv("console");
		if(!env)
			setenv("console","console=ttyS0,115200");
	}
	else
	{
		sprintf(buff,"console=ttyS0,115200 mtdparts=cfi_flash:0x%x(root),0x%x(uboot)ro",
			flashSize - ubootSize, ubootSize);
		env = getenv("console");
		if(!env)
			setenv("console",buff);
	}

		
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
		int board_id = mvBoardIdGet();	
		switch(board_id){
		    case(DB_78XX0_ID):
			sprintf(tmp_buf,"%d", DB_78XX0_BP_MLL_ID);
			board_id = DB_78XX0_BP_MLL_ID; 
			/* Set CPU ID to MRVL */
			MV_REG_BIT_SET(CPU_CONFIG_REG(whoAmI()),CCR_MRVL_CPU_ID);
			
			break;
		    default:
			sprintf(tmp_buf,"%d", board_id);
			board_id = board_id; 
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

#if defined (MV_INC_BOARD_NOR_FLASH)
        env = getenv("enaFlashBuf");
        if( ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
                setenv("enaFlashBuf","no");
        else
                setenv("enaFlashBuf","yes");
#endif

	/* CPU streaming */
        env = getenv("enaCpuStream");
        if(!env || ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
                setenv("enaCpuStream","yes");
        else
                setenv("enaCpuStream","no");

	/* Write allocation */
	env = getenv("enaWrAllo");
	if( !env || ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
		setenv("enaWrAllo","no");
	else
		setenv("enaWrAllo","yes");

	/* FPU */
	env = getenv("enaFPU");
	if( !env || ( ((strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) )))
		setenv("enaFPU","yes");
	else
		setenv("enaFPU","no");

	/* Pex mode */
	env = getenv("pexMode");
	if( env && ( ((strcmp(env,"EP") == 0) || (strcmp(env,"ep") == 0) )))
		setenv("pexMode","EP");
	else
		setenv("pexMode","RC");

    	env = getenv("disL2Cache");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
        	setenv("disL2Cache","no"); 
    	else
        	setenv("disL2Cache","yes");

    	env = getenv("setL2CacheWT");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
        	setenv("setL2CacheWT","no"); 
    	else
        	setenv("setL2CacheWT","yes");

    	env = getenv("disL2Prefetch");
    	if(!env || ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
        	setenv("disL2Prefetch","yes"); 
    	else
        	setenv("disL2Prefetch","no");
#if !defined(MV76100) && !defined(MV632X)
	env = getenv("setL2Size256K");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
        	setenv("setL2Size256K","no"); 
    	else
        	setenv("setL2Size256K","yes");
#else
        	setenv("setL2Size256K","yes");
#endif

    	env = getenv("disL2Ecc");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
        	setenv("disL2Ecc","no"); 
    	else
        	setenv("disL2Ecc","yes");


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
                setenv("MALLOC_len",tmp_buf);
	}
         
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

#if defined(MV_INC_BOARD_QD_SWITCH)
	    env = getenv("netbsd_netconfig");
	    if(!env)
	    setenv("netbsd_netconfig","mv_net_config=<((mgi0,00:00:11:22:33:44,0)(mgi1,00:00:11:22:33:55,1:2:3:4)),mtu=1500>");
#endif
	    env = getenv("netbsd_set_args");
	    if(!env)
	    	setenv("netbsd_set_args","setenv bootargs nfsroot=$(netbsd_server):$(rootpath) fs=$(netbsd_fs) \
ip=$(netbsd_ip) serverip=$(netbsd_server) mask=$(netbsd_mask) gw=$(netbsd_gw) rootdev=$(netbsd_rootdev) \
ethaddr=$(ethaddr) eth1addr=$(eth1addr) ethmtu=$(ethmtu) eth1mtu=$(eth1mtu) $(netbsd_netconfig)");

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
#if defined(DUAL_OS_SHARED_MEM_78200)
			setenv("bootargs_root","mem=120M root=/dev/nfs rw");
#else
            setenv("bootargs_root","root=/dev/nfs rw");
#endif
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
#if defined(MV_INC_BOARD_QD_SWITCH)
                setenv("bootargs_end",CFG_BOOTARGS_END_SWITCH);
#else
                setenv("bootargs_end",CFG_BOOTARGS_END);
#endif
	}
	
        env = getenv("image_name");
        if(!env)
                setenv("image_name","uImage");
 

#if (CONFIG_BOOTDELAY >= 0)
        env = getenv("bootcmd");
        if(!env)
#if defined(MV_INCLUDE_TDM) && defined(MV_INC_BOARD_QD_SWITCH)
	    setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(console) $(bootargs_root) nfsroot=$(serverip):$(rootpath) cpu0=$(cpu0_res) cpu1=$(cpu1_res) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig) $(mvPhoneConfig);  bootm 0x400000; ");
#elif defined(MV_INC_BOARD_QD_SWITCH)
            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(console) $(bootargs_root) nfsroot=$(serverip):$(rootpath) cpu0=$(cpu0_res) cpu1=$(cpu1_res) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig);  bootm 0x400000; ");
#elif defined(MV_INCLUDE_TDM)
            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(console) $(bootargs_root) nfsroot=$(serverip):$(rootpath) cpu0=$(cpu0_res) cpu1=$(cpu1_res) \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvPhoneConfig);  bootm 0x400000; ");
#else
            setenv("bootcmd","tftpboot 0x400000 $(image_name);\
setenv bootargs $(console) $(bootargs_root) nfsroot=$(serverip):$(rootpath) cpu0=$(cpu0_res) cpu1=$(cpu1_res) \
ip=$(ipaddr):$(serverip)$(bootargs_end);  bootm 0x400000; ");
#endif
#endif /* (CONFIG_BOOTDELAY >= 0) */

        env = getenv("standalone");
        if(!env)
#if defined(DUAL_OS_SHARED_MEM_78200)
			setenv("standalone","lockSem 1;fsload 0x2000000 $(image_name);setenv bootargs $(console) root=/dev/mtdblock0 rw \
	ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig) $(mvPhoneConfig); bootm 0x2000000;");
#else
#if defined(MV_INCLUDE_TDM) && defined(MV_INC_BOARD_QD_SWITCH)
	    setenv("standalone","fsload 0x2000000 $(image_name);setenv bootargs $(console) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig) $(mvPhoneConfig); bootm 0x2000000;");
#elif defined(MV_INC_BOARD_QD_SWITCH)
            setenv("standalone","fsload 0x2000000 $(image_name);setenv bootargs $(console) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvNetConfig); bootm 0x2000000;");
#elif defined(MV_INCLUDE_TDM)
            setenv("standalone","fsload 0x2000000 $(image_name);setenv bootargs $(console) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end) $(mvPhoneConfig); bootm 0x2000000;");
#else
            setenv("standalone","fsload 0x2000000 $(image_name);setenv bootargs $(console) root=/dev/mtdblock0 rw \
ip=$(ipaddr):$(serverip)$(bootargs_end); bootm 0x2000000;");
#endif
#endif                 
       /* Set boodelay to 3 sec, if Monitor extension are disabled */
        if(!enaMonExt()){
                setenv("bootdelay","3");
		setenv("disaMvPnp","no");
	}

	/* Disable PNP config of Marvel memory controller devices. */
        env = getenv("disaMvPnp");
        if(!env)
                setenv("disaMvPnp","no");

#if (defined(MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH))
#include "cesa/mvMD5.h"
	/* Generate random ip and mac address */
	/* Read DRAM FTDLL register to create random data for enc */
	unsigned int xi, xj, xk, xl, i;
	char ethaddr[4][30];

	char temp[20];
	MV_U32 random[16];
	unsigned char digest[16];

	MV_REG_BIT_SET(0x1478, BIT7);
	for(i=0; i < 16;i++)
		random[i] = MV_REG_READ(0x1470);
	
	/* Run MD5 over the ftdll buffer */
#ifndef MV_TINY_IMAGE
	mvMD5((unsigned char*)random, 64, digest);
#endif

	xi = (digest[0]%254);
	/* No valid ip with one of the fileds has the value 0 */
	if (xi == 0)
		xi+=2;

	xj = (digest[1]%254);
	/* No valid ip with one of the fileds has the value 0 */
	if (xj == 0)
		xj+=2;

	/* Check if the ip address is the same as the server ip */
	if ((xj == 1) && (xi == 11))
		xi+=2;

	xk = digest[2];
	xl = digest[3];

	sprintf(ethaddr[0],"00:50:43:%02x:%02x:%02x",xk,xi,xj);
	sprintf(ethaddr[1],"00:50:43:%02x:%02x:%02x",xl,xi,xj);
	sprintf(ethaddr[2],"00:50:43:%02x:%02x:%02x",xk,xj,xi);
	sprintf(ethaddr[3],"00:50:43:%02x:%02x:%02x",xl,xj,xi);

	/* MAC addresses */
	for (i = 0 ;i < mvCtrlEthMaxPortGet();i++)
	{
	    sprintf( temp,(i ? "eth%daddr" : "ethaddr"), i);
	    env = getenv(temp);
	    if(!env)
                setenv(temp,ethaddr[i]);

	    sprintf( temp,(i ? "eth%dmtu" : "ethmtu"), i);
	    env = getenv(temp);
	    if(!env)
                setenv(temp,"1500");
	}
#if defined(MV_INCLUDE_TDM)
        /* Set mvPhoneConfig env parameter */
        env = getenv("mvPhoneConfig");
        if(!env )
            setenv("mvPhoneConfig","mv_phone_config=dev0:fxs,dev1:fxo");
#endif
#if defined(MV_INC_BOARD_QD_SWITCH)
        /* Set mvNetConfig env parameter */
        env = getenv("mvNetConfig");
        if(!env )
		    setenv("mvNetConfig","mv_net_config=(00:aa:bb:cc:dd:ee,0)(00:11:22:33:44:55,1:2:3:4),mtu=1500");
#endif
#endif /*  (MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH) */

#if defined(MV_INCLUDE_USB)
	/* USB Host */
	env = getenv("usb0Mode");
	if(!env)
		setenv("usb0Mode",ENV_USB0_MODE);

	env = getenv("usb1Mode");
	if(!env)
		setenv("usb1Mode",ENV_USB1_MODE);
	
	env = getenv("usb2Mode");
	if(!env)
		setenv("usb2Mode",ENV_USB2_MODE);
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

#if defined(MV_NAND)
	env = getenv("nandEcc");
	if(!env)
    {
        setenv("nandEcc", "1bit");
    }
#endif

#if (CONFIG_COMMANDS & CFG_CMD_RCVR)
	env = getenv("netretry");
	if (!env)
		setenv("netretry","no");

	env = getenv("rcvrip");
	if (!env)
		setenv("rcvrip",RCVR_IP_ADDR);

	env = getenv("loadaddr");
	if (!env)
		setenv("loadaddr",RCVR_LOAD_ADDR);

	env = getenv("autoload");
	if (!env)
		setenv("autoload","no");

#if (CONFIG_COMMANDS & CFG_CMD_RCVR)
	/* Check the recovery trigger */
	recoveryDetection();
#endif
#endif
	int vxWorksFlags = 0;

#if defined(MV78200) && (defined(DUAL_OS_78200) || defined(DUAL_OS_SHARED_MEM_78200))
	char ethnum[20];
	
        /* U-boot none shared device policy */
	/*
	env = getenv("cpu0_res");
	if (env)
	    mvSocUnitMapFillTable(env, MASTER_CPU, strstr);
	env = getenv("cpu1_res");
	if (env)
	    mvSocUnitMapFillTable(env, SLAVE_CPU, strstr);
	*/
    vxWorksFlags = mvSocUnitMapFillFlagFormTable();

    /* CPU resource arguments */
	env = getenv("cpu0_res");
	if (!env)
#if defined(MV_SPI_BOOT)
		setenv("cpu0_res","egiga0,egiga1,pcie0,pcie1,sata,nor,nand,usb0,usb1,usb2,xor");
#elif defined(MV_NAND_BOOT)
		setenv("cpu0_res","egiga0,egiga1,pcie0,pcie1,sata,nor,spi,usb0,usb1,usb2,xor");
#else
		setenv("cpu0_res","egiga0,egiga1,pcie0,pcie1,sata,nand,spi,usb0,usb1,usb2,xor");
#endif

	env = getenv("cpu1_res");
	if (!env)
#ifdef MV6323
		setenv("cpu1_res","egiga2");
#else
        setenv("cpu1_res","egiga2,egiga3");
#endif

        /* Second linux boot arguments */
        env = getenv("ipaddr2");
        if(!env)
                setenv("ipaddr2","10.4.50.166");

        env = getenv("image_name2");
        if(!env)
                setenv("image_name2","uImage");

        env = getenv("console2");
        if(!env)
                setenv("console2","console=ttyS0,115200");

        env = getenv("rootpath2");
        if(!env)
                setenv("rootpath2","/mnt/ARM_FS/");

        env = getenv("bootargs_end2");
        if(!env)
                setenv("bootargs_end2",":::DB78xx0:eth0:none");

        env = getenv("bootcmd2");
        if(!env)
#if defined(DUAL_OS_SHARED_MEM_78200)
            setenv("bootcmd2","lockSem 1;tftpboot 0xA000000 $(image_name2); setenv bootargs2 $(console2) \
$(bootargs_root2) nfsroot=$(serverip):$(rootpath2) cpu0=$(cpu0_res) cpu1=$(cpu1_res) ip=$(ipaddr2):$(serverip)$(bootargs_end2); \
bootm 0xA000000; ");
#else
		setenv("bootcmd2","tftpboot 0x2000000 $(image_name2); setenv bootargs2 $(console2) \
$(bootargs_root2) nfsroot=$(serverip):$(rootpath2) cpu0=$(cpu0_res) cpu1=$(cpu1_res) ip=$(ipaddr2):$(serverip)$(bootargs_end2); \
bootm 0x2000000; ");
#endif
        env = getenv("bootargs_root2");
        if(!env)
#if defined(DUAL_OS_SHARED_MEM_78200)
			setenv("bootargs_root2","mem=128M root=/dev/nfs rw");
#else
			setenv("bootargs_root2","root=/dev/nfs rw");
#endif
        env = getenv("enaMP");
        if(!env)
                setenv("enaMP","no");
#endif


	/* vxWorks boot arguments */
        env = getenv("vxworks_en");
	if( !env || ( ((strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) )))
		setenv("vxworks_en","no");
	else
	{
	    char* buff = (char *)0x1100;
	    int usbFlag = 0;
	    char usbFlagSt[10];
	    setenv("vxworks_en","yes");
	
	    /* Getting image name for each CPU */	
	    sprintf(buff,"mgi(0,0) host:");
	    if (whoAmI())
		env = getenv("image_name2");
	    else
		env = getenv("image_name");
	    strcat(buff,env);

	    /* Getting ip address and server ip */
	    env = getenv("serverip");
	    strcat(buff, " h=");
	    strcat(buff,env);
	    
	    if (whoAmI())
		env = getenv("ipaddr2");
	    else
		env = getenv("ipaddr");

	    strcat(buff, " e=");
	    strcat(buff,env);

	    /* Getting USB mode for each port */
	    env = getenv("usb0Mode");
	    if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
	    {
		usbFlag |= 0x2000;
	    }

	    env = getenv("usb1Mode");
	    if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
	    {
		usbFlag |= 0x4000;
	    }

	    env = getenv("usb2Mode");
	    if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
	    {
		usbFlag |= 0x8000;
	    }

	    strcat(buff, ":ffff0000 u=anonymous pw=target f=");
	    sprintf(usbFlagSt,"0x%x",usbFlag);
	    strcat(buff, usbFlagSt);
	    setenv("vxWorks_bootargs",buff);
	    MV_REG_WRITE(GENERAL_USAGE_REGISTER_0, vxWorksFlags);
	}

        return;
}

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	/* Check if to use the LED's for debug or to use single led for init and Linux heartbeat */
	mvBoardDebug7Seg(0);
	return 0;
}
#endif

#if defined(MV78200)
extern MV_VOID mvDramIfSlaveCpuWinInit(MV_VOID);
extern MV_VOID mvDramIfCpuWinCopy(MV_VOID);
void second_cpu_realloc_and_load(void)
{
    char *env;

    if (whoAmI() == MASTER_CPU)
    {	
        /* default no MP */
        env = getenv("enaMP");
        if(env && ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
	{
		if (IS_CPU1_ENABLED)
		{
			printf("\nRelocate and enable second CPU\n");
			/* Copy CPU 0 address map to CPU 1 */
			mvAhbToMbusCpuWinCopy();			
#if defined(DUAL_OS_78200)
                	/* kick of the second CPU!! */
			if (mvDramIfBankSizeGet(2))
			{
			    /* Copy CPU 0 DRAM address map to CPU 1 */
			    mvDramIfSlaveCpuWinInit();
			    MV_REG_BIT_RESET(CPU_CTRL_STAT_REG(1), BIT3);
			}
			else
			    printf("\nFail to enabled second CPU - no Dram on CS 2!\n");
#else
			/* Copy CPU 0 DRAM address map to CPU 1 */
			mvDramIfCpuWinCopy();
#if defined(DUAL_OS_SHARED_MEM_78200)
			/* Copy CPU 0 DRAM address map to CPU 1 */			
			MV_REG_BIT_RESET(CPU_CONFIG_REG(1), BIT1);
			{
				volatile unsigned int *a = (0x0);	
				extern volatile unsigned int* _start;
				a[0] = 0xe3a00008; 	/*mov	r0, #8	; 0x4*/
				a[1] = 0xe590f000; 	/*ldr	pc, [r0]*/
				a[2] = ((unsigned int)&_start)+0x70000;
				__asm__ __volatile__ ("mcr p15, 0, r0, c7, c10, 4");
			}
			/* kick off the second CPU!! */
#endif
			MV_REG_BIT_RESET(CPU_CTRL_STAT_REG(1), BIT3);
#endif
		}
	}
	else
		setenv("enaMP", "no");
    }
}
#endif

int misc_init_r (void)
{
	char name[128];
	
    if (whoAmI() == MASTER_CPU)
    {
	mvBoardDebug7Seg(5);

    }
	/* init special env variables */
	misc_init_r_env();

	mvCpuNameGet(name);
	printf("\nCPU %d: %s\n", whoAmI(), name);

	mv_cpu_init();

#if defined(MV_INCLUDE_MONT_EXT)
	if(enaMonExt()){
		printf("\n      Marvell monitor extension:\n");
		mon_extension_after_relloc();
	}
    	printf("\n");		
#endif /* MV_INCLUDE_MONT_EXT */

    	printf("\n");		
	/* init the units decode windows */
    if (whoAmI() == MASTER_CPU)
    {
	misc_init_r_dec_win();

	mvBoardDebug7Seg(6);
    }
#ifdef CONFIG_PCI
#if !defined(MV_MEM_OVER_PCI_WA) && !defined(MV_MEM_OVER_PEX_WA)
#ifdef MV78200
    if ( ! ((whoAmI() == SLAVE_CPU) && enaMonExt() && enaMP()) )
#endif
       pci_init();
#endif
#endif
    if (whoAmI() == MASTER_CPU)
    {
	mvBoardDebug7Seg(7);
    }
	
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
 
/* exported for EEMBC */
MV_U32 mvGetRtcSec(void)
{
#if (CONFIG_COMMANDS & CFG_CMD_DATE)
        MV_RTC_TIME time;
#ifdef MV_INCLUDE_RTC
        mvRtcTimeGet(&time);
#elif defined CONFIG_RTC_DS1338_DS1339
        mvRtcDS133xTimeGet(&time);
#endif
	return (time.minutes * 60) + time.seconds;
#else
	return 0;
#endif
}

void reset_cpu(void)
{
	mvBoardReset();
}

void mv_cpu_init(void)
{
	char *env;
	volatile unsigned int temp;

    /* Invalidate and Unlock CPU L1 I-cache which was locked in jump.s */
    temp = 0;
    __asm__ __volatile__("mcr    p15, 1, %0, c7, c5, 0" : "=r" (temp));
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    __asm__ __volatile__("mcr    p15, 1, %0, c9, c0, 1" : "=r" (temp));
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");

	if (whoAmI())
		MV_REG_BIT_SET(CPU_CONFIG_REG(1), BIT1);

	/*CPU streaming & write allocate */
	env = getenv("enaWrAllo");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
	{
		__asm__ __volatile__("mrc    p15, 1, %0, c15, c1, 0" : "=r" (temp));
		temp |= BIT28;
		__asm__ __volatile__("mcr    p15, 1, %0, c15, c1, 0" :: "r" (temp));
		
	}
	else
	{
		__asm__ __volatile__("mrc    p15, 1, %0, c15, c1, 0" : "=r" (temp));
		temp &= ~BIT28;
		__asm__ __volatile__("mcr    p15, 1, %0, c15, c1, 0" :: "r" (temp));
	}

	env = getenv("enaCpuStream");
	if(!env || (strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0) )
	{
		__asm__ __volatile__("mrc    p15, 1, %0, c15, c1, 0" : "=r" (temp));
		temp &= ~BIT29;
		__asm__ __volatile__("mcr    p15, 1, %0, c15, c1, 0" :: "r" (temp));
	}
	else
	{
		__asm__ __volatile__("mrc    p15, 1, %0, c15, c1, 0" : "=r" (temp));
		temp |= BIT29;
		__asm__ __volatile__("mcr    p15, 1, %0, c15, c1, 0" :: "r" (temp));
	}
		
	/* Verify write allocate and streaming */
	printf("\n");
	__asm__ __volatile__("mrc    p15, 1, %0, c15, c1, 0" : "=r" (temp));
	if (temp & BIT29)
		printf("Streaming enabled \n");
	else
		printf("Streaming disabled \n");
	if (temp & BIT28)
		printf("Write allocate enabled\n");
	else
		printf("Write allocate disabled\n");

	env = getenv("enaFPU");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))  
	{
		/* init and Enable FPU to Run Fast Mode */
		printf("FPU initialized to Run Fast Mode.\n");
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
		printf("FPU not initialized\n");
		/* Disable */
		temp = fmrx(FPEXC);
		temp &= ~FPEXC_ENABLE;
		fmxr(FPEXC, temp);
	}

	/* DCache Pref  */
	env = getenv("enaDCPref");
	if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		MV_REG_BIT_SET( CPU_CONFIG_REG(whoAmI()) , CCR_DCACH_PREF_BUF_ENABLE);

	if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
		MV_REG_BIT_RESET( CPU_CONFIG_REG(whoAmI()) , CCR_DCACH_PREF_BUF_ENABLE);

	/* ICache Pref  */
	env = getenv("enaICPref");
        if(env && ((strcmp(env,"yes") == 0) ||  (strcmp(env,"Yes") == 0)))
		MV_REG_BIT_SET( CPU_CONFIG_REG(whoAmI()) , CCR_ICACH_PREF_BUF_ENABLE);
	
	if(env && ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
		MV_REG_BIT_RESET( CPU_CONFIG_REG(whoAmI()) , CCR_ICACH_PREF_BUF_ENABLE);

	/* Set L2C WT mode - Set bit 17 */
	temp = MV_REG_READ(CPU_CTRL_STAT_REG(whoAmI()));
    	env = getenv("setL2CacheWT");
    	if(!env || ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
		temp |= BIT17;
	else
		temp &= ~BIT17;

	/* Set L2C size */
    	env = getenv("setL2Size256K");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
		temp &= ~BIT21;
	else
		temp |= BIT21;
	MV_REG_WRITE(CPU_CTRL_STAT_REG(whoAmI()),temp);

	/* Set L2 ECC en/dis - Set bit 22 */
	temp = MV_REG_READ(CPU_CONFIG_REG(whoAmI()));
    	env = getenv("disL2Ecc");
    	if(env && ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
		temp |= BIT22;
	else
		temp &= ~BIT22;
	MV_REG_WRITE(CPU_CONFIG_REG(whoAmI()),temp);

	/* L2Cache settings */
	asm ("mrc p15, 1, %0, c15, c1, 0":"=r" (temp));

	/* Disable L2C pre fetch - Set bit 24 */
    	env = getenv("disL2Prefetch");
    	if(env && ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
		temp &= ~BIT24;
	else
		temp |= BIT24;

	asm ("mcr p15, 1, %0, c15, c1, 0": :"r" (temp));

    /* WA for CPU L2 cache clock ratio limited to 1:3 erratum FE-CPU-180*/
    MV_REG_BIT_RESET(CPU_TIMING_ADJUST_REG(0), BIT28);
    MV_REG_BIT_RESET(CPU_TIMING_ADJUST_REG(1), BIT28);
    while((MV_REG_READ(CPU_TIMING_ADJUST_REG(0)) & BIT28) == BIT28);

	/* enable L2C - Set bit 22 */
    	env = getenv("disL2Cache");
    	if(!env || ( (strcmp(env,"no") == 0) || (strcmp(env,"No") == 0) ) )
		temp |= BIT22;
	else
		temp &= ~BIT22;
	
	asm ("mcr p15, 1, %0, c15, c1, 0": :"r" (temp));


	/* Enable i cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (temp));
	temp |= BIT12;
	/* Change reset vector to address 0x0 */
	temp &= ~BIT13;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (temp));
}
#if 0
/*******************************************************************************
* mvBoardMppModuleTypePrint - print module detect
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_VOID mvMppModuleTypePrint(MV_VOID)
{

	MV_BOARD_MPP_GROUP_CLASS devClass;
	MV_BOARD_MPP_TYPE_CLASS mppGroupType;
	MV_U32 devId;
	MV_U32 maxMppGrp = 1;
	
	devId = mvCtrlModelGet();

	switch(devId){
		case MV_6281_DEV_ID:
			maxMppGrp = MV_6281_MPP_MAX_MODULE;
			break;
        case MV_6282_DEV_ID:
            maxMppGrp = MV_6282_MPP_MAX_MODULE;
            break;
		case MV_6192_DEV_ID:
			maxMppGrp = MV_6192_MPP_MAX_MODULE;
			break;
		case MV_6180_DEV_ID:
			maxMppGrp = MV_6180_MPP_MAX_MODULE;
			break;		
	}

	for (devClass = 0; devClass < maxMppGrp; devClass++)
	{
		mppGroupType = mvBoardMppGroupTypeGet(devClass);

		switch(mppGroupType)
		{
			case MV_BOARD_TDM:
				printf("Module %d is TDM\n", devClass);
				break;
			case MV_BOARD_AUDIO:
				printf("Module %d is AUDIO\n", devClass);
				break;
			case MV_BOARD_RGMII:
				printf("Module %d is RGMII\n", devClass);
				break;
			case MV_BOARD_GMII:
				printf("Module %d is GMII\n", devClass);
				break;
			case MV_BOARD_TS:
				printf("Module %d is TS\n", devClass);
				break;
			default:
				break;
		}
	}
}
#endif
/* Set unit in power off mode acording to the detection of MPP */
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
int mv_set_power_scheme(void)
{
	MV_U32 devId;
	
	devId = mvCtrlModelGet();

    switch(devId)
    {
        case MV_6321_DEV_ID:
            mvCtrlPwrClckSet(PEX_UNIT_ID, 1, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 3, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 4, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 5, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 6, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 7, MV_FALSE);
            mvCtrlPwrClckSet(SATA_UNIT_ID, 0, MV_FALSE);
            MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, BIT13);
            mvCtrlPwrClckSet(SATA_UNIT_ID, 1, MV_FALSE);
            MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, BIT15);
            mvCtrlPwrClckSet(USB_UNIT_ID, 1, MV_FALSE);
            mvCtrlPwrClckSet(USB_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 3, MV_FALSE);
            break;
        case MV_6322_DEV_ID:
            mvCtrlPwrClckSet(PEX_UNIT_ID, 1, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 3, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 5, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 6, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 7, MV_FALSE);
            mvCtrlPwrClckSet(SATA_UNIT_ID, 0, MV_FALSE);
            MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, BIT13);
            mvCtrlPwrClckSet(SATA_UNIT_ID, 1, MV_FALSE);
            MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, BIT15);
            mvCtrlPwrClckSet(USB_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 3, MV_FALSE);
            break;
        case MV_6323_DEV_ID:
            mvCtrlPwrClckSet(PEX_UNIT_ID, 1, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 2, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 3, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 5, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 6, MV_FALSE);
            mvCtrlPwrClckSet(PEX_UNIT_ID, 7, MV_FALSE);
            mvCtrlPwrClckSet(SATA_UNIT_ID, 1, MV_FALSE);
            MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, BIT15);
            mvCtrlPwrClckSet(ETH_GIG_UNIT_ID, 3, MV_FALSE);
            break;
    }
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
}
#endif	/* CFG_FLASH_CFI_DRIVER */

#if 0
#if (CONFIG_COMMANDS & CFG_CMD_RCVR)
static void recoveryDetection(void)
{
    	MV_32 stateButtonBit = mvBoarGpioPinNumGet(BOARD_GPP_WPS_BUTTON,0);
	MV_32 buttonHigh = 0;
	char* env;

	/* Check if auto recovery is en */	
	env = getenv("enaAutoRecovery");
 	if(!env || ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
               	setenv("enaAutoRecovery","yes");
        else
	{
               	setenv("enaAutoRecovery","no");
		rcvrflag = 0;
		return;
	}

	if (stateButtonBit == MV_ERROR)
	{	
		rcvrflag = 0;
		return;
	}

	if (stateButtonBit > 31)
	{
		stateButtonBit = stateButtonBit % 32;
		buttonHigh = 1;
	}

	/* Set state input indication pin as input */
	MV_REG_BIT_SET(GPP_DATA_OUT_EN_REG(buttonHigh),(1<<stateButtonBit));

	/* check if recovery triggered - button is pressed */
	if (!(MV_REG_READ(GPP_DATA_IN_REG(buttonHigh)) & (1 << stateButtonBit)))
	{	
		rcvrflag = 1;
	}
}

extern int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void recoveryHandle(void)
{
	char cmd[256];
	char img[10];
	char * argv[3];
	char *env;
	MV_32 imagAddr = 0x400000;	
	MV_32 imagSize = 0;	
	char ip[16]= {"dhcp"};


	/* Show Recovery start indication - both BLUE and RED blinking */

	/* Perform the DHCP */
	printf("Aquiring an IP address using DHCP...\n");
	if (NetLoop(DHCP) == -1)
	{
		mvOsDelay(1000);
		if (NetLoop(DHCP) == -1)
		{
			mvOsDelay(1000);
			if (NetLoop(DHCP) == -1)
			{
				ulong tmpip;
				printf("Failed to retreive an IP address assuming default (%s)!\n", getenv("rcvrip"));
				tmpip = getenv_IPaddr ("rcvrip");
				NetCopyIP(&NetOurIP, &tmpip);			
				sprintf(ip, "static");
			}
		}
	}

	/* Perform the recovery */
	printf("Starting the Recovery process to retreive the file...\n");
	if ((imagSize = NetLoop(RCVR)) == -1)
	{
		printf("Failed\n");
		return;
	}

	/* Boot the downloaded image */	
	env = getenv("loadaddr");
	if (!env)
		printf("Missing loadaddr environment variable assuming default (0x400000)!\n");
	else
		imagAddr = simple_strtoul(env, NULL, 16); /* get the loadaddr env var */

	printf("Update bootcmd\n");
	sprintf(cmd,"setenv bootargs $(console) root=/dev/ram0 rootfstype=squashfs initrd=0x%x,0x%x ramdisk_size=%d recovery=%s ; bootm 0x%x;", imagAddr + 0x200000, (imagSize - 0x300000), (imagSize - 0x300000)/1024, ip, imagAddr);
	printf("\nbootcmd: %s\n", cmd);
	setenv("bootcmd", cmd);

	printf("Booting the image (@ 0x%x)...\n", imagAddr);

	sprintf(cmd, "boot");
	sprintf(img, "0x%x", imagAddr);
	argv[0] = cmd;
	argv[1] = img;

	do_bootd(NULL, 0, 2, argv);
}

void recoveryCheck(void)
{
	/* Start the recovery process if indicated by user */
	if (rcvrflag)
	{
		recoveryHandle();
	}
}
#endif
#endif

#ifdef MV_INC_BOARD_SPI_FLASH
#include <environment.h>
#include "sflash/mvSFlash.h"

void memcpyFlash(env_t *env_ptr, void* buffer, MV_U32 size)
{
    MV_SFLASH_INFO *pFlash;
    pFlash = getMvFlashInfo(BOOT_FLASH_INDEX);
    mvSFlashBlockRd(pFlash,(MV_U32 *)env_ptr - mvFlashBaseAddrGet(pFlash),
                    (MV_U8 *)buffer, size);
}
#endif

#if defined(DUAL_OS_78200) 
void mvUsbSwapWindows()
{
    /* Disable the window that belongs to the CS of the second CPU */
    MV_REG_BIT_RESET(MV_USB_WIN_CTRL_REG(0, 1 - whoAmI()), 1);
    MV_REG_BIT_SET(MV_USB_WIN_CTRL_REG(0, whoAmI()), 1);
    MV_REG_WRITE(MV_USB_WIN_BASE_REG(0, whoAmI()), 0x0);
}
#endif

