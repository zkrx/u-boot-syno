/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
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
 *
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
 
/* PCI.c - PCI functions */

#include <common.h>
#include <config.h>
#include <command.h>
#if (CONFIG_COMMANDS & CFG_CMD_PCI)


#include <pci.h>
#include "pci-if/mvPciIf.h"
#include "mvCpuIf.h"
#include "pci-if/pci_util/mvPciUtils.h"


#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif /* DEBUG */

/* global definetion */
#define REG_NUM_MASK	(0x3F << 2)
/* global indicate wether we are in the scan process */
unsigned int bus_scan = 0;
extern unsigned int whoAmI(void);


#if CONFIG_COMMANDS & CFG_CMD_BSP

/******************************************************************************
* Category     - PCI0
* Functionality- Scans PCI0 for devices and prints relevant information
* Need modifications (Yes/No) - No
*****************************************************************************/
MV_BOOL scanPci(MV_U32 host)
{
    MV_U32    index,numOfElements=4*8,barIndex;
    MV_PCI_DEVICE      pciDevices[4*8]; //3 slots and us,Max 8 functions per slot

    memset (&pciDevices,0,12*sizeof(MV_PCI_DEVICE));

    if (mvPciScan(host, pciDevices , &numOfElements) != MV_OK )
    {
	DB(printf("scanPci:mvPciScan failed for host %d \n",host));
	return MV_FALSE;
    }

    
    for(index = 0; index < numOfElements ; index++)
    {
	    printf("\nBus: %x Device: %x Func: %x Vendor ID: %x Device ID: %x\n",
	    pciDevices[index].busNumber,
            pciDevices[index].deviceNum,
	    pciDevices[index].function,
            pciDevices[index].venID,
            pciDevices[index].deviceID);

		printf("-------------------------------------------------------------------\n");

		printf("Class: %s\n",pciDevices[index].type);

		/* check if we are bridge*/
		if ((pciDevices[index].baseClassCode == PCI_BRIDGE_CLASS)&&
			(pciDevices[index].subClassCode == P2P_BRIDGE_SUB_CLASS_CODE))
		{
			printf("Primary Bus:0x%x \tSecondary Bus:0x%x \tSubordinate Bus:0x%x\n",
							pciDevices[index].p2pPrimBusNum,
							pciDevices[index].p2pSecBusNum,
							pciDevices[index].p2pSubBusNum);
			
			printf("IO Base:0x%x \t\tIO Limit:0x%x",pciDevices[index].p2pIObase,
							pciDevices[index].p2pIOLimit);

			(pciDevices[index].bIO32)? (printf(" (32Bit IO)\n")):
								(printf(" (16Bit IO)\n"));

			printf("Memory Base:0x%x \tMemory Limit:0x%x\n",pciDevices[index].p2pMemBase,
							pciDevices[index].p2pMemLimit);

			printf("Pref Memory Base:0x%x \tPref Memory Limit:0x%x",
				   pciDevices[index].p2pPrefMemBase,
							pciDevices[index].p2pPrefMemLimit);

			(pciDevices[index].bPrefMem64)? (printf(" (64Bit PrefMem)\n")):
								(printf(" (32Bit PrefMem)\n"));
			if (pciDevices[index].bPrefMem64)
			{
				printf("Pref Base Upper 32bit:0x%x \tPref Limit Base Upper32 bit:0x%x\n",
								pciDevices[index].p2pPrefBaseUpper32Bits,
								pciDevices[index].p2pPrefLimitUpper32Bits);
			}
		}

	for (barIndex = 0 ; barIndex < pciDevices[index].barsNum ; barIndex++)
        {

	   if (pciDevices[index].pciBar[barIndex].barType == PCI_64BIT_BAR)
	   {
           printf("PCI_BAR%d (%s-%s) base: %x%08x%s",barIndex,
                  (pciDevices[index].pciBar[barIndex].barMapping == PCI_MEMORY_BAR)?"Mem":"I/O",
                  "64bit",
                  pciDevices[index].pciBar[barIndex].barBaseHigh,
                  pciDevices[index].pciBar[barIndex].barBaseLow,
                  (pciDevices[index].pciBar[barIndex].barBaseLow == 0)?"\t\t":"\t");
	   }
	   else if (pciDevices[index].pciBar[barIndex].barType == PCI_32BIT_BAR)
	   {
             printf("PCI_BAR%d (%s-%s) base: %x%s",barIndex,
                (pciDevices[index].pciBar[barIndex].barMapping == PCI_MEMORY_BAR)?"Mem":"I/O",
                "32bit",
                pciDevices[index].pciBar[barIndex].barBaseLow,
                (pciDevices[index].pciBar[barIndex].barBaseLow == 0)?"\t\t\t":"\t\t");
           }
	
         if(pciDevices[index].pciBar[barIndex].barSizeHigh != 0)
            printf("size: %d%08d bytes\n",pciDevices[index].pciBar[barIndex].barSizeHigh,
                                         pciDevices[index].pciBar[barIndex].barSizeLow);
         else
            printf("size: %d bytes\n", pciDevices[index].pciBar[barIndex].barSizeLow);
        }
    }
    return MV_TRUE;
}


int sp_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	MV_U32 host = 0;


        if (argc > 1) {
                host = simple_strtoul (argv[1], NULL, 10);
        }
	if(host >= mvCtrlPexMaxIfGet()){ 
		printf("PCI %d doesn't exist\n",host);		
		return 1;
	}
	if( scanPci(host) == MV_FALSE)
		printf("PCI %d Scan - FAILED!!.\n",host);
	return 1;
}
	
U_BOOT_CMD(
	sp,      2,     1,      sp_cmd,
	"sp	- Scan PCI bus.\n",
	" [0/1] \n"
	"\tScan and detecect all devices on mvPCI bus 0/1 \n"
	"\t(This command can be used only if enaMonExt is set!)\n"
);

int me_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        MV_U32 host = 0;
        if (argc > 1) 
	{
                host = simple_strtoul (argv[1], NULL, 10);
        }

		if(host >= mvCtrlPexMaxIfGet()) 
		{
			printf("Master %d doesn't exist\n",host); 
			return 1;
		} 

        if(mvPciIfMasterEnable(host,MV_TRUE)  == MV_OK)
                printf("PCI %d Master enabled.\n",host);
        else
                printf("PCI %d Master enabled -FAILED!!\n",host);
 
        return 1;
}
 
U_BOOT_CMD(
        me,      2,      1,      me_cmd,
        "me	- PCI master enable\n",
        " [0/1] \n"
        "\tEnable the MV device as Master on PCI 0/1. \n"
);

int se_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        MV_U32 host=0,dev = 0,bus=0;

	if(argc != 4)
	{
                printf ("Usage:\n%s\n", cmdtp->usage);
                return 1;
	}
        host = simple_strtoul (argv[1], NULL, 10);
        bus = simple_strtoul (argv[2], NULL, 16);
        dev = simple_strtoul (argv[3], NULL, 16);

        if(host >= mvCtrlPexMaxIfGet())
		{ 
			printf("PCI %d doesn't exist\n",host); 
			return 1;
		} 
        if(mvPciIfSlaveEnable(host,bus,dev,MV_TRUE) == MV_OK )
                printf("PCI %d Bus %d Slave 0x%x enabled.\n",host,bus,dev);
        else
                printf("PCI %d Bus %d Slave 0x%x enabled - FAILED!!.\n",host,bus,dev);
        return 1;
}
         
U_BOOT_CMD(
        se,      4,     1,      se_cmd,
        "se	- PCI Slave enable\n",
        " [0/1] bus dev \n"
        "\tEnable the PCI device as Slave on PCI 0/1. \n"
);

/******************************************************************************
* Functionality- The commands changes the pci remap register and displays the
*                address to be used in order to access PCI 0.
*****************************************************************************/
int mapPci_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

    MV_ADDR_WIN pciWin;
	MV_TARGET target=0;
    MV_U32 host=0,effectiveBaseAddress=0;

    pciWin.baseLow=0;
    pciWin.baseHigh=0;
 
    if (argc > 1) {
        host = simple_strtoul(argv[1], NULL, 10);
    }
    if(argc > 2) {
        pciWin.baseLow = simple_strtoul(argv[2], NULL, 16);
    }
    if(host >= mvCtrlPexMaxIfGet()){
 	printf("PCI %d doesn't exist\n",host);
	return 1;
    }

    target = PCI0_MEM0 + (2 * host);

    printf("mapping pci %x to address 0x%x\n",host,pciWin.baseLow);
	
#if defined(MV_INCLUDE_PEX) || defined(MV_INCLUDE_PCI)
	effectiveBaseAddress = mvCpuIfPciIfRemap(target,&pciWin);
#endif

	if ( effectiveBaseAddress == 0xffffffff)
	{
		printf("Error remapping\n");
		return 1;
	}

    printf("PCI %x Access base address : %x\n",host,effectiveBaseAddress);
    return 1;
}
 
U_BOOT_CMD(
        mp,      3,     1,      mapPci_cmd,
        "mp	- map PCI BAR\n",
        " [0/1] address \n"
        "\tChange the remap of PCI 0/1 window 0 to address 'addrress'.\n"
        "\tIt also displays the new access address, since the remap is not always\n"
        "\tthe same as requested. \n"
);

#endif

MV_U32 mv_mem_ctrl_dev(MV_U32 pciIf, MV_U32 bus,MV_U32 dev)
{
	MV_U32 ven, class;
	ven =    mvPciIfConfigRead(pciIf,bus,dev,0,PCI_VENDOR_ID) & 0xffff;
	class = (mvPciIfConfigRead(pciIf,bus,dev,0,PCI_REVISION_ID) >> 16 ) & 0xffff;
	/* if we got any other Marvell PCI cards ignore it. */
	if(((ven == 0x11ab) && (class == PCI_CLASS_MEMORY_OTHER))||
	  ((ven == 0x11ab) && (class == PCI_CLASS_BRIDGE_HOST)))
	{
		return 1;
	}
	return 0;
}

static int mv_read_config_dword(struct pci_controller *hose,
				pci_dev_t dev,
				int offset, u32* value)
{
	MV_U32 bus,func,regOff,dev_no;
	char *env;

	bus = PCI_BUS(dev);
	dev_no = PCI_DEV(dev);

	func = (MV_U32)PCI_FUNC(dev);
	regOff = (MV_U32)offset & REG_NUM_MASK;

	/*  We will scan only ourselves and the PCI slots that exist on the 
		board, because we may have a case that we have one slot that has
		a Cardbus connector, and because CardBus answers all IDsels we want
		to scan only this slot and ourseleves.
	*/
	
	#if defined(MV_INCLUDE_PCI)	
	env = getenv("pciMode");
	if ((PCI_IF_TYPE_CONVEN_PCIX == mvPciIfTypeGet((MV_U32)hose->cfg_addr)) && 
		((strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0)))
	{
		if ( !mvBoardIsOurPciSlot(bus, dev_no) && 
		    (mvBoardIdGet() != DB_88F5181_DDR1_MNG) &&
		    (DB_88F5181_DDR1_PRPMC != mvBoardIdGet()) &&
		    (DB_88F5181_DDR1_PEXPCI != mvBoardIdGet()))
		{
			*value = 0xffffffff;
			return 0;
		}
	}
	#endif /* defined(MV_INCLUDE_PCI) */
	
	if( bus_scan == 1 )
	{
		env = getenv("disaMvPnp");
		if(env && ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
		{
			if( mv_mem_ctrl_dev((MV_U32)hose->cfg_addr, bus, dev_no) )
			{
				*value = 0xffffffff;
				return 0;
			}
		}
	}
	DB(printf("mv_read_config_dword hose->cfg_addr %x\n",hose->cfg_addr);)
	DB(printf("mv_read_config_dword bus %x\n",bus);)
	DB(printf("mv_read_config_dword dev_no %x\n",dev_no);)
	DB(printf("mv_read_config_dword func %x\n",func);)
	DB(printf("mv_read_config_dword regOff %x\n",regOff);)

	*value = (u32) mvPciIfConfigRead((MV_U32)hose->cfg_addr,bus,dev_no,func,regOff);
	
	DB(printf("mv_read_config_dword value %x\n",*value);)

	return 0;
}

static int mv_write_config_dword(struct pci_controller *hose,
				 pci_dev_t dev,
				 int offset, u32 value)
{
	MV_U32 bus,func,regOff,dev_no;

	bus = PCI_BUS(dev);
	dev_no = PCI_DEV(dev);
	func = (MV_U32)PCI_FUNC(dev);
	regOff = offset & REG_NUM_MASK;
	mvPciIfConfigWrite((MV_U32)hose->cfg_addr,bus,dev_no,func,regOff,value);

	return 0;
}



static void mv_setup_ide(struct pci_controller *hose,
			 pci_dev_t dev, struct pci_config_table *entry)
{
    static const int ide_bar[]={8,4,8,4,16,1024};
    u32 bar_response, bar_value;
    int bar;

    for (bar=0; bar<6; bar++)
    {
	/*ronen different function for 3rd bank.*/
	unsigned int offset = (bar < 2)? bar*8: 0x100 + (bar-2)*8;

	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + offset, 0x0);
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0 + offset, &bar_response);

	pciauto_region_allocate(bar_response & PCI_BASE_ADDRESS_SPACE_IO ?
				hose->pci_io : hose->pci_mem, ide_bar[bar], &bar_value);

	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + bar*4, bar_value);
    }
}

static void mv_setup_host(struct pci_controller *hose,
			 pci_dev_t dev, struct pci_config_table *entry)
{
	//skip our host
	DB(printf("skipping :bus=%x dev=%x fun=%x\n",
			  (unsigned int)PCI_BUS(dev),
			  (unsigned int)PCI_DEV(dev),
			  (unsigned int)PCI_FUNC(dev)));
	return;
}


static void mv_pci_bus_mode_display(MV_U32 host)
{

	if (PCI_IF_TYPE_PEX == mvPciIfTypeGet(host))
	{
		#if defined(MV_INCLUDE_PEX)
		
		MV_PEX_MODE pexMode;
		if (mvPexModeGet(mvPciRealIfNumGet(host),&pexMode) != MV_OK)
		{
			printf("mv_pci_bus_mode_display: mvPexModeGet failed\n");
		}
		switch (pexMode.pexType)
		{
		case MV_PEX_ROOT_COMPLEX:
			printf("PEX %d: PCI Express Root Complex Interface\n",host);
			break;
		case MV_PEX_END_POINT:
			printf("PEX %d: PCI Express End Point Interface\n",host);
			break;
		}

		if (!(pexMode.pexLinkUp))
		{
		    printf("PEX interface detected no Link.\n");
		}
		else
		{
		    if (MV_PEX_WITDH_X1 ==  pexMode.pexWidth)
		    {
			printf("PEX interface detected Link X1\n");
		    }
		    else
		    {
			printf("PEX interface detected Link X4\n");
		    }
		}
		
		return ;

		#endif /* MV_INCLUDE_PEX */
	}




	
	if (PCI_IF_TYPE_CONVEN_PCIX == mvPciIfTypeGet(host))
	{
		#if defined(MV_INCLUDE_PCI)

		MV_PCI_MODE pciMode;
		
		if (mvPciModeGet(mvPciRealIfNumGet(host),&pciMode) != MV_OK)
		{
			printf("mv_pci_bus_mode_display: mvPciIfModeGet failed\n");

		}

		switch (pciMode.pciType)
		{
		 case MV_PCI_CONV:

			printf("PCI %d: Conventional PCI",host);
			break;

		 case MV_PCIX:

			printf("PCI %d: PCI-X",host);
			break;

		 default:
			printf("PCI %d: Unknown",host);
			return;
			break;

		}

		printf(", speed = %d\n",pciMode.pciSpeed);

		#endif /* #if defined(MV_INCLUDE_PCI) */
	}



}

struct pci_config_table mv_config_table[] = {
    { PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
      PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, mv_setup_ide},

    { PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
      PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, mv_setup_host}, //PCI host

    {}

};


/* Defines for more modularity of the pci_init_board function */

struct pci_controller  pci_hose[8];


#if (MV_PCI_IF_MAX_IF == 2)

#define PCI_IF_MEM(pciIf)	((pciIf==0)?PCI_IF0_MEM0:PCI_IF1_MEM0)
#define PCI_IF_REMAPED_MEM_BASE(pciIf) ((pciIf==0)?PCI_IF0_REMAPED_MEM_BASE:PCI_IF1_REMAPED_MEM_BASE)
#define PCI_IF_MEM_BASE(pciIf)	((pciIf==0)?PCI_IF0_MEM0_BASE:PCI_IF1_MEM0_BASE)
#define PCI_IF_MEM_SIZE(pciIf)	((pciIf==0)?PCI_IF0_MEM0_SIZE:PCI_IF1_MEM0_SIZE)
#define PCI_IF_IO_BASE(pciIf)	((pciIf==0)?PCI_IF0_IO_BASE:PCI_IF1_IO_BASE)
#define PCI_IF_IO_SIZE(pciIf)	((pciIf==0)?PCI_IF0_IO_SIZE:PCI_IF1_IO_SIZE)

#else

#define PCI_IF_MEM(pciIf)	(PCI_IF0_MEM0)
#define PCI_IF_REMAPED_MEM_BASE(pciIf) (PCI_IF0_REMAPED_MEM_BASE)
#define PCI_IF_MEM_BASE(pciIf)	(PCI_IF0_MEM0_BASE)
#define PCI_IF_MEM_SIZE(pciIf)	(PCI_IF0_MEM0_SIZE)
#define PCI_IF_IO_BASE(pciIf)	(PCI_IF0_IO_BASE)
#define PCI_IF_IO_SIZE(pciIf)	(PCI_IF0_IO_SIZE)

#endif

/* because of CIV tem needs we are gonna do a remap to PCI memory */
#define PCI_IF0_REMAPED_MEM_BASE	0x90000000
#define PCI_IF1_REMAPED_MEM_BASE	0x90000000

void
pci_init_board(void)
{
	MV_U32 pciIfNum = mvCtrlPexMaxIfGet();
	MV_U32 pciIf=0;
	MV_ADDR_WIN rempWin;
	char *env;
	int first_busno=0;
	int status;
    	MV_CPU_DEC_WIN  cpuAddrDecWin;
	PCI_IF_MODE	pciIfMode = PCI_IF_MODE_HOST;
	int pciIfStart = 0;
#ifdef MV78200
	int pciIfAccess[2] = {0,0};
	int i = 0;
	char str[30];
#endif

    if(pciIfNum == 0)
        return;
	env = getenv("disaMvPnp");

	if(env && ( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ) )
		printf("Warning: skip configuration of Marvell devices!!!\n");
#if defined(MV_INCLUDE_PEX) && defined(MV78XX0)
	/* Power down the none lanes 0.1, 0.2, and 0.3 if PEX0 is X4 */
	if ( !(PCI0_IS_QUAD_X1) )
	{
	    for (pciIf = 1; pciIf < 4; pciIf++)
		mvCtrlPwrClckSet(PEX_UNIT_ID, mvPciRealIfNumGet(pciIf),MV_FALSE);
	}

	/* Power down the none lanes 1.1, 1.2, and 1.3 if PEX1 is X4 */
	if ( !(PCI1_IS_QUAD_X1) )
	{
	    for (pciIf = 5; pciIf < 8; pciIf++)
		mvCtrlPwrClckSet(PEX_UNIT_ID, mvPciRealIfNumGet(pciIf),MV_FALSE);
	}
#endif

#ifdef MV78200
#if defined(DUAL_OS_78200) || defined(DUAL_OS_SHARED_MEM_78200)
	/* Check CPU access to PEX */
    if(mvSocUnitIsMappedToThisCpu(PEX00))
		pciIfAccess[0] = 1;

    if(mvSocUnitIsMappedToThisCpu(PEX10))
		pciIfAccess[1] = 1;
#if !defined(MV632X)
    /* Calc pci If start and last */
	pciIfNum  = 0;
	for(i=1; i >= 0 ; i--)
	{
	    if(pciIfAccess[i] == 1)
	    {
		pciIfStart = (i * 4);
		pciIfNum  += 4;
	    }
	}
	pciIfNum += pciIfStart;
#endif
#endif
#endif

	/* start pci scan */
	for (pciIf = pciIfStart; pciIf < pciIfNum; pciIf++)
	{
		pci_hose[pciIf].config_table = mv_config_table;
		if (PCI_IF_TYPE_PEX == mvPciIfTypeGet(pciIf))
		{
			if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, mvPciRealIfNumGet(pciIf))) 
			{
				continue;
			}
		}
#if defined(MV_INCLUDE_PCI)
		else
		{
			if (MV_FALSE == mvCtrlPwrClckGet(PCI_UNIT_ID, mvPciRealIfNumGet(pciIf)))
			{
				continue;
			}
		}
#endif
		/* device or host ? */
		if (PCI_IF_TYPE_PEX == mvPciIfTypeGet(pciIf))
		{
#if defined(MV_INCLUDE_PEX)
#if !defined(MV_88F6183) && !defined(MV_88F6183L) && !defined(MV_88F6082) && !defined(MV88F6281) && \
            !defined(MV88F6192) && !defined(MV88F6180)  && !defined(MV88F6190) && !defined(MV88F6282) && \
		!defined(MV88F6701) && !defined(MV88F6702) 
			MV_PEX_MODE pexMode;
			if (mvPexModeGet(mvPciRealIfNumGet(pciIf),&pexMode) != MV_OK)
			{
				printf("pci_init_board: mvPexModeGet failed\n");
			}

			if (MV_PEX_ROOT_COMPLEX == pexMode.pexType)	
			    pciIfMode = PCI_IF_MODE_HOST;
			else 
			    pciIfMode = PCI_IF_MODE_DEVICE;

#else
			/* Set pex mode incase S@R not exist */
			env = getenv("pexMode");

			if( env && ( ((strcmp(env,"EP") == 0) || (strcmp(env,"ep") == 0) )))
			{
				pciIfMode = PCI_IF_MODE_DEVICE;
			}
			else
			{	
				pciIfMode = PCI_IF_MODE_HOST;
			}

#if defined(MV88F6282)
			/* 6282 Pex 1 interface is only RC */
			if (pciIf == 1)
				pciIfMode = PCI_IF_MODE_HOST;
#endif
#endif
#endif
		}
		else /* PCI */
		{
			#if defined(MV_INCLUDE_PCI)
			char *env;			
			env = getenv("pciMode");
			if((!env) || (strcmp(env,"host") == 0) || (strcmp(env,"Host") == 0) )
			{
				pciIfMode = PCI_IF_MODE_HOST;
			}
			else
			{
				pciIfMode = PCI_IF_MODE_DEVICE;
			}
			#endif
		}
#ifdef MV78200
        if ((pciIf == 0 && !mvSocUnitIsMappedToThisCpu(PEX00)) ||
            (pciIf == 4 && !mvSocUnitIsMappedToThisCpu(PEX10)))
            continue;
        else
#endif
        {
    		if ((status = mvPciIfInit(pciIf, pciIfMode)) == MV_ERROR)
    		{
    			printf("pci_init_board:Error calling mvPciIfInit for pciIf %d\n",pciIf);	
    		}
    		else
    		{
    			if (status == MV_OK)
    		    		mv_pci_bus_mode_display(pciIf);
    			else
                {
                    /* Interface with no link */
                    printf("PEX %d: interface detected no Link.\n", pciIf);
                    continue;
                }
    		}
        }

		if (PCI_IF_TYPE_CONVEN_PCIX == mvPciIfTypeGet(pciIf))
		{
			#if defined(MV_INCLUDE_PCI)
			if(enaMonExt()) 
			{
				/* WA for first PCI interface */
				/* mvPciWaFix(mvPciRealIfNumGet(pciIf)); */
			}
			#endif /* MV_INCLUDE_PCI */

			if (pciIfMode == PCI_IF_MODE_DEVICE)
			{
				mvPciIfLocalDevNumSet(pciIf, 0x1f);
			}
		}

#if defined(DB_PRPMC)
		mvPciIfLocalDevNumSet(pciIf, 0x1f);
#endif
		
		/* start Uboot PCI scan */
		if (pciIf == 0)
			pci_hose[pciIf].first_busno = 0;
		else
			pci_hose[pciIf].first_busno = pci_hose[pciIf-1].last_busno + 1;

		/* start Uboot PCI scan */
		pci_hose[pciIf].current_busno = pci_hose[pciIf].first_busno;
		pci_hose[pciIf].last_busno = 0xff;

		if (mvPciIfLocalBusNumSet(pciIf,pci_hose[pciIf].first_busno) != MV_OK)
		{
			printf("pci_init_board:Error calling mvPciIfLocalBusNumSet for pciIf %d\n",pciIf);	
		}

		/* If no link on the interface it will not be scan */
		if (status == MV_NO_SUCH)
		{
			pci_hose[pciIf].last_busno =pci_hose[pciIf].first_busno;
			continue;
		}

#ifdef PCI_DIS_INTERFACE
		/* The disable interface will not be scan */
		if (pciIf == PCI_DIS_INTERFACE)
		{
			printf("***Interface is disable***\n");	
			pci_hose[pciIf].last_busno =pci_hose[pciIf].first_busno;
			continue;
		}
#endif
		if (MV_OK != mvCpuIfTargetWinGet(PCI_MEM(pciIf, 0), &cpuAddrDecWin))
		{
            		printf("%s: ERR. mvCpuIfTargetWinGet failed\n", __FUNCTION__);
	    		return ;
		}

		rempWin.baseLow = ((cpuAddrDecWin.addrWin.baseLow & 0x0fffffff) | PCI_IF_REMAPED_MEM_BASE(pciIf));
		rempWin.baseHigh = 0;

		/* perform a remap for the PEX0 interface*/
		if (0xffffffff == mvCpuIfPciIfRemap(PCI_MEM(pciIf, 0),&rempWin))
		{
			printf("%s:mvCpuIfPciIfRemap failed\n",__FUNCTION__);
			return;
		}

		/* PCI memory space */
		pci_set_region(pci_hose[pciIf].regions + 0,
				rempWin.baseLow, /* bus address */
				cpuAddrDecWin.addrWin.baseLow,
				cpuAddrDecWin.addrWin.size,
				PCI_REGION_MEM);

		if (MV_OK != mvCpuIfTargetWinGet(PCI_IO(pciIf), &cpuAddrDecWin))
		{
			/* No I/O space */
			pci_hose[pciIf].region_count = 1;
		}
		else
		{

		    /* PCI I/O space */
		    pci_set_region(pci_hose[pciIf].regions + 1,
				    cpuAddrDecWin.addrWin.baseLow,
				    cpuAddrDecWin.addrWin.baseLow,
				    cpuAddrDecWin.addrWin.size,
				    PCI_REGION_IO);
		    pci_hose[pciIf].region_count = 2;
		}

		pci_set_ops(&pci_hose[pciIf],
				pci_hose_read_config_byte_via_dword,
				pci_hose_read_config_word_via_dword,
				mv_read_config_dword,
				pci_hose_write_config_byte_via_dword,
				pci_hose_write_config_word_via_dword,
				mv_write_config_dword);

		pci_hose[pciIf].cfg_addr = (unsigned int*) pciIf;

		pci_hose[pciIf].config_table[1].bus = mvPciIfLocalBusNumGet(pciIf);
		pci_hose[pciIf].config_table[1].dev = mvPciIfLocalDevNumGet(pciIf);

		pci_register_hose(&pci_hose[pciIf]);

		if (pciIfMode == PCI_IF_MODE_HOST)
		{
			MV_U32 pciData=0,baseClassCode=0,subClassCode;
			
			bus_scan = 1;

			pci_hose[pciIf].last_busno = pci_hose_scan(&pci_hose[pciIf]);
			bus_scan = 0;

			pciData = mvPciIfConfigRead(pciIf,pci_hose[pciIf].first_busno,1,0, PCI_CLASS_CODE_AND_REVISION_ID);
	
			baseClassCode = (pciData & PCCRIR_BASE_CLASS_MASK) >> PCCRIR_BASE_CLASS_OFFS;
			subClassCode = 	(pciData & PCCRIR_SUB_CLASS_MASK) >> PCCRIR_SUB_CLASS_OFFS;

			if ((baseClassCode == PCI_BRIDGE_CLASS) && 
			   (subClassCode == P2P_BRIDGE_SUB_CLASS_CODE))
			{
			    mvPciIfConfigWrite(pciIf,pci_hose[pciIf].first_busno,1,0,P2P_MEM_BASE_LIMIT,pciData);
			    /* In rthe bridge : We want to open its memory and
			    IO to the maximum ! after the u-boot Scan */
			    /* first the IO */
			    pciData  = mvPciIfConfigRead(pciIf,pci_hose[pciIf].first_busno,1,0,P2P_IO_BASE_LIMIT_SEC_STATUS);
			    /* keep the secondary status */
			    pciData  &= PIBLSS_SEC_STATUS_MASK;
			    /* set to the maximum - 0 - 0xffff */
			    pciData  |= 0xff00;
			    mvPciIfConfigWrite(pciIf,pci_hose[pciIf].first_busno,1,0,P2P_IO_BASE_LIMIT_SEC_STATUS,pciData);

			    /* the the Memory */
			    pciData  = mvPciIfConfigRead(pciIf,pci_hose[pciIf].first_busno,1,0,P2P_MEM_BASE_LIMIT);
			    /* set to the maximum - PCI_IF_REMAPED_MEM_BASE(pciIf) - 0xf000000 */
			    pciData  = 0xEFF00000 | (PCI_IF_REMAPED_MEM_BASE(pciIf) >> 16);
			    mvPciIfConfigWrite(pciIf,pci_hose[pciIf].first_busno,1,0,P2P_MEM_BASE_LIMIT,pciData);
			}
		}
		else /* PCI_IF_MODE_HOST */
		{
			pci_hose[pciIf].last_busno = pci_hose[pciIf].first_busno;
		}

		first_busno = pci_hose[pciIf].last_busno + 1;
	}
#ifdef DB_FPGA
	MV_REG_BIT_RESET(PCI_BASE_ADDR_ENABLE_REG(0), BIT10);
#endif
}

#endif /* CONFIG_PCI */

