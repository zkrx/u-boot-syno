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
#include <mpc8xx.h>
#include <malloc.h>
#include <config.h>
#include "twsi/mvTwsi.h"

#if CONFIG_COMMANDS & CFG_CMD_I2C

#define MAX_I2C_RETRYS	    10
#define I2C_DELAY	    300  /* Should be at least the # of MHz of Tclk */
#undef	DEBUG_I2C
//#define DEBUG_I2C

#ifdef DEBUG_I2C
#define DP(x) x
#else
#define DP(x)
#endif

/* Assuming that there is only one master on the bus (us) */

void
i2c_init(MV_U8 chanNum, int speed, int slaveaddr)
{
	MV_TWSI_ADDR slave;
	slave.type = ADDR7_BIT;
	slave.address = slaveaddr;
	mvTwsiInit(chanNum, speed, CFG_TCLK, &slave, 0);
}

/*
 * Read interface:
 *   dev_addr:I2C chip address, range 0..127
 *   offset:  Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   data:    Where to read the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int
i2c_read(MV_U8 chanNum, MV_U8 dev_addr, unsigned int offset, int alen, MV_U8* data, int len)
{
	MV_TWSI_SLAVE	twsiSlave;
	unsigned int i2cFreq = CFG_I2C_SPEED;
	DP(puts("i2c_read\n"));
	twsiSlave.slaveAddr.type = ADDR7_BIT;
	twsiSlave.slaveAddr.address = dev_addr;
	if(alen != 0){
		twsiSlave.validOffset = MV_TRUE;
		twsiSlave.offset = offset;
		if(alen == 2)
		{
			twsiSlave.moreThen256 = MV_TRUE;
		}
		else
		{
			twsiSlave.moreThen256 = MV_FALSE;
		}
	}
	
	i2c_init(chanNum, i2cFreq,0); /* set the i2c frequency */
	return mvTwsiRead (chanNum, &twsiSlave, data, len);
}

/*
 * Write interface:
 *   dev_addr:I2C chip address, range 0..127
 *   offset:  Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   data:    Where to write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */

uchar
i2c_write(MV_U8 chanNum, uchar dev_addr, unsigned int offset, int alen, uchar* data, int len)
{
	MV_TWSI_SLAVE twsiSlave;
	unsigned int i2cFreq = CFG_I2C_SPEED;
	DP(puts("i2c_write\n"));
	twsiSlave.slaveAddr.type = ADDR7_BIT;
	twsiSlave.slaveAddr.address = dev_addr;
	if(alen != 0){
		twsiSlave.validOffset = MV_TRUE;
		twsiSlave.offset = offset;
		if(alen == 2)
		{
			twsiSlave.moreThen256 = MV_TRUE;
		}
		else
		{
			twsiSlave.moreThen256 = MV_FALSE;
		}
	}

	i2c_init(chanNum, i2cFreq,0); /* set the i2c frequency */	

 	return mvTwsiWrite (chanNum, &twsiSlave, data, len);
}

/* function to determine if an I2C device is present */
/* chip = device address of chip to check for */
/* */
/* returns 0 = sucessful, the device exists */
/*         anything other than zero is failure, no device */
int i2c_probe (MV_U8 chanNum, uchar chip)
{

        /* We are just looking for an <ACK> back. */
        /* To see if the device/chip is there */

	MV_TWSI_ADDR eepromAddress;

	unsigned int status = 0;
	unsigned int i2cFreq = CFG_I2C_SPEED;

	DP(puts("i2c_probe\n"));

	i2c_init(chanNum, i2cFreq,0); /* set the i2c frequency */

	status = mvTwsiStartBitSet(chanNum);

	if (status) {
		DP(printf("Transaction start failed: 0x%02x\n", status));
		mvTwsiStopBitSet(chanNum);
		return (int)status;
	}

	eepromAddress.type = ADDR7_BIT;
	eepromAddress.address = chip;

	status = mvTwsiAddrSet(chanNum, &eepromAddress, MV_TWSI_WRITE); /* send the slave address */
	if (status) {
		DP(printf("Failed to set slave address: 0x%02x\n", status));
		mvTwsiStopBitSet(chanNum);
		return (int)status;
	}
	DP(printf("address %#x returned %#x\n",chip,MV_REG_READ(TWSI_STATUS_BAUDE_RATE_REG(chanNum))));

        /* issue a stop bit */
        mvTwsiStopBitSet(chanNum);

	DP(printf("*** successful completion \n"));
        return 0; /* successful completion */
}
#endif
