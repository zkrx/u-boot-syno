/*
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
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
 */

#include <config.h>
#include <common.h>
#include "mustang_sdiodef.h"
#include <mmc.h>
#include <asm/errno.h>
#include <part.h>
#include "mvOs.h"

#ifdef CONFIG_MMC
#define DELAY 50
//#define debug printf
extern int
fat_register_device(block_dev_desc_t *dev_desc, int part_no);

static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
static uchar mmc_buf[MMC_BLOCK_SIZE];
static mmc_csd_t mmc_csd;
static int mmc_ready = 0;
static int rca ;



static uchar *
/****************************************************/
mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype)
/****************************************************/
{
	static ulong resp[4];
	int count ;
	ushort done ;
	int err = 0 ;

	debug("mmc_cmd %x, arg: %x,xfer: %x,resp: %x, wait : %x\n", cmd, arg, xfermode, resptype, waittype);
	//clear status 
	SDIO_REG_WRITE16(SDIO_ERR_INTR_EN, SDIO_REG_READ16(SDIO_ERR_INTR_STATUS));
	SDIO_REG_WRITE16(SDIO_NOR_INTR_EN, SDIO_REG_READ16(SDIO_NOR_INTR_STATUS));
	//disable interrupts
	SDIO_REG_WRITE16(SDIO_NOR_INTR_EN, 0);
	SDIO_REG_WRITE16(SDIO_ERR_INTR_EN, 0);
	
	udelay(2000);
/*	count = 10000 ;
	while((SDIO_REG_READ16(SDIO_PRESENT_STATE0) & CARD_BUSY)) {
		udelay(DELAY);
		if (--count <= 0 ) {
			// card busy, can't sent cmd
			printf("card too busy \n");
			return 0;
		}
	}
*/
	SDIO_REG_WRITE16(SDIO_ARG_LOW, (ushort)(arg&0xffff) );
   	SDIO_REG_WRITE16(SDIO_ARG_HI, (ushort)(arg>>16) );
	SDIO_REG_WRITE16(SDIO_XFER_MODE, xfermode);
	if( (cmd == MMC_CMD_READ_BLOCK) || (cmd == 25) )
	{
		SDIO_REG_WRITE16(SDIO_CMD, ((cmd << 8) | resptype | 0x3c ) );
		debug("cmd reg : %x\n", SDIO_REG_READ16( SDIO_CMD )) ;
	}
	else
	{
		SDIO_REG_WRITE16(SDIO_CMD, ((cmd << 8) | resptype ) );
	}

	done = SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & waittype;
	count = 10000 ;

	while( done!=waittype)
	{
//		udelay(DELAY);
		udelay(2000);
		done = SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & waittype;
/*		if( SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & 0x8000 )
		{		
			printf("Error! cmd : %d, err : %x\n", cmd, SDIO_REG_READ16(SDIO_ERR_INTR_STATUS) ) ;
			return 0 ;	// error happen 
		}
		if( --count <= 0 )
		{
			debug("cmd timeout, status : %x\n", SDIO_REG_READ16(SDIO_NOR_INTR_STATUS));
			debug("xfer mode : %x\n", SDIO_REG_READ16(SDIO_XFER_MODE));
			err = 1 ;
			break;
			//return 0 ;
		}
*/
	}

	// write clear the status 
	SDIO_REG_WRITE16(SDIO_NOR_INTR_STATUS, waittype);
	//SDIO_REG_WRITE16(SDIO_NOR_INTR_EN, SDIO_REG_READ16(SDIO_NOR_INTR_STATUS));

	switch (resptype & 0x3) {
		case SDIO_CMD_RSP_48:
		case SDIO_CMD_RSP_48BUSY:
			resp[0] = ( SDIO_REG_READ16(SDIO_RSP0) << (16+6)) |
				(SDIO_REG_READ16(SDIO_RSP1) << 6) | 
				((SDIO_REG_READ16(SDIO_RSP2))& 0x3f) ;
			break;

		case SDIO_CMD_RSP_136:
			resp[0] = ( SDIO_REG_READ16(SDIO_RSP5) << (16+14)) |
				(SDIO_REG_READ16(SDIO_RSP6) << 14) | 
				((SDIO_REG_READ16(SDIO_RSP7))& 0x3fff) ;
			resp[1] = ( SDIO_REG_READ16(SDIO_RSP3) << (16+14)) | 
				(SDIO_REG_READ16(SDIO_RSP4) << 14) | 
				((SDIO_REG_READ16(SDIO_RSP5)>> 2 )& 0x3fff) ;
			resp[2] = ( SDIO_REG_READ16(SDIO_RSP1) << (16+14)) | 
				(SDIO_REG_READ16(SDIO_RSP2) << 14) | 
				((SDIO_REG_READ16(SDIO_RSP3)>> 2 )& 0x3fff) ;
			resp[3] = (SDIO_REG_READ16(SDIO_RSP0) << 14) | 
				((SDIO_REG_READ16(SDIO_RSP1)>> 2 )& 0x3fff) ;
			break;
		default:
			return 0;
	}
#ifdef MMC_DEBUG
	int i;
	printf("MMC resp :");
	for (i=0; i<4; ++i ) {
		printf(" %08x", resp[i]);
	}
	printf("\n");
#endif
	if( err )
		return 0 ;
	else
		return (uchar*)resp;
}

int
/****************************************************/
mmc_block_read(uchar *dst, ulong src, ulong len)
/****************************************************/
{
	uchar *resp;
	//ushort argh, argl;
	//ulong status;

	if (len == 0) {
		return 0;
	}

	debug("mmc_block_rd dst %lx src %lx len %d\n", (ulong)dst, src, len);

	//mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype);
	/* set block len */
	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, len, 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );

	// prepare for dma transfer 
	//SDIO_REG_WRITE16(SDIO_SYS_ADDR_LOW,((ulong)(dst)>>16)&0xffff);
	SDIO_REG_WRITE16(SDIO_SYS_ADDR_LOW,((ulong)(dst))&0xffff);
	//SDIO_REG_WRITE16(SDIO_SYS_ADDR_HI,((ulong)(dst))&0xffff);
	SDIO_REG_WRITE16(SDIO_SYS_ADDR_HI,(0x8000 | ((ulong)(dst)>>16)&0xffff));
	SDIO_REG_WRITE16(SDIO_BLK_SIZE,len);
	SDIO_REG_WRITE16(SDIO_BLK_COUNT,1);

	
	/* send read command */
	resp = mmc_cmd(MMC_CMD_READ_BLOCK, src, 0x10 , // 0x12,
			SDIO_CMD_RSP_48, SDIO_NOR_DMA_INI);

	return 0;
}

int
/****************************************************/
mmc_block_write(ulong dst, uchar *src, int len)
/****************************************************/
{
	//uchar *resp;
	//ushort argh, argl;
	//ulong status;
	return -1 ;
#if 0
	if (len == 0) {
		return 0;
	}

	debug("mmc_block_wr dst %lx src %lx len %d\n", dst, (ulong)src, len);

	argh = len >> 16;
	argl = len & 0xffff;

	/* set block len */
	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, argh, argl, MMC_CMDAT_R1);

	/* send write command */
	argh = dst >> 16;
	argl = dst & 0xffff;
	MMC_STRPCL = MMC_STRPCL_STOP_CLK;
	MMC_NOB = 1;
	MMC_BLKLEN = len;
	resp = mmc_cmd(MMC_CMD_WRITE_BLOCK, argh, argl,
			MMC_CMDAT_R1|MMC_CMDAT_WRITE|MMC_CMDAT_BLOCK|MMC_CMDAT_DATA_EN);

	MMC_I_MASK = ~MMC_I_MASK_TXFIFO_WR_REQ;
	while (len) {
		if (MMC_I_REG & MMC_I_REG_TXFIFO_WR_REQ) {
			int i, bytes = min(32,len);

			for (i=0; i<bytes; i++) {
				MMC_TXFIFO = *src++;
			}
			if (bytes < 32) {
				MMC_PRTBUF = MMC_PRTBUF_BUF_PART_FULL;
			}
			len -= bytes;
		}
		status = MMC_STAT;
		if (status & MMC_STAT_ERRORS) {
			printf("MMC_STAT error %lx\n", status);
			return -1;
		}
	}
	MMC_I_MASK = ~MMC_I_MASK_DATA_TRAN_DONE;
	while (!(MMC_I_REG & MMC_I_REG_DATA_TRAN_DONE));
	MMC_I_MASK = ~MMC_I_MASK_PRG_DONE;
	while (!(MMC_I_REG & MMC_I_REG_PRG_DONE));
	status = MMC_STAT;
	if (status & MMC_STAT_ERRORS) {
		printf("MMC_STAT error %lx\n", status);
		return -1;
	}
	return 0;
#endif
}


int
/****************************************************/
mmc_read(ulong src, uchar *dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; aligned_start +=mmc_block_size/4, src += mmc_block_size/4, dst += mmc_block_size/4) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		//if ((mmc_block_read((uchar *)(dst), src, mmc_block_size)) < 0) {
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size/4)) < 0) {
//		if ((mmc_block_read((uchar*)0x2000000, aligned_start, mmc_block_size)) < 0) {
		 	printf("mmc block read error\n");
			return -1;
		}
		//printf("mem copy from %x to %x, size %d\n", (ulong)mmc_buf, (ulong)dst, mmc_block_size );
		memcpy(dst, mmc_buf, mmc_block_size/4);
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
		dst += part_len;
		src += part_len;
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (uchar *)src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
	src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
	}
	return 0;
}

ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
/****************************************************/
{
	int mmc_block_size = MMC_BLOCK_SIZE;
	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
	return blkcnt;
}

int
/****************************************************/
mmc_init(int verbose)
/****************************************************/
{
 	int retries, rc = -ENODEV;
	uchar *resp;
	uchar scrs[16] ;

	// Initial Host Ctrl : Timeout : max , Normal Speed mode, 4-bit data mode
	// Big Endian, SD memory Card, Push_pull CMD Line 
	SDIO_REG_WRITE16(SDIO_HOST_CTRL, 
		( 0xf << SDIO_HOST_CTRL_TMOUT_SHIFT ) | 
		SDIO_HOST_CTRL_DATA_WIDTH_4_BITS | 
		SDIO_HOST_CTRL_BIG_ENDAN | 
		SDIO_HOST_CTRL_PUSH_PULL_EN | 
		SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY );

	SDIO_REG_WRITE16(SDIO_CLK_CTRL, 0);

	//enable status
	SDIO_REG_WRITE16(SDIO_NOR_STATUS_EN, 0xffff);
	SDIO_REG_WRITE16(SDIO_ERR_STATUS_EN, SDIO_ERR_INTR_MASK);

	//disable interrupts
	SDIO_REG_WRITE16(SDIO_NOR_INTR_EN, 0);
	SDIO_REG_WRITE16(SDIO_ERR_INTR_EN, SDIO_ERR_INTR_MASK);

	SDIO_REG_WRITE16(SDIO_SW_RESET,0x100);
	udelay(10000);

	mmc_csd.c_size = 0;

	/* reset */
	retries = 10;
	//mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype);
	resp = mmc_cmd(0, 0, 0, SDIO_CMD_RSP_NONE , SDIO_NOR_CMD_DONE );
	debug("cmd 0 resp : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
	resp = mmc_cmd(55, 0, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );
	debug("cmd 55 resp : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
	resp = mmc_cmd(41, 0xff8000, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );

	debug("cmd 41 resp : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
	while (retries-- && resp && !(resp[3] & 0x80)) {
		resp = mmc_cmd(55, 0, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );
		resp = mmc_cmd(41, 0xff8000, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );
		debug("cmd 41 resp : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
	}

	if( resp[3] & 0x80 )
		printf("sd memory ready\n");
	else
	{
		printf("Sd memory error\n");
		return rc;
	}

	/* try to get card id */
	resp = mmc_cmd(2, 0, 0, SDIO_CMD_RSP_136, SDIO_NOR_CMD_DONE );
	debug("cmd 2 resp 0 : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
	debug("cmd 2 resp 1 : %x %x %x %x\n", resp[4], resp[5], resp[6], resp[7] ); 
	debug("cmd 2 resp 2 : %x %x %x %x\n", resp[8], resp[9], resp[10], resp[11] ); 
	debug("cmd 2 resp 3 : %x %x %x %x\n", resp[12], resp[13], resp[14], resp[15] ); 
	if (resp) {
		/* TODO configure mmc driver depending on card attributes */
		mmc_cid_t *cid = (mmc_cid_t *)resp;
		if (verbose) {
			printf("MMC found. Card desciption is:\n");
			printf("Manufacturer ID = %02x%02x%02x\n",
							cid->id[0], cid->id[1], cid->id[2]);
			printf("HW/FW Revision = %x %x\n",cid->hwrev, cid->fwrev);
			cid->hwrev = cid->fwrev = 0;	/* null terminate string */
			printf("Product Name = %s\n",cid->name);
			printf("Serial Number = %02x%02x%02x\n",
							cid->sn[0], cid->sn[1], cid->sn[2]);
			printf("Month = %d\n",cid->month);
			printf("Year = %d\n",1997 + cid->year);
		}
		/* fill in device description */
		mmc_dev.if_type = IF_TYPE_MMC;
		mmc_dev.part_type = PART_TYPE_DOS;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = 0;
		/* FIXME fill in the correct size (is set to 128MByte) */
		mmc_dev.blksz = 512;
		mmc_dev.lba = 0x40000;
		printf(mmc_dev.vendor,"Man %02x%02x%02x Snr %02x%02x%02x",
				cid->id[0], cid->id[1], cid->id[2],
				cid->sn[0], cid->sn[1], cid->sn[2]);
		printf(mmc_dev.product,"%s",cid->name);
		printf(mmc_dev.revision,"%x %x",cid->hwrev, cid->fwrev);
		mmc_dev.removable = 0;
		mmc_dev.block_read = mmc_bread;

	//mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype);
		/* MMC exists, get CSD too */
		resp = mmc_cmd(MMC_CMD_SET_RCA, 0, 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );
		rca = resp[2] | (resp[3] << 8 ) ;
		resp = mmc_cmd(MMC_CMD_SEND_CSD, rca<<16, 0, SDIO_CMD_RSP_136,SDIO_NOR_CMD_DONE );
		if (resp) {
			mmc_csd_t *csd = (mmc_csd_t *)resp;
			memcpy(&mmc_csd, csd, sizeof(csd));
			debug("cmd 9 resp 0 : %x %x %x %x\n", resp[0], resp[1], resp[2], resp[3] ); 
			debug("cmd 9 resp 1 : %x %x %x %x\n", resp[4], resp[5], resp[6], resp[7] ); 
			debug("cmd 9 resp 2 : %x %x %x %x\n", resp[8], resp[9], resp[10], resp[11] ); 
			debug("cmd 9 resp 3 : %x %x %x %x\n", resp[12], resp[13], resp[14], resp[15] ); 
			rc = 0;
			/* Calc device size */
			debug("cmd 9 csd->c_size : %x\n", csd->c_size ); 
			debug("cmd 9 csd->c_size_mult1 : %x\n", csd->c_size_mult1 ); 
			debug("cmd 9 csd->read_bl_len : %x\n", csd->read_bl_len ); 
			debug("cmd 9 csd->tran_speed : %x\n", csd->tran_speed ); 
			debug("cmd 9 csd->nsac : %x\n", csd->nsac ); 
			debug("cmd 9 csd->taac : %x\n", csd->taac ); 
			/* FIXME add verbose printout for csd */
			mmc_ready = 1;
		}
	}

	resp = mmc_cmd(MMC_CMD_SELECT_CARD, rca<<16, 0, SDIO_CMD_RSP_48BUSY, SDIO_NOR_CMD_DONE);
#if 0
	resp = mmc_cmd(MMC_CMD_SELECT_CARD, 0, 0, SDIO_CMD_RSP_48BUSY, SDIO_NOR_CMD_DONE);

	resp = mmc_cmd(MMC_CMD_SELECT_CARD, rca<<16, 0, SDIO_CMD_RSP_48BUSY, SDIO_NOR_CMD_DONE);
#endif
	resp = mmc_cmd(55, rca<<16, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );

	resp = mmc_cmd(6, (rca<<16) | 0x2 , 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );

#if 0
	// prepare for dma transfer 
	SDIO_REG_WRITE16(SDIO_BLK_SIZE,8);
	SDIO_REG_WRITE16(SDIO_BLK_COUNT,1);
	SDIO_REG_WRITE16(SDIO_SYS_ADDR_LOW,((ulong)(scrs))&0xffff);
	SDIO_REG_WRITE16(SDIO_SYS_ADDR_HI,((ulong)(scrs)>>16)&0xffff);
	//ACMD 51
	resp = mmc_cmd(51, 0, 0x12,  SDIO_CMD_RSP_48, SDIO_NOR_DMA_INI );

	printf("scrs : %x %x %x %x %x %x %x %x\n", scrs[0], scrs[1], scrs[2], scrs[3], scrs[4], scrs[5], scrs[6], scrs[7] );
#endif
	fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */

	return rc;
}

int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 256 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0xffffffff) {
		return 1;
	}
	return 0;
}

#endif	/* CONFIG_MMC */
