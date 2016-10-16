/*
 * (C) Copyright 2005 2N TELEKOMUNIKACE, Ladislav Michl
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

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>
#include <mvTypes.h>
#include "mvBoardEnvLib.h"
#ifdef MV78200
extern MV_BOOL mvSocUnitIsMappedToThisCpu(MV_SOC_UNIT unit);
#endif

/*
 *	hardware specific access to control-lines
 */
#define	MASK_CLE	0x01
#define	MASK_ALE	0x02


static void mv_nand_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
	/* TODO add auto detect for nand width */
	MV_U32 shift = 0;
	
	if (shift == MV_ERROR)
	{
		printf("No NAND detection\n");
		shift = 0;
	}

	IO_ADDR_W &= ~((MASK_ALE|MASK_CLE) << shift);
	switch (cmd) {
		case NAND_CTL_SETCLE: IO_ADDR_W |= (MASK_CLE << shift); break;
		case NAND_CTL_SETALE: IO_ADDR_W |= (MASK_ALE << shift); break;
	}
	this->IO_ADDR_W = (void *) IO_ADDR_W;
}


int board_nand_init(struct nand_chip *nand)
{
#ifdef MV78200
    /* Check in dual CPU system which CPU use nand */
 /*   if (mvSocUnitIsMappedToThisCpu(NAND_FLASH))
    {*/
#endif
#if defined(MV_LARGE_PAGE)
    	nand->options = NAND_SAMSUNG_LP_OPTIONS;
#endif

    	nand->eccmode = NAND_ECC_SOFT;
    	nand->hwcontrol = mv_nand_hwcontrol;
    	nand->chip_delay = 30;
    	return 0;
#ifdef MV78200
/*    }
    else
    	return 1;*/
#endif
}
#endif
