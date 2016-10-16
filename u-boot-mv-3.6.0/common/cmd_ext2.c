/*
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
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

/*
 * Ext2fs support
 */
#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_EXT2)
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <ext2fs.h>
#if ((CONFIG_COMMANDS & CFG_CMD_USB) && defined(CONFIG_USB_STORAGE))
#include <usb.h>
#endif
#ifdef SYNO_VENDOR
#include "rtc.h"
#endif /* SYNO_VENDOR */

#ifndef CONFIG_DOS_PARTITION
#error DOS partition support must be selected
#endif

/* #define	EXT2_DEBUG */

#ifdef	EXT2_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

static block_dev_desc_t *get_dev (char* ifname, int dev)
{
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
	if (strncmp(ifname,"ide",3)==0) {
		extern block_dev_desc_t * ide_get_dev(int dev);
		return((dev >= CFG_IDE_MAXDEVICE) ? NULL : ide_get_dev(dev));
	}
#endif
#if (CONFIG_COMMANDS & CFG_CMD_SCSI)
	if (strncmp(ifname,"scsi",4)==0) {
		extern block_dev_desc_t * scsi_get_dev(int dev);
		return((dev >= CFG_SCSI_MAXDEVICE) ? NULL : scsi_get_dev(dev));
	}
#endif
#if ((CONFIG_COMMANDS & CFG_CMD_USB) && defined(CONFIG_USB_STORAGE))
	if (strncmp(ifname,"usb",3)==0) {
		extern block_dev_desc_t * usb_stor_get_dev(int dev);
		return((dev >= USB_MAX_STOR_DEV) ? NULL : usb_stor_get_dev(dev));
	}
#endif
#if defined(CONFIG_MMC)
	if (strncmp(ifname,"mmc",3)==0) {
		extern block_dev_desc_t *  mmc_get_dev(int dev);
		return((dev >= 1) ? NULL : mmc_get_dev(dev));
	}
#endif
#if defined(CONFIG_SYSTEMACE)
	if (strcmp(ifname,"ace")==0) {
		extern block_dev_desc_t *  systemace_get_dev(int dev);
		return((dev >= 1) ? NULL : systemace_get_dev(dev));
	}
#endif
	return(NULL);
}

int do_ext2ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = "/";
	int dev=0;
	int part=1;
	char *ep;
	block_dev_desc_t *dev_desc=NULL;
	int part_length;

	if (argc < 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return(1);
	}
	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc=get_dev(argv[1],dev);

	if (dev_desc == NULL) {
		printf ("\n** Block device %s %d not supported\n", argv[1], dev);
		return(1);
	}

	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return(1);
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	if (argc == 4) {
	    filename = argv[3];
	}

	PRINTF("Using device %s %d:%d, directory: %s\n", argv[1], dev, part, filename);

	if ((part_length = ext2fs_set_blk_dev(dev_desc, part)) == 0) {
		printf ("** Bad partition - %s %d:%d **\n",  argv[1], dev, part);
		ext2fs_close();
		return(1);
	}

	if (!ext2fs_mount(part_length)) {
		printf ("** Bad ext2 partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		ext2fs_close();
		return(1);
	}

	if (ext2fs_ls (filename)) {
		printf ("** Error ext2fs_ls() **\n");
		ext2fs_close();
		return(1);
	};

	ext2fs_close();

	return(0);
}

U_BOOT_CMD(
	ext2ls,	4,	1,	do_ext2ls,
	"ext2ls  - list files in a directory (default /)\n",
	"<interface> <dev[:part]> [directory]\n"
	"    - list files from 'dev' on 'interface' in a 'directory'\n"
);

/******************************************************************************
 * Ext2fs boot command intepreter. Derived from diskboot
 */
int do_ext2load (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *filename = NULL;
	char *ep;
	int dev, part = 1;
	ulong addr = 0, part_length, filelen;
	disk_partition_t info;
	block_dev_desc_t *dev_desc = NULL;
	char buf [12];
	unsigned long count;
	char *addr_str;

	switch (argc) {
	case 3:
		addr_str = getenv("loadaddr");
		if (addr_str != NULL) {
			addr = simple_strtoul (addr_str, NULL, 16);
		} else {
			addr = CFG_LOAD_ADDR;
		}
		filename = getenv ("bootfile");
		count = 0;
		break;
	case 4:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = getenv ("bootfile");
		count = 0;
		break;
	case 5:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = argv[4];
		count = 0;
		break;
	case 6:
		addr = simple_strtoul (argv[3], NULL, 16);
		filename = argv[4];
		count = simple_strtoul (argv[5], NULL, 16);
		break;

	default:
		if (cmdtp)
			printf ("Usage:\n%s\n", cmdtp->usage);
		return(1);
	}

	if (!filename) {
		puts ("\n** No boot file defined **\n");
		return(1);
	}

	dev = (int)simple_strtoul (argv[2], &ep, 16);
	dev_desc=get_dev(argv[1],dev);
	if (dev_desc==NULL) {
		printf ("\n** Block device %s %d not supported\n", argv[1], dev);
		return(1);
	}
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return(1);
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	PRINTF("Using device %s%d, partition %d\n", argv[1], dev, part);

	if (part != 0) {
		if (get_partition_info (dev_desc, part, &info)) {
			printf ("** Bad partition %d **\n", part);
			return(1);
		}

		if (strncmp((char *)info.type, BOOT_PART_TYPE, sizeof(info.type)) != 0) {
			printf ("\n** Invalid partition type \"%.32s\""
				" (expect \"" BOOT_PART_TYPE "\")\n",
				info.type);
			return(1);
		}
		PRINTF ("\nLoading from block device %s device %d, partition %d: "
			"Name: %.32s  Type: %.32s  File:%s\n",
			argv[1], dev, part, info.name, info.type, filename);
	} else {
		PRINTF ("\nLoading from block device %s device %d, File:%s\n",
			argv[1], dev, filename);
	}


	if ((part_length = ext2fs_set_blk_dev(dev_desc, part)) == 0) {
		printf ("** Bad partition - %s %d:%d **\n",  argv[1], dev, part);
		ext2fs_close();
		return(1);
	}

	if (!ext2fs_mount(part_length)) {
		printf ("** Bad ext2 partition or disk - %s %d:%d **\n",  argv[1], dev, part);
		ext2fs_close();
		return(1);
	}

	filelen = ext2fs_open(filename);
	if (filelen < 0) {
		printf("** File not found %s\n", filename);
		ext2fs_close();
		return(1);
	}

	if ((count < filelen) && (count != 0)) {
	    filelen = count;
	}

	if (ext2fs_read((char *)addr, filelen) != filelen) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n", filename, argv[1], dev, part);
		ext2fs_close();
		return(1);
	}

	ext2fs_close();

	/* Loading ok, update default load address */
	load_addr = addr;

	printf ("\n%ld bytes read\n", filelen);
	sprintf(buf, "%lX", filelen);
	setenv("filesize", buf);

	return(filelen);
}

U_BOOT_CMD(
	ext2load,	6,	0,	do_ext2load,
	"ext2load- load binary file from a Ext2 filesystem\n",
	"<interface> <dev[:part]> [addr] [filename] [bytes]\n"
	"    - load binary file 'filename' from 'dev' on 'interface'\n"
	"      to address 'addr' from ext2 filesystem\n"
);

#endif	/* CONFIG_COMMANDS & CFG_CMD_EXT2 */


#if (CONFIG_COMMANDS & CFG_CMD_EXT2)

#ifdef SYNO_VENDOR
#include <malloc.h>
char szVender[1024];

#define OFF_MTD_VENDER_SN               32
#define OFF_MTD_VENDER_CUSTOM_SN        64
#define OFF_MTD_VENDER_TEST_FLAG        96

#define SN_LENGTH               10
#define CUSTOM_SN_LENGTH        31
#define TEST_FLAG_LENGTH        92

// mac address for synomanutil.c
typedef struct _tag_SYNO_MAC_ADDR_ {
                unsigned char   rgAddr[6];
                unsigned char   bChkSum;
} __attribute__((packed)) SYNO_MAC_ADDR, *PSYNO_MAC_ADDR;

// serial number for synomanutil.c
typedef struct _tag_SYNO_SN_ {
                char    rgSN[SN_LENGTH];
                unsigned char    bChkSum;
} __attribute__((packed)) SYNO_SN, *PSYNO_SN;

typedef struct _tag_SYNO_CUSTOM_SN_ {
                char    rgCustomSN[CUSTOM_SN_LENGTH];
                unsigned char    bChkSum;
} __attribute__((packed)) SYNO_CUSTOM_SN, *PSYNO_CUSTOM_SN;

typedef struct _tag_SYNO_TEST_FLAG_ {
                unsigned uiPTBurninMemoryCounter;   // P/T Burn-in Memory Test Time Counter
                unsigned uiPTBurninDMACounter;      // P/T Burn-in DMA Test Time Counter
                unsigned uiFTBurninCounter;         // F/T Burn-in DMA Test Time Counter
                char     rgPTFuncTestError[16];     // P/T Procomm Test Error Flag
                char     rgPTBurninError[16];       // P/T Burn-in Error Flag
                char     rgFTFuncTestError[32];     // F/T Test Error Flag
                char     rgFTBurninError[16];       // F/T Burn-in Error Flag
} __attribute__((packed)) SYNO_TEST_FLAG, *PSYNO_TEST_FLAG;

typedef struct _tag_SYNO_MTD_VENDER_ {
                SYNO_MAC_ADDR   macAddr1;
                SYNO_MAC_ADDR   macAddr2;
} __attribute__((packed)) SYNO_MTD_VENDER, *PSYNO_MTD_VENDER;

typedef struct _tag_SYNO_MTD_VENDER_SN_ {
                SYNO_SN serialNumber;
} __attribute__((packed)) SYNO_MTD_VENDER_SN, *PSYNO_MTD_VENDER_SN;

typedef struct _tag_SYNO_MTD_VENDER_CUSTOM_SN_ {
                SYNO_CUSTOM_SN serialCustomNumber;
} __attribute__((packed)) SYNO_MTD_VENDER_CUSTOM_SN, *PSYNO_MTD_VENDER_CUSTOM_SN;

typedef struct _tag_SYNO_MTD_VENDER_TEST_FLAG_ {
                SYNO_TEST_FLAG testFlag;
} __attribute__((packed)) SYNO_MTD_VENDER_TEST_FLAG, *PSYNO_MTD_VENDER_TEST_FLAG;

typedef struct _tag_SYNO_MTD_VENDER_ALL_ {
                SYNO_MTD_VENDER stMac;
                unsigned char Pad1[32-sizeof(SYNO_MTD_VENDER)];
                SYNO_MTD_VENDER_SN stSerial;
                unsigned char Pad2[32-sizeof(SYNO_MTD_VENDER_SN)];
                SYNO_MTD_VENDER_CUSTOM_SN stCustomSerial;
                unsigned char Pad3[32-sizeof(SYNO_MTD_VENDER_CUSTOM_SN)];
                SYNO_MTD_VENDER_TEST_FLAG stTestFlag;
} __attribute__((packed)) SYNO_MTD_VENDER_ALL, *PSYNO_MTD_VENDER_ALL;

int extract_vendor(const SYNO_MTD_VENDER_ALL *pVenderData)
{
	char *ptr = (char *)pVenderData;
	unsigned char chksum = 0;
	int nrZero = 0; // count the number of zero field in mac address
	int i = 0;
	char szBuf[64], 
		 szMac1[64], 
		 szMac2[64], 
		 szSN[64], 
		 szCustomSN[64];
	int len;
	//mac1
	memset(szMac1, 0, sizeof(szMac1));
	printf("            MAC1: ");
	for (i = 0, nrZero = 0; i < 6; i++) {
		chksum += pVenderData->stMac.macAddr1.rgAddr[i];
		if ( 0 == pVenderData->stMac.macAddr1.rgAddr[i] ) {
			nrZero++;
		}
	}
	if ( (chksum == 0 && 6 == nrZero) ||
			chksum != pVenderData->stMac.macAddr1.bChkSum ) {
		sprintf(szMac1, "%s", get_random_ethaddr(0));
		printf("Empty or Invalid checksum (Random: %s)\n", szMac1);
	} else {
		ptr = szMac1;
		for (i = 0; i < 6; i++)
		{
			if ( pVenderData->stMac.macAddr1.rgAddr[i] < 0x10 ) { 
				*ptr = '0';
				ptr++;
				sprintf(ptr, "%x", pVenderData->stMac.macAddr1.rgAddr[i]);
				ptr++;
			} else {
				sprintf(ptr, "%x", pVenderData->stMac.macAddr1.rgAddr[i]);
				ptr+=2;
			}
		}
		printf("%s\n", szMac1);
	}

	//mac2
	chksum = 0;
	memset(szMac2, 0, sizeof(szMac2));
	printf("            MAC2: ");
	for (i = 0, nrZero = 0; i < 6; i++) {
		chksum += pVenderData->stMac.macAddr2.rgAddr[i];
		if ( 0 == pVenderData->stMac.macAddr2.rgAddr[i] ) {
			nrZero++;
		}
	}
	if ( (chksum == 0 && 6 == nrZero) ||
			chksum != pVenderData->stMac.macAddr2.bChkSum ) {
		sprintf(szMac2, "%s", get_random_ethaddr(1));
		printf("Empty or Invalid checksum (Random: %s)\n", szMac2);
	} else {
		ptr = szMac2;
		for (i = 0; i < 6; i++) {
			if ( pVenderData->stMac.macAddr2.rgAddr[i] < 0x10 ) {
				*ptr = '0';
				ptr++;
				sprintf(ptr, "%x", pVenderData->stMac.macAddr2.rgAddr[i]);
				ptr++;
			} else {
				sprintf(ptr, "%x", pVenderData->stMac.macAddr2.rgAddr[i]);
				ptr+=2;
			}
		}
		printf("%s\n", szMac2);
	}

	//serial number
	chksum = 0;
	memset(szSN, 0, sizeof(szSN));
	printf("          Serial: ");
	for (i = 0; i < SN_LENGTH; i++) {
		chksum += pVenderData->stSerial.serialNumber.rgSN[i];
	}
	if ( chksum == 0 || 
			chksum != pVenderData->stSerial.serialNumber.bChkSum ) {
		printf("Empty or Invalid checksum\n");
	} else {
		ptr = szSN;
		for (i = 0; i < SN_LENGTH; i++) {
			*ptr = pVenderData->stSerial.serialNumber.rgSN[i];
			ptr++;
		}
		printf("%s\n", szSN);
	}

	//custom serial number
	chksum = 0;
	memset(szCustomSN, 0, sizeof(szCustomSN));
	printf("   Custom Serial: ");
	for (i = 0; i < CUSTOM_SN_LENGTH; i++) {
		chksum += pVenderData->stCustomSerial.serialCustomNumber.rgCustomSN[i];
	}
	if ( chksum == 0 ||
			chksum != pVenderData->stCustomSerial.serialCustomNumber.bChkSum )
	{
		printf("Empty or Invalid checksum\n");
	} else {
		ptr = szCustomSN;
		for (i = 0; i < CUSTOM_SN_LENGTH; i++) {
			*ptr = pVenderData->stCustomSerial.serialCustomNumber.rgCustomSN[i];
			ptr++;
		}
		printf("%s\n", szCustomSN);
	}

	memset(szVender, 0, sizeof(szVender));
	ptr = szVender;

	sprintf(szBuf, " mac1=%s mac2=%s ", szMac1, szMac2);
	len = strlen(szBuf);
	sprintf(ptr, "%s", szBuf);
	ptr += len;

	if ( '\0' != szSN[0] ) {
		sprintf(szBuf, "sn=%s ", szSN);
		len = strlen(szBuf);
		sprintf(ptr, "%s", szBuf);
		ptr += len;
	}

	if ( '\0' != szCustomSN[0] ) {
		sprintf(szBuf, "custom_sn=%s ", szCustomSN);
		len = strlen(szBuf);
		sprintf(ptr, "%s", szBuf);
		ptr += len;
	}
	return 0;
}
#define LEN_VENDOR_DATA 65536
char gszVendor[256];

int do_vendor (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if 0
	int i, j;
#endif
	unsigned char *pVendorData;
	int cbRead;
	char *pargv[] = {
		"ext2load", /* Interface */ NULL, /* dev */ NULL,
		/* addr */ NULL, /* filename */ NULL, /* Size (hex)*/ "10000"};
	char szAddr[20];

	pVendorData = (unsigned char *)malloc(LEN_VENDOR_DATA);
	if (NULL == pVendorData) {
		printf("Failed to malloc buffer!\n");
		return 1;
	}
	/* Load vendor data from file */
	pargv[1] = argv[1];
	pargv[2] = argv[2];
	sprintf(szAddr, "%x", pVendorData);
	pargv[3] = szAddr;
	pargv[4] = argv[3];
	cbRead = do_ext2load(cmdtp, flag, 6, pargv);
	if (LEN_VENDOR_DATA != cbRead) {
		printf("Load failed! %d byte read\n", cbRead);
		free(pVendorData);
		return 1;
	}
#if 0
	for (i = 0; i < 1024; i+=16) {
		printf("%08x: ", pVendorData+i);
		for (j = 0; j < 16; j++) {
			printf("%02x ", *(pVendorData+i+j));
		}
		for (j = 0; j < 16; j++) {
			if(0x20 < (*(pVendorData+i+j)) && 0x7e > (*(pVendorData+i+j))) {
			printf("%c", *(pVendorData+i+j));
			} else {
				printf(".");
			}
		}
		printf("\n");
	}
#endif
	/* Convert raw data into string */
	extract_vendor((SYNO_MTD_VENDER_ALL *)pVendorData);
	free(pVendorData);
	return 0;
}

U_BOOT_CMD(
	vendor,	4,	0,	do_vendor,
	"vendor  - load vendor info\n",
	""
);

#ifdef SYNO_CHECKSUM

#define SZF_CHECKSUM  "/checksum.syno"
#define SZEXT2LOAD "ext2load"
#define SZPARTITION1 "0:1"
#define SZPARTITION2 "0:2"
#define SZZIMAGE_ENCRYPTED "/zImage Encrypted: "
#define SZRDBIN_ENCRYPTED "/rd.bin Encrypted: "

/**
 * This function is used to find out the byte number of zImage(or rd.bin)
 *
 * @param pAddrTmp The address of zImage(or rd.bin)
 *
 * @return : the byte number of zImage(or rd.bin)
*/
static int
FindOutByteNumber (const char *pAddrTmp)
{
	int ByteNumber = 0;

	while (*(pAddrTmp + ByteNumber) != '\n') {
		ByteNumber ++;
	}

	return ByteNumber;
}

/**
 * This function is used to generate the checksum of zImage(or rd.bin)
 * and compare the difference with the original checksum of zImage(or rd.bin).
 *
 * @param pAddrTmp The address of zImage(or rd.bin)
 * @param szCksum The original checksum of zImage(or rd.bin)
 * @param filelen The file size
 *
 * @return 0 : The checksum not match
 * 1 : The checksum match
 * -1 : error or invalid address and data
*/
static int
ChecksumGenAndCheck (const char *pAddrTmp, const char *szCksum, const int filelen, const int ByteNumber)
{
	int i = 0;
	int match = -1;
	int GencksumByteNumber = 1;
	unsigned int uGencksum = 0, uTempGencksum = 0;
	char *szGencksum = NULL;

	if (NULL == pAddrTmp || NULL == szCksum || 2 > filelen) {
		printf("Invalid address or data\n");
		goto END;
	}

	for (i = 0; i < filelen; i++) {
		uGencksum = (unsigned char)(uGencksum >> 8) | (uGencksum << 8);
		uGencksum ^= *(pAddrTmp + i);
		uGencksum ^= (unsigned char)(uGencksum & 0xff) >> 4;
		uGencksum ^= (uGencksum << 8) << 4;
		uGencksum ^= ((uGencksum & 0xff) << 4) << 1;
	}

	uTempGencksum = uGencksum;
	//Calculate for getting the Byte number of uGencksum
	while ((uTempGencksum = uTempGencksum / 16) != 0) {
		GencksumByteNumber ++;
	}
	if (GencksumByteNumber != ByteNumber) {
		match = 0;
		goto END;
	}
	szGencksum = malloc(sizeof(char) * GencksumByteNumber);
	if (NULL == szGencksum) {
		printf("Failure for allocating memory block to szGencksum\n");
		goto END;
	}
	sprintf(szGencksum, "%x", uGencksum);

	for (i = 0; i < ByteNumber; i++) {
		if (szCksum[i] != szGencksum[i]) {
			match = 0;
			free(szGencksum);
			goto END;
		}
	}

	free(szGencksum);
	match = 1;

END:
	return match;
}

/**
 * The function is to implement the command "checksumcmd".
 * It is used for the checksum mechanism.
 * And the content of checksum.syno is like :
 * /zImage Encrypted: XXXXXXXX /rd.bin Encrypted: XXXXXXXX
 *
 * @return 1 : Error
 * 0 : Success
 */
int
do_checksumcmd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i = 0;
	int err = 1;
	int ZimageByteNumber = 0, RdbinByteNumber = 0;
	int ChecksumAddrIndex = 0;
	long filelen = -1;
	char *szFileZimage = NULL;
	char *szFileRdbin = NULL;
	char *pAddrCksum = NULL;
	char *pAddrZimage = NULL;
	char *pAddrRdbin = NULL;
	char *pSaveCksumAddr[2] = {NULL};
	char *szArgv[5] = {'\0'};
	char *szZimageCksum = NULL;
	char *szRdbinCksum = NULL;

	if (6 != argc) {
		printf("The command input is wrong\n");
		printf("%s <interface> [address for zImage loading] [zImagefilename] [address for rd.bin loading] [rd.binfilename]\n", argv[0]);
		goto END;
	}

	szFileZimage = argv[3];
	szFileRdbin = argv[5];
	if (!szFileZimage || !szFileRdbin) {
		printf("\n** No boot file defined **\n");
		goto END;
	}

	szArgv[0] = SZEXT2LOAD;
	szArgv[1] = argv[1];
	szArgv[2] = SZPARTITION2;
	szArgv[3] = argv[2];//assign the address argv[2] to szArgv[3]
	szArgv[4] = SZF_CHECKSUM;
	filelen = do_ext2load(cmdtp, flag, (sizeof (szArgv) / sizeof (szArgv[0])), szArgv);
	if (1 == filelen) {
		goto END;
	}
	pAddrCksum = (char *)simple_strtoul(argv[2], NULL, 16);
	for (i = 0; i < filelen; i++) {
		if (!strncmp(pAddrCksum + i, SZZIMAGE_ENCRYPTED, strlen(SZZIMAGE_ENCRYPTED))) {
			pSaveCksumAddr[ChecksumAddrIndex] = pAddrCksum + i + strlen(SZZIMAGE_ENCRYPTED);
			ZimageByteNumber = FindOutByteNumber(pSaveCksumAddr[ChecksumAddrIndex]);
			ChecksumAddrIndex ++;
			i = i + strlen(SZZIMAGE_ENCRYPTED) + ZimageByteNumber - 1;//Let for loop index i point to next line
		} else if (!strncmp(pAddrCksum + i, SZRDBIN_ENCRYPTED, strlen(SZRDBIN_ENCRYPTED))) {
			pSaveCksumAddr[ChecksumAddrIndex] = pAddrCksum + i + strlen(SZRDBIN_ENCRYPTED);
			RdbinByteNumber = FindOutByteNumber(pSaveCksumAddr[ChecksumAddrIndex]);
			ChecksumAddrIndex ++;
			i = i + strlen(SZRDBIN_ENCRYPTED) + RdbinByteNumber - 1;//Let for loop index i point to next line
		}
	}

	if (NULL == (szZimageCksum = malloc(sizeof(char) * (ZimageByteNumber)))) {
		printf("Failure for allocating memory block to szZimageCksum\n");
		goto CKSUMERROR;
	}
	if (NULL == (szRdbinCksum = malloc(sizeof(char) * (RdbinByteNumber)))) {
		free(szZimageCksum);
		printf("Failure for allocating memory block to szRdbinCksum\n");
		goto CKSUMERROR;
	}
	memcpy(szZimageCksum, pSaveCksumAddr[0], ZimageByteNumber);
	memcpy(szRdbinCksum, pSaveCksumAddr[1], RdbinByteNumber);

	szArgv[4] = szFileZimage;
	filelen = do_ext2load(cmdtp, flag, (sizeof (szArgv) / sizeof (szArgv[0])), szArgv);
	if (1 == filelen) {
		goto END;
	}
	pAddrZimage = (char *)simple_strtoul(argv[2], NULL, 16);
	if (1 > ChecksumGenAndCheck(pAddrZimage, szZimageCksum, filelen, ZimageByteNumber)) {
		free(szZimageCksum);
		free(szRdbinCksum);
		goto CKSUMERROR;
	}

	szArgv[3] = argv[4];//change the address to argv[4]
	szArgv[4] = szFileRdbin;
	filelen = do_ext2load(cmdtp, flag, (sizeof (szArgv) / sizeof (szArgv[0])), szArgv);
	if (1 == filelen) {
		goto END;
	}
	pAddrRdbin = (char *)simple_strtoul(argv[4], NULL, 16);
	if (1 > ChecksumGenAndCheck(pAddrRdbin, szRdbinCksum, filelen, RdbinByteNumber)) {
		free(szZimageCksum);
		free(szRdbinCksum);
		goto CKSUMERROR;
	}

	err = 0;
	free(szZimageCksum);
	free(szRdbinCksum);
	printf("verification OK!\n");
	printf("Load from %s\n", szArgv[2]);
	goto END;

CKSUMERROR:
	szArgv[2] = SZPARTITION1;
	szArgv[3] = argv[2];
	szArgv[4] = szFileZimage;
	filelen = do_ext2load(cmdtp, flag, (sizeof (szArgv) / sizeof (szArgv[0])), szArgv);
	if (1 == filelen) {
		goto END;
	}

	szArgv[3] = argv[4];
	szArgv[4] = szFileRdbin;
	filelen = do_ext2load(cmdtp, flag, (sizeof (szArgv) / sizeof (szArgv[0])), szArgv);
	if (1 == filelen) {
		goto END;
	}

	printf("verification Failed!\n");
	printf("Load from %s\n", szArgv[2]);

END:
	return err;
}

U_BOOT_CMD(
		    checksumcmd, 6,  0,  do_checksumcmd,
			"checksumcmd- check out the CRCchecksum value of zImage and rd.bin individually\n"
							"if the CRCchecksum values are all correct then boot the kernel image from partition 2\n"
							"Otherwise, boot from partition 1\n",
			"<interface> [address for zImage loading] [zImagefilename] [address for rd.bin loading] [rd.binfilename]\n"
);
#endif /* SYNO_CHECKSUM */

#endif /* SYNO_VENDOR */

#endif /* CFG_CMD_EXT2 */

