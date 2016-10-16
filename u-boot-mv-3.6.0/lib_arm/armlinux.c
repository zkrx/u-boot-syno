/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif

extern unsigned int whoAmI(void);

#ifdef SYNO_DISK_STATION
#include "syno_custom.h"
#endif

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern void mvEgigaStrToMac( char *source , char *dest );

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_VFD) || \
    defined (CONFIG_LCD) ||	\
    defined (CONFIG_MARVELL_TAG)


static void setup_start_tag (bd_t *bd);

# ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags (bd_t *bd);
# endif
static void setup_commandline_tag (bd_t *bd, char *commandline);

#if 0
static void setup_ramdisk_tag (bd_t *bd);
#endif
# ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag (bd_t *bd, ulong initrd_start,
			      ulong initrd_end);
# endif
static void setup_end_tag (bd_t *bd);

# if defined (CONFIG_VFD) || defined (CONFIG_LCD)
static void setup_videolfb_tag (gd_t *gd);
# endif

#if defined (CONFIG_MARVELL_TAG)
static void setup_marvell_tag(void);
#endif

#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
static struct tag *params[2];
#else
static struct tag *params[1];
#endif
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
extern image_header_t header[2]; /* from cmd_bootm.c */
#else
extern image_header_t header[1]; /* from cmd_bootm.c */
#endif

void do_bootm_linux (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		     ulong addr, ulong *len_ptr, int verify)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong len = 0, checksum;
	ulong initrd_start, initrd_end;
	ulong data;
	void (*theKernel)(int zero, int arch, uint params);
	image_header_t *hdr;
	bd_t *bd = gd->bd;
	unsigned int cpu = 0;
	unsigned int cpuindex = 0;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline;
#ifdef CONFIG_MARVELL
#if defined(MV78200)
	cpu = whoAmI();
	if (cpu == 0)
#endif
		commandline = getenv ("bootargs");
#if defined(MV78200)
	else
		commandline = getenv ("bootargs2");
#endif
#else
	commandline = getenv ("bootargs");
#endif
#endif
#if defined(DUAL_OS_SHARED_MEM_78200)
	cpuindex = cpu;
#endif
	hdr = &header[cpuindex];
	theKernel = (void (*)(int, int, uint))ntohl(hdr->ih_ep);

	debug(" theKernel %x\n", theKernel);

	/*
	 * Check if there is an initrd image
	 */
	if (argc >= 3) {
		SHOW_BOOT_PROGRESS (9);

		addr = simple_strtoul (argv[2], NULL, 16);

		printf ("## Loading Ramdisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
#ifdef CONFIG_HAS_DATAFLASH
		if (addr_dataflash (addr)) {
			read_dataflash (addr, sizeof (image_header_t),
					(char *) &header[cpuindex]);
		} else
#endif
			memcpy (&header[cpuindex], (char *) addr,
				sizeof (image_header_t));

#if defined(CONFIG_SYNO_MV88F6281)
                /* If we don't copy F8280000 to 800000, kernel will think ramdisk
                 * is in F8280000. But when it try to find the F8280000, the
                 * flash driver has not start yet. So kernel will not be able to
                 * access F8280000.
                 */
                if (addr > 0xF0000000 ) {
                        ulong src, dest;
                        int count;

                        src = addr;
                        dest = ntohl(hdr->ih_load);
                        count = ntohl(hdr->ih_size) + sizeof (image_header_t);
                        while (count-- > 0) {
                                *((u_char *)dest) = *((u_char *)src);

                                src = src + 1;
                                dest = dest + 1;
                        }
                        data = ntohl(hdr->ih_load) + sizeof (image_header_t);
                }
#endif

		if (ntohl (hdr->ih_magic) != IH_MAGIC) {
			printf ("Bad Magic Number\n");
			SHOW_BOOT_PROGRESS (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong) &header[cpuindex];
		len = sizeof (image_header_t);

		checksum = ntohl (hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (const char *) data, len) != checksum) {
			printf ("Bad Header Checksum\n");
			SHOW_BOOT_PROGRESS (-11);
			do_reset (cmdtp, flag, argc, argv);
		}

		SHOW_BOOT_PROGRESS (10);

		print_image_hdr (hdr);

		data = addr + sizeof (image_header_t);
		len = ntohl (hdr->ih_size);

#ifdef CONFIG_HAS_DATAFLASH
		if (addr_dataflash (addr)) {
			read_dataflash (data, len, (char *) CFG_LOAD_ADDR);
			data = CFG_LOAD_ADDR;
		}
#endif

		if (verify) {
			ulong csum = 0;

			printf ("   Verifying Checksum ... ");
			csum = crc32 (0, (const char *) data, len);
			if (csum != ntohl (hdr->ih_dcrc)) {
				printf ("Bad Data CRC\n");
				SHOW_BOOT_PROGRESS (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			printf ("OK\n");
		}

		SHOW_BOOT_PROGRESS (11);

		if ((hdr->ih_os != IH_OS_LINUX) ||
		    (hdr->ih_arch != IH_CPU_ARM) ||
		    (hdr->ih_type != IH_TYPE_RAMDISK)) {
			printf ("No Linux ARM Ramdisk Image\n");
			SHOW_BOOT_PROGRESS (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

#if defined(CONFIG_B2) || defined(CONFIG_EVB4510) || defined(CONFIG_ARMADILLO)
		/*
		 *we need to copy the ramdisk to SRAM to let Linux boot
		 */
		memmove ((void *) ntohl(hdr->ih_load), (uchar *)data, len);
		data = ntohl(hdr->ih_load);
#endif /* CONFIG_B2 || CONFIG_EVB4510 */

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type == IH_TYPE_MULTI) && (len_ptr[1])) {
		ulong tail = ntohl (len_ptr[0]) % 4;
		int i;

		SHOW_BOOT_PROGRESS (13);

		/* skip kernel length and terminator */
		data = (ulong) (&len_ptr[2]);
		/* skip any additional image length fields */
		for (i = 1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl (len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len = ntohl (len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		SHOW_BOOT_PROGRESS (14);

		len = data = 0;
	}

#ifdef	DEBUG
	if (!data) {
		printf ("No initrd\n");
	}
#endif

	if (data) {
		initrd_start = data;
		initrd_end = initrd_start + len;
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

	SHOW_BOOT_PROGRESS (15);

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong) theKernel);

#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	/*Calculate boot params offset*/	
	bd->bi_boot_params = ntohl(hdr->ih_ep)-0x8000+0x100;
#endif

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD) || \
    defined (CONFIG_MARVELL_TAG)
	setup_start_tag (bd);
#ifdef CONFIG_SERIAL_TAG
	setup_serial_tag (&params[cpuindex]);
#endif
#ifdef CONFIG_REVISION_TAG
	setup_revision_tag (&params[cpuindex]);
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
	setup_memory_tags (bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag (bd, commandline);
#endif
#ifdef CONFIG_INITRD_TAG
	if (initrd_start && initrd_end)
		setup_initrd_tag (bd, initrd_start, initrd_end);
#endif
#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
	setup_videolfb_tag ((gd_t *) gd);
#endif
#if defined (CONFIG_MARVELL_TAG)
        /* Linux open port doesn't support the Marvell TAG */
	char *env = getenv("mainlineLinux");
	if(!env || ((strcmp(env,"no") == 0) ||  (strcmp(env,"No") == 0)))
	    setup_marvell_tag ();
#endif
	setup_end_tag (bd);
#endif

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");
	
#ifdef CONFIG_USB_DEVICE
	{
		extern void udc_disconnect (void);
		udc_disconnect ();
	}
#endif

#ifdef SYNO_DISK_STATION
        syno_disk_led_set(DISK_LED_OFF);
#endif

	cleanup_before_linux ();
#if defined(CONFIG_MARVELL) && defined(DUAL_OS_SHARED_MEM_78200)
	mvSemaUnlock(1);
#endif	
	theKernel (0, bd->bi_arch_number, bd->bi_boot_params);
}


#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) || \
    defined (CONFIG_INITRD_TAG) || \
    defined (CONFIG_SERIAL_TAG) || \
    defined (CONFIG_REVISION_TAG) || \
    defined (CONFIG_LCD) || \
    defined (CONFIG_VFD) || \
    defined (CONFIG_MARVELL_TAG)
static void setup_start_tag (bd_t *bd)
{
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif
	params[cpu] = (struct tag *) bd->bi_boot_params;
	params[cpu]->hdr.tag = ATAG_CORE;
	params[cpu]->hdr.size = tag_size (tag_core);

	params[cpu]->u.core.flags = 0;
	params[cpu]->u.core.pagesize = 0;
	params[cpu]->u.core.rootdev = 0;

	params[cpu] = tag_next (params[cpu]);
}


#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags (bd_t *bd)
{
	int i;
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params[cpu]->hdr.tag = ATAG_MEM;
		params[cpu]->hdr.size = tag_size (tag_mem32);

		params[cpu]->u.mem.start = bd->bi_dram[i].start;
		params[cpu]->u.mem.size = bd->bi_dram[i].size;

		params[cpu] = tag_next (params[cpu]);
	}
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */


static void setup_commandline_tag (bd_t *bd, char *commandline)
{
	char *p;
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif
#ifdef SYNO_VENDOR
	extern char szVender[];
#endif /* SYNO_VENDOR */

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params[cpu]->hdr.tag = ATAG_CMDLINE;
#ifdef SYNO_VENDOR
	params[cpu]->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + strlen(szVender) + 1 + 4) >> 2;
	sprintf(params[cpu]->u.cmdline.cmdline, "%s%s", p, szVender);
#else /* SYNO_VENDOR */
	params[cpu]->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params[cpu]->u.cmdline.cmdline, p);
#endif /* SYNO_VENDOR */
	params[cpu] = tag_next (params[cpu]);
}


#ifdef CONFIG_INITRD_TAG
static void setup_initrd_tag (bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params[cpu]->hdr.tag = ATAG_INITRD2;
	params[cpu]->hdr.size = tag_size (tag_initrd);

	params[cpu]->u.initrd.start = initrd_start;
	params[cpu]->u.initrd.size = initrd_end - initrd_start;

	params[cpu] = tag_next (params[cpu]);
}
#endif /* CONFIG_INITRD_TAG */


#if defined (CONFIG_VFD) || defined (CONFIG_LCD)
extern ulong calc_fbsize (void);
static void setup_videolfb_tag (gd_t *gd)
{
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	/* An ATAG_VIDEOLFB node tells the kernel where and how large
	 * the framebuffer for video was allocated (among other things).
	 * Note that a _physical_ address is passed !
	 *
	 * We only use it to pass the address and size, the other entries
	 * in the tag_videolfb are not of interest.
	 */
	params[cpu]->hdr.tag = ATAG_VIDEOLFB;
	params[cpu]->hdr.size = tag_size (tag_videolfb);

	params[cpu]->u.videolfb.lfb_base = (u32) gd->fb_base;
	/* Fb size is calculated according to parameters for our panel
	 */
	params[cpu]->u.videolfb.lfb_size = calc_fbsize();

	params[cpu] = tag_next (params[cpu]);
}
#endif /* CONFIG_VFD || CONFIG_LCD */

#if defined(CONFIG_MARVELL_TAG)
static void setup_marvell_tag (void)
{
	char *env;
	char temp[20];
	int i;
	unsigned int boardId;
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	params[cpu]->hdr.tag = ATAG_MARVELL;
	params[cpu]->hdr.size = tag_size (tag_mv_uboot);

	params[cpu]->u.mv_uboot.uboot_version = VER_NUM;
    if(strcmp(getenv("nandEcc"), "4bit") == 0)
    {
        params[cpu]->u.mv_uboot.nand_ecc = 4;
    }
    else if(strcmp(getenv("nandEcc"), "1bit") == 0)
    {
        params[cpu]->u.mv_uboot.nand_ecc = 1;
    }

	extern unsigned int mvBoardIdGet(void);	

	boardId = mvBoardIdGet();
	params[cpu]->u.mv_uboot.uboot_version |= boardId;

	params[cpu]->u.mv_uboot.tclk = CFG_TCLK;
	params[cpu]->u.mv_uboot.sysclk = CFG_BUS_CLK;
	
#if defined(MV78XX0)
	/* Dual CPU Firmware load address */
        env = getenv("fw_image_base");
        if(env)
		params[cpu]->u.mv_uboot.fw_image_base = simple_strtoul(env, NULL, 16);
	else
		params[cpu]->u.mv_uboot.fw_image_base = 0;

	/* Dual CPU Firmware size */
        env = getenv("fw_image_size");
        if(env)
		params[cpu]->u.mv_uboot.fw_image_size = simple_strtoul(env, NULL, 16);
	else
		params[cpu]->u.mv_uboot.fw_image_size = 0;
#endif

#if defined(MV_INCLUDE_USB)
    extern unsigned int mvCtrlUsbMaxGet(void);

    for (i = 0 ; i < mvCtrlUsbMaxGet(); i++)
    {
	sprintf( temp, "usb%dMode", i);
	env = getenv(temp);
	if((!env) || (strcmp(env,"Host") == 0 ) || (strcmp(env,"host") == 0) )
		params[cpu]->u.mv_uboot.isUsbHost |= (1 << i);
	else
		params[cpu]->u.mv_uboot.isUsbHost &= ~(1 << i);

    }
#endif /*#if defined(MV_INCLUDE_USB)*/
#if defined(MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH)
	extern unsigned int mvCtrlEthMaxPortGet(void);
	extern int mvMacStrToHex(const char* macStr, unsigned char* macHex);

	for (i = 0 ;i < 4;i++)
	{
		memset(params[cpu]->u.mv_uboot.macAddr[i], 0, sizeof(params[cpu]->u.mv_uboot.macAddr[i]));
		params[cpu]->u.mv_uboot.mtu[i] = 0; 
	}

	for (i = 0 ;i < mvCtrlEthMaxPortGet();i++)
	{
/* only on RD-6281-A egiga0 defined as eth1 */
#if defined (RD_88F6281A)
		sprintf( temp,(i==0 ? "eth1addr" : "ethaddr"));
#else
        sprintf( temp,(i ? "eth%daddr" : "ethaddr"), i);
#endif

        env = getenv(temp);
	    if (env)
		mvMacStrToHex(env, params[cpu]->u.mv_uboot.macAddr[i]);

/* only on RD-6281-A egiga0 defined as eth1 */
#if defined (RD_88F6281A)
		sprintf( temp,(i==0 ? "eth1mtu" : "ethmtu"));
#else
        sprintf( temp,(i ? "eth%dmtu" : "ethmtu"), i);
#endif

        env = getenv(temp);
        if (env)
            params[cpu]->u.mv_uboot.mtu[i] = simple_strtoul(env, NULL, 10); 
	}
#endif /* (MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH) */

	params[cpu] = tag_next (params[cpu]);
}	
#endif

#ifdef CONFIG_SERIAL_TAG
void setup_serial_tag (struct tag **tmp)
{
	struct tag *params = *tmp;
	struct tag_serialnr serialnr;
	void get_board_serial(struct tag_serialnr *serialnr);
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	get_board_serial(&serialnr);
	params[cpu]->hdr.tag = ATAG_SERIAL;
	params[cpu]->hdr.size = tag_size (tag_serialnr);
	params[cpu]->u.serialnr.low = serialnr.low;
	params[cpu]->u.serialnr.high= serialnr.high;
	params[cpu] = tag_next (params[cpu]);
	*tmp = params;
}
#endif

#ifdef CONFIG_REVISION_TAG
void setup_revision_tag(struct tag **in_params)
{
	u32 rev = 0;
	u32 get_board_rev(void);
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	rev = get_board_rev();
	params[cpu]->hdr.tag = ATAG_REVISION;
	params[cpu]->hdr.size = tag_size (tag_revision);
	params[cpu]->u.revision.rev = rev;
	params[cpu] = tag_next (params[cpu]);
}
#endif  /* CONFIG_REVISION_TAG */


static void setup_end_tag (bd_t *bd)
{
	unsigned int cpu = 0;
#if defined(MV78200) && defined(DUAL_OS_SHARED_MEM_78200)
	cpu = whoAmI();
#endif

	params[cpu]->hdr.tag = ATAG_NONE;
	params[cpu]->hdr.size = 0;
}

#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG || CONFIG_INITRD_TAG */
