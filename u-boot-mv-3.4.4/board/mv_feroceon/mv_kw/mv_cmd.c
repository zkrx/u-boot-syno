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

#include <config.h>
#include <common.h>
#include <command.h>
#include <pci.h>

#include "mvTypes.h"
#include "mvCtrlEnvLib.h"

#if defined(MV_INC_BOARD_NOR_FLASH)
#include "norflash/mvFlash.h"
#endif

#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
#include "eth-phy/mvEthPhy.h"
#endif

#if defined(MV_INCLUDE_PEX)
#include "pex/mvPex.h"
#endif

#if defined(MV_INCLUDE_IDMA)
#include "idma/mvIdma.h"
#include "sys/mvSysIdma.h"
#endif

#if defined(CFG_NAND_BOOT) || defined(CFG_CMD_NAND)
#include <nand.h>

/* references to names in cmd_nand.c */
#define NANDRW_READ		0x01
#define NANDRW_WRITE	0x00
#define NANDRW_JFFS2	0x02
//extern struct nand_chip nand_dev_desc[];
extern nand_info_t nand_info[];       /* info for NAND chips */
/* int nand_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);
 int nand_erase(struct nand_chip* nand, size_t ofs,
				size_t len, int clean);
*/
extern int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts);
extern int nand_write_opts(nand_info_t *meminfo, const nand_write_options_t *opts);


#endif /* CFG_NAND_BOOT */

#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
#if !defined(CFG_NAND_BOOT)
static unsigned int flash_in_which_sec(flash_info_t *fl,unsigned int offset)
{
	unsigned int sec_num;
	if(NULL == fl)
		return 0xFFFFFFFF;

	for( sec_num = 0; sec_num < fl->sector_count ; sec_num++){
		/* If last sector*/
		if (sec_num == fl->sector_count -1)
		{
			if((offset >= fl->start[sec_num]) && 
			   (offset <= (fl->size + fl->start[0] - 1)) )
			{
				return sec_num;
			}

		}
		else
		{
			if((offset >= fl->start[sec_num]) && 
			   (offset < fl->start[sec_num + 1]) )
			{
				return sec_num;
			}

		}
	}
	/* return illegal sector Number */
	return 0xFFFFFFFF;

}

#endif /* !defined(CFG_NAND_BOOT) */


/*******************************************************************************
burn a u-boot.bin on the Boot Flash
********************************************************************************/
extern flash_info_t flash_info[];       /* info for FLASH chips */
#include <net.h>
#include "bootstrap_def.h"
#if (CONFIG_COMMANDS & CFG_CMD_NET)
/* 
 * 8 bit checksum 
 */
static u8 checksum8(u32 start, u32 len,u8 csum)
{
    register u8 sum = csum;
	volatile u8* startp = (volatile u8*)start;

    if (len == 0)
	return csum;

    do{
	  	sum += *startp;
		startp++;
    }while(--len);

    return (sum);
}

/*
 * Check the extended header and execute the image
 */
static MV_U32 verify_extheader(ExtBHR_t *extBHR)
{
	MV_U8	chksum;


	/* Caclulate abd check the checksum to valid */
	chksum = checksum8((MV_U32)extBHR , EXT_HEADER_SIZE -1, 0);
	if (chksum != (*(MV_U8*)((MV_U32)extBHR + EXT_HEADER_SIZE - 1)))
	{
		printf("Error! invalid extende header checksum\n");
		return MV_FAIL;
	}
	
    return MV_OK;
}
/*
 * Check the CSUM8 on the main header
 */
static MV_U32 verify_main_header(BHR_t *pBHR, MV_U8 headerID)
{
	MV_U8	chksum;

	/* Verify Checksum */
	chksum = checksum8((MV_U32)pBHR, sizeof(BHR_t) -1, 0);

	if (chksum != pBHR->checkSum)
	{
		printf("Error! invalid image header checksum\n");
		return MV_FAIL;
	}

	/* Verify Header */
	if (pBHR->blockID != headerID)
	{
		printf("Error! invalid image header ID\n");
		return MV_FAIL;
	}
	
	/* Verify Alignment */
	if (pBHR->blockSize & 0x3)
	{
		printf("Error! invalid image header alignment\n");
		return MV_FAIL;
	}

	if ((cpu_to_le32(pBHR->destinationAddr) & 0x3) && (cpu_to_le32(pBHR->destinationAddr) != 0xffffffff))
	{
		printf("Error! invalid image header destination\n");
		return MV_FAIL;
	}

	if ((cpu_to_le32(pBHR->sourceAddr) & 0x3) && (pBHR->blockID != IBR_HDR_SATA_ID))
	{
		printf("Error! invalid image header source\n");
		return MV_FAIL;
	}

    return MV_OK;
}

#if defined(CFG_NAND_BOOT)
/* Boot from NAND flash */
/* Write u-boot image into the nand flash */
int nand_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int filesize;
	MV_U32 ret = 0;
	extern char console_buffer[];
	nand_info_t *nand = &nand_info[0];
	nand_erase_options_t er_opts;
	nand_write_options_t wr_opts;

	load_addr = CFG_LOAD_ADDR; 
	if(argc == 2) {
		copy_filename (BootFile, argv[1], sizeof(BootFile));
	}
	else { 
		copy_filename (BootFile, "u-boot.bin", sizeof(BootFile));
		printf("using default file \"u-boot.bin\" \n");
	}
 
	if ((filesize = NetLoop(TFTP)) < 0)
		return 0;

#ifdef MV_BOOTROM
    	BHR_t*           	tmpBHR = (BHR_t*)load_addr;
	ExtBHR_t*   		extBHR = (ExtBHR_t*)(load_addr + BHR_HDR_SIZE);
	MV_U32			errCode=0;

	/* Verify Main header checksum */
	errCode = verify_main_header(tmpBHR, IBR_HDR_NAND_ID);
	if (errCode) 
		return 0;

	/* Verify that the extended header is valid */
	errCode = verify_extheader(extBHR);
	if (errCode) 
		return 0;
#endif

	printf("\n**Warning**\n");
	printf("If U-Boot Endiannes is going to change (LE->BE or BE->LE), Then Env parameters should be overriden..\n");
	printf("Override Env parameters? (y/n)");
	readline(" ");
	if( strcmp(console_buffer,"Y") == 0 || 
	    strcmp(console_buffer,"yes") == 0 ||
	    strcmp(console_buffer,"y") == 0 ) {

		printf("Erase Env parameters sector %d... ",CFG_ENV_OFFSET);
		memset(&er_opts, 0, sizeof(er_opts));
		er_opts.offset = CFG_ENV_OFFSET;
		er_opts.length = CFG_ENV_SECT_SIZE;
		er_opts.quiet  = 1;

		nand_erase_opts(nand, &er_opts);
		//nand_erase(nand_dev_desc + 0, CFG_ENV_OFFSET, CFG_ENV_SECT_SIZE, 0);
		printf("\n");
	}

	printf("Erase %d - %d ... ",CFG_MONITOR_BASE, CFG_MONITOR_LEN);
	memset(&er_opts, 0, sizeof(er_opts));
	er_opts.offset = CFG_MONITOR_BASE;
	er_opts.length = CFG_MONITOR_LEN;
	er_opts.quiet  = 1;
	nand_erase_opts(nand, &er_opts);
	//nand_erase(nand_dev_desc + 0, CFG_MONITOR_BASE, CFG_MONITOR_LEN, 0);
	
	printf("\nCopy to Nand Flash... ");
	memset(&wr_opts, 0, sizeof(wr_opts));
	wr_opts.buffer	= (u_char*) load_addr;
	wr_opts.length	= CFG_MONITOR_LEN;
	wr_opts.offset	= CFG_MONITOR_BASE;
	/* opts.forcejffs2 = 1; */
	wr_opts.pad	= 1;
	wr_opts.blockalign = 1;
	wr_opts.quiet      = 1;
	ret = nand_write_opts(nand, &wr_opts);
	/* ret = nand_rw(nand_dev_desc + 0,
				  NANDRW_WRITE | NANDRW_JFFS2, CFG_MONITOR_BASE, CFG_MONITOR_LEN,
			      &total, (u_char*)0x100000 + CFG_MONITOR_IMAGE_OFFSET);
	*/
  	if (ret)
		printf("Error - NAND burn faild!\n");
	else
		printf("\ndone\n");	

	return 1;
}

U_BOOT_CMD(
        bubt,      2,     1,      nand_burn_uboot_cmd,
        "bubt	- Burn an image on the Boot Nand Flash.\n",
        " file-name \n"
        "\tBurn a binary image on the Boot Nand Flash, default file-name is u-boot.bin .\n"
);

/* Write nboot loader image into the nand flash */
int nand_burn_nboot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int filesize;
	MV_U32 ret = 0;
	extern char console_buffer[];
	nand_info_t *nand = &nand_info[0];
	nand_erase_options_t er_opts;
	nand_write_options_t wr_opts;


	load_addr = CFG_LOAD_ADDR; 
	if(argc == 2) {
		copy_filename (BootFile, argv[1], sizeof(BootFile));
	}
	else { 
		copy_filename (BootFile, "nboot.bin", sizeof(BootFile));
		printf("using default file \"nboot.bin\" \n");
	}
 
	if ((filesize = NetLoop(TFTP)) < 0)
		return 0;
 
	printf("Erase %d - %d ... ",CFG_NBOOT_BASE, CFG_NBOOT_LEN);
	memset(&er_opts, 0, sizeof(er_opts));
	er_opts.offset = CFG_NBOOT_BASE;
	er_opts.length = CFG_NBOOT_LEN;
	er_opts.quiet  = 1;

	nand_erase_opts(nand, &er_opts);
	//nand_erase(nand_dev_desc + 0, CFG_NBOOT_BASE, CFG_NBOOT_LEN , 0);
	
	printf("\nCopy to Nand Flash... ");
	memset(&wr_opts, 0, sizeof(wr_opts));
	wr_opts.buffer	= (u_char*) load_addr;
	wr_opts.length	= CFG_NBOOT_LEN;
	wr_opts.offset	= CFG_NBOOT_BASE;
	/* opts.forcejffs2 = 1; */
	wr_opts.pad	= 1;
	wr_opts.blockalign = 1;
	wr_opts.quiet      = 1;
	ret = nand_write_opts(nand, &wr_opts);
	/* ret = nand_rw(nand_dev_desc + 0,
				  NANDRW_WRITE | NANDRW_JFFS2, CFG_NBOOT_BASE, CFG_NBOOT_LEN,
			      &total, (u_char*)0x100000);
	*/
  	if (ret)
		printf("Error - NAND burn faild!\n");
	else
		printf("\ndone\n");	

	return 1;
}

U_BOOT_CMD(
        nbubt,      2,     1,      nand_burn_nboot_cmd,
        "nbubt	- Burn a boot loader image on the Boot Nand Flash.\n",
        " file-name \n"
        "\tBurn a binary boot loader image on the Boot Nand Flash, default file-name is nboot.bin .\n"
);

#else
/* Boot from Nor flash */
int nor_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int filesize;
	MV_U32 s_first,s_end,env_sec;
	extern char console_buffer[];


	s_first = flash_in_which_sec(&flash_info[BOOT_FLASH_INDEX], CFG_MONITOR_BASE);
	s_end = flash_in_which_sec(&flash_info[BOOT_FLASH_INDEX], CFG_MONITOR_BASE + CFG_MONITOR_LEN -1);

	env_sec = flash_in_which_sec(&flash_info[BOOT_FLASH_INDEX], CFG_ENV_ADDR);


	load_addr = CFG_LOAD_ADDR; 
	if(argc == 2) {
		copy_filename (BootFile, argv[1], sizeof(BootFile));
	}
	else { 
		copy_filename (BootFile, "u-boot.bin", sizeof(BootFile));
		printf("using default file \"u-boot.bin\" \n");
	}
 
	if ((filesize = NetLoop(TFTP)) < 0)
		return 0;
 
#ifdef MV_BOOTROM
    	BHR_t*           	tmpBHR = (BHR_t*)load_addr;
	ExtBHR_t*   		extBHR = (ExtBHR_t*)(load_addr + BHR_HDR_SIZE);
	MV_U32			errCode=0;

	/* Verify Main header checksum */
	errCode = verify_main_header(tmpBHR, IBR_HDR_SPI_ID);
	if (errCode) 
		return 0;

	/* Verify that the extended header is valid */
	errCode = verify_extheader(extBHR);
	if (errCode) 
		return 0;
#endif

	printf("Un-Protect Flash Monitor space\n");
	flash_protect (FLAG_PROTECT_CLEAR,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + CFG_MONITOR_LEN - 1,
		       &flash_info[BOOT_FLASH_INDEX]);

	printf("\n**Warning**\n");
	printf("If U-Boot Endiannes is going to change (LE->BE or BE->LE), Then Env parameters should be overriden..\n");
	printf("Override Env parameters? (y/n)");
	readline(" ");
	if( strcmp(console_buffer,"Y") == 0 || 
	    strcmp(console_buffer,"yes") == 0 ||
	    strcmp(console_buffer,"y") == 0 ) {

		flash_protect (FLAG_PROTECT_CLEAR,
				   flash_info[BOOT_FLASH_INDEX].start[env_sec],
				   flash_info[BOOT_FLASH_INDEX].start[env_sec] + CFG_ENV_SECT_SIZE - 1,
				   &flash_info[BOOT_FLASH_INDEX]);

		printf("Erase Env parameters sector %d... ",env_sec);
		flash_erase(&flash_info[BOOT_FLASH_INDEX], env_sec, env_sec);
	
		if ((mvCtrlModelGet() != MV_6082_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6183_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6183L_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6281_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6192_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6190_DEV_ID) &&
		    (mvCtrlModelGet() != MV_6180_DEV_ID))
		flash_protect (FLAG_PROTECT_SET,
				   flash_info[BOOT_FLASH_INDEX].start[env_sec],
				   flash_info[BOOT_FLASH_INDEX].start[env_sec] + CFG_ENV_SECT_SIZE - 1,
				   &flash_info[BOOT_FLASH_INDEX]);

	}

	printf("Erase %d - %d sectors... ",s_first,s_end);
	flash_erase(&flash_info[BOOT_FLASH_INDEX], s_first, s_end);

	printf("Copy to Flash... ");

	flash_write ( (uchar *)(CFG_LOAD_ADDR + CFG_MONITOR_IMAGE_OFFSET),
				  (ulong)CFG_MONITOR_BASE,
				  (ulong)(filesize - CFG_MONITOR_IMAGE_OFFSET));

	printf("done\nProtect Flash Monitor space\n");
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + CFG_MONITOR_LEN - 1,
		       &flash_info[BOOT_FLASH_INDEX]);

	return 1;
}

U_BOOT_CMD(
        bubt,      2,     1,      nor_burn_uboot_cmd,
        "bubt	- Burn an image on the Boot Flash.\n",
        " file-name \n"
        "\tBurn a binary image on the Boot Flash, default file-name is u-boot.bin .\n"
);
#endif /* defined(CFG_NAND_BOOT) */
#endif /* (CONFIG_COMMANDS & CFG_CMD_NET) */



/*******************************************************************************
Reset environment variables.
********************************************************************************/
extern flash_info_t flash_info[];       /* info for FLASH chips */
int resetenv_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CFG_NAND_BOOT)
	printf("Erase Env parameters offset 0x%x... ",CFG_ENV_OFFSET);
	nand_info_t *nand = &nand_info[0];
	nand_erase_options_t er_opts;
	memset(&er_opts, 0, sizeof(er_opts));
	er_opts.offset = CFG_ENV_OFFSET;
	er_opts.length = CFG_ENV_SECT_SIZE;
	er_opts.quiet  = 1;

	nand_erase_opts(nand, &er_opts);
	//nand_erase(nand_dev_desc + 0, CFG_ENV_OFFSET, CFG_ENV_SECT_SIZE, 0);
	printf("done");
#else
	MV_U32 env_sec = flash_in_which_sec(&flash_info[0], CFG_ENV_ADDR);
	
	if (env_sec == -1)
	{
		printf("Could not find ENV Sector\n");
		return 0;
	}

	printf("Un-Protect ENV Sector\n");

	flash_protect(FLAG_PROTECT_CLEAR,
				  CFG_ENV_ADDR,CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
				  &flash_info[0]);

	
	printf("Erase sector %d ... ",env_sec);
	flash_erase(&flash_info[0], env_sec, env_sec);
	printf("done\nProtect ENV Sector\n");
	
	flash_protect(FLAG_PROTECT_SET,
				  CFG_ENV_ADDR,CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
				  &flash_info[0]);

#endif /* defined(CFG_NAND_BOOT) */
	printf("\nWarning: Default Environment Variables will take effect Only after RESET \n\n");
	return 1;
}

U_BOOT_CMD(
        resetenv,      1,     1,      resetenv_cmd,
        "resetenv	- Return all environment variable to default.\n",
        " \n"
        "\t Erase the environemnt variable sector.\n"
);

#endif /* #if (CONFIG_COMMANDS & CFG_CMD_FLASH) */
#if CONFIG_COMMANDS & CFG_CMD_BSP

/******************************************************************************
* Category     - General
* Functionality- The commands allows the user to view the contents of the MV
*                internal registers and modify them.
* Need modifications (Yes/No) - no
*****************************************************************************/
int ir_cmd( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	MV_U32 regNum = 0x0, regVal, regValTmp, res;
	MV_8 regValBin[40];
	MV_8 cmd[40];
	int i,j = 0, flagm = 0;
	extern MV_8 console_buffer[];

	if( argc == 2 ) {
		regNum = simple_strtoul( argv[1], NULL, 16 );
	}
	else { 
		printf( "Usage:\n%s\n", cmdtp->usage );
		return 0;
	}                                                                                                        

	regVal = MV_REG_READ( regNum );
	regValTmp = regVal;
	printf( "Internal register 0x%x value : 0x%x\n ",regNum, regVal );
	printf( "\n    31      24        16         8         0" );
	printf( "\n     |       |         |         |         |\nOLD: " );

	for( i = 31 ; i >= 0 ; i-- ) {
		if( regValTmp > 0 ) {
			res = regValTmp % 2;
			regValTmp = (regValTmp - res) / 2;
			if( res == 0 )
				regValBin[i] = '0';
			else
				regValBin[i] = '1';
		}
		else
			regValBin[i] = '0';
	}

	for( i = 0 ; i < 32 ; i++ ) {
		printf( "%c", regValBin[i] );
		if( (((i+1) % 4) == 0) && (i > 1) && (i < 31) )
			printf( "-" );
	}

	readline( "\nNEW: " );
	strcpy(cmd, console_buffer);
	if( (cmd[0] == '0') && (cmd[1] == 'x') ) {
		regVal = simple_strtoul( cmd, NULL, 16 );
		flagm=1;
	}
	else {
		for( i = 0 ; i < 40 ; i++ ) {
			if(cmd[i] == '\0')
				break;
			if( i == 4 || i == 9 || i == 14 || i == 19 || i == 24 || i == 29 || i == 34 )
				continue;
			if( cmd[i] == '1' ) {
				regVal = regVal | (0x80000000 >> j);
				flagm = 1;
			}
			else if( cmd[i] == '0' ) {
				regVal = regVal & (~(0x80000000 >> j));
				flagm = 1;
			}
			j++;
		}
	}

	if( flagm == 1 ) {
		MV_REG_WRITE( regNum, regVal );
		printf( "\nNew value = 0x%x\n\n", MV_REG_READ(regNum) );
	}
	return 1;
}

U_BOOT_CMD(
	ir,      2,     1,      ir_cmd,
	"ir	- reading and changing MV internal register values.\n",
	" address\n"
	"\tDisplays the contents of the internal register in 2 forms, hex and binary.\n"
	"\tIt's possible to change the value by writing a hex value beginning with \n"
	"\t0x or by writing 0 or 1 in the required place. \n"
    	"\tPressing enter without any value keeps the value unchanged.\n"
);

/******************************************************************************
* Category     - General
* Functionality- Display the auto detect values of the TCLK and SYSCLK.
* Need modifications (Yes/No) - no
*****************************************************************************/
int clk_cmd( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    	printf( "TCLK %dMhz, SYSCLK %dMhz (UART baudrate %d)\n",
		mvTclkGet()/1000000, mvSysClkGet()/1000000, CONFIG_BAUDRATE);
	return 1;
}

U_BOOT_CMD(
	dclk,      1,     1,      clk_cmd,
	"dclk	- Display the MV device CLKs.\n",
	" \n"
	"\tDisplay the auto detect values of the TCLK and SYSCLK.\n"
);

/******************************************************************************
* Functional only when using Lauterbach to load image into DRAM
* Category     - DEBUG
* Functionality- Display the array of registers the u-boot write to.
*
*****************************************************************************/
#if defined(REG_DEBUG)
int reg_arry[REG_ARRAY_SIZE][2];
int reg_arry_index = 0;
int print_registers( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	printf("Register display\n");

	for (i=0; i < reg_arry_index; i++)
		printf("Index %d 0x%x=0x%08x\n", i, (reg_arry[i][0] & 0x000fffff), reg_arry[i][1]);
	
	/* Print DRAM registers */	
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1500, MV_REG_READ(0x1500));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1504, MV_REG_READ(0x1504));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1508, MV_REG_READ(0x1508));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x150c, MV_REG_READ(0x150c));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1510, MV_REG_READ(0x1510));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1514, MV_REG_READ(0x1514));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1518, MV_REG_READ(0x1518));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x151c, MV_REG_READ(0x151c));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1400, MV_REG_READ(0x1400));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1404, MV_REG_READ(0x1404));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1408, MV_REG_READ(0x1408));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x140c, MV_REG_READ(0x140c));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1410, MV_REG_READ(0x1410));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x141c, MV_REG_READ(0x141c));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1420, MV_REG_READ(0x1420));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1424, MV_REG_READ(0x1424));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1428, MV_REG_READ(0x1428));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x147c, MV_REG_READ(0x147c));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1494, MV_REG_READ(0x1494));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x1498, MV_REG_READ(0x1498));
	printf("Index %d 0x%x=0x%08x\n", i++, 0x149c, MV_REG_READ(0x149c));

	printf("Number of Reg %d \n", i);

	return 1;
}

U_BOOT_CMD(
	printreg,      1,     1,      print_registers,
	"printreg	- Display the register array the u-boot write to.\n",
	" \n"
	"\tDisplay the register array the u-boot write to.\n"
);
#endif

#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)
/******************************************************************************
* Category     - Etherent
* Functionality- Display PHY ports status (using SMI access).
* Need modifications (Yes/No) - No
*****************************************************************************/
int sg_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(MV_INC_BOARD_QD_SWITCH)
		printf( "Switch status not supported\n");
#else
	MV_U32 port;
	for( port = 0 ; port < mvCtrlEthMaxPortGet(); port++ ) {

		printf( "PHY %d :\n", port );
		printf( "---------\n" );

		mvEthPhyPrintStatus( mvBoardPhyAddrGet(port) );

		printf("\n");
	}
#endif
	return 1;
}

U_BOOT_CMD(
	sg,      1,     1,      sg_cmd,
	"sg	- scanning the PHYs status\n",
	" \n"
	"\tScan all the Gig port PHYs and display their Duplex, Link, Speed and AN status.\n"
);
#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */

#if defined(MV_INCLUDE_IDMA)

/******************************************************************************
* Category     - DMA
* Functionality- Perform a DMA transaction
* Need modifications (Yes/No) - No
*****************************************************************************/
int mvDma_cmd( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	MV_8 cmd[20], c;
	extern MV_8 console_buffer[];
	MV_U32 chan, src, dst, byteCount, ctrlLo;
	MV_DMA_DEC_WIN win;
	MV_BOOL err;

	/* IDMA channel */
	if( argc == 2 ) 
		chan = simple_strtoul( argv[1], NULL, 16 );
	else
		chan = 0;

	/* source address */
	while(1) {
		readline( "Source Address: " );
		strcpy( cmd, console_buffer );
		src = simple_strtoul( cmd, NULL, 16 );
		if( src == 0xffffffff ) printf( "Bad address !!!\n" );
		else break;
	}

	/* desctination address */
	while(1) {
		readline( "Destination Address: " );
		strcpy(cmd, console_buffer);
		dst = simple_strtoul( cmd, NULL, 16 );
		if( dst == 0xffffffff ) printf("Bad address !!!\n");
		else break;
	}

	/* byte count */
	while(1) {
		readline( "Byte Count (up to 16M (0xffffff-1)): " );
		strcpy( cmd, console_buffer );
		byteCount = simple_strtoul( cmd, NULL, 16 );
		if( (byteCount > 0xffffff) || (byteCount == 0) ) printf("Bad value !!!\n");
		else break;
	}

	/* compose the command */
	ctrlLo = ICCLR_BLOCK_MODE | ICCLR_NON_CHAIN_MODE | ICCLR_SRC_INC | ICCLR_DST_INC;


	if (byteCount > _64K)
	{
		ctrlLo |= ICCLR_DESC_MODE_16M;
	}

	/* set data transfer limit */
	while(1) {
		printf( "Data transfer limit:\n" );
		printf( "(1) 8   bytes at a time.\n" );
		printf( "(2) 16  bytes at a time.\n" );
		printf( "(3) 32  bytes at a time.\n" );
		printf( "(4) 64  bytes at a time.\n" );
		printf( "(5) 128 bytes at a time.\n" );

		c = getc(); 
		printf( "%c\n", c );

		err = MV_FALSE;

		switch( c ) {
			case 13: /* Enter */
				ctrlLo |= (ICCLR_DST_BURST_LIM_32BYTE | ICCLR_SRC_BURST_LIM_32BYTE);
				printf( "32 bytes at a time.\n" );
				break;
			case '1':
				ctrlLo |= (ICCLR_DST_BURST_LIM_8BYTE | ICCLR_SRC_BURST_LIM_8BYTE);
				break;
			case '2':
				ctrlLo |= (ICCLR_DST_BURST_LIM_16BYTE | ICCLR_SRC_BURST_LIM_16BYTE);
				break;
			case '3':
				ctrlLo |= (ICCLR_DST_BURST_LIM_32BYTE | ICCLR_SRC_BURST_LIM_32BYTE);
				break;
			case '4':
				ctrlLo |= (ICCLR_DST_BURST_LIM_64BYTE | ICCLR_SRC_BURST_LIM_64BYTE);
				break;
			case '5':
				ctrlLo |= (ICCLR_DST_BURST_LIM_128BYTE | ICCLR_SRC_BURST_LIM_128BYTE);
				break;
			default:
				printf( "Bad value !!!\n" );
				err = MV_TRUE;
		}

		if( !err ) break;
	}

	/* set ovveride source option */
	while(1) {
		printf( "Override Source:\n" ); 
		printf( "(0) - no override\n" );
		mvDmaWinGet( 1, &win );
		printf( "(1) - use Win1 (%s)\n",mvCtrlTargetNameGet(win.target));
		mvDmaWinGet( 2, &win );
		printf( "(2) - use Win2 (%s)\n",mvCtrlTargetNameGet(win.target));
		mvDmaWinGet( 3, &win );
		printf( "(3) - use Win3 (%s)\n",mvCtrlTargetNameGet(win.target));

		c = getc(); 
		printf( "%c\n", c );

		err = MV_FALSE;

		switch( c ) {
			case 13: /* Enter */
			case '0':
				printf( "No override\n" );
				break;
			case '1':
				ctrlLo |= ICCLR_OVRRD_SRC_BAR(1);
				break;
			case '2':
				ctrlLo |= ICCLR_OVRRD_SRC_BAR(2);
				break;
			case '3':
				ctrlLo |= ICCLR_OVRRD_SRC_BAR(3);
				break;
			default:
				printf("Bad value !!!\n");
				err = MV_TRUE;
		}

		if( !err ) break;
	}

	/* set override destination option */
	while(1) {
		printf( "Override Destination:\n" ); 
		printf( "(0) - no override\n" );
		mvDmaWinGet( 1, &win );
		printf( "(1) - use Win1 (%s)\n",mvCtrlTargetNameGet(win.target));
		mvDmaWinGet( 2, &win );
		printf( "(2) - use Win2 (%s)\n",mvCtrlTargetNameGet(win.target));
		mvDmaWinGet( 3, &win );
		printf( "(3) - use Win3 (%s)\n",mvCtrlTargetNameGet(win.target));

		c = getc(); 
		printf( "%c\n", c );

		err = MV_FALSE;

	        switch( c ) {
			case 13: /* Enter */
			case '0':
				printf( "No override\n" );
				break;
			case '1':
				ctrlLo |= ICCLR_OVRRD_DST_BAR(1);
				break;
			case '2':
				ctrlLo |= ICCLR_OVRRD_DST_BAR(2);
				break;
			case '3':
				ctrlLo |= ICCLR_OVRRD_DST_BAR(3);
				break;
			default:
				printf("Bad value !!!\n");
				err = MV_TRUE;
		}

		if( !err ) break;
	}

	/* wait for previous transfer completion */
	while( mvDmaStateGet(chan) != MV_IDLE );

	/* issue the transfer */
	mvDmaCtrlLowSet( chan, ctrlLo ); 
	mvDmaTransfer( chan, src, dst, byteCount, 0 );

	/* wait for completion */
	while( mvDmaStateGet(chan) != MV_IDLE );

	printf( "Done...\n" );
	return 1;
}

U_BOOT_CMD(
	dma,      2,     1,      mvDma_cmd,
	"dma	- Perform DMA\n",
	" \n"
	"\tPerform DMA transaction with the parameters given by the user.\n"
);

#endif /* #if defined(MV_INCLUDE_IDMA) */

/******************************************************************************
* Category     - Memory
* Functionality- Displays the MV's Memory map
* Need modifications (Yes/No) - Yes
*****************************************************************************/
int displayMemoryMap_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	mvCtrlAddrDecShow();
	return 1;
}

U_BOOT_CMD(
	map,      1,     1,      displayMemoryMap_cmd,
	"map	- Diasplay address decode windows\n",
	" \n"
	"\tDisplay controller address decode windows: CPU, PCI, Gig, DMA, XOR and COMM\n"
);



#include "ddr2/spd/mvSpd.h"
#if defined(MV_INC_BOARD_DDIM)

/******************************************************************************
* Category     - Memory
* Functionality- Displays the SPD information for a givven dimm
* Need modifications (Yes/No) - 
*****************************************************************************/
              
int dimminfo_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        int num = 0;
 
        if (argc > 1) {
                num = simple_strtoul (argv[1], NULL, 10);
        }
 
        printf("*********************** DIMM%d *****************************\n",num);
 
        dimmSpdPrint(num);
 
        printf("************************************************************\n");
         
        return 1;
}
 
U_BOOT_CMD(
        ddimm,      2,     1,      dimminfo_cmd,
        "ddimm  - Display SPD Dimm Info\n",
        " [0/1]\n"
        "\tDisplay Dimm 0/1 SPD information.\n"
);

/******************************************************************************
* Category     - Memory
* Functionality- Copy the SPD information of dimm 0 to dimm 1
* Need modifications (Yes/No) - 
*****************************************************************************/
              
int spdcpy_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
 
        printf("Copy DIMM 0 SPD data into DIMM 1 SPD...");
 
        if (MV_OK != dimmSpdCpy())
        	printf("\nDIMM SPD copy fail!\n");
 	else
        	printf("Done\n");
         
        return 1;
}
 
U_BOOT_CMD(
        spdcpy,      2,     1,      spdcpy_cmd,
        "spdcpy  - Copy Dimm 0 SPD to Dimm 1 SPD \n",
        ""
        ""
);
#endif /* #if defined(MV_INC_BOARD_DDIM) */

/******************************************************************************
* Functionality- Go to an address and execute the code there and return,
*    defualt address is 0x40004
*****************************************************************************/
extern void cpu_dcache_flush_all(void);
extern void cpu_icache_flush_invalidate_all(void);

void mv_go(unsigned long addr,int argc, char *argv[])
{
	int rc;
	addr = MV_CACHEABLE(addr);
	char* envCacheMode = getenv("cacheMode");
 
	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */

    if(envCacheMode && (strcmp(envCacheMode,"write-through") == 0))
	{	
		int i=0;

		/* Flush Invalidate I-Cache */
		cpu_icache_flush_invalidate_all();

		/* Drain write buffer */
		asm ("mcr p15, 0, %0, c7, c10, 4": :"r" (i));
		

	}
	else /*"write-back"*/
	{
		int i=0;

		/* Flush Invalidate I-Cache */
		cpu_icache_flush_invalidate_all();

		/* Drain write buffer */
		asm ("mcr p15, 0, %0, c7, c10, 4": :"r" (i));
		

		/* Flush invalidate D-cache */
		cpu_dcache_flush_all();


    }


	rc = ((ulong (*)(int, char *[]))addr) (--argc, &argv[1]);
 
        return;
}

int g_cmd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        ulong   addr;

	if(!enaMonExt()){
		printf("This command can be used only if enaMonExt is set!\n");
		return 0;
	}

	addr = 0x40000;

        if (argc > 1) {
		addr = simple_strtoul(argv[1], NULL, 16);
        }
	mv_go(addr,argc,&argv[0]);
	return 1;
}                                                                                                                     

U_BOOT_CMD(
	g,      CFG_MAXARGS,     1,      g_cmd,
        "g	- start application at cached address 'addr'(default addr 0x40000)\n",
        " addr [arg ...] \n"
	"\tStart application at address 'addr'cachable!!!(default addr 0x40004/0x240004)\n"
	"\tpassing 'arg' as arguments\n"
	"\t(This command can be used only if enaMonExt is set!)\n"
);

/******************************************************************************
* Functionality- Searches for a value
*****************************************************************************/
int fi_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    MV_U32 s_address,e_address,value,i,tempData;
    MV_BOOL  error = MV_FALSE;

    if(!enaMonExt()){
	printf("This command can be used only if enaMonExt is set!\n");
	return 0;}

    if(argc == 4){
	value = simple_strtoul(argv[1], NULL, 16);
	s_address = simple_strtoul(argv[2], NULL, 16);
	e_address = simple_strtoul(argv[3], NULL, 16);
    }else{ printf ("Usage:\n%s\n", cmdtp->usage);
    	return 0;
    }     

    if(s_address == 0xffffffff || e_address == 0xffffffff) error = MV_TRUE;
    if(s_address%4 != 0 || e_address%4 != 0) error = MV_TRUE;
    if(s_address > e_address) error = MV_TRUE;
    if(error)
    {
	printf ("Usage:\n%s\n", cmdtp->usage);
        return 0;
    }
    for(i = s_address; i < e_address ; i+=4)
    {
        tempData = (*((volatile unsigned int *)i));
        if(tempData == value)
        {
            printf("Value: %x found at ",value);
            printf("address: %x\n",i);
            return 1;
        }
    }
    printf("Value not found!!\n");
    return 1;
}

U_BOOT_CMD(
	fi,      4,     1,      fi_cmd,
	"fi	- Find value in the memory.\n",
	" value start_address end_address\n"
	"\tSearch for a value 'value' in the memory from address 'start_address to\n"
	"\taddress 'end_address'.\n"
	"\t(This command can be used only if enaMonExt is set!)\n"
);

/******************************************************************************
* Functionality- Compare the memory with Value.
*****************************************************************************/
int cmpm_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    MV_U32 s_address,e_address,value,i,tempData;
    MV_BOOL  error = MV_FALSE;

    if(!enaMonExt()){
	printf("This command can be used only if enaMonExt is set!\n");
	return 0;}

    if(argc == 4){
	value = simple_strtoul(argv[1], NULL, 16);
	s_address = simple_strtoul(argv[2], NULL, 16);
	e_address = simple_strtoul(argv[3], NULL, 16);
    }else{ printf ("Usage:\n%s\n", cmdtp->usage);
    	return 0;
    }     

    if(s_address == 0xffffffff || e_address == 0xffffffff) error = MV_TRUE;
    if(s_address%4 != 0 || e_address%4 != 0) error = MV_TRUE;
    if(s_address > e_address) error = MV_TRUE;
    if(error)
    {
	printf ("Usage:\n%s\n", cmdtp->usage);
        return 0;
    }
    for(i = s_address; i < e_address ; i+=4)
    {
        tempData = (*((volatile unsigned int *)i));
        if(tempData != value)
        {
            printf("Value: %x found at address: %x\n",tempData,i);
        }
    }
    return 1;
}

U_BOOT_CMD(
	cmpm,      4,     1,      cmpm_cmd,
	"cmpm	- Compare Memory\n",
	" value start_address end_address.\n"
	"\tCompare the memory from address 'start_address to address 'end_address'.\n"
	"\twith value 'value'\n"
	"\t(This command can be used only if enaMonExt is set!)\n"
);



#if 0
/******************************************************************************
* Category     - Etherent
* Functionality- Display PHY ports status (using SMI access).
* Need modifications (Yes/No) - No
*****************************************************************************/
int eth_show_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ethRegs(argv[1]);
	ethPortRegs(argv[1]);
	ethPortStatus(argv[1]);
	ethPortQueues(argv[1],0,0,1);
	return 1;
}

U_BOOT_CMD(
	ethShow,      2,    2,      eth_show_cmd,
	"ethShow	- scanning the PHYs status\n",
	" \n"
	"\tScan all the Gig port PHYs and display their Duplex, Link, Speed and AN status.\n"
);
#endif

#if defined(MV_INCLUDE_PEX)

#include "pci/mvPci.h"

int pcie_phy_read_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    	MV_U16 phyReg;

    	mvPexPhyRegRead(simple_strtoul( argv[1], NULL, 16 ),
	                simple_strtoul( argv[2], NULL, 16), &phyReg);

	printf ("0x%x\n", phyReg);

	return 1;
}

U_BOOT_CMD(
	pciePhyRead,      3,     3,      pcie_phy_read_cmd,
	"phyRead	- Read PCI-E Phy register\n",
	" PCI-E_interface Phy_offset. \n"
	"\tRead the PCI-E Phy register. \n"
);


int pcie_phy_write_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	mvPexPhyRegWrite(simple_strtoul( argv[1], NULL, 16 ),
					 simple_strtoul( argv[2], NULL, 16 ),
					 simple_strtoul( argv[3], NULL, 16 ));

	return 1;
}

U_BOOT_CMD(
	pciePhyWrite,      4,     4,      pcie_phy_write_cmd,
	"pciePhyWrite	- Write PCI-E Phy register\n",
	" PCI-E_interface Phy_offset value.\n"
	"\tWrite to the PCI-E Phy register.\n"
);

#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */
#if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH)

#include "eth-phy/mvEthPhy.h"

int phy_read_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    	MV_U16 phyReg;

    	mvEthPhyRegRead(simple_strtoul( argv[1], NULL, 16 ),
	                simple_strtoul( argv[2], NULL, 16), &phyReg);

	printf ("0x%x\n", phyReg);

	return 1;
}

U_BOOT_CMD(
	phyRead,      3,     3,      phy_read_cmd,
	"phyRead	- Read Phy register\n",
	" Phy_address Phy_offset. \n"
	"\tRead the Phy register. \n"
);


int phy_write_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	mvEthPhyRegWrite(simple_strtoul( argv[1], NULL, 16 ),
					 simple_strtoul( argv[2], NULL, 16 ),
					 simple_strtoul( argv[3], NULL, 16 ));

	return 1;
}

U_BOOT_CMD(
	phyWrite,      4,     4,      phy_write_cmd,
	"phyWrite	- Write Phy register\n",
	" Phy_address Phy_offset value.\n"
	"\tWrite to the Phy register.\n"
);

#endif /* #if defined(MV_INCLUDE_UNM_ETH) || defined(MV_INCLUDE_GIG_ETH) */

#endif /* MV_TINY */

int _4BitSwapArry[] = {0,8,4,0xc,2,0xa,6,0xe,1,9,5,0xd,3,0xb,7,0xf};
int _3BitSwapArry[] = {0,4,2,6,1,5,3,7};

int do_satr(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd, *s;
	MV_U8 data0=0, data1=0, devNum0=0, devNum1=0;
	MV_U8 moreThenOneDev=0, regNum = 0;
	MV_U8 mask0=0, mask1=0, shift0=0, shift1=0;
	MV_U8 val=0, width=0;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strncmp(cmd, "read", 4) != 0 && strncmp(cmd, "write", 5) != 0)
		goto usage;

	/* read write */
	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		int read;


		read = strncmp(cmd, "read", 4) == 0; /* 1 = read, 0 = write */

		/* In write mode we have additional value */
		if (!read)
		{
			if (argc < 3)
				goto usage;
			else
				/* Value for write */
				val = (ulong)simple_strtoul(argv[2], NULL, 16);
		}

		printf("\nS@R %s: ", read ? "read" : "write");
		s = strchr(cmd, '.');
		if ((s != NULL) && (strcmp(s, ".cpu") == 0))
		{
#ifdef DB_88F6180A
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 0;
			mask0 = 0x7;
			shift0 = 0;
			mask1 = 0x0;
			shift1 = 0;
			width = 3;
#else
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 0;
			mask0 = 0xf;
			shift0 = 0;
			mask1 = 0x0;
			shift1 = 0;
			width = 4;
#endif
		}

		if ((s != NULL) && (strcmp(s, ".cpu2ddr") == 0))
		{
			moreThenOneDev = 1;
			regNum = 0;
			devNum0 = 0;
			devNum1 = 1;
			mask0 = 0x10;
			shift0 = 4;
			mask1 = 0x7;
			shift1 = 1;
			width = 4;
		}

		if ((s != NULL) && (strcmp(s, ".cpu2L2") == 0))
		{
			moreThenOneDev = 1;
			regNum = 0;
			devNum0 = 1;
			devNum1 = 2;
			mask0 = 0x18;
			shift0 = 3;
			mask1 = 0x1;
			shift1 = 2;
			width = 3;
		}

		if ((s != NULL) && (strcmp(s, ".SSCG") == 0)) 
		{
#ifdef DB_88F6180A
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 0;
			mask0 = 0x8;
			shift0 = 3;
			mask1 = 0x0;
			shift1 = 0;
#else
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 2;
			mask0 = 0x4;
			shift0 = 2;
			mask1 = 0x0;
			shift1 = 0;
#endif
		}

		if ((s != NULL) && (strcmp(s, ".PEXCLK") == 0)) 
		{
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 2;
			mask0 = 0x10;
			shift0 = 4;
			mask1 = 0x0;
			shift1 = 0;
		}

		if ((s != NULL) && (strcmp(s, ".MPP18") == 0)) 
		{
			moreThenOneDev = 0;
			regNum = 0;
			devNum0 = 2;
			mask0 = 0x8;
			shift0 = 3;
			mask1 = 0x0;
			shift1 = 0;
		}
			
		if (read) {
			/* read */
			data0 = mvBoarTwsiSatRGet(devNum0, regNum);
			if (moreThenOneDev)
				data1 = mvBoarTwsiSatRGet(devNum1, regNum);

			data0 = ((data0 & mask0) >> shift0);

			if (moreThenOneDev)
			{
				data1 = ((data1 & mask1) << shift1);
				data0 |= data1;
			}
			
			/* Swap value */
			switch(width)
			{
				case 4:
					data0 = _4BitSwapArry[data0];
					break;
				case 3:
					data0 = _3BitSwapArry[data0];
					break;
				case 2:
					data0 = (((data0 & 0x1) << 0x1) | ((data0 & 0x2) >> 0x1));
					break;
			}

			printf("Read S@R val %x\n", data0);

		} else {

			/* Swap value */
			switch(width)
			{
				case 4:
					val = _4BitSwapArry[val];
					break;
				case 3:
					val = _3BitSwapArry[val];
					break;
				case 2:
					val = (((val & 0x1) << 0x1) | ((val & 0x2) >> 0x1));
					break;
			}

			/* read modify write */
			data0 = mvBoarTwsiSatRGet(devNum0, regNum);
			data0 = (data0 & ~mask0);
			data0 |= ((val << shift0) & mask0);
			if (mvBoarTwsiSatRSet(devNum0, regNum, data0) != MV_OK)
			{
				printf("Write S@R first device val %x fail\n", data0);
				return 1;
			}
			printf("Write S@R first device val %x succeded\n", data0);

			if (moreThenOneDev)
			{
				data1 = mvBoarTwsiSatRGet(devNum1, regNum);
				data1 = (data1 & ~mask1);
				data1 |= ((val >> shift1) & mask1);
				if (mvBoarTwsiSatRSet(devNum1, regNum, data1) != MV_OK)
				{
					printf("Write S@R second device val %x fail\n", data1);
					return 1;
				}
				printf("Write S@R second device val %x succeded\n", data1);
			}
		}

		return 0;
	}

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

#ifdef DB_88F6180A
U_BOOT_CMD(SatR, 5, 1, do_satr,
	"SatR - sample at reset sub-system, relevent for DB only\n",
	"read.cpu 		- read cpu clock from S@R devices\n"
    "SatR read.SSCG		- read SSCG state from S@R devices [0 ~ en]\n"
	"SatR write.cpu val	- write cpu clock val to S@R devices [0,1,..,7]\n"
    "SatR write.SSCG val	- write SSCG state val to S@R devices [0 ~ en]\n"
);
#else
U_BOOT_CMD(SatR, 5, 1, do_satr,
	"SatR - sample at reset sub-system, relevent for DB only\n",
	"read.cpu 		- read cpu clock from S@R devices\n"
	"SatR read.cpu2ddr	- read cpu2ddr clock ratio from S@R devices\n"
	"SatR read.cpu2L2	- read cpu2L2 clock ratio from S@R devices\n"
	"SatR read.SSCG		- read SSCG state from S@R devices [0 ~ en]\n"
    "SatR read.PEXCLK   - read PCI-E clock state from S@R devices [0 ~ input]\n"
    "SatR read.MPP18	- reserved\n"
	"SatR write.cpu val	- write cpu clock val to S@R devices [0,1,..,F]\n"
	"SatR write.cpu2ddr val	- write cpu2ddr clock ratio val to S@R devices [0,1,..,F]\n"
	"SatR write.cpu2L2 val	- write cpu2L2 clock ratio val to S@R devices [0,1,..,7]\n"
	"SatR write.SSCG val	- write SSCG state val to S@R devices [0 ~ en]\n"
    "SatR write.PEXCLK	- write PCI-E clock state from S@R devices [0 ~ input]\n"
    "SatR write.MPP18	- reserved\n"
);
#endif

#if (CONFIG_COMMANDS & CFG_CMD_RCVR)
extern void recoveryHandle(void);
int do_rcvr (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	recoveryHandle();
	return 1;
}

U_BOOT_CMD(
	rcvr,	3,	1,	do_rcvr,
	"rcvr\t- Satrt recovery process (Distress Beacon with TFTP server)\n",
	"\n"
);
#endif	/* CFG_CMD_RCVR */
