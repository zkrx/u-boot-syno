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


//#define DEBUG




#include <config.h>


#include <common.h>


#include "mvsdmmc.h"


#include <mmc.h>


#include <asm/errno.h>


#include <part.h>


#include "mvOs.h"




#ifdef CONFIG_MMC




static int is_sdhc;




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




/* MMC_DEFAULT_RCA should probably be just 1, but this may break other code


   that expects it to be shifted. */


static u_int16_t rca = 0;




static u_int32_t mmc_size(const struct mmc_csd *csd)


{


	u_int32_t block_len, mult, blocknr;




	block_len = csd->read_bl_len << 12;


	mult = csd->c_size_mult1 << 8;


	blocknr = (csd->c_size+1) * mult;




	return blocknr * block_len;


}




static int isprint (unsigned char ch)


{


	if (ch >= 32 && ch < 127)


		return (1);




	return (0);


}




static int toprint(char *dst, char c)


{


	if (isprint(c)) {


		*dst = c;


		return 1;


	}




	return sprintf(dst,"\\x%02x", c);




}




static void print_mmc_cid(mmc_cid_t *cid)


{


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




static void print_sd_cid(sd_cid_t *cid)


{


	int len;


	char tbuf[64];




	printf("SD%s found. Card desciption is:\n", is_sdhc?"HC":"");




	len = 0;


	len += toprint(&tbuf[len], cid->oid_0);


	len += toprint(&tbuf[len], cid->oid_1);


	tbuf[len] = 0;




	printf("Manufacturer:       0x%02x, OEM \"%s\"\n",


	    cid->mid, tbuf);




	len = 0;


	len += toprint(&tbuf[len], cid->pnm_0);


	len += toprint(&tbuf[len], cid->pnm_1);


	len += toprint(&tbuf[len], cid->pnm_2);


	len += toprint(&tbuf[len], cid->pnm_3);


	len += toprint(&tbuf[len], cid->pnm_4);


	tbuf[len] = 0;




	printf("Product name:       \"%s\", revision %d.%d\n",


		tbuf, 


	    cid->prv >> 4, cid->prv & 15);




	printf("Serial number:      %u\n",


	    cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |


	    cid->psn_3);


	printf("Manufacturing date: %d/%d\n",


	    cid->mdt_1 & 15,


	    2000+((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));




	printf("CRC:                0x%02x, b0 = %d\n",


	    cid->crc >> 1, cid->crc & 1);


}




static void mvsdmmc_set_clock(unsigned int clock)


{


	unsigned int m;




	m = MVSDMMC_BASE_FAST_CLOCK/(2*clock) - 1;




	debug("mvsdmmc_set_clock: dividor = 0x%x clock=%d\n",


		      m, clock);




	SDIO_REG_WRITE32(SDIO_CLK_DIV, m & 0x7ff);




	if (isprint(1))


	udelay(10*1000);


}






static ulong *


/****************************************************/


mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype)


/****************************************************/


{


	static ulong resp[4];


	ushort done ;


	int err = 0 ;


	ulong curr, start, diff, hz;


	ushort response[8], resp_indx = 0;




	debug("mmc_cmd %x, arg: %x,xfer: %x,resp: %x, wait : %x\n", cmd, arg, xfermode, resptype, waittype);







	start = get_ticks();


	hz = get_tbclk();




	while((SDIO_REG_READ16(SDIO_PRESENT_STATE0) & CARD_BUSY)) {


		curr = get_ticks();

		if (curr < start)
			printf("test1 - curr < start\n");

		diff = (long) curr - (long) start;


		if (diff > (3*hz))


		{


			// 3 seconds timeout, card busy, can't sent cmd


			printf("card too busy \n");


			return 0;


		}


	}


	//clear status 


	SDIO_REG_WRITE16(SDIO_NOR_INTR_STATUS, 0xffff);


	SDIO_REG_WRITE16(SDIO_ERR_INTR_STATUS, 0xffff);



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


		SDIO_REG_WRITE16(SDIO_CMD, ((cmd << 8) | resptype | (1 << 7)) );


	}




	done = SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & waittype;


	start = get_ticks();




	while( done!=waittype)


	{


		done = SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & waittype;




		if( SDIO_REG_READ16(SDIO_NOR_INTR_STATUS) & 0x8000 )


		{		


			printf("Error! cmd : %d, err : %04x\n", cmd, SDIO_REG_READ16(SDIO_ERR_INTR_STATUS) ) ;


			return 0 ;	// error happen 


		}




		curr = get_ticks();

		if (curr < start)
			printf("test2 - curr < start\n");


		diff = (long) curr - (long) start;


		if (diff > (3*hz))


		{


			printf("cmd timeout, status : %04x\n", SDIO_REG_READ16(SDIO_NOR_INTR_STATUS));


			printf("xfer mode : %04x\n", SDIO_REG_READ16(SDIO_XFER_MODE));


			err = 1 ;


			break;


		}


	}




	for (resp_indx = 0 ; resp_indx < 8; resp_indx++)


		response[resp_indx] = SDIO_REG_READ16(SDIO_RSP(resp_indx));




	memset(resp, 0, sizeof(resp));




	switch (resptype & 0x3) {


		case SDIO_CMD_RSP_48:


		case SDIO_CMD_RSP_48BUSY:


			resp[0] = ((response[2] & 0x3f) << (8 - 8)) |


				((response[1] & 0xffff) << (14 - 8)) |


				((response[0] & 0x3ff) << (30 - 8));


			resp[1] = ((response[0] & 0xfc00) >> 10);


			break;




		case SDIO_CMD_RSP_136:


			resp[3] = ((response[7] & 0x3fff) << 8)	|


				((response[6] & 0x3ff) << 22);


			resp[2] = ((response[6] & 0xfc00) >> 10)	|


				((response[5] & 0xffff) << 6)	|


				((response[4] & 0x3ff) << 22);


			resp[1] = ((response[4] & 0xfc00) >> 10)	|


				((response[3] & 0xffff) << 6)	|


				((response[2] & 0x3ff) << 22);


			resp[0] = ((response[2] & 0xfc00) >> 10)	|


				((response[1] & 0xffff) << 6)	|


				((response[0] & 0x3ff) << 22);


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


		return NULL ;


	else


		return resp;


}




int


/****************************************************/


mmc_block_read(uchar *dst, ulong src, ulong len)


/****************************************************/


{


	ulong *resp;


	//ushort argh, argl;


	//ulong status;




	if (len == 0) {


		return 0;


	}




	if (is_sdhc) {


		/* SDHC: use block address */


		src >>= 9;


	}




	debug("mmc_block_rd dst %lx src %lx len %d\n", (ulong)dst, src, len);




#if 0


	/* set block len */


	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, len, 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


	if (!resp) {


		printf("mmc_block_read: set blk len fails\n");


	 	return -EIO;


	}


#endif




	// prepare for dma transfer 


	SDIO_REG_WRITE16(SDIO_SYS_ADDR_LOW,((ulong)(dst))&0xffff);


	SDIO_REG_WRITE16(SDIO_SYS_ADDR_HI,(((ulong)dst)>>16)&0xffff);


	SDIO_REG_WRITE16(SDIO_BLK_SIZE,len);


	SDIO_REG_WRITE16(SDIO_BLK_COUNT,1);


	


	/* send read command */


	resp = mmc_cmd(MMC_CMD_READ_BLOCK, src, 0x10 , // 0x12,


			SDIO_CMD_RSP_48, SDIO_NOR_XFER_DONE);


	if (!resp) {


		printf("mmc_block_read: mmc read block cmd fails\n");


		return -EIO;


	}




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


	for (; src < aligned_end; aligned_start +=mmc_block_size, src += mmc_block_size, dst += mmc_block_size) {


		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",


		src, (ulong)dst, end, part_start, part_end, aligned_start, aligned_end);


		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0) {


		 	printf("mmc block read error\n");


			return -1;


		}


		//printf("mem copy from %x to %x, size %d\n", (ulong)mmc_buf, (ulong)dst, mmc_block_size );


		memcpy(dst, mmc_buf, mmc_block_size);


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


	ulong *resp;


	int sd_ver20;


	int is_sd;


	ushort reg;


	uchar cidbuf[64];




	sd_ver20 = 0;


	is_sdhc = 0;


	is_sd = 0;




	// Initial Host Ctrl : Timeout : max , Normal Speed mode, 4-bit data mode


	// Big Endian, SD memory Card, Push_pull CMD Line 


	SDIO_REG_WRITE16(SDIO_HOST_CTRL, 


		SDIO_HOST_CTRL_TMOUT(0xf) | 


		SDIO_HOST_CTRL_DATA_WIDTH_4_BITS | 


		SDIO_HOST_CTRL_BIG_ENDIAN | 


		SDIO_HOST_CTRL_PUSH_PULL_EN | 


		SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY );




	SDIO_REG_WRITE16(SDIO_CLK_DIV, 0xFA0);

	SDIO_REG_WRITE16(SDIO_CLK_CTRL, 0);




	//enable status


	SDIO_REG_WRITE16(SDIO_NOR_STATUS_EN, 0xffff);


	SDIO_REG_WRITE16(SDIO_ERR_STATUS_EN, 0xffff);




	//disable interrupts


	SDIO_REG_WRITE16(SDIO_NOR_INTR_EN, 0);


	SDIO_REG_WRITE16(SDIO_ERR_INTR_EN, 0);




	SDIO_REG_WRITE16(SDIO_SW_RESET,0x100);


	udelay(100000);




	mmc_csd.c_size = 0;




	/* reset */


	retries = 10;


	//mmc_cmd(ulong cmd, ulong arg, ushort xfermode, ushort resptype, ushort waittype);


	resp = mmc_cmd(0, 0, 0, SDIO_CMD_RSP_NONE , SDIO_NOR_CMD_DONE );


	debug("cmd 0 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




	debug ("trying to detect SD card version\n");


	resp = mmc_cmd(8, 0x000001aa, 0,  SDIO_CMD_RSP_48BUSY, SDIO_NOR_CMD_DONE );


	debug("cmd 8 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 


	if (resp && (resp[0] & 0x1ff)==0x1aa) {


		debug("sd version 2.0 card detected\n");


		sd_ver20 = 1;


	}




	if (sd_ver20)


		retries = 50;


	else


		retries = 10;




	while (retries--) {


		resp = mmc_cmd(55, 0, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


		debug("cmd 55 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




		if (sd_ver20) 


			resp = mmc_cmd(41, 0x40300000, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


		else


			resp = mmc_cmd(41, 0x00300000, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );




		debug("cmd 41 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




		if (resp && (resp[0] & 0x80000000)) {


			debug ("detected SD card\n");


			is_sd = 1;


			break;


		}




		udelay(100*1000);


	}




	if (retries <= 0 && !is_sd) {


		debug ("failed to detect SD card, trying MMC\n");


		retries = 10;


		while (retries--) {


			resp = mmc_cmd(1, 0, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


			debug("cmd 01 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




			if (resp && (resp[0] & 0x80000000)) {


				debug ("detected MMC card\n");


				reg = SDIO_REG_READ16(SDIO_HOST_CTRL);


				reg &= ~(0x3<<1);


				reg |= SDIO_HOST_CTRL_CARD_TYPE_IO_MMC;


				SDIO_REG_WRITE16(SDIO_HOST_CTRL, reg);


				break;


			}




			udelay(100*1000);


		}


	}


		


	if (retries <= 0) {


		debug ("detect fails\n");


		return -ENODEV;


	}




	/* try to get card id */


	resp = mmc_cmd(2, 0, 0, SDIO_CMD_RSP_136, SDIO_NOR_CMD_DONE );


	debug("cmd 2 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




	if (resp == NULL) {


		debug ("read cid fails\n");


		return -ENODEV;


	}




	if (is_sd) {


		sd_cid_t *cid = (sd_cid_t *) resp;




		memcpy(cidbuf, resp, sizeof(sd_cid_t));




		sprintf((char *) mmc_dev.vendor, 


			"Man %02x OEM %c%c \"%c%c%c%c%c\"",


			cid->mid, cid->oid_0, cid->oid_1,


			cid->pnm_0, cid->pnm_1, cid->pnm_2, cid->pnm_3, cid->pnm_4);




		sprintf((char *) mmc_dev.product, "%d", 


			(cid->psn_0 << 24) | (cid->psn_1 <<16) | (cid->psn_2 << 8) | (cid->psn_3 << 8));


		


		sprintf((char *) mmc_dev.revision, "%d.%d", cid->prv>>4, cid->prv & 0xff);


		


	} else {


		/* TODO configure mmc driver depending on card attributes */


		mmc_cid_t *cid = (mmc_cid_t *) resp;




		memcpy(cidbuf, resp, sizeof(sd_cid_t));






		sprintf((char *) mmc_dev.vendor, 


			"Man %02x%02x%02x Snr %02x%02x%02x",


			cid->id[0], cid->id[1], cid->id[2],


			cid->sn[0], cid->sn[1], cid->sn[2]);


		sprintf((char *) mmc_dev.product, "%s", cid->name);


		sprintf((char *) mmc_dev.revision, "%x %x", cid->hwrev, cid->fwrev);


	}


		


	/* fill in device description */


	mmc_dev.if_type = IF_TYPE_MMC;


	mmc_dev.part_type = PART_TYPE_DOS;


	mmc_dev.dev = 0;


	mmc_dev.lun = 0;


	mmc_dev.type = 0;




	/* FIXME fill in the correct size (is set to 128MByte) */


	mmc_dev.blksz = 512;


	mmc_dev.lba = 0x10000;




	mmc_dev.removable = 0;


	mmc_dev.block_read = mmc_bread;




	/* MMC exists, get CSD too */


	resp = mmc_cmd(MMC_CMD_SET_RCA, 0, 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


	if (resp == NULL) {


		debug ("set rca fails\n");


		return -ENODEV;


	}


	debug("cmd3 resp : 0x%08x 0x%08x 0x%08x 0x%08x\n", resp[0], resp[1], resp[2], resp[3]);




	if (is_sd)


		rca = resp[0] >> 16;


	else 


		rca = 0;




	resp = mmc_cmd(MMC_CMD_SEND_CSD, rca<<16, 0, SDIO_CMD_RSP_136,SDIO_NOR_CMD_DONE );


	debug("cmd 9 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 


	if (resp == NULL) {


		debug ("read csd fails\n");


		return -ENODEV;


	}




	memcpy(&mmc_csd, (mmc_csd_t *) resp, sizeof(mmc_csd_t));


	rc = 0;


	mmc_ready = 1;




	/* FIXME add verbose printout for csd */


	debug ("size = %u\n", mmc_size(&mmc_csd));




	resp = mmc_cmd(7, rca<<16, 0, SDIO_CMD_RSP_48BUSY, SDIO_NOR_CMD_DONE);


	if (resp == NULL) {


		debug ("select card fails\n");


		return -ENODEV;


	}


	debug("cmd 7 resp : %08x %08x %08x %08x\n", resp[0], resp[1], resp[2], resp[3] ); 




	if (is_sd) {


		resp = mmc_cmd(55, rca<<16, 0,  SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


		if (resp == NULL) {


			debug ("cmd55 fails\n");


			return -ENODEV;


		}


		debug("cmd55 resp : 0x%08x 0x%08x 0x%08x 0x%08x\n", resp[0], resp[1], resp[2], resp[3]);




		resp = mmc_cmd(6, (rca<<16) | 0x2 , 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


		if (resp == NULL) {


			debug ("cmd55 fails\n");


			return -ENODEV;


		}


		debug("cmd6 resp : 0x%08x 0x%08x 0x%08x 0x%08x\n", resp[0], resp[1], resp[2], resp[3]);


	}




	resp = (ulong *) &mmc_csd;


	debug("csd: 0x%08x 0x%08x 0x%08x 0x%08x\n", resp[0], resp[1], resp[2], resp[3]);




	/* check SDHC */


	if ((resp[0]&0xf0000000)==0x40000000)


		is_sdhc = 1;




	/* set block len */


	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, 512, 0, SDIO_CMD_RSP_48, SDIO_NOR_CMD_DONE );


	if (!resp) {


		printf("mmc_block_read: set blk len fails\n");


	 	return -EIO;


	}




	if (verbose) {


		if (is_sd) 


			print_sd_cid((sd_cid_t *) cidbuf);


		else 


			print_mmc_cid((mmc_cid_t *) cidbuf);


	}




	mvsdmmc_set_clock(25000000);




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


