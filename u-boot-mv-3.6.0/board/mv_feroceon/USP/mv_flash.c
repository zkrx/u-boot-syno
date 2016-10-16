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
#include <command.h>
#include <config.h>
#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
#include <console.h>
#include "norflash/mvFlash.h"
#include "device/mvDeviceRegs.h"
#include "mvBoardEnvSpec.h"

#include "mvOs.h"
#include "mvCtrlEnvLib.h"
#include "mvSysHwConfig.h"
#include "mvCpuIf.h"

#if defined (MV_INCLUDE_INTEG_MFLASH)
#include "mvMFlash.h"
#include "mvMFlashSpec.h"
#include "mvSMFlash.h"
#include "mvPMFlash.h"
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
#include "sflash/mvSFlash.h"
#endif

/* #define MV_DEBUG */
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#define FLASH_ROM       0xFFFD       /* unknown flash type                   */
#define FLASH_RAM       0xFFFE       /* unknown flash type                   */
#define FLASH_MAN_UNKNOWN 0xFFFF0000
#define NL_HASHES_PER_LINE 65

#define CFI_CMDSET_INTEL_EXTENDED   1
#define CFI_CMDSET_AMD_STANDARD	    2
#define CFI_CMDSET_INTEL_STANDARD   3
#define CFI_CMDSET_AMD_EXTENDED	    4

extern unsigned int whoAmI(void);

void flashInfoCopy(flash_info_t *flash_info, MV_FLASH_INFO *pFlash);
MV_U32 getMvFlashInfoNum(flash_info_t *flash_info);
MV_FLASH_INFO* getMvFlashInfo(int index);
int mv_flash_real_protect(flash_info_t *info, long sector, int prot);
int mv_flash_real_protect_bank(flash_info_t *info, int prot);
#if defined (MV_INCLUDE_INTEG_MFLASH)
void mflash_print_info (MV_MFLASH_INFO * pInfo);
unsigned long mflash_init (MV_MFLASH_INFO * mflInfo);
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
void sflash_print_info (MV_SFLASH_INFO * pInfo);
unsigned long sflash_init (MV_SFLASH_INFO * sflInfo);
#endif

/* use CFG_MAX_FLASH_BANKS_DETECT if defined */
#ifdef CFG_MAX_FLASH_BANKS_DETECT
static MV_FLASH_INFO   mvFlashInfo[CFG_MAX_FLASH_BANKS_DETECT];
extern flash_info_t	flash_info[CFG_MAX_FLASH_BANKS_DETECT];	/* FLASH chips info */
extern int mv_board_num_flash_banks;
#else
static MV_FLASH_INFO   mvFlashInfo[CFG_MAX_FLASH_BANKS];
extern flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */
#endif

/* Static Variables */
#if defined (MV_INCLUDE_INTEG_MFLASH)
static MV_MFLASH_INFO  mflash;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
static MV_SFLASH_INFO  sflash = {0x0, 0, 0, 0, 0, 0, MV_INVALID_DEVICE_NUMBER};
#endif

static unsigned long norFlashBanksNum = 0;
static unsigned long mFlashBanksNum = 0;
static unsigned long sFlashBanksNum = 0;

typedef enum
{
    MV_UNKNOWN_FLASH = 0,
    MV_NOR_FLASH,
    MV_MARVELL_FLASH,
    MV_SPI_FLASH
} MV_FLASH_TYPE;

/* return NULL in case didn't find a matching mvFlashInfo */
MV_FLASH_INFO* getMvFlashInfo(int index)
{
	if (CFG_MAX_FLASH_BANKS < index)
		return NULL;
	return &mvFlashInfo[index];
}

/* based on index get the type of flash */
static MV_FLASH_TYPE mvFlashTypeFromIndexGet(MV_U32 indx)
{
    if ((indx >= 0) && (indx < norFlashBanksNum))
        return MV_NOR_FLASH;
    else if ((indx >= norFlashBanksNum) && \
             (indx < (norFlashBanksNum + mFlashBanksNum)))
        return MV_MARVELL_FLASH;
    else if ((indx >= (norFlashBanksNum + mFlashBanksNum)) && 
             (indx < (norFlashBanksNum + mFlashBanksNum + sFlashBanksNum)))
        return MV_SPI_FLASH;

    return MV_UNKNOWN_FLASH;
}

/* return 0xFFFFFFFF in case didn't find a matching mvFlashInfo */
MV_U32	
getMvFlashInfoNum(flash_info_t * pFlash_info)
{
	MV_U32 i;

	DB(mvOsPrintf("%s\n", __FUNCTION__));
	if( pFlash_info->flash_id == FLASH_UNKNOWN )
	{
		DB(printf("%s : ERROR didn't find mvFlashInfo matching to flash_info_t.\n", __FUNCTION__));
		return 0xFFFFFFFF;
	}

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++)
	{
		if (&flash_info[i] == pFlash_info)		
		{
			DB(printf("getMvFlashInfoNum found mvFlashInfo %d \n",i));
			return i;		
		}
		DB(mvOsPrintf(" %x %x \n",mvFlashBaseAddrGet(&mvFlashInfo[i]), mvFlashVenIdGet(&mvFlashInfo[i])));
	}

	DB(mvOsPrintf("%s(%d): ERROR didn't find mvFlashInfo matching to flash_info_t.\n", __FUNCTION__));

	return 0xFFFFFFFF;
}


/* make sure that #define CFG_FLASH_PROTECTION present in MV64xx.h */
 
int 
mv_flash_real_protect(flash_info_t *info, long sector, int prot)
{
	MV_U32 s;
	s = getMvFlashInfoNum(info);
	if(s >= CFG_MAX_FLASH_BANKS)
	{
		DB(printf("mv_flash_real_protect illegal mvFlashInfo \n")); 
		return 1;
	}

    switch(mvFlashTypeFromIndexGet(s))
    {
#if defined (MV_INC_BOARD_NOR_FLASH)
        case MV_NOR_FLASH:
            if( mvFlashSecLockSet(&mvFlashInfo[s],sector,prot) != MV_OK)
		        return 1; /* Write was not completed */
	        info->protect[sector] = prot;   
            break;
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
        case MV_MARVELL_FLASH:
            /* printf("MFlash WP modification per sector is not supported by this command!!\n"); */
	    if(mv_flash_real_protect_bank(info, prot) != MV_OK)
		return 1;
            break;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
        case MV_SPI_FLASH:
            /* printf("SPI Flash WP modification per sector is not supported by this command!!\n"); */
	    if(mv_flash_real_protect_bank(info, prot) != MV_OK)
		return 1;
            break;
#endif

        default:
		    return 1;
    }
	
	return 0; /* write completed succefully */
}

int 
mv_flash_real_protect_bank(flash_info_t *info, int prot)
{
    MV_U32 s;    
    MV_U32 i;

    s = getMvFlashInfoNum(info);
    if(s >= CFG_MAX_FLASH_BANKS)
    {
	DB(printf("mv_flash_real_protect_bank illegal mvFlashInfo \n")); 
	return 1;
    }

    switch(mvFlashTypeFromIndexGet(s))
    {
#if defined (MV_INC_BOARD_NOR_FLASH)        
        case MV_NOR_FLASH:
            for (i=0; i<info->sector_count; ++i) 
            {
                if( mvFlashSecLockSet(&mvFlashInfo[s],i,prot) != MV_OK)
		            return 1; /* Write was not completed */

                info->protect[i] = prot;   
                putc ('.');
            }
            break;
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
        case MV_MARVELL_FLASH:
            if (mvMFlashWriteProtectSet(&mflash, prot) != MV_OK)
                return 1;

            for (i=0; i<info->sector_count; ++i) 
                info->protect[i] = prot;   
            break;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
        case MV_SPI_FLASH:
            if (prot)
            {
                if (mvSFlashWpRegionSet(&sflash, MV_WP_ALL) != MV_OK)
                    return 1;
            }
            else
            {
                if (mvSFlashWpRegionSet(&sflash, MV_WP_NONE) != MV_OK)
                    return 1;
            }

            for (i=0; i<info->sector_count; ++i) 
                info->protect[i] = prot;   
            break;
#endif

        default:
		    return 1;
    }
	
	return 0; /* write completed succefully */
}

/********************************************************************
* Copy global CFI info into Marvell FLash info
********************************************************************/
void flashMvInfoCopy(flash_info_t *flash_info, MV_FLASH_INFO *pFlash)
{
	MV_U32 i;

	/* Check if flash_info containe CFI data */	
	if(mvFlashVenIdGet(pFlash) != FLASH_UNKNOWN)
	{
		return;
	}		

	if(flash_info->flash_id == FLASH_UNKNOWN)
	{
		pFlash->flashSpec.flashVen = FLASH_UNKNOWN;
		return;
	}		
	
	/* convert to MV_FLASH_INFO manufactor Id */
	switch (flash_info->vendor) 
	{
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			pFlash->flashSpec.flashVen = INTEL_MANUF;
		break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			pFlash->flashSpec.flashVen = AMD_MANUF;
		break;
		default:
			pFlash->flashSpec.flashVen = 0;
		break;
	}

	/* convert to MV_FLASH_INFO flash Id */
	pFlash->flashSpec.flashId = (flash_info->flash_id & FLASH_TYPEMASK);
	
	/* other flash info parameters */
	pFlash->flashSpec.size = (flash_info->size / (flash_info->portwidth / flash_info->chipwidth));
	pFlash->flashSpec.sectorNum = flash_info->sector_count;
	pFlash->busWidth = flash_info->portwidth;
	pFlash->devWidth = flash_info->chipwidth;

	/* sectors parameters */
	for(i = 0; i <  flash_info->sector_count; i++)
	{
		pFlash->sector[i].baseOffs = flash_info->start[i] - flash_info->start[0];
		pFlash->sector[i].protect = flash_info->protect[i];
		if (i + 1 <  flash_info->sector_count)
			pFlash->sector[i].size = flash_info->start[i+1] - flash_info->start[i];
		else
			pFlash->sector[i].size = (flash_info->start[0] + flash_info->size) - flash_info->start[i];
	} 

	pFlash->baseAddr = flash_info->start[0];

	return;
}

void 	
flashInfoCopy(flash_info_t *flash_info, MV_FLASH_INFO *pFlash)
{
	MV_U32 i;

	/* Check if flash_info containe CFI data */	
	if(flash_info->flash_id != FLASH_UNKNOWN)
	{
		return;
	}		

	if(mvFlashVenIdGet(pFlash) == FLASH_UNKNOWN)
	{
		flash_info->flash_id = FLASH_UNKNOWN;
		return;
	}		
	/* convert to U-Boot manufactor Id */
	flash_info->flash_id = ((mvFlashVenIdGet(pFlash) << 16) & FLASH_VENDMASK);
	/* convert to U-Boot flash Id */
	flash_info->flash_id |= (mvFlashDevIdGet(pFlash) & FLASH_TYPEMASK);
	
	/* other flash info parameters */
	flash_info->size = mvFlashSizeGet(pFlash);
	flash_info->sector_count = mvFlashNumOfSecsGet(pFlash);
	flash_info->portwidth = mvFlashBusWidthGet(pFlash);
	flash_info->chipwidth = mvFlashDevWidthGet(pFlash);

	/* sectors parameters */
	for(i = 0; i < mvFlashNumOfSecsGet(pFlash); i++)
	{
		flash_info->start[i] = mvFlashSecOffsGet(pFlash,i) + mvFlashBaseAddrGet(pFlash);
		flash_info->protect[i] = mvFlashSecLockGet(pFlash,i);
	} 

	return;
}

#if defined (MV_INCLUDE_INTEG_MFLASH)
void 	
mflashInfoCopy(flash_info_t *flash_info, MV_MFLASH_INFO *pFlash)
{
	MV_U32 i;
    MV_U32 mfId;
    MV_U16 devId;
    MV_BOOL wp;

    /* Check if the Model was detected during the init stage and read the WP status */
	if ((pFlash->flashModel == MV_MFLASH_MODEL_UNKNOWN) ||
        (mvMFlashIdGet(pFlash, &mfId, &devId) != MV_OK) ||
        (mvMFlashWriteProtectGet(pFlash, &wp) != MV_OK))
	{
		flash_info->flash_id = FLASH_UNKNOWN;
		return;
	}		

	/* Discard the manufacturer ID and use the device ID alnoe because we lack 16 bits */
	flash_info->flash_id = devId;
	
	/* other flash info parameters */
	flash_info->size = (pFlash->sectorSize * pFlash->sectorNumber);
	flash_info->sector_count = pFlash->sectorNumber;
	flash_info->portwidth = 1;  /* Assuming 8bit although that this parameter is not used */
	flash_info->chipwidth = 1;  /* Assuming 8bit although that this parameter is not used */       

	/* sectors parameters */
	for(i = 0; i < pFlash->sectorNumber; i++)
	{
		flash_info->start[i] = (pFlash->baseAddr + (i * pFlash->sectorSize));
	    flash_info->protect[i] = wp;
	} 

	return;
}
#endif /* #if defined (MV_INCLUDE_INTEG_MFLASH) */

#if defined (MV_INC_BOARD_SPI_FLASH)
void 	
sflashInfoCopy(flash_info_t *flash_info, MV_SFLASH_INFO *pFlash)
{
	MV_U32 i;
    MV_SFLASH_WP_REGION wp;
    MV_BOOL wpVal;

    /* Check if the Model was detected during the init stage and read thw WP status */
	if ((pFlash->manufacturerId == 0) ||
        (mvSFlashWpRegionGet(pFlash, &wp) != MV_OK))
	{
		flash_info->flash_id = FLASH_UNKNOWN;
		return;
	}		

	/* Discard the manufacturer ID and use the device ID alnoe because we lack 16 bits */
	flash_info->flash_id = ((pFlash->manufacturerId << 16) | pFlash->deviceId);
	
	/* other flash info parameters */
	flash_info->size = (pFlash->sectorSize * pFlash->sectorNumber);
	flash_info->sector_count = pFlash->sectorNumber;
	flash_info->portwidth = 0;  /* Serial interface */
	flash_info->chipwidth = 0;  /* Serial interface */       

    if (wp == MV_WP_ALL)
        wpVal = MV_TRUE;
    else
        wpVal = MV_FALSE;

	/* sectors parameters */
	for(i = 0; i < pFlash->sectorNumber; i++)
	{
		flash_info->start[i] = (pFlash->baseAddr + (i * pFlash->sectorSize));
	    flash_info->protect[i] = wpVal;
	} 

	return;
}
#endif

unsigned long
mvFlash_init (void)
{

	MV_U32 i;
	unsigned long current_bank_size=0;
	unsigned long flash_size=0;
	int spi_access = 0;
	char *env;

#if defined (MV_INC_BOARD_SPI_FLASH) || defined (MV_INCLUDE_INTEG_MFLASH)
	MV_U32 bankIndx;
#endif /* defined (MV_INC_BOARD_SPI_FLASH) || defined (MV_INCLUDE_INTEG_MFLASH) */

    /* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		mvFlashInfo[i].flashSpec.flashVen = FLASH_UNKNOWN;
	}

#if defined (MV_INC_BOARD_NOR_FLASH)
    
    /* Get the number of devices of each Flash type */
    norFlashBanksNum = mvBoardGetDevicesNumber(BOARD_DEV_NOR_FLASH);
	
	DB(printf("Number of NOR flashes on board %d\n",norFlashBanksNum);)    

    if (norFlashBanksNum > CFG_MAX_FLASH_BANKS)
    {
        printf("%s ERROR: Number of flash devices exceeds the CFG_MAX_FLASH_BANKS=0x%x\n", 
	__FUNCTION__,CFG_MAX_FLASH_BANKS);
        return 0;
    }

	for (i=0; i < norFlashBanksNum; ++i)
	{
		/* CFI driver detected the device */
		if (flash_info[i].flash_id != FLASH_UNKNOWN)
		{
			printf("[%ldkB@%x] ", flash_info[i].size/1024, flash_info[i].start[0]);		
			continue;
		}

		mvFlashInfo[i].baseAddr = mvBoardGetDeviceBaseAddr(i, BOARD_DEV_NOR_FLASH);
		DB(printf("mvFlashInfo[%d].baseAddr %x\n",i,mvFlashInfo[i].baseAddr);)
		mvFlashInfo[i].busWidth = mvBoardGetDeviceBusWidth(i, BOARD_DEV_NOR_FLASH)/8;
		DB(printf("mvFlashInfo[%d].busWidth %x\n",i,mvFlashInfo[i].busWidth);)
		mvFlashInfo[i].devWidth = mvBoardGetDeviceWidth(i, BOARD_DEV_NOR_FLASH)/8;
		DB(printf("mvFlashInfo[%d].devWidth %x\n",i,mvFlashInfo[i].devWidth);)

		if (mvFlashInfo[i].baseAddr == 0xffffffff) continue;

		current_bank_size = mvFlashInit(&mvFlashInfo[i]);

		flashInfoCopy(&flash_info[i], &mvFlashInfo[i]);

		printf("[%ldkB@%x] ", flash_info[i].size/1024, flash_info[i].start[0]);		

		if (flash_info[i].flash_id == FLASH_UNKNOWN) 
        	{
			printf ("## Unknown FLASH at %08x: Size = 0x%08lx = %ld MB\n",
			        mvFlashInfo[i].baseAddr, current_bank_size, current_bank_size<<20);
		}

		flash_size += current_bank_size;
	}
	
	/* Copy from CFI to  MV_FLASH_INFO the CFI data */
	for (i=0; i < norFlashBanksNum; ++i)
		flashMvInfoCopy(&flash_info[i], &mvFlashInfo[i]);

#endif /* #if defined (MV_INC_BOARD_NOR_FLASH) */

#if defined (MV_INCLUDE_INTEG_MFLASH)

#ifdef MV_INCLUDE_INTEG_MFLASH_SPI
	if (mvCtrlSpiBusModeSet(MV_SPI_CONN_TO_MFLASH) != MV_OK)
	{
		printf("Failed to set the SPI bus to the MFlash	mode\n");
	}
#endif
    mflash.ifMode = MV_SPI_CONN_UNKNOWN;        /* to be detected from H/W */
    mflash.baseAddr = mvCpuIfTargetWinBaseLowGet(MFLASH_CS);
    mflash.flashModel = MV_MFLASH_MODEL_UNKNOWN; /* will be detected in init */
    mflash.sectorSize = 0;                       /* will be detected in init */
    mflash.sectorNumber = 0;                     /* will be detected in init */
    mflash.infoSize = 0;                         /* will be detected in init */

    /* Try to initialize the Marvell Flash - returns the size of flash detected */
    if (mflash_init(&mflash) != 0)
    {
        /* update the number of Marvell flashes detected to be used in index calculation */
        mFlashBanksNum = 1;
        DB(printf("Number of Marvell flashes on board %d\n", mFlashBanksNum);)

        /* check that we did not exceed the MAX flash banks supported */
        if ((norFlashBanksNum + mFlashBanksNum) > CFG_MAX_FLASH_BANKS)
        {
            printf("%s ERROR: Number of flash devices exceeds the CFG_MAX_FLASH_BANKS\n", __FUNCTION__);
            return 0;
        }

        /* Set the default sector size to small - needed for saveenv in the UBoot */
        if (mvMFlashSectorSizeSet(&mflash, MV_MFLASH_SECTOR_SIZE_SMALL) != MV_OK)
        {
            printf("%s ERROR: Failed to set MFlash sector size to SMALL\n", __FUNCTION__);
            return 0;
        }

        current_bank_size = (mflash.sectorSize * mflash.sectorNumber);

        bankIndx = norFlashBanksNum;
        mflashInfoCopy(&flash_info[bankIndx], &mflash);

        printf("[%ldkB@%x] ", current_bank_size/1024, mflash.baseAddr);		

	    if (mflash.flashModel == MV_MFLASH_MODEL_UNKNOWN) 
        {
		    printf ("## Unknown MFLASH at %08x: Size = 0x%08lx = %ld MB\n",
		            mflash.baseAddr, current_bank_size, current_bank_size<<20);
	    }

	    flash_size += current_bank_size;
    }
    else
    {
        DB(printf("Failed to initialize the Marvell Flash!\n");)
    }

#endif /* #if defined (MV_INCLUDE_INTEG_MFLASH) */

#if defined (MV_INC_BOARD_SPI_FLASH)

#ifdef MV78200
    /* Check in dual CPU system which CPU use spi */
    if (mvSocUnitIsMappedToThisCpu(SPI_FLASH))
	    spi_access = 1;
#else
    spi_access = 1;
#endif

    if (spi_access)
    {
    sflash.baseAddr         = mvCpuIfTargetWinBaseLowGet(SPI_CS);
    sflash.manufacturerId   = 0;                        /* will be detected in init */
    sflash.deviceId         = 0;                        /* will be detected in init */
    sflash.sectorSize       = 0;                        /* will be detected in init */
    sflash.sectorNumber     = 0;                        /* will be detected in init */
    sflash.pageSize         = 0;                        /* will be detected in init */
    sflash.index            = MV_INVALID_DEVICE_NUMBER; /* will be detected in init */

    /* Try to initialize an external SPI flash and retreive the size in case of success */
    if ((current_bank_size = sflash_init(&sflash)) != 0)
    {
        /* update the number of SPI flashes detected to be used in index calculation */
        sFlashBanksNum = 1; 
        DB(printf("Number of SPI flashes on board %d\n", sFlashBanksNum);)

        /* check that we did not exceed the MAX flash banks supported */
        if ((norFlashBanksNum + mFlashBanksNum + sFlashBanksNum) > CFG_MAX_FLASH_BANKS)
        {
            printf("%s ERROR: Number of flash devices exceeds the CFG_MAX_FLASH_BANKS\n", __FUNCTION__);
            return 0;
        }    

        /* convert the SFlash structure into the Uboot structre */
        bankIndx = (norFlashBanksNum + mFlashBanksNum);
        sflashInfoCopy(&flash_info[bankIndx], &sflash);

        printf("[%ldkB@%x] ", current_bank_size/1024, sflash.baseAddr);		

        /* verify that the manufacturer Id was detected */
	    if (sflash.manufacturerId == 0) 
        {
		    printf ("## Unknown SFLASH at %08x: Size = 0x%08lx = %ld MB\n",
		            sflash.baseAddr, current_bank_size, current_bank_size<<20);
	    }

	    flash_size += current_bank_size;
    }
    else
    {
        DB(printf("Failed to initialize the SPI Flash!\n");)
	}
    }

#endif /* #if defined (MV_INC_BOARD_SPI_FLASH) */ 

    /* Set the write protection */
    if (norFlashBanksNum >= 1)
    {
        flash_protect (FLAG_PROTECT_SET,
		           CFG_MONITOR_BASE,
		           CFG_MONITOR_BASE + CFG_MONITOR_LEN - 1,
		           &flash_info[0]);


	    flash_protect(FLAG_PROTECT_SET,
				      CFG_ENV_ADDR,CFG_ENV_ADDR + CFG_ENV_SECT_SIZE,
				      &flash_info[0]);
    }

    return flash_size;

}


/*-----------------------------------------------------------------------
 */
void
mv_flash_print_info  (flash_info_t *info)
{
	MV_U32 s;

	s = getMvFlashInfoNum(info);
	if(s >= CFG_MAX_FLASH_BANKS)
	{
		printf("missing or unknown FLASH type\n"); 
		return;
	}

    switch(mvFlashTypeFromIndexGet(s))
    {
#if defined (MV_INC_BOARD_NOR_FLASH)
        case MV_NOR_FLASH:
            flashPrint(&mvFlashInfo[s]);
            break;
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
        case MV_MARVELL_FLASH:
            mflash_print_info(&mflash);
            break;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
        case MV_SPI_FLASH:
            sflash_print_info(&sflash);
            break;
#endif

        default:
		    printf("Unknown flash type on selected bank!\n");
    }
	
	return;
}

/*-----------------------------------------------------------------------
 */



int
mv_flash_erase (flash_info_t *info, int s_first, int s_last)
{
	MV_U32 i,s;	
    	MV_STATUS ret;
#if defined (MV_INCLUDE_INTEG_MFLASH)
	MV_BOOL mvMFstatus;
#endif
#if defined (MV_INC_BOARD_SPI_FLASH)
	MV_SFLASH_WP_REGION mvSFstatus;
#endif
	
	DB(printf("%s: sectors %d - %d \n", __FUNCTION__, s_first, s_last));
	s = getMvFlashInfoNum(info);
	if(s >= CFG_MAX_FLASH_BANKS)
	{
		DB(printf("%s: illegal mvFlashInfo \n", __FUNCTION__)); 
		return 1;
	}
	printf("\n");

	for(i = s_first; i <= s_last; i++)
	{		
        switch(mvFlashTypeFromIndexGet(s))
        {
#if defined (MV_INC_BOARD_NOR_FLASH)
            case MV_NOR_FLASH:
                DB(printf("%s: calling mvFlashSecErase sector %d, base=%x, sec=%x\n", __FUNCTION__, i,mvFlashInfo[s].baseAddr, mvFlashInfo[s].sector[i].baseOffs));
                ret = mvFlashSecErase(&mvFlashInfo[s],i);
                break;
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
            case MV_MARVELL_FLASH:
                DB(printf("%s: calling mvMFlashSecErase sector %d, base=%x, sec=%x\n", __FUNCTION__, i,mvFlashInfo[s].baseAddr, mvFlashInfo[s].sector[i].baseOffs));
		/* Check protect on/off */
		mvMFlashWriteProtectGet(&mflash,&mvMFstatus);
		if (mvMFstatus == MV_TRUE)
		{
			ret = MV_FAIL;
			break;
		}
                ret = mvMFlashSecErase(&mflash, i);
                break;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
            case MV_SPI_FLASH:
                DB(printf("%s: calling mvSFlashSectorErase sector %d, base=%x, sec=%x\n", __FUNCTION__, i,mvFlashInfo[s].baseAddr, mvFlashInfo[s].sector[i].baseOffs));
		/* Check protect on/off */

		mvSFlashWpRegionGet (&sflash, &mvSFstatus);
		if (mvSFstatus == MV_WP_ALL)
		{
			ret = MV_FAIL;
			break;
		}
	         ret = mvSFlashSectorErase(&sflash, i);
                break;
#endif

            default:
                mv_flash_print_info(info);
			    return 1;
        }

		if(ret != MV_OK)
		{
			printf("Erase fail!\n");
			mv_flash_print_info(info);
			return 1;
		}
        
        putc ('.');
        if (((i+1) % (NL_HASHES_PER_LINE)) == 0) 
		{
            puts ("\n");
        }
	}
	puts("\n");

	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int
mv_write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	unsigned int s = 0;    

	s = getMvFlashInfoNum(info);
	if(s >= CFG_MAX_FLASH_BANKS)
	{
		DB(printf("mv_write_buff illegal mvFlashInfo \n")); 
		return 1;
	}

    switch(mvFlashTypeFromIndexGet(s))
    {
#if defined (MV_INC_BOARD_NOR_FLASH)
        char *env;

        case MV_NOR_FLASH:
            env = getenv("enaFlashBuf");
	        if(( (strcmp(env,"yes") == 0) || (strcmp(env,"Yes") == 0) ))
	        {
		        if( mvFlashBlockWr(&mvFlashInfo[s],addr - mvFlashBaseAddrGet(&mvFlashInfo[s]) ,cnt,src) != cnt)
			        return 1; /* Write was not completed */
	        }
	        else
	        {
		        if( mvFlashBlockUnbufWr(&mvFlashInfo[s],addr - mvFlashBaseAddrGet(&mvFlashInfo[s]) ,cnt,src) != cnt)
			        return 1; /* Write was not completed */
	        }
            break;
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
        case MV_MARVELL_FLASH:
            if (mvMFlashBlockWr(&mflash, (addr - mflash.baseAddr), cnt, src, MV_FALSE /* no compare */) != MV_OK)
                return 1;   /* Write was not completed */
            break;
#endif

#if defined (MV_INC_BOARD_SPI_FLASH)
        case MV_SPI_FLASH:
            if (mvSFlashBlockWr(&sflash, (addr - sflash.baseAddr), src, cnt) != MV_OK)
                return 1;   /* Write was not completed */
            break;
#endif

        default:
		    return 1;
    }

	return 0; /* write completed succefully */
}

#if (defined (MV_INCLUDE_INTEG_MFLASH)|| defined (MV_INC_BOARD_SPI_FLASH))
/*******************************************************************************
* rangeStrDecode - Decode a range string into 2 integers
*
* DESCRIPTION:
*       Decode a string indicating a range like M-N into 2 integers M and N.
*
********************************************************************************/
static int rangeStrDecode (char * str, int * first, int * last, unsigned int sectNum)
{
    char * sl, * ep;

    if ((sl = strchr (str, '-')) == NULL) /* a single sector */
    {
        /* only a single bank is specified */
        *first = simple_strtoul (str, &ep, 10);
        if ((ep == str) || (*ep != '\0') || (*first >= sectNum) || (*first < 0))
            return -1;

        *last = *first;
        return 0;
    }
    else /* we have a complete range M-N */
    {
        *sl++ = '\0';
        *first = simple_strtoul (str, &ep, 10);
        if ((ep == str) || (*ep != '\0') || (*first >= sectNum) || (*first < 0))
            return -1;

        *last = simple_strtoul (sl, &ep, 10);
        if ((ep == str) || (*ep != '\0') || (*last >= sectNum) || (*last < 0))
            return -1;
    }

    if (*first > *last)
        return -1;

    return 0;
}
#endif /* (defined (MV_INCLUDE_INTEG_MFLASH)|| defined (MV_INC_BOARD_SPI_FLASH)) */

/*#############################################################################*/
/*#############################################################################*/
/*######################     MFLASH   COMMANDS    #############################*/
/*#############################################################################*/
/*#############################################################################*/

#if defined (MV_INCLUDE_INTEG_MFLASH)
/*******************************************************************************
* mflash_init - Detect and Initialize the Integrated Marvell Flash
*
* DESCRIPTION:
*       Try to detect the Marvell Flash integrated in the SOC. Based on the 
*       model detected, determine the flash size
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
unsigned long mflash_init (MV_MFLASH_INFO * mflInfo)
{
    /* based on the SPI mode try to detect the Mflash device interface type */
    if (mvCtrlSpiBusModeDetect() == MV_SPI_CONN_TO_MFLASH)
        mflInfo->ifMode = MV_MFLASH_SPI; 
    else               
        mflInfo->ifMode = MV_MFLASH_PARALLEL;

    /* Try to detect the flash and initialize it over the IF deteced */
    if (mvMFlashInit(mflInfo) != MV_OK)
    {
        printf("%s ERROR: MFlash init failed\n", __FUNCTION__);
        return 0;
    }

    return (mflInfo->sectorSize * mflInfo->sectorNumber);
}

/*******************************************************************************
* mflash_print_info - Print the Marvell flash info
*
* DESCRIPTION:
*       Print the flash information of the specified instance
*
* INPUT:
*       pInfo: pointer to the instance flash information structure.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
void mflash_print_info (MV_MFLASH_INFO * pInfo)
{
    MV_U32 manf = 0;
    MV_U16 dev = 0;
    MV_BOOL wp;

    printf("\nFlash Base Address  : 0x%08x",pInfo->baseAddr);

    if (pInfo->ifMode == MV_MFLASH_SPI)
        printf("\nFlash Mode          : SPI");
    else if (pInfo->ifMode == MV_MFLASH_PARALLEL)
        printf("\nFlash Mode          : Parallel");
    else
        printf("\nFlash Mode          : Unknown");

    if (pInfo->flashModel != 0)
        printf("\nFlash Model         : SUNOL %d",(MV_U32)pInfo->flashModel);
    else
        printf("\nFlash Model         : Unknown");

    printf("\nSector Size         : %dK", (pInfo->sectorSize / 1024));
    printf("\nNumber of sectors   : %d", pInfo->sectorNumber);

    /* Get the Flash ID from the chip */
    if (mvMFlashIdGet(pInfo, &manf, &dev) != MV_OK)
    {
        printf("\n\nFailed to get the Manufacturer and Device IDs!\n");
        return;
    }

    /* Get the WP status from the chip */
    if (mvMFlashWriteProtectGet(pInfo, &wp) != MV_OK)
    {
        printf("\n\nFailed to get the WP status!\n");
        return;
    }

    printf("\nManufacturer ID     : %08x", manf);
    printf("\nDevice ID           : %03x", (dev >> 4));
    printf("\nDevice Revision     : %01x", (dev & 0xF));
    if (wp)
        printf("\nWrite Protection    : Enabled");
    else
        printf("\nWrite Protection    : Disabled");

    printf("\n\n");

}

/*******************************************************************************
* mflash_erase - Erase a set of sectors
*
* DESCRIPTION:
*       Erase a set of sectors in the main flash region.
*
* INPUT:
*       pInfo: pointer to the instance flash information structure.
*       s_first: first sector to erase.
*       s_last: last sector to erase.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
int mflash_erase (MV_MFLASH_INFO *pInfo, MV_U32 s_first, MV_U32 s_last)
{
    MV_U32 i;

    for (i=s_first; i<=s_last; i++)
    {
        /* perform the erase in serial mode */
        if (mvMFlashSecErase(pInfo, i) != MV_OK)
            return 1;
    }

    return 0;
}

/*******************************************************************************
* mflash_cmd - mflash command implimentation
*
* DESCRIPTION:
*       parse and decode the mflash command 
*
* INPUT:
*       cmdtp: command table pointer
*		flag: flags
*		argc: command argument count
*		argv: command argument vector
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*******************************************************************************/
int mflash_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int first, last;
    MV_U32 source, offset, size, dest;
    MV_BOOL verify;
    char * cp;

    if (mFlashBanksNum < 1)
    {
        printf ("No Marvell Flash detected!\n");
		return 1;
    }

    if (argc < 2) 
    {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (strcmp(argv[1], "info") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 2)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        mflash_print_info(&mflash);
	} 
    else if (strcmp(argv[1], "protect") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 3)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        if (strcmp(argv[2], "on") == 0)       /* the whole chip */
        {
            if (mvMFlashWriteProtectSet(&mflash, MV_TRUE) != 0)
                printf("\nFailed to enable write protection!\n");
        }
        else if (strcmp(argv[2], "off") == 0)       /* the whole chip */
        {            
            if (mvMFlashWriteProtectSet(&mflash, MV_FALSE) != 0)
                printf("\nFailed to disable write protection!\n");
        }
        else
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
		    return 1;
        }        
	} 
    else if (strcmp(argv[1], "sector") == 0) 
    {   		
   		MV_U32 i,j;
   		
        /* check that we have the correct number of parameters */
        if (argc != 3)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        if (strcmp(argv[2], "small") == 0)       /* the whole chip */
        {            
            if (mvMFlashSectorSizeSet(&mflash, 0x1000) != 0)
			{
                printf("\nFailed to enable write protection!\n");
                return 1;
			}
        }
        else if (strcmp(argv[2], "large") == 0)       /* the whole chip */
        {           
            if (mvMFlashSectorSizeSet(&mflash, 0x8000) != 0)
            {
            	printf("\nFailed to disable write protection!\n");
            	return 1;
            }
        }
        else
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
		    return 1;
        }        
             
    	/* Find the Mflash entry and update the sector count and protection status */
    	for (i=0; i<CFG_MAX_FLASH_BANKS; i++)
    	{
		if (mvFlashTypeFromIndexGet(i) == MV_MARVELL_FLASH)
		{
                	MV_BOOL wp = MV_FALSE;
			flash_info[i].sector_count = mflash.sectorNumber;

                if (mvMFlashWriteProtectGet(&mflash, &wp) != MV_OK)
                {
                    printf("\nFailed to get the WP status!\n");
            	    return 1;
                }

                /* update the WP status and sector offsets */
                for (j=0; j<flash_info[i].sector_count; j++)
                {
                    flash_info[i].protect[j] = wp;   
                    flash_info[i].start[j] = (mflash.baseAddr + (j * mflash.sectorSize));
                }

				break;		
			}
    	}					
	} 
    else if (strcmp(argv[1], "erase") == 0)     /* erase command */
    {
        /* check that we have the correct number of parameters */
        if (argc != 3)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

	    if (strcmp(argv[2], "chip") == 0)       /* the whole chip */
        {
            if (mvMFlashChipErase(&mflash) != 0)
                printf("\nFailed to erase the whole MFlash chip.\n", first, last);	
        }
        else if (strcmp(argv[2], "main") == 0)  /* only the main region */
        {
            if (mvMFlashMainErase(&mflash) != 0)
                printf("\nFailed to erase the main region.\n", first, last);	
        }
        else if (strcmp(argv[2], "info") == 0)  /* ony the information region */
        {
            if (mvMFlashInfErase(&mflash) != 0)
                printf("\nFailed to erase the information region\n", first, last);	
        }
        else /* check if a range of sectors is specified */
        {
            if (rangeStrDecode(argv[2], &first, &last, mflash.sectorNumber) == 0)
            {
                if (mflash_erase(&mflash, first, last) != 0)
                    printf("\nFailed to erase sectors %d-%d\n", first, last);	
            }
            else
            {
                printf ("Usage:\n%s\n", cmdtp->usage);
		        return 1;
            }
	    }	
	} 
    else if (strcmp(argv[1], "read") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 6)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        offset = simple_strtoul (argv[3], &cp, 16);
        if ((cp == argv[3]) || (*cp != '\0') || (offset < 0))
        {
            printf ("Invalid offset.");
            return 1;
        }

        dest = simple_strtoul (argv[4], &cp, 16);
        if ((cp == argv[4]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid dest.");
            return 1;
        }

        size = simple_strtoul (argv[5], &cp, 16);
        if ((cp == argv[5]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid size.");
            return 1;
        }

        if (strcmp(argv[2], "main") == 0)  /* ony the information region */
        {            
            if (mvMFlashBlockRd(&mflash, offset, size, (MV_U8*) dest) != MV_OK)
                printf ("Failed to read from the MFlash main region!\n");
        }
        else if (strcmp(argv[2], "info") == 0)  /* ony the information region */
        {
            if (mvMFlashBlockInfRd(&mflash, offset, size, (MV_U8*) dest) != MV_OK)
                printf ("Failed to read from the MFlash information region!\n");
        }
        else
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }
	}
    else if (strcmp(argv[1], "write") == 0) 
    {
        /* check that we have the correct number of parameters */
        if ((argc != 6) && (argc != 7))
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        source = simple_strtoul (argv[3], &cp, 16);
        if ((cp == argv[3]) || (*cp != '\0') || (source < 0))
        {
            printf ("Invalid size!\n");
            return 1;
        }

        offset = simple_strtoul (argv[4], &cp, 16);
        if ((cp == argv[4]) || (*cp != '\0') || (offset < 0))
        {
            printf ("Invalid offset!\n");
            return 1;
        }
        size = simple_strtoul (argv[5], &cp, 16);
        if ((cp == argv[5]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid size!\n");
            return 1;
        }

        /* check if the verify flag was used */
        if (argc == 7)
        {
            if (strcmp(argv[6], "verify") != 0)
            {
                printf("Invalid verify flag!\n");
                return 1;
            }

            verify = MV_TRUE;
        }
        else
            verify = MV_FALSE;

	    if (strcmp(argv[2], "main") == 0)  /* ony the information region */
        {
            if (mvMFlashBlockWr(&mflash,offset, size, (MV_U8*) source, verify) != MV_OK)
            {
                printf ("Failed to write to the MFlash main region!\n");
	            return 1;
            }
        }
        else if (strcmp(argv[2], "info") == 0)  /* ony the information region */
        {	      
            if (mvMFlashInfBlockWr(&mflash,offset, size, (MV_U8*) source, verify) != MV_OK)
            {
                printf ("Failed to write to the MFlash information region!\n");
	            return 1;
            }
        }
        else
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }
	}
    else 
    {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}	

	return 1;
}

/*U_BOOT_CMD(name,maxargs,rep,cmd,usage,help)*/
U_BOOT_CMD(
        mflash, 8, 1, mflash_cmd,
        "mflash\t- read, write or erase the Marvell Flash.\n",
        "info\n    - Retreive the Marvell flash infomation.\n"
        "mflash protect <on | off> \n    - Enable or disable write protect over the whole MFlash chip.\n"
        "mflash sector <small | large> \n    - Set the sector size to small (4K).\n"
        "mflash erase <chip | main | info> \n    - Erase the whole chip, main or information region.\n"
        "mflash erase SF[-SL]\n    - Erase (set back to 0xFF) sectors SF-SL in main region.\n"
        "mflash read <main | info> offset dest size\n    - Read data from the main or info region into the destination address.\n"
        "mflash write <main | info> source offset size [verify]\n    - Write a buffer into the main or information region.\n"
);
#endif /* #if defined (MV_INCLUDE_INTEG_MFLASH) */



/*#############################################################################*/
/*#############################################################################*/
/*######################     SFLASH   COMMANDS    #############################*/
/*#############################################################################*/
/*#############################################################################*/

#if defined (MV_INC_BOARD_SPI_FLASH)
/*******************************************************************************
* sflash_init - Detect and Initialize an external SPI flash device
*
* DESCRIPTION:
*       Try to detect the SPI Flash. Based on the model detected, determine 
*       the flash size.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
unsigned long sflash_init (MV_SFLASH_INFO * sflInfo)
{
#if (defined (MV_INCLUDE_SPI) && defined (MV_INCLUDE_INTEG_MFLASH))
    /* First check that SIP mode is configured */
    if (mvCtrlSpiBusModeDetect() != MV_SPI_CONN_TO_EXT_FLASH)
    {
        mvOsPrintf("%s ERROR: SPI interface is not routed to external SPI flash!\n", __FUNCTION__);
        return 0;
    }
#endif

    /* Try to detect the flash and initialize it over SPI */
    if (mvSFlashInit(sflInfo) != MV_OK)
    {
        mvOsPrintf("%s ERROR: SFlash init falied!\n", __FUNCTION__);
        return 0;
    }

    /* return the size of the detected SFLash */
    return (mvSFlashSizeGet(sflInfo));
}

/*******************************************************************************
* sflash_print_info - Print the SPI flash info
*
* DESCRIPTION:
*       Print the flash information of the specified instance
*
* INPUT:
*       pInfo: pointer to the instance flash information structure.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
void sflash_print_info (MV_SFLASH_INFO * pInfo)
{
    MV_SFLASH_WP_REGION wp;

    printf("\nFlash Base Address  : 0x%08x", pInfo->baseAddr);
    printf("\nFlash Model         : %s", mvSFlashModelGet(pInfo));
    printf("\nManufacturer ID     : 0x%02x", pInfo->manufacturerId);
    printf("\nDevice Id           : 0x%04x", pInfo->deviceId);
    printf("\nSector Size         : %dK", (pInfo->sectorSize / 1024));
    printf("\nNumber of sectors   : %d", pInfo->sectorNumber);
    printf("\nPage Size           : %d", pInfo->pageSize);

    if (mvSFlashWpRegionGet(pInfo, &wp) != MV_OK)
    {
        DB(mvOsPrintf("%s WARNING: Failed to get write protect mode!\n", __FUNCTION__);)
        printf("\nWrite Protection    : UNKNOWN");
    }
    else
    {
        switch (wp)
        {
            case MV_WP_NONE:
                printf("\nWrite Protection    : Off");
                break;
            case MV_WP_UPR_1OF64:
                printf("\nWrite Protection    : Upper 1 of 64");
                break;
            case MV_WP_UPR_1OF32:
                printf("\nWrite Protection    : Upper 1 of 32");
                break;
            case MV_WP_UPR_1OF16:
                printf("\nWrite Protection    : Upper 1 of 16");
                break;
            case MV_WP_UPR_1OF8:
                printf("\nWrite Protection    : Upper 1 of 8");
                break;
            case MV_WP_UPR_1OF4:
                printf("\nWrite Protection    : Upper 1 of 4");
                break;
            case MV_WP_UPR_1OF2:
                printf("\nWrite Protection    : Upper 1 of 2");
                break;
            case MV_WP_ALL:
                printf("\nWrite Protection    : All");
                break;
            default:
                DB(mvOsPrintf("%s WARNING: Invalid Write protect mode!\n", __FUNCTION__);)
                printf("\nWrite Protection    : UNKNOWN");
                break;
        }
    }

    printf("\n\n");
}

/*******************************************************************************
* sflash_erase - Erase a set of sectors
*
* DESCRIPTION:
*       Erase a set of sectors in the SPI flash
*
* INPUT:
*       pInfo: pointer to the instance flash information structure.
*       s_first: first sector to erase.
*       s_last: last sector to erase.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
int sflash_erase (MV_SFLASH_INFO *pInfo, MV_U32 s_first, MV_U32 s_last)
{
    MV_U32 i;

    for (i=s_first; i<=s_last; i++)
    {
        /* perform the erase in serial mode */
        if (mvSFlashSectorErase(pInfo, i) != MV_OK)
        {
            DB(mvOsPrintf("%s WARNING: Failed to erase sector %d in SFlash %s!\n", \
                      __FUNCTION__, i, mvSFlashModelGet(pInfo));)
            return 1;
        }
    }

    return 0;
}

/***********************************************************************/
/************************ U-Boot Commands ******************************/
/***********************************************************************/

/*******************************************************************************
* sflash_cmd - sflash command implimentation
*
* DESCRIPTION:
*       parse and decode the sflash command 
*
* INPUT:
*       cmdtp: command table pointer
*		flag: flags
*		argc: command argument count
*		argv: command argument vector
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*******************************************************************************/
int sflash_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int first, last;
    MV_U32 source, offset, size, dest;
    char * cp;

    if (sFlashBanksNum < 1)
    {
        printf ("No SPI Flash detected!\n");
		return 1;
    }

    if (argc < 2) 
    {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (strcmp(argv[1], "info") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 2)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        sflash_print_info(&sflash);
	} 
    else if (strcmp(argv[1], "protect") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 3)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        if (strcmp(argv[2], "off") == 0)
        {
            if (mvSFlashWpRegionSet(&sflash, MV_WP_NONE) != 0)
                printf("\nFailed to set the write protection to NONE.\n");	
        }
        else if (strcmp(argv[2], "on") == 0) 
        {
            if (mvSFlashWpRegionSet(&sflash, MV_WP_ALL) != 0)
                printf("\nFailed to set the write protection to ALL.\n");	
        }
        else
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
		    return 1;
        }        
	}     
    else if (strcmp(argv[1], "erase") == 0)     /* erase command */
    {
        /* check that we have the correct number of parameters */
        if (argc != 3)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

	    if (strcmp(argv[2], "all") == 0)       /* the whole chip */
        {
            if (mvSFlashChipErase(&sflash) != 0)
                printf("\nFailed to erase the whole SPI flash chip.\n");	
        }
        else /* check if a range of sectors is specified */
        {
            if (rangeStrDecode(argv[2], &first, &last, sflash.sectorNumber) == 0)
            {
                if (sflash_erase(&sflash, first, last) != 0)
                    printf("\nFailed to erase sectors %d-%d\n", first, last);	
            }
            else
            {
                printf ("Usage:\n%s\n", cmdtp->usage);
		        return 1;
            }
	    }	
	} 
    else if (strcmp(argv[1], "read") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 5)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        offset = simple_strtoul (argv[2], &cp, 16);
        if ((cp == argv[2]) || (*cp != '\0') || (offset < 0))
        {
            printf ("Invalid offset.");
            return 1;
        }

        dest = simple_strtoul (argv[3], &cp, 16);
        if ((cp == argv[3]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid size.");
            return 1;
        }

        size = simple_strtoul (argv[4], &cp, 16);
        if ((cp == argv[4]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid size.");
            return 1;
        }
        
        if (mvSFlashBlockRd(&sflash, offset, (MV_U8*)dest, size) != MV_OK)
        {
            printf ("Failed to read from the SPI flash!\n");
            return 1;
        }
	}
    else if (strcmp(argv[1], "write") == 0) 
    {
        /* check that we have the correct number of parameters */
        if (argc != 5)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
	        return 1;
        }

        source = simple_strtoul (argv[2], &cp, 16);
        if ((cp == argv[2]) || (*cp != '\0') || (source < 0))
        {
            printf ("Invalid size.");
            return 1;
        }

        offset = simple_strtoul (argv[3], &cp, 16);
        if ((cp == argv[3]) || (*cp != '\0') || (offset < 0))
        {
            printf ("Invalid offset.");
            return 1;
        }
        size = simple_strtoul (argv[4], &cp, 16);
        if ((cp == argv[4]) || (*cp != '\0') || (size < 0))
        {
            printf ("Invalid size!\n");
            return 1;
        }

        if (mvSFlashBlockWr(&sflash, offset, (MV_U8*) source, size) != MV_OK)
        {
            printf ("Failed to write to the SPI flash!\n");
            return 1;
        }
	}
    else 
    {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}	

	return 1;
}

/*U_BOOT_CMD(name,maxargs,rep,cmd,usage,help)*/
U_BOOT_CMD(
        sflash, 6, 1, sflash_cmd,
        "sflash\t- read, write or erase the external SPI Flash.\n",
        "info\n    - Retreive the SPI flash infomation.\n"
        "sflash protect <on | off> \n    - Enable or diasble write protection over the whole SPI flash.\n"
        "sflash erase all \n    - Erase (set back to 0xFF) the whole chip.\n"
        "sflash erase SF[-SL]\n    - Erase (set back to 0xFF) sectors SF-SL.\n"
        "sflash read offset dest size\n    - Read data from the flash into the destination address.\n"
        "sflash write source offset size\n    - Program a buffer from the source address into the flash offset (size in bytes).\n"
);
#endif
#endif /* #if (CONFIG_COMMANDS & CFG_CMD_FLASH) */

