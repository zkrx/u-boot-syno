#ifdef CONFIG_SYNO_MV88F6281
/*
 * Synology Kirkwood U-boot command
 *
 * Maintained by:  KueiHuan Chen <khchen@synology.com>
 *
 * Copyright 2009-2010 Synology, Inc.  All rights reserved.
 * Copyright 2009-2010 KueiHuan.Chen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <config.h>
#include <common.h>
#include <command.h>
#include <pci.h>

#include "mvTypes.h"
#include "mvCtrlEnvLib.h"
#include "command.h"
#include "eth-phy/mvEthPhy.h"

#include <uart/mvUart.h>

#define WAKE_ON_LAN_ENABLE  0x6C
#define SOFTWARE_SHUTDOWN   0x31
#define SOFTWARE_REBOOT     0x43
int WOLTest_cmd(cmd_tbl_t *cmdtp,
				int flag,
				int argc,
				char *argv[])
{
	int clock_divisor = (CFG_TCLK / 16)/9600;
	MV_U32 phy_addr = simple_strtoul( argv[1], NULL, 16 );
	MV_U16 phy_id_1, phy_id_2;

	if (argc != 2) {
		printf("Please give phy address\n");
		return -1;
	}

	mvEthPhyRegWrite(phy_addr, 0x16, 0x0);
	mvEthPhyRegRead(phy_addr, 0x2, &phy_id_1);
	mvEthPhyRegRead(phy_addr, 0x3, &phy_id_2);

	printf("Your phy chip 0x%x\n", (phy_id_1<<16 | phy_id_2));
	if (0x01410e90 != (phy_id_1<<16 | phy_id_2)) {
		printf("Your phy not support WOL\n");
		return 1;
	}

	printf("Use 00:50:43:8e:2c:09 as testing Mac Address\n");

	/* Link
	mvEthPhyRegWrite(phy_addr, 0x16, 0x0);
	mvEthPhyRegWrite(phy_addr, 0x12, 0x80);
	mvEthPhyRegWrite(phy_addr, 0x16, 0x3);
	mvEthPhyRegWrite(phy_addr, 0x12, 0x4985);
	mvEthPhyRegWrite(phy_addr, 0x16, 0x11);
	mvEthPhyRegWrite(phy_addr, 0x10, 0x2500);
	*/

	mvEthPhyRegWrite(phy_addr, 0x16, 0x0);
	mvEthPhyRegWrite(phy_addr, 0x12, 0x80);
	mvEthPhyRegWrite(phy_addr, 0x16, 0x3);
	mvEthPhyRegWrite(phy_addr, 0x12, 0x4985);
	mvEthPhyRegWrite(phy_addr, 0x16, 0x11);
	mvEthPhyRegWrite(phy_addr, 0x17, 0x092c);
	mvEthPhyRegWrite(phy_addr, 0x18, 0x8e43);
	mvEthPhyRegWrite(phy_addr, 0x19, 0x5000);
	mvEthPhyRegWrite(phy_addr, 0x10, 0x4500);
	mvEthPhyRegWrite(phy_addr, 0x16, 0x0);

	printf("Shutdown DS\n");


	mvUartInit(1, clock_divisor, mvUartBase(1));
	/* enable wol setting */
	mvUartPutc(1, WAKE_ON_LAN_ENABLE);
	/* shutdown */
	mvUartPutc(1, SOFTWARE_SHUTDOWN);

	return 1;
}

U_BOOT_CMD(
	WOLTest,      2,     2,      WOLTest_cmd,
	"WOLTest	- Wake On Lan u-boot testing\n",
	" Parameter: Phy_address\n"
	"\tuse 00:50:43:8e:2c:09 as default Mac address\n"
	"\tThis command will shutdown automatically\n"
);

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
/******************************************************************************
* Category     - Etherent
* Functionality- Display PHY ports status (using SMI access).
* Need modifications (Yes/No) - No
*****************************************************************************/
int sg_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	MV_U32 port;
	for( port = 0 ; port < mvCtrlEthMaxPortGet(); port++ ) {

		printf( "PHY %d  mvBoardPhyAddrGet(port) %d:\n", port, mvBoardPhyAddrGet(port) );
		printf( "---------\n" );

		mvEthPhyPrintStatus( mvBoardPhyAddrGet(port) );

		printf("\n");
	}

	return 1;
}

U_BOOT_CMD(
	sg,      1,     1,      sg_cmd,
	"sg	- scanning the PHYs status\n",
	" \n"
	"\tScan all the Gig port PHYs and display their Duplex, Link, Speed and AN status.\n"
);
#endif
