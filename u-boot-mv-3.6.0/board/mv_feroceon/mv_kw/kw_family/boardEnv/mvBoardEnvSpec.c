/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

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
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#include "mvCommon.h"
#include "mvBoardEnvLib.h"
#include "mvBoardEnvSpec.h"
#include "twsi/mvTwsi.h"

#define DB_88F6281A_BOARD_PCI_IF_NUM            0x0
#define DB_88F6281A_BOARD_TWSI_DEF_NUM		    0x7
#define DB_88F6281A_BOARD_MAC_INFO_NUM		    0x2
#define DB_88F6281A_BOARD_GPP_INFO_NUM		    0x1
#define DB_88F6281A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6281A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define DB_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define DB_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define DB_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define DB_88F6281A_BOARD_DEBUG_LED_NUM		    0x0
#define DB_88F6281A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define DB_88F6281A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_88F6281A_BOARD_NAND_CONTROL		        0x01c00541


MV_BOARD_TWSI_INFO	db88f6281AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6281AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_MPP_TYPE_INFO db88f6281AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_AUTO}
	}; 

MV_BOARD_GPP_INFO db88f6281AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_TSU_DIRCTION, 33}
	/*muxed with TDM/Audio module via IOexpender
	{BOARD_GPP_SDIO_DETECT, 38},
	{BOARD_GPP_USB_VBUS, 49}*/
	};

MV_DEV_CS_INFO db88f6281AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */         
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
	 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	db88f6281AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6281A_MPP0_7,		
	DB_88F6281A_MPP8_15,		
	DB_88F6281A_MPP16_23,		
	DB_88F6281A_MPP24_31,		
	DB_88F6281A_MPP32_39,		
	DB_88F6281A_MPP40_47,		
	DB_88F6281A_MPP48_55		
	}}};


MV_BOARD_INFO db88f6281AInfo = {
	"DB-88F6281A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6281A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6281AInfoBoardMppTypeInfo,
	DB_88F6281A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6281AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6281A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6281AInfoBoardDeCsInfo,
	DB_88F6281A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6281AInfoBoardTwsiDev,					
	DB_88F6281A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6281AInfoBoardMacInfo,
	DB_88F6281A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6281AInfoBoardGppInfo,
	DB_88F6281A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,						/* ledsPolarity */		
	DB_88F6281A_OE_LOW,				/* gppOutEnLow */
	DB_88F6281A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6281A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6281A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6281A_BOARD_NAND_READ_PARAMS,
    DB_88F6281A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6281A_BOARD_NAND_CONTROL
};


#define RD_88F6281A_BOARD_PCI_IF_NUM		0x0
#define RD_88F6281A_BOARD_TWSI_DEF_NUM		0x2
#define RD_88F6281A_BOARD_MAC_INFO_NUM		0x2
#define RD_88F6281A_BOARD_GPP_INFO_NUM		0x5
#define RD_88F6281A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6281A_BOARD_MPP_CONFIG_NUM		0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define RD_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define RD_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define RD_88F6281A_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define RD_88F6281A_BOARD_DEBUG_LED_NUM		0x0
#define RD_88F6281A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define RD_88F6281A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define RD_88F6281A_BOARD_NAND_CONTROL		        0x01c00541

MV_BOARD_MAC_INFO rd88f6281AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0xa},
    {BOARD_MAC_SPEED_AUTO, 0xb}
	}; 

MV_BOARD_SWITCH_INFO rd88f6281AInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode, MV_32 switchOnPort} */
	{{38, {0, 1, 2, 3, -1}, 5, 2, 0},
	 {-1, {-1}, -1, -1, -1}};

MV_BOARD_TWSI_INFO	rd88f6281AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0xFF, ADDR7_BIT}, /* dummy entry to align with modules indexes */
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6281AInfoBoardMppTypeInfo[] = 
	{{MV_BOARD_RGMII, MV_BOARD_TDM}
	}; 

MV_DEV_CS_INFO rd88f6281AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
		 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_GPP_INFO rd88f6281AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_GPP_SDIO_DETECT, 28},
    {BOARD_GPP_USB_OC, 29},
    {BOARD_GPP_WPS_BUTTON, 35},
    {BOARD_GPP_MV_SWITCH, 38},
    {BOARD_GPP_USB_VBUS, 49}
	};

MV_BOARD_MPP_INFO	rd88f6281AInfoBoardMppConfigValue[] = 
	{{{
	RD_88F6281A_MPP0_7,		
	RD_88F6281A_MPP8_15,		
	RD_88F6281A_MPP16_23,		
	RD_88F6281A_MPP24_31,		
	RD_88F6281A_MPP32_39,		
	RD_88F6281A_MPP40_47,		
	RD_88F6281A_MPP48_55		
	}}};

MV_BOARD_INFO rd88f6281AInfo = {
	"RD-88F6281A",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6281A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6281AInfoBoardMppTypeInfo,
	RD_88F6281A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6281AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6281A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6281AInfoBoardDeCsInfo,
	RD_88F6281A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f6281AInfoBoardTwsiDev,					
	RD_88F6281A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6281AInfoBoardMacInfo,
	RD_88F6281A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6281AInfoBoardGppInfo,
	RD_88F6281A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	RD_88F6281A_OE_LOW,				/* gppOutEnLow */
	RD_88F6281A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6281A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6281A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	rd88f6281AInfoBoardSwitchInfo,			/* pSwitchInfo */
    RD_88F6281A_BOARD_NAND_READ_PARAMS,
    RD_88F6281A_BOARD_NAND_WRITE_PARAMS,
    RD_88F6281A_BOARD_NAND_CONTROL
};


#define DB_88F6192A_BOARD_PCI_IF_NUM            0x0
#define DB_88F6192A_BOARD_TWSI_DEF_NUM		    0x7
#define DB_88F6192A_BOARD_MAC_INFO_NUM		    0x2
#define DB_88F6192A_BOARD_GPP_INFO_NUM		    0x3
#define DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6192A_BOARD_MPP_CONFIG_NUM		0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define DB_88F6192A_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define DB_88F6192A_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define DB_88F6192A_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define DB_88F6192A_BOARD_DEBUG_LED_NUM		    0x0
#define DB_88F6192A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define DB_88F6192A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_88F6192A_BOARD_NAND_CONTROL		        0x01c00541

MV_BOARD_TWSI_INFO	db88f6192AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6192AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_MPP_TYPE_INFO db88f6192AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO db88f6192AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
		 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_GPP_INFO db88f6192AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
    {BOARD_GPP_SDIO_WP, 20},
	{BOARD_GPP_USB_VBUS, 22},
	{BOARD_GPP_SDIO_DETECT, 23},
	};

MV_BOARD_MPP_INFO	db88f6192AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6192A_MPP0_7,		
	DB_88F6192A_MPP8_15,		
	DB_88F6192A_MPP16_23,		
	DB_88F6192A_MPP24_31,		
	DB_88F6192A_MPP32_35
	}}};

MV_BOARD_INFO db88f6192AInfo = {
	"DB-88F6192A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6192AInfoBoardMppTypeInfo,
	DB_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	DB_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6192AInfoBoardDeCsInfo,
	DB_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6192AInfoBoardTwsiDev,					
	DB_88F6192A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6192AInfoBoardMacInfo,
	DB_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6192AInfoBoardGppInfo,
	DB_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6192A_BOARD_NAND_READ_PARAMS,
    DB_88F6192A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6192A_BOARD_NAND_CONTROL
};

MV_BOARD_INFO db88f6701AInfo = {
	"DB-88F6701A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6192AInfoBoardMppTypeInfo,
	DB_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	DB_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6192AInfoBoardDeCsInfo,
	DB_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6192AInfoBoardTwsiDev,					
	DB_88F6192A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6192AInfoBoardMacInfo,
	DB_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6192AInfoBoardGppInfo,
	DB_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6192A_BOARD_NAND_READ_PARAMS,
    DB_88F6192A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6192A_BOARD_NAND_CONTROL
};

MV_BOARD_INFO db88f6702AInfo = {
	"DB-88F6702A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6192AInfoBoardMppTypeInfo,
	DB_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	DB_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6192AInfoBoardDeCsInfo,
	DB_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6192AInfoBoardTwsiDev,					
	DB_88F6192A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6192AInfoBoardMacInfo,
	DB_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6192AInfoBoardGppInfo,
	DB_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6192A_BOARD_NAND_READ_PARAMS,
    DB_88F6192A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6192A_BOARD_NAND_CONTROL
};

#define DB_88F6190A_BOARD_MAC_INFO_NUM		0x1

MV_BOARD_INFO db88f6190AInfo = {
	"DB-88F6190A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6192AInfoBoardMppTypeInfo,
	DB_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	DB_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6192AInfoBoardDeCsInfo,
	DB_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6192AInfoBoardTwsiDev,					
	DB_88F6190A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6192AInfoBoardMacInfo,
	DB_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6192AInfoBoardGppInfo,
	DB_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6192A_BOARD_NAND_READ_PARAMS,
    DB_88F6192A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6192A_BOARD_NAND_CONTROL
};

#define RD_88F6192A_BOARD_PCI_IF_NUM		0x0
#define RD_88F6192A_BOARD_TWSI_DEF_NUM		0x0
#define RD_88F6192A_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6192A_BOARD_GPP_INFO_NUM		0xE
#define RD_88F6192A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6192A_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6192A_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6192A_BOARD_DEBUG_LED_NUM		0x3
#define RD_88F6192A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define RD_88F6192A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define RD_88F6192A_BOARD_NAND_CONTROL		        0x01c00541

MV_U8	rd88f6192AInfoBoardDebugLedIf[] =
	{17, 28, 29};

MV_BOARD_MAC_INFO rd88f6192AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_MPP_TYPE_INFO rd88f6192AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO rd88f6192AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */

MV_BOARD_GPP_INFO rd88f6192AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_USB_VBUS_EN, 10},
	{BOARD_GPP_USB_HOST_DEVICE, 11},
	{BOARD_GPP_RESET, 14},
	{BOARD_GPP_POWER_ON_LED, 15},
	{BOARD_GPP_HDD_POWER, 16},
	{BOARD_GPP_WPS_BUTTON, 24},
	{BOARD_GPP_TS_BUTTON_C, 25},
	{BOARD_GPP_USB_VBUS, 26},
	{BOARD_GPP_USB_OC, 27},
	{BOARD_GPP_TS_BUTTON_U, 30},
	{BOARD_GPP_TS_BUTTON_R, 31},
	{BOARD_GPP_TS_BUTTON_L, 32},
	{BOARD_GPP_TS_BUTTON_D, 34},
	{BOARD_GPP_FAN_POWER, 35}
	};

MV_BOARD_MPP_INFO	rd88f6192AInfoBoardMppConfigValue[] = 
	{{{
	RD_88F6192A_MPP0_7,		
	RD_88F6192A_MPP8_15,		
	RD_88F6192A_MPP16_23,		
	RD_88F6192A_MPP24_31,		
	RD_88F6192A_MPP32_35
	}}};

MV_BOARD_INFO rd88f6192AInfo = {
	"RD-88F6192A-NAS",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6192AInfoBoardMppTypeInfo,
	RD_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6192AInfoBoardDeCsInfo,
	RD_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	NULL,					
	RD_88F6192A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6192AInfoBoardMacInfo,
	RD_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6192AInfoBoardGppInfo,
	RD_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f6192AInfoBoardDebugLedIf,
	0,										/* ledsPolarity */		
	RD_88F6192A_OE_LOW,				/* gppOutEnLow */
	RD_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    RD_88F6192A_BOARD_NAND_READ_PARAMS,
    RD_88F6192A_BOARD_NAND_WRITE_PARAMS,
    RD_88F6192A_BOARD_NAND_CONTROL
};

MV_BOARD_INFO rd88f6190AInfo = {
	"RD-88F6190A-NAS",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6192A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6192AInfoBoardMppTypeInfo,
	RD_88F6192A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6192AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6192A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6192AInfoBoardDeCsInfo,
	RD_88F6192A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	NULL,					
	RD_88F6192A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6192AInfoBoardMacInfo,
	RD_88F6192A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6192AInfoBoardGppInfo,
	RD_88F6192A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f6192AInfoBoardDebugLedIf,
	0,										/* ledsPolarity */		
	RD_88F6192A_OE_LOW,				/* gppOutEnLow */
	RD_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    RD_88F6192A_BOARD_NAND_READ_PARAMS,
    RD_88F6192A_BOARD_NAND_WRITE_PARAMS,
    RD_88F6192A_BOARD_NAND_CONTROL
};

#define DB_88F6180A_BOARD_PCI_IF_NUM		0x0
#define DB_88F6180A_BOARD_TWSI_DEF_NUM		0x5
#define DB_88F6180A_BOARD_MAC_INFO_NUM		0x1
#define DB_88F6180A_BOARD_GPP_INFO_NUM		0x0
#define DB_88F6180A_BOARD_MPP_GROUP_TYPE_NUM	0x2
#define DB_88F6180A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6180A_BOARD_DEVICE_CONFIG_NUM	    0x1
#define DB_88F6180A_BOARD_DEBUG_LED_NUM		0x0
#define DB_88F6180A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define DB_88F6180A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_88F6180A_BOARD_NAND_CONTROL		        0x01c00541

MV_BOARD_TWSI_INFO	db88f6180AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
    {BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
    {BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
    {BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6180AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_GPP_INFO db88f6180AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	/* Muxed with TDM/Audio module via IOexpender
	{BOARD_GPP_USB_VBUS, 6} */
	};

MV_BOARD_MPP_TYPE_INFO db88f6180AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_AUTO}
	}; 

MV_DEV_CS_INFO db88f6180AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */         
#else
		 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	db88f6180AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6180A_MPP0_7,		
	DB_88F6180A_MPP8_15,
    DB_88F6180A_MPP16_23,
    DB_88F6180A_MPP24_31,		
    DB_88F6180A_MPP32_39,
    DB_88F6180A_MPP40_44
	}}};

MV_BOARD_INFO db88f6180AInfo = {
	"DB-88F6180A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6180A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6180AInfoBoardMppTypeInfo,
	DB_88F6180A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6180AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,					/* intsGppMaskHigh */
	DB_88F6180A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6180AInfoBoardDeCsInfo,
	DB_88F6180A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6180AInfoBoardTwsiDev,					
	DB_88F6180A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6180AInfoBoardMacInfo,
	DB_88F6180A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	NULL,
	DB_88F6180A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_88F6180A_OE_LOW,				/* gppOutEnLow */
	DB_88F6180A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6180A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6180A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6180A_BOARD_NAND_READ_PARAMS,
    DB_88F6180A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6180A_BOARD_NAND_CONTROL
};


#define RD_88F6281A_PCAC_BOARD_PCI_IF_NUM		0x0
#define RD_88F6281A_PCAC_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6281A_PCAC_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6281A_PCAC_BOARD_GPP_INFO_NUM		0x0
#define RD_88F6281A_PCAC_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6281A_PCAC_BOARD_MPP_CONFIG_NUM		0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define RD_88F6281A_PCAC_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define RD_88F6281A_PCAC_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define RD_88F6281A_PCAC_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define RD_88F6281A_PCAC_BOARD_DEBUG_LED_NUM		0x4
#define RD_88F6281A_PCAC_BOARD_NAND_READ_PARAMS		    0x000C0282
#define RD_88F6281A_PCAC_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define RD_88F6281A_PCAC_BOARD_NAND_CONTROL		        0x01c00541

MV_U8	rd88f6281APcacInfoBoardDebugLedIf[] =
	{38, 39, 40, 41};

MV_BOARD_MAC_INFO rd88f6281APcacInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_TWSI_INFO	rd88f6281APcacInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_TWSI_OTHER, 0xa7, ADDR7_BIT}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6281APcacInfoBoardMppTypeInfo[] = 
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO rd88f6281APcacInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
	 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	rd88f6281APcacInfoBoardMppConfigValue[] = 
	{{{
	RD_88F6281A_PCAC_MPP0_7,		
	RD_88F6281A_PCAC_MPP8_15,		
	RD_88F6281A_PCAC_MPP16_23,		
	RD_88F6281A_PCAC_MPP24_31,		
	RD_88F6281A_PCAC_MPP32_39,		
	RD_88F6281A_PCAC_MPP40_47,		
	RD_88F6281A_PCAC_MPP48_55		
	}}};

MV_BOARD_INFO rd88f6281APcacInfo = {
	"RD-88F6281A-PCAC",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6281A_PCAC_BOARD_MPP_GROUP_TYPE_NUM,	/* numBoardMppGroupType */
	rd88f6281APcacInfoBoardMppTypeInfo,
	RD_88F6281A_PCAC_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6281APcacInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6281A_PCAC_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f6281APcacInfoBoardDeCsInfo,
	RD_88F6281A_PCAC_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f6281APcacInfoBoardTwsiDev,					
	RD_88F6281A_PCAC_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f6281APcacInfoBoardMacInfo,
	RD_88F6281A_PCAC_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	0,
	RD_88F6281A_PCAC_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	RD_88F6281A_PCAC_OE_LOW,			/* gppOutEnLow */
	RD_88F6281A_PCAC_OE_HIGH,			/* gppOutEnHigh */
	RD_88F6281A_PCAC_OE_VAL_LOW,			/* gppOutValLow */
	RD_88F6281A_PCAC_OE_VAL_HIGH,			/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 	 					/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    RD_88F6281A_PCAC_BOARD_NAND_READ_PARAMS,
    RD_88F6281A_PCAC_BOARD_NAND_WRITE_PARAMS,
    RD_88F6281A_PCAC_BOARD_NAND_CONTROL
};


#define DB_88F6280A_BOARD_PCI_IF_NUM            0x0
#define DB_88F6280A_BOARD_TWSI_DEF_NUM		    0x7
#define DB_88F6280A_BOARD_MAC_INFO_NUM		    0x1
#define DB_88F6280A_BOARD_GPP_INFO_NUM		    0x0
#define DB_88F6280A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6280A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define DB_88F6280A_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define DB_88F6280A_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define DB_88F6280A_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define DB_88F6280A_BOARD_DEBUG_LED_NUM		    0x0
#define DB_88F6280A_BOARD_NAND_READ_PARAMS		    0x000C0282
#define DB_88F6280A_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_88F6280A_BOARD_NAND_CONTROL		        0x01c00541


MV_BOARD_TWSI_INFO	db88f6280AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6280AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_MPP_TYPE_INFO db88f6280AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO db88f6280AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */         
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
	 {{0, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	db88f6280AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6280A_MPP0_7,		
	DB_88F6280A_MPP8_15,		
	DB_88F6280A_MPP16_23,		
	DB_88F6280A_MPP24_31,		
	DB_88F6280A_MPP32_39,		
	DB_88F6280A_MPP40_47,		
	DB_88F6280A_MPP48_55		
	}}};


MV_BOARD_INFO db88f6280AInfo = {
	"DB-88F6280A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6280A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6280AInfoBoardMppTypeInfo,
	DB_88F6280A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6280AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6280A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6280AInfoBoardDeCsInfo,
	DB_88F6280A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6280AInfoBoardTwsiDev,					
	DB_88F6280A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6280AInfoBoardMacInfo,
	DB_88F6280A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	NULL,
	DB_88F6280A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,						/* ledsPolarity */		
	DB_88F6280A_OE_LOW,				/* gppOutEnLow */
	DB_88F6280A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6280A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6280A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6280A_BOARD_NAND_READ_PARAMS,
    DB_88F6280A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6280A_BOARD_NAND_CONTROL
};


#define RD_88F6282A_BOARD_PCI_IF_NUM            	0x0
#define RD_88F6282A_BOARD_TWSI_DEF_NUM		0x0
#define RD_88F6282A_BOARD_MAC_INFO_NUM		0x2
#define RD_88F6282A_BOARD_GPP_INFO_NUM		0x3
#define RD_88F6282A_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6282A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6282A_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6282A_BOARD_NAND_READ_PARAMS	0x000E02C3
#define RD_88F6282A_BOARD_NAND_WRITE_PARAMS	0x00040405
#define RD_88F6282A_BOARD_NAND_CONTROL		0x01c00541

MV_BOARD_TWSI_INFO	rd88f6282aInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	};

MV_BOARD_MAC_INFO rd88f6282aInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{BOARD_MAC_SPEED_1000M, 0x10}
	}; 

MV_BOARD_MPP_TYPE_INFO rd88f6282aInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_RGMII, MV_BOARD_TDM}
	}; 

MV_BOARD_GPP_INFO rd88f6282aInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_GPP_HDD_POWER, 35},
    	{BOARD_GPP_FAN_POWER, 34},
	{BOARD_GPP_WPS_BUTTON, 29},
	};

MV_DEV_CS_INFO rd88f6282aInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */         
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
	 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	rd88f6282aInfoBoardMppConfigValue[] = 
	{{{
	RD_88F6282A_MPP0_7,		
	RD_88F6282A_MPP8_15,		
	RD_88F6282A_MPP16_23,		
	RD_88F6282A_MPP24_31,		
	RD_88F6282A_MPP32_39,		
	RD_88F6282A_MPP40_47,		
	RD_88F6282A_MPP48_55		
	}}};


MV_BOARD_SWITCH_INFO rd88f6282aInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode, MV_32 switchOnPort} */
	 {{-1, {-1}, -1, -1, -1},
	{38, {0, 1, 2, 3, -1}, 5, 2, 1}}; /* use -1 as linkStatusIrq for polling */

MV_BOARD_INFO rd88f6282aInfo = {
	"RD-88F6282A",					/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6282A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6282aInfoBoardMppTypeInfo,
	RD_88F6282A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6282aInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	BIT6,						/* intsGppMaskHigh */
	RD_88F6282A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6282aInfoBoardDeCsInfo,
	RD_88F6282A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f6282aInfoBoardTwsiDev,					
	RD_88F6282A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6282aInfoBoardMacInfo,
	RD_88F6282A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6282aInfoBoardGppInfo,
	0,						/* activeLedsNumber */              
	NULL,
	0,						/* ledsPolarity */		
	RD_88F6282A_OE_LOW,				/* gppOutEnLow */
	RD_88F6282A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6282A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6282A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	rd88f6282aInfoBoardSwitchInfo,			/* pSwitchInfo */
    	RD_88F6282A_BOARD_NAND_READ_PARAMS,
    	RD_88F6282A_BOARD_NAND_WRITE_PARAMS,
    	RD_88F6282A_BOARD_NAND_CONTROL
};

#define DB_88F6282A_BOARD_PCI_IF_NUM            0x0
#define DB_88F6282A_BOARD_TWSI_DEF_NUM		    0x7
#define DB_88F6282A_BOARD_MAC_INFO_NUM		    0x2
#define DB_88F6282A_BOARD_GPP_INFO_NUM		    0x1
#define DB_88F6282A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6282A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define DB_88F6282A_BOARD_DEVICE_CONFIG_NUM	    0x1
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define DB_88F6282A_BOARD_DEVICE_CONFIG_NUM	    0x2
#else
    #define DB_88F6282A_BOARD_DEVICE_CONFIG_NUM	    0x1
#endif
#define DB_88F6282A_BOARD_DEBUG_LED_NUM		    0x0
#define DB_88F6282A_BOARD_NAND_READ_PARAMS		    0x000E02C3
#define DB_88F6282A_BOARD_NAND_WRITE_PARAMS		    0x00040405
#define DB_88F6282A_BOARD_NAND_CONTROL		        0x01c00541

MV_BOARD_TWSI_INFO	db88f6282AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6282AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_MPP_TYPE_INFO db88f6282AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_AUTO}
	}; 

MV_BOARD_GPP_INFO db88f6282AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_TSU_DIRCTION, 33}
	/*muxed with TDM/Audio module via IOexpender
	{BOARD_GPP_SDIO_DETECT, 38},
	{BOARD_GPP_USB_VBUS, 49}*/
	};

MV_DEV_CS_INFO db88f6282AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */         
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {1, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
	 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	db88f6282AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6282A_MPP0_7,		
	DB_88F6282A_MPP8_15,		
	DB_88F6282A_MPP16_23,		
	DB_88F6282A_MPP24_31,		
	DB_88F6282A_MPP32_39,		
	DB_88F6282A_MPP40_47,		
	DB_88F6282A_MPP48_55		
	}}};


MV_BOARD_INFO db88f6282AInfo = {
	"DB-88F6282A-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6282A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6282AInfoBoardMppTypeInfo,
	DB_88F6282A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6282AInfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6282A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6282AInfoBoardDeCsInfo,
	DB_88F6282A_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6282AInfoBoardTwsiDev,					
	DB_88F6282A_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6282AInfoBoardMacInfo,
	DB_88F6282A_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6282AInfoBoardGppInfo,
	DB_88F6282A_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,						/* ledsPolarity */		
	DB_88F6282A_OE_LOW,				/* gppOutEnLow */
	DB_88F6282A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6282A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6282A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_88F6282A_BOARD_NAND_READ_PARAMS,
    DB_88F6282A_BOARD_NAND_WRITE_PARAMS,
    DB_88F6282A_BOARD_NAND_CONTROL
};


/* 6281 Sheeva Plug*/

#define SHEEVA_PLUG_BOARD_PCI_IF_NUM		        0x0
#define SHEEVA_PLUG_BOARD_TWSI_DEF_NUM		        0x0
#define SHEEVA_PLUG_BOARD_MAC_INFO_NUM		        0x1
#define SHEEVA_PLUG_BOARD_GPP_INFO_NUM		        0x0
#define SHEEVA_PLUG_BOARD_MPP_GROUP_TYPE_NUN        0x1
#define SHEEVA_PLUG_BOARD_MPP_CONFIG_NUM		    0x1
#define SHEEVA_PLUG_BOARD_DEVICE_CONFIG_NUM	        0x1
#define SHEEVA_PLUG_BOARD_DEBUG_LED_NUM		        0x1
#define SHEEVA_PLUG_BOARD_NAND_READ_PARAMS		    0x000E02C2
#define SHEEVA_PLUG_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define SHEEVA_PLUG_BOARD_NAND_CONTROL		        0x01c00541

MV_U8	sheevaPlugInfoBoardDebugLedIf[] =
	{49};

MV_BOARD_MAC_INFO sheevaPlugInfoBoardMacInfo[] = 
    /* {{MV_BOARD_MAC_SPEED	boardMacSpeed,	MV_U8	boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x0}}; 

MV_BOARD_TWSI_INFO	sheevaPlugInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_TWSI_OTHER, 0x0, ADDR7_BIT}};

MV_BOARD_MPP_TYPE_INFO sheevaPlugInfoBoardMppTypeInfo[] = 
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO sheevaPlugInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */

MV_BOARD_MPP_INFO	sheevaPlugInfoBoardMppConfigValue[] = 
	{{{
	RD_SHEEVA_PLUG_MPP0_7,		
	RD_SHEEVA_PLUG_MPP8_15,		
	RD_SHEEVA_PLUG_MPP16_23,		
	RD_SHEEVA_PLUG_MPP24_31,		
	RD_SHEEVA_PLUG_MPP32_39,		
	RD_SHEEVA_PLUG_MPP40_47,		
	RD_SHEEVA_PLUG_MPP48_55		
	}}};

MV_BOARD_INFO sheevaPlugInfo = {
	"SHEEVA PLUG",				                /* boardName[MAX_BOARD_NAME_LEN] */
	SHEEVA_PLUG_BOARD_MPP_GROUP_TYPE_NUN,		/* numBoardMppGroupType */
	sheevaPlugInfoBoardMppTypeInfo,
	SHEEVA_PLUG_BOARD_MPP_CONFIG_NUM,		    /* numBoardMppConfig */
	sheevaPlugInfoBoardMppConfigValue,
	0,						                    /* intsGppMaskLow */
	0,					                        /* intsGppMaskHigh */
	SHEEVA_PLUG_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	sheevaPlugInfoBoardDeCsInfo,
	SHEEVA_PLUG_BOARD_TWSI_DEF_NUM,			    /* numBoardTwsiDev */
	sheevaPlugInfoBoardTwsiDev,					
	SHEEVA_PLUG_BOARD_MAC_INFO_NUM,			    /* numBoardMacInfo */
	sheevaPlugInfoBoardMacInfo,
	SHEEVA_PLUG_BOARD_GPP_INFO_NUM,			    /* numBoardGppInfo */
	0,
	SHEEVA_PLUG_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	sheevaPlugInfoBoardDebugLedIf,
	0,										/* ledsPolarity */		
	RD_SHEEVA_PLUG_OE_LOW,				            /* gppOutEnLow */
	RD_SHEEVA_PLUG_OE_HIGH,				        /* gppOutEnHigh */
	RD_SHEEVA_PLUG_OE_VAL_LOW,				        /* gppOutValLow */
	RD_SHEEVA_PLUG_OE_VAL_HIGH,				    /* gppOutValHigh */
	0,						                    /* gppPolarityValLow */
	0, 						                    /* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    SHEEVA_PLUG_BOARD_NAND_READ_PARAMS,
    SHEEVA_PLUG_BOARD_NAND_WRITE_PARAMS,
    SHEEVA_PLUG_BOARD_NAND_CONTROL
};

/* Customer specific board place holder*/

#define DB_CUSTOMER_BOARD_PCI_IF_NUM		        0x0
#define DB_CUSTOMER_BOARD_TWSI_DEF_NUM		        0x0
#define DB_CUSTOMER_BOARD_MAC_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_GPP_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_MPP_GROUP_TYPE_NUN        0x0
#define DB_CUSTOMER_BOARD_MPP_CONFIG_NUM		    0x0
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
    #define DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM	    0x0
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
    #define DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM	    0x0
#else
    #define DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM	    0x0
#endif
#define DB_CUSTOMER_BOARD_DEBUG_LED_NUM		0x0
#define DB_CUSTOMER_BOARD_NAND_READ_PARAMS		    0x000E02C2
#define DB_CUSTOMER_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_CUSTOMER_BOARD_NAND_CONTROL		        0x01c00541

MV_U8	dbCustomerInfoBoardDebugLedIf[] =
	{0};

MV_BOARD_MAC_INFO dbCustomerInfoBoardMacInfo[] = 
    /* {{MV_BOARD_MAC_SPEED	boardMacSpeed,	MV_U8	boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x0}}; 

MV_BOARD_TWSI_INFO	dbCustomerInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_TWSI_OTHER, 0x0, ADDR7_BIT}};

MV_BOARD_MPP_TYPE_INFO dbCustomerInfoBoardMppTypeInfo[] = 
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO dbCustomerInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) && defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#elif defined(MV_NAND) && defined(MV_SPI_BOOT)
		 {
         {0, N_A, BOARD_DEV_NAND_FLASH, 8},	   /* NAND DEV */
         {2, N_A, BOARD_DEV_SPI_FLASH, 8},	   /* SPI DEV */
         };
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */         
#endif

MV_BOARD_MPP_INFO	dbCustomerInfoBoardMppConfigValue[] = 
	{{{
	DB_CUSTOMER_MPP0_7,		
	DB_CUSTOMER_MPP8_15,		
	DB_CUSTOMER_MPP16_23,		
	DB_CUSTOMER_MPP24_31,		
	DB_CUSTOMER_MPP32_39,		
	DB_CUSTOMER_MPP40_47,		
	DB_CUSTOMER_MPP48_55		
	}}};

MV_BOARD_INFO dbCustomerInfo = {
	"DB-CUSTOMER",				                /* boardName[MAX_BOARD_NAME_LEN] */
	DB_CUSTOMER_BOARD_MPP_GROUP_TYPE_NUN,		/* numBoardMppGroupType */
	dbCustomerInfoBoardMppTypeInfo,
	DB_CUSTOMER_BOARD_MPP_CONFIG_NUM,		    /* numBoardMppConfig */
	dbCustomerInfoBoardMppConfigValue,
	0,						                    /* intsGppMaskLow */
	0,					                        /* intsGppMaskHigh */
	DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	dbCustomerInfoBoardDeCsInfo,
	DB_CUSTOMER_BOARD_TWSI_DEF_NUM,			    /* numBoardTwsiDev */
	dbCustomerInfoBoardTwsiDev,					
	DB_CUSTOMER_BOARD_MAC_INFO_NUM,			    /* numBoardMacInfo */
	dbCustomerInfoBoardMacInfo,
	DB_CUSTOMER_BOARD_GPP_INFO_NUM,			    /* numBoardGppInfo */
	0,
	DB_CUSTOMER_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	0,										/* ledsPolarity */		
	DB_CUSTOMER_OE_LOW,				            /* gppOutEnLow */
	DB_CUSTOMER_OE_HIGH,				        /* gppOutEnHigh */
	DB_CUSTOMER_OE_VAL_LOW,				        /* gppOutValLow */
	DB_CUSTOMER_OE_VAL_HIGH,				    /* gppOutValHigh */
	0,						                    /* gppPolarityValLow */
	0, 						                    /* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    DB_CUSTOMER_BOARD_NAND_READ_PARAMS,
    DB_CUSTOMER_BOARD_NAND_WRITE_PARAMS,
    DB_CUSTOMER_BOARD_NAND_CONTROL
};

#ifdef CONFIG_SYNO_MV88F6281

#define SYNO_DS109_BOARD_MPP_GROUP_TYPE_NUM  0x1
MV_BOARD_MPP_TYPE_INFO SYNO_DS109InfoBoardMppTypeInfo[] =
{
        { MV_BOARD_AUTO, MV_BOARD_AUDIO }
};

#define SYNO_DS109_BOARD_MPP_CONFIG_NUM             0x1
#define SYNO_DS109_MPP0_7                    0x01002222
#define SYNO_DS109_MPP8_15                   0x03303311
#define SYNO_DS109_MPP16_23                  0x00550000
#define SYNO_DS109_MPP24_31                  0x00000000
#define SYNO_DS109_MPP32_39                  0x44440000
#define SYNO_DS109_MPP40_47                  0x00044444
#define SYNO_DS109_MPP48_55                  0x00000000

MV_BOARD_MPP_INFO SYNO_DS109InfoBoardMppConfigValue[] =
{
       {
               {
                       SYNO_DS109_MPP0_7,
                       SYNO_DS109_MPP8_15,
                       SYNO_DS109_MPP16_23,
                       SYNO_DS109_MPP24_31,
                       SYNO_DS109_MPP32_39,
                       SYNO_DS109_MPP40_47,
                       SYNO_DS109_MPP48_55
               }
       }
};

#define SYNO_DS109_BOARD_DEVICE_CONFIG_NUM   0x1
/*
Pin 		Mode	Signal select and definition	Input/output	Pull-up/pull-down
MPP[0:3]	0x2	SPI signal		
MPP[4:7]	0x0	GPIO		
MPP[8:9]	0x1	TWSI		
MPP[10:11]	0x3	UART0		
MPP12		0x0	GPIO		
MPP[13:14]	0x3	UART1		
MPP[15:19]	0x0	GPIO		
MPP[20]		0x5	SATA port 1 ACT LED		output		See 8.3 for detail
MPP[21]		0x5	SATA port 0 ACT LED		output		See 8.3 for detail
MPP[22]		0x0	SATA port 1 FAULT LED		output	
MPP[23]		0x0	SATA port 0 FAULT LED		output	
MPP[24:35]	0x0	GPIO		
MPP31		0x0	HDD2 enable (2 bay only)	output		Default Low, Hi=ON
MPP32		0x0	FAN_182				output		Default Low, Low=off
MPP33		0x0	FAN_150				output		Default Low, Low=off
MPP34		0x0	FAN_100				output		Default Hi, Low=off
MPP35		0x0	FAN SENSE			input		Hi=Fail, pulse=running
MPP[36:44]	0x4	I2S		
MPP[45:49]	0x0	GPIO		
*/

MV_DEV_CS_INFO SYNO_DS109InfoBoardDeCsInfo[] =
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
/* NAND DEV */
{
       {0, N_A, BOARD_DEV_NAND_FLASH, 8}
};
#else
/* SPI DEV */
{
       {2, N_A, BOARD_DEV_SPI_FLASH, 8}
};
#endif

#define SYNO_DS109_BOARD_TWSI_DEF_NUM                        0x1
MV_BOARD_TWSI_INFO      SYNO_DS109InfoBoardTwsiDev[] =
{
        {BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT},
};

#define SYNO_DS109_BOARD_MAC_INFO_NUM                        0x1
MV_BOARD_MAC_INFO SYNO_DS109InfoBoardMacInfo[] =
{
       {BOARD_MAC_SPEED_AUTO, 0x8}
};

#define SYNO_DS109_BOARD_GPP_INFO_NUM                        0x1E
MV_BOARD_GPP_INFO SYNO_DS109InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, 22},
	{SYNO_KW_GPP_HDD1_FAIL_LED, 23},
	{SYNO_KW_GPP_HDD2_POWER, 31},
	{SYNO_KW_GPP_FAN_RESISTER_A, 32},
	{SYNO_KW_GPP_FAN_RESISTER_B, 33},
	{SYNO_KW_GPP_FAN_RESISTER_C, 34},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_0, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_1, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_2, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_3, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_INTER_LOCK, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};

#define SYNO_DS109_OE_LOW                    0x0
#define SYNO_DS109_OE_HIGH                   0x0
#define SYNO_DS109_OE_VAL_LOW                (BIT22|BIT23)
#define SYNO_DS109_OE_VAL_HIGH               (BIT0|BIT1)

MV_BOARD_INFO SYNO_DS109_INFO = {
    "Synology Disk Station",                      /* boardName[MAX_BOARD_NAME_LEN] */

    SYNO_DS109_BOARD_MPP_GROUP_TYPE_NUM,          /* numBoardMppGroupType */
    SYNO_DS109InfoBoardMppTypeInfo,

    SYNO_DS109_BOARD_MPP_CONFIG_NUM,              /* numBoardMppConfig */
    SYNO_DS109InfoBoardMppConfigValue,

    0,                                            /* intsGppMaskLow */
    0,                                            /* intsGppMaskHigh */

    SYNO_DS109_BOARD_DEVICE_CONFIG_NUM,           /* numBoardDevIf */
    SYNO_DS109InfoBoardDeCsInfo,

    SYNO_DS109_BOARD_TWSI_DEF_NUM,                /* numBoardTwsiDev */
    SYNO_DS109InfoBoardTwsiDev,

    SYNO_DS109_BOARD_MAC_INFO_NUM,                /* numBoardMacInfo */
    SYNO_DS109InfoBoardMacInfo,

    SYNO_DS109_BOARD_GPP_INFO_NUM,                /* numBoardGppInfo */
    SYNO_DS109InfoBoardGppInfo,

    0,                                            /* activeLedsNumber */

    NULL,
    N_A,                                          /* ledsPolarity */

    SYNO_DS109_OE_LOW,                            /* gppOutEnLow */
    SYNO_DS109_OE_HIGH,                           /* gppOutEnHigh */
    SYNO_DS109_OE_VAL_LOW,                        /* gppOutValLow */
    SYNO_DS109_OE_VAL_HIGH,                       /* gppOutValHigh */
    0,
	0,
    NULL,                                          /* pSwitchInfo */
	0,
	0,
	0
};


#define SYNO_DS409slim_BOARD_MPP_GROUP_TYPE_NUM  0x1
MV_BOARD_MPP_TYPE_INFO SYNO_DS409slimInfoBoardMppTypeInfo[] =
{
        { MV_BOARD_AUTO, MV_BOARD_AUDIO }
};      

#define SYNO_DS409slim_BOARD_MPP_CONFIG_NUM             0x1
#define SYNO_DS409slim_MPP0_7                    0x01002222
#define SYNO_DS409slim_MPP8_15                   0x03303311
#define SYNO_DS409slim_MPP16_23                  0x00000000
#define SYNO_DS409slim_MPP24_31                  0x00000000
#define SYNO_DS409slim_MPP32_39                  0x44440000
#define SYNO_DS409slim_MPP40_47                  0x00044444
#define SYNO_DS409slim_MPP48_55                  0x00000000

MV_BOARD_MPP_INFO SYNO_DS409slimInfoBoardMppConfigValue[] =
{
       {
               {
                       SYNO_DS409slim_MPP0_7,
                       SYNO_DS409slim_MPP8_15,
                       SYNO_DS409slim_MPP16_23,
                       SYNO_DS409slim_MPP24_31,
                       SYNO_DS409slim_MPP32_39,
                       SYNO_DS409slim_MPP40_47,
                       SYNO_DS409slim_MPP48_55
               }
       }
};

#define SYNO_DS409slim_BOARD_DEVICE_CONFIG_NUM   0x1
/*
Pin 		Mode	Signal definition	Input/output	Pull-up/pull-down
MPP[0:3]	0x2	SPI signal		
MPP[4:7]	0x0	GPIO		
MPP[8:9]	0x1	TWSI		
MPP[10:11]	0x3	UART0		
MPP12		0x0	GPIO		
MPP[13:14]	0x3	UART1		
MPP[15:19]	0x0	GPIO		
MPP[20:21]	0x0	DISKA0/A1		output		00=off (default), 01=Green,10=Amber,
MPP[22:23]	0x0	DISKB0/B1		output	
MPP[24:25]	0x0	DISKC0/C1		output	
MPP[26:27]	0x0	DISKD0/D1		output	
MPP[28:30]	0x0	GPIO		
MPP31		0x0	BP-LOCK-OUT		input		Low=good, Hi=NG
MPP32		0x0	FAN_150	(409slim)	output		Hi=ON, Low=off
MPP33		0x0	FAN_120			output		Default low, Low=off
MPP34		0x0	FAN_100 		output		Default low, Low=off
MPP35		0x0	FAN SENSE		input		Hi=Fail, pulse=running
MPP[36:44]	0x4	I2S		
MPP[45:49]	0x0	GPIO		
*/

MV_DEV_CS_INFO SYNO_DS409slimInfoBoardDeCsInfo[] =
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
/* NAND DEV */
{
       {0, N_A, BOARD_DEV_NAND_FLASH, 8}
};
#else
/* SPI DEV */
{
       {2, N_A, BOARD_DEV_SPI_FLASH, 8}
};
#endif

#define SYNO_DS409slim_BOARD_TWSI_DEF_NUM                      0x1
MV_BOARD_TWSI_INFO      SYNO_DS409slimInfoBoardTwsiDev[] =
{
        {BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT},
};

#define SYNO_DS409slim_BOARD_MAC_INFO_NUM                      0x1
MV_BOARD_MAC_INFO SYNO_DS409slimInfoBoardMacInfo[] =
{
       {BOARD_MAC_SPEED_AUTO, 0x8}
};

#define SYNO_DS409slim_BOARD_GPP_INFO_NUM                      0x1E
MV_BOARD_GPP_INFO SYNO_DS409slimInfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD1_LED_0, 20},
	{SYNO_KW_GPP_HDD1_LED_1, 21},
	{SYNO_KW_GPP_HDD2_LED_0, 22},
	{SYNO_KW_GPP_HDD2_LED_1, 23},
	{SYNO_KW_GPP_HDD3_LED_0, 24},
	{SYNO_KW_GPP_HDD3_LED_1, 25},
	{SYNO_KW_GPP_HDD4_LED_0, 26},
	{SYNO_KW_GPP_HDD4_LED_1, 27},
	{SYNO_KW_GPP_INTER_LOCK, 31},
	{SYNO_KW_GPP_FAN_RESISTER_A, 32},
	{SYNO_KW_GPP_FAN_RESISTER_B, 33},
	{SYNO_KW_GPP_FAN_RESISTER_C, 34},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_0, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_1, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_2, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_3, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};

#define SYNO_DS409slim_OE_LOW                    0x0
#define SYNO_DS409slim_OE_HIGH                   0x0
#define SYNO_DS409slim_OE_VAL_LOW                (BIT20|BIT21|BIT22|BIT23|BIT24|BIT25|BIT26|BIT27)
#define SYNO_DS409slim_OE_VAL_HIGH               (BIT0|BIT1)

MV_BOARD_INFO SYNO_DS409slim_INFO = {
    "Synology Disk Station",                          /* boardName[MAX_BOARD_NAME_LEN] */

    SYNO_DS409slim_BOARD_MPP_GROUP_TYPE_NUM,          /* numBoardMppGroupType */
    SYNO_DS409slimInfoBoardMppTypeInfo,

    SYNO_DS409slim_BOARD_MPP_CONFIG_NUM,              /* numBoardMppConfig */
    SYNO_DS409slimInfoBoardMppConfigValue,

    0,                                                /* intsGppMaskLow */
    0,                                                /* intsGppMaskHigh */

    SYNO_DS409slim_BOARD_DEVICE_CONFIG_NUM,           /* numBoardDevIf */
    SYNO_DS409slimInfoBoardDeCsInfo,

    SYNO_DS409slim_BOARD_TWSI_DEF_NUM,                /* numBoardTwsiDev */
    SYNO_DS409slimInfoBoardTwsiDev,

    SYNO_DS409slim_BOARD_MAC_INFO_NUM,                /* numBoardMacInfo */
    SYNO_DS409slimInfoBoardMacInfo,

    SYNO_DS409slim_BOARD_GPP_INFO_NUM,                /* numBoardGppInfo */
    SYNO_DS409slimInfoBoardGppInfo,

    0,                                                /* activeLedsNumber */

    NULL,
    N_A,                                              /* ledsPolarity */    
    
    SYNO_DS409slim_OE_LOW,                            /* gppOutEnLow */
    SYNO_DS409slim_OE_HIGH,                           /* gppOutEnHigh */
    SYNO_DS409slim_OE_VAL_LOW,                        /* gppOutValLow */
    SYNO_DS409slim_OE_VAL_HIGH,                       /* gppOutValHigh */    
	0,
	0,
    NULL,                                              /* pSwitchInfo */
	0,
	0,
	0
};

/* DS409/RS409/DS509 GPP multiplexing */
#define SYNO_DS409_BOARD_MPP_GROUP_TYPE_NUM  0x1
MV_BOARD_MPP_TYPE_INFO SYNO_DS409InfoBoardMppTypeInfo[] =
{
        { MV_BOARD_AUTO, MV_BOARD_AUTO }
};      

#define SYNO_DS409_BOARD_MPP_CONFIG_NUM             0x1
#define SYNO_DS409_MPP0_7                    0x01002222
#define SYNO_DS409_MPP8_15                   0x03303311
#define SYNO_DS409_MPP16_23                  0x33330000
#define SYNO_DS409_MPP24_31                  0x33003333
#define SYNO_DS409_MPP32_39                  0x00005533
#define SYNO_DS409_MPP40_47                  0x00000000
#define SYNO_DS409_MPP48_55                  0x00000000

MV_BOARD_MPP_INFO SYNO_DS409InfoBoardMppConfigValue[] =
{
       {
               {
                       SYNO_DS409_MPP0_7,
                       SYNO_DS409_MPP8_15,
                       SYNO_DS409_MPP16_23,
                       SYNO_DS409_MPP24_31,
                       SYNO_DS409_MPP32_39,
                       SYNO_DS409_MPP40_47,
                       SYNO_DS409_MPP48_55
               }
       }
};

/*
Pin 		Mode	Signal definition	Input/output	Pull-up/pull-down
MPP[0:3]	0x2		SPI signal		
MPP[4:7]	0x0		GPIO		
MPP[8:9]	0x1		TWSI		
MPP[10:11]	0x3		UART0		
MPP12		0x0		ALERM_LED			output	
MPP[13:14]	0x3		UART1		
MPP[15:17]	0x0		FAN 150, FAN 120, FAN 100	output	
MPP[18]		0x0		FAN sense			input	
MPP[19]		0x0		INTER-LOCK			input	
MPP[20:23]	0x0		RGMII G2 TX[0:3]	output	
MPP[24:27]	0x0		RGMII G2 RX[0:3]	input	
MPP[28:29]	0x0		MODEL_ID			input			MODEL_ID:
														00: DS409
														01: DS509
														10:RS409
														11:RS409RP
MPP[30:31]	0x0		RGMII TX_CTL, RGMII TX_CLK	output	
MPP[32:33]	0x0		RGMII RX_CTL, RGMII RX_CLK	input	
MPP34		0x3		ACT internal SATA port 2	output	
MPP35		0x3		ACT internal SATA port 1	output	
MPP[36:45]	0x0		DISK LED STATE		output	
MPP46		0x0		BUZZER_MUTE_REQ		input	
MPP47		0x0		BUZZER_MUTE_ACK		output	
MPP48		0x0		RPS1_ON				input	
MPP49		0x0		RPS2_ON				input			
*/
#define SYNO_DS409_BOARD_DEVICE_CONFIG_NUM   0x1
MV_DEV_CS_INFO SYNO_DS409InfoBoardDeCsInfo[] =
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
/* NAND DEV */
{
       {0, N_A, BOARD_DEV_NAND_FLASH, 8}
};
#else
/* SPI DEV */
{
       {2, N_A, BOARD_DEV_SPI_FLASH, 8}
};
#endif

#define SYNO_DS409_BOARD_MAC_INFO_NUM                      0x2
MV_BOARD_MAC_INFO SYNO_DS409InfoBoardMacInfo[] =
{
       {BOARD_MAC_SPEED_AUTO, 0x8},
	   {BOARD_MAC_SPEED_AUTO, 0x9}
};

#define SYNO_DS409_BOARD_GPP_INFO_NUM                      0x1E
MV_BOARD_GPP_INFO SYNO_DS409InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_ALERM_LED, 12},
	{SYNO_KW_GPP_FAN_RESISTER_A, 15},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 18},
	{SYNO_KW_GPP_INTER_LOCK, 19},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_HDD1_LED_0, 36},
	{SYNO_KW_GPP_HDD1_LED_1, 37},
	{SYNO_KW_GPP_HDD2_LED_0, 38},
	{SYNO_KW_GPP_HDD2_LED_1, 39},
	{SYNO_KW_GPP_HDD3_LED_0, 40},
	{SYNO_KW_GPP_HDD3_LED_1, 41},
	{SYNO_KW_GPP_HDD4_LED_0, 42},
	{SYNO_KW_GPP_HDD4_LED_1, 43},
	{SYNO_KW_GPP_HDD5_LED_0, 44},
	{SYNO_KW_GPP_HDD5_LED_1, 45},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, 46},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, 47},
	{SYNO_KW_GPP_RPS1_ON, 48},
	{SYNO_KW_GPP_RPS2_ON, 49},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_2, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_3, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
};

#define SYNO_DS409_OE_LOW                    0x0
#define SYNO_DS409_OE_HIGH                   0x0
#define SYNO_DS409_OE_VAL_LOW                (BIT12|BIT15|BIT16|BIT17)
#define SYNO_DS409_OE_VAL_HIGH               (BIT0|BIT1|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT15)

MV_BOARD_INFO SYNO_DS409_INFO = {
    "Synology Disk Station",                          /* boardName[MAX_BOARD_NAME_LEN] */

    SYNO_DS409_BOARD_MPP_GROUP_TYPE_NUM,          /* numBoardMppGroupType */
    SYNO_DS409InfoBoardMppTypeInfo,

    SYNO_DS409_BOARD_MPP_CONFIG_NUM,              /* numBoardMppConfig */
    SYNO_DS409InfoBoardMppConfigValue,

    0,                                                /* intsGppMaskLow */
    0,                                                /* intsGppMaskHigh */

    SYNO_DS409_BOARD_DEVICE_CONFIG_NUM,           /* numBoardDevIf */
    SYNO_DS409InfoBoardDeCsInfo,

    0,                						/* numBoardTwsiDev */
    NULL,

    SYNO_DS409_BOARD_MAC_INFO_NUM,                /* numBoardMacInfo */
    SYNO_DS409InfoBoardMacInfo,

    SYNO_DS409_BOARD_GPP_INFO_NUM,                /* numBoardGppInfo */
    SYNO_DS409InfoBoardGppInfo,

    0,                                                /* activeLedsNumber */

    NULL,
    N_A,                                              /* ledsPolarity */    
    
    SYNO_DS409_OE_LOW,                            /* gppOutEnLow */
    SYNO_DS409_OE_HIGH,                           /* gppOutEnHigh */
    SYNO_DS409_OE_VAL_LOW,                        /* gppOutValLow */
    SYNO_DS409_OE_VAL_HIGH,                       /* gppOutValHigh */    
	0,
	0,
    NULL,                                              /* pSwitchInfo */
	0,
	0,
	0
};

/**********************************
*   DS211, DS111, DS212, DS212p
***********************************/
/*
Pin         Mode    Signal select and definition    output only     Pull-up/pull-down
MPP0        0x2     SPI signal
MPP[1:3]    0x2     SPI signal                      V
MPP4        0x0     GPIO (2bay is inter lock)
MPP5        0x0     GPIO                            V
MPP6        0x1     SYSRST_OUTn
MPP7        0x0     GPIO                            V
MPP[8:9]    0x1     TWSI
MPP10       0x3     UART0                           V
MPP11       0x3     UART0
MPP12       0x0     GPIO                            V
MPP[13:14]  0x3     UART1
MPP15       0x0     FAN_150                                         Default 1 Hi, Low=off
MPP16       0x0     FAN_100                                         Default 1 Hi, Low=off
MPP17       0x0     FAN_33                                          Default 0 Low, Hi=ON
MPP[18:19]  0x0     GPIO                            V
MPP20       0x5     SATA port 1 ACT LED                             See 8.3 for detail
MPP21       0x5     SATA port 0 ACT LED                             See 8.3 for detail
MPP22       0x0     SATA port 1 PRESENT LED
MPP23       0x0     SATA port 0 PRESENT LED
MPP[24:27]  0x0     GPIO
MPP[28:29]  0x0     GPIO Model ID 0,1
MPP[30:32]  0x0     GPIO
MPP33       0x0     GPIO                            V
MPP34       0x0     GPO  (2 bay HDD2 enable)        V
MPP35       0x0     FAN SENSE                                       Hi=Fail, pulse=running
MPP[36:45]  0x0     GPIO
MPP[46:47]  0x0     GPIO Model ID 2,3
MPP[48:49]  0x0     GPIO
*/
#define DS211_MPP0_7		0x01002222
#define DS211_MPP8_15		0x03303311
#define DS211_MPP16_23		0x00550000
#define DS211_MPP24_31		0x00000000
#define DS211_MPP32_39		0x00000000
#define DS211_MPP40_47		0x00000000
#define DS211_MPP48_55		0x00000000

MV_BOARD_MPP_INFO	DS211InfoBoardMppConfigValue[] =
{
	{
		{
			DS211_MPP0_7,
			DS211_MPP8_15,
			DS211_MPP16_23,
			DS211_MPP24_31,
			DS211_MPP32_39,
			DS211_MPP40_47,
			DS211_MPP48_55
		}
	}
};

#define DS211_BOARD_MAC_INFO_NUM 1
MV_BOARD_MAC_INFO DS211InfoBoardMacInfo[] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
{
#ifdef SYNO_OPTION_6282_PHY_WOL //used by DS212, DS211pr1, DS212p
	{BOARD_MAC_SPEED_AUTO, 0x1},
#else
	{BOARD_MAC_SPEED_AUTO, 0x8},
#endif
};


#define SYNO_DS211_BOARD_GPP_INFO_NUM 0x1E
MV_BOARD_GPP_INFO SYNO_DS211InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, 22},
	{SYNO_KW_GPP_HDD1_FAIL_LED, 23},
	{SYNO_KW_GPP_HDD2_POWER, 34},
	{SYNO_KW_GPP_FAN_RESISTER_A, 15},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_INTER_LOCK, 4},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_MODEL_ID_2, 46},
	{SYNO_KW_GPP_MODEL_ID_3, 47},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};

#define SYNO_DS211_OE_LOW                    0x0
#define SYNO_DS211_OE_HIGH                   0x0
#define SYNO_DS211_OE_VAL_LOW                (BIT15|BIT16|BIT22|BIT23)
#define SYNO_DS211_OE_VAL_HIGH               0x0
MV_BOARD_INFO SYNO_DS211_INFO = {
	"Synology Disk Station",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6282A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6282AInfoBoardMppTypeInfo,
	DB_88F6282A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	DS211InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6282A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6282AInfoBoardDeCsInfo,
	0,			/* numBoardTwsiDev */
	NULL,
	DS211_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	DS211InfoBoardMacInfo,
	SYNO_DS211_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	SYNO_DS211InfoBoardGppInfo,
	0,			/* activeLedsNumber */
	NULL,
	0,						/* ledsPolarity */
	SYNO_DS211_OE_LOW,				/* gppOutEnLow */
	SYNO_DS211_OE_HIGH,				/* gppOutEnHigh */
	SYNO_DS211_OE_VAL_LOW,				/* gppOutValLow */
	SYNO_DS211_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    0,
    0,
    0
};

/************************
*   DS411slim
*************************/
/*
Pin         Mode    Signal select and definition    output only     Pull-up/pull-down
MPP0        0x2     SPI signal
MPP[1:3]    0x2     SPI signal                      V
MPP4        0x0     Inter lock
MPP5        0x0     GPIO                            V
MPP6        0x1     SYSRST_OUTn
MPP7        0x0     GPIO                            V
MPP[8:9]    0x1     TWSI
MPP10       0x3     UART0                           V
MPP11       0x3     UART0
MPP12       0x0     GPIO                            V
MPP[13:14]  0x3     UART1
MPP15       0x0     FAN_150                                         Default 1 Hi, Low=off
MPP16       0x0     FAN_100                                         Default 1 Hi, Low=off
MPP17       0x0     FAN_33                                          Default 0 Low, Hi=ON
MPP[18:19]  0x0     GPIO                            V
MPP[20:23]	0x3     GE1 TXD
MPP[24:27]  0x3     GE1 RXD
MPP[28:29]  0x0     GPIO Model ID 0,1
MPP30       0x3     GE1_RXCTL
MPP31       0x3     GE1_RXCLK
MPP32       0x3     GE1_TCLKOUT
MPP33       0x3     GE1_TXCTL                       V
MPP34       0x0     GPO  (2 bay HDD2 enable)        V
MPP35       0x0     FAN SENSE                                       Hi=Fail, pulse=running
MPP36       0x0     GPIO Disk LED A0
MPP37       0x0     GPIO Disk LED A1
MPP38       0x0     GPIO Disk LED B0
MPP39       0x0     GPIO Disk LED B1
MPP40       0x0     GPIO Disk LED C0
MPP41       0x0     GPIO Disk LED C1
MPP42       0x0     GPIO Disk LED D0
MPP43       0x0     GPIO Disk LED D1
MPP44       0x0     GPIO HDD3 enable
MPP45       0x0     GPIO HDD4 enable
MPP[46:47]  0x0     GPIO Model ID 2,3
MPP[48:49]  0x0     GPIO
*/
#if defined(SYNO_GPIO_FOR_6282_SLIM) || defined(SYNO_GPIO_FOR_6282_DS)
#define SYNO_6282_4BAY_MPP0_7		0x01002222
#define SYNO_6282_4BAY_MPP8_15		0x03303311
#define SYNO_6282_4BAY_MPP16_23		0x00000000
#define SYNO_6282_4BAY_MPP24_31		0x00000000
#define SYNO_6282_4BAY_MPP32_39		0x00000000
#define SYNO_6282_4BAY_MPP40_47		0x00000000
#define SYNO_6282_4BAY_MPP48_55		0x00000000
#elif defined(SYNO_GPIO_FOR_6282_RS)
#define SYNO_6282_4BAY_MPP0_7		0x01002222
#define SYNO_6282_4BAY_MPP8_15		0x03303311
#define SYNO_6282_4BAY_MPP16_23		0x33330000
#define SYNO_6282_4BAY_MPP24_31		0x33003333
#define SYNO_6282_4BAY_MPP32_39		0x00000033
#define SYNO_6282_4BAY_MPP40_47		0x00000000
#define SYNO_6282_4BAY_MPP48_55		0x00000000
#else
#define SYNO_6282_4BAY_MPP0_7		0x01002222
#define SYNO_6282_4BAY_MPP8_15		0x03303311
#define SYNO_6282_4BAY_MPP16_23		0x00000000
#define SYNO_6282_4BAY_MPP24_31		0x00000000
#define SYNO_6282_4BAY_MPP32_39		0x00000000
#define SYNO_6282_4BAY_MPP40_47		0x00000000
#define SYNO_6282_4BAY_MPP48_55		0x00000000
#endif
MV_BOARD_MPP_INFO	SYNO_6282_4Bay_InfoBoardMppConfigValue[] =
{
	{
		{
			SYNO_6282_4BAY_MPP0_7,
			SYNO_6282_4BAY_MPP8_15,
			SYNO_6282_4BAY_MPP16_23,
			SYNO_6282_4BAY_MPP24_31,
			SYNO_6282_4BAY_MPP32_39,
			SYNO_6282_4BAY_MPP40_47,
			SYNO_6282_4BAY_MPP48_55
		}
	}
};

#define SYNO_DS_6282_4BAY_BOARD_GPP_INFO_NUM 0x20
#if defined(SYNO_GPIO_FOR_6282_SLIM)
MV_BOARD_GPP_INFO SYNO_DS_6282_4Bay_InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_RESISTER_A, 15},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_FAN_FAIL_B, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_FAIL_C, GPIO_UNDEF},
	{SYNO_KW_GPP_INTER_LOCK, 4},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_MODEL_ID_2, 46},
	{SYNO_KW_GPP_MODEL_ID_3, 47},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_0, 36},
	{SYNO_KW_GPP_HDD1_LED_1, 37},
	{SYNO_KW_GPP_HDD2_LED_0, 38},
	{SYNO_KW_GPP_HDD2_LED_1, 39},
	{SYNO_KW_GPP_HDD3_LED_0, 40},
	{SYNO_KW_GPP_HDD3_LED_1, 41},
	{SYNO_KW_GPP_HDD4_LED_0, 42},
	{SYNO_KW_GPP_HDD4_LED_1, 43},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};
#elif defined(SYNO_GPIO_FOR_6282_DS)
MV_BOARD_GPP_INFO SYNO_DS_6282_4Bay_InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, 34},
	{SYNO_KW_GPP_FAN_RESISTER_A, 15},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_FAN_FAIL_B, 48},
	{SYNO_KW_GPP_FAN_FAIL_C, GPIO_UNDEF},
	{SYNO_KW_GPP_INTER_LOCK, 4},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_MODEL_ID_2, 46},
	{SYNO_KW_GPP_MODEL_ID_3, 47},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, 44},
	{SYNO_KW_GPP_HDD4_POWER, 45},
	{SYNO_KW_GPP_HDD1_LED_0, 36},
	{SYNO_KW_GPP_HDD1_LED_1, 37},
	{SYNO_KW_GPP_HDD2_LED_0, 38},
	{SYNO_KW_GPP_HDD2_LED_1, 39},
	{SYNO_KW_GPP_HDD3_LED_0, 40},
	{SYNO_KW_GPP_HDD3_LED_1, 41},
	{SYNO_KW_GPP_HDD4_LED_0, 42},
	{SYNO_KW_GPP_HDD4_LED_1, 43},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};
#elif defined(SYNO_GPIO_FOR_6282_RS)
MV_BOARD_GPP_INFO SYNO_DS_6282_4Bay_InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_RESISTER_A, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_FAN_FAIL_B, 44},
	{SYNO_KW_GPP_FAN_FAIL_C, 45},
	{SYNO_KW_GPP_INTER_LOCK, 4},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_MODEL_ID_2, 46},
	{SYNO_KW_GPP_MODEL_ID_3, 47},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_0, 36},
	{SYNO_KW_GPP_HDD1_LED_1, 37},
	{SYNO_KW_GPP_HDD2_LED_0, 38},
	{SYNO_KW_GPP_HDD2_LED_1, 39},
	{SYNO_KW_GPP_HDD3_LED_0, 40},
	{SYNO_KW_GPP_HDD3_LED_1, 41},
	{SYNO_KW_GPP_HDD4_LED_0, 42},
	{SYNO_KW_GPP_HDD4_LED_1, 43},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, 34},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};
#else
MV_BOARD_GPP_INFO SYNO_DS_6282_4Bay_InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_FAIL_LED, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_POWER, 34},
	{SYNO_KW_GPP_FAN_RESISTER_A, 15},
	{SYNO_KW_GPP_FAN_RESISTER_B, 16},
	{SYNO_KW_GPP_FAN_RESISTER_C, 17},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_FAN_FAIL_B, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_FAIL_C, GPIO_UNDEF},
	{SYNO_KW_GPP_INTER_LOCK, 4},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, 29},
	{SYNO_KW_GPP_MODEL_ID_2, 46},
	{SYNO_KW_GPP_MODEL_ID_3, 47},
	{SYNO_KW_GPP_HDD1_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_POWER, 44},
	{SYNO_KW_GPP_HDD4_POWER, 45},
	{SYNO_KW_GPP_HDD1_LED_0, 36},
	{SYNO_KW_GPP_HDD1_LED_1, 37},
	{SYNO_KW_GPP_HDD2_LED_0, 38},
	{SYNO_KW_GPP_HDD2_LED_1, 39},
	{SYNO_KW_GPP_HDD3_LED_0, 40},
	{SYNO_KW_GPP_HDD3_LED_1, 41},
	{SYNO_KW_GPP_HDD4_LED_0, 42},
	{SYNO_KW_GPP_HDD4_LED_1, 43},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};
#endif

#if defined(SYNO_GPIO_FOR_6282_SLIM) || defined(SYNO_GPIO_FOR_6282_DS)
#define DS_6282_4BAY_BOARD_MAC_INFO_NUM 1
MV_BOARD_MAC_INFO DS_6282_4BAY_InfoBoardMacInfo[] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
{
#ifdef SYNO_GPIO_FOR_6282_DS //DS411 has a different PHY address
	{BOARD_MAC_SPEED_AUTO, 0x1},
#else //DS411slim
	{BOARD_MAC_SPEED_AUTO, 0x8},
#endif
};
#elif defined(SYNO_GPIO_FOR_6282_RS)
#define DS_6282_4BAY_BOARD_MAC_INFO_NUM 2
MV_BOARD_MAC_INFO DS_6282_4BAY_InfoBoardMacInfo[] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
};
#else
#define DS_6282_4BAY_BOARD_MAC_INFO_NUM 1
MV_BOARD_MAC_INFO DS_6282_4BAY_InfoBoardMacInfo[] =
    /* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
{
    {BOARD_MAC_SPEED_AUTO, 0x8},
};
#endif

#define SYNO_6282_4BAY_OE_LOW                    0x0
#define SYNO_6282_4BAY_OE_HIGH                   0x0
#define SYNO_6282_4BAY_OE_VAL_LOW                (BIT15|BIT16)
#define SYNO_6282_4BAY_OE_VAL_HIGH               (BIT5|BIT7|BIT9|BIT11)
MV_BOARD_INFO SYNO_4BAY_INFO = {
	"Synology Disk Station",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6282A_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6282AInfoBoardMppTypeInfo,
	DB_88F6282A_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	SYNO_6282_4Bay_InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6282A_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6282AInfoBoardDeCsInfo,
	0,			/* numBoardTwsiDev */
	NULL,
	DS_6282_4BAY_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	DS_6282_4BAY_InfoBoardMacInfo,
	SYNO_DS_6282_4BAY_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	SYNO_DS_6282_4Bay_InfoBoardGppInfo,
	0,			/* activeLedsNumber */
	NULL,
	0,						/* ledsPolarity */
	SYNO_6282_4BAY_OE_LOW,				/* gppOutEnLow */
	SYNO_6282_4BAY_OE_HIGH,				/* gppOutEnHigh */
	SYNO_6282_4BAY_OE_VAL_LOW,				/* gppOutValLow */
	SYNO_6282_4BAY_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL,						/* pSwitchInfo */
    0,
    0,
    0
};

#define SYNO_6702_1BAY_BOARD_MPP_GROUP_TYPE_NUM  0x1
MV_BOARD_MPP_TYPE_INFO SYNO_6702_1BAYInfoBoardMppTypeInfo[] =
{
        { MV_BOARD_AUTO, MV_BOARD_AUDIO }
};

#define SYNO_6702_1BAY_BOARD_MPP_CONFIG_NUM             0x1
#define SYNO_6702_1BAY_MPP7_0                    0x01222222
#define SYNO_6702_1BAY_MPP15_8                   0x30005511
#define SYNO_6702_1BAY_MPP23_16                  0x00000003
#define SYNO_6702_1BAY_MPP31_24                  0x00000000
#define SYNO_6702_1BAY_MPP39_32                  0x00000000
#define SYNO_6702_1BAY_MPP47_40                  0x00000000
#define SYNO_6702_1BAY_MPP55_48                  0x00000000

MV_BOARD_MPP_INFO SYNO_6702_1BAY_InfoBoardMppConfigValue[] =
{
       {
               {
                       SYNO_6702_1BAY_MPP7_0,
                       SYNO_6702_1BAY_MPP15_8,
                       SYNO_6702_1BAY_MPP23_16,
                       SYNO_6702_1BAY_MPP31_24,
                       SYNO_6702_1BAY_MPP39_32,
                       SYNO_6702_1BAY_MPP47_40,
                       SYNO_6702_1BAY_MPP55_48
               }
       }
};

/*
 * Marvellvell 886f6702 1 bay, for ds112j
 *
Pin 		Mode	Signal select and definition	Input/output	Pull-up/pull-down
MPP[0:3]	0x2			SPI signal						out             x000
MPP[4]		0x2			UART0 RXD						in
MPP[5]      0x2			UART0 TXD						out				1
MPP[6]	    0x1			SYSRST_OUTn						out
MPP[7]		0x0			Fan speed low					out				1
MPP[8]		0x1			TW_SDA							in/out
MPP[9]		0x1			TW_SCK							in/out
MPP[10]		0x5			SATA1_LEDn						out				1	
MPP[11]		0x5			SATA0 LEDn						out
MPP[12]		0x0			HDD_PWR_EN_1					out				1
MPP[13]		0x0			SATA port 1 FAULTn LED			out	
MPP[14]     0x3			MRRSTJ (Reserved)				in
MPP[15]		0x3			UART0 TXD						out
MPP[16]     0x3			UART0 RXD						in
MPP[17]		0x0			SATA port 0 FAULTn LED			out	
MPP[18]		0x0			Fan speed middle				out				1
MPP[19]		0x0			Fan speed high					out				1
MPP[20:27]	0x0			Reserved
MPP[28]		0x0			Model ID						in
*/

#define SYNO_6702_1BAY_BOARD_GPP_INFO_NUM                        0x1E
MV_BOARD_GPP_INFO SYNO_6702_1BAY_InfoBoardGppInfo[] =
{
	{SYNO_KW_GPP_HDD2_FAIL_LED, 13},
	{SYNO_KW_GPP_HDD1_FAIL_LED, 17},
	{SYNO_KW_GPP_HDD2_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_FAN_RESISTER_A, 7},
	{SYNO_KW_GPP_FAN_RESISTER_B, 18},
	{SYNO_KW_GPP_FAN_RESISTER_C, 19},
	{SYNO_KW_GPP_FAN_FAIL, 35},
	{SYNO_KW_GPP_HDD1_POWER, 12},
	{SYNO_KW_GPP_HDD3_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_POWER, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_0, 28},
	{SYNO_KW_GPP_MODEL_ID_1, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_2, GPIO_UNDEF},
	{SYNO_KW_GPP_MODEL_ID_3, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD1_LED_0, GPIO_UNDEF}, //used for external SATA controller
	{SYNO_KW_GPP_HDD1_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD2_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD3_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD4_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_0, GPIO_UNDEF},
	{SYNO_KW_GPP_HDD5_LED_1, GPIO_UNDEF},
	{SYNO_KW_GPP_INTER_LOCK, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_REQ, GPIO_UNDEF},
	{SYNO_KW_GPP_BUZZER_MUTE_ACK, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS1_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_RPS2_ON, GPIO_UNDEF},
	{SYNO_KW_GPP_ALERM_LED, GPIO_UNDEF},
};

MV_DEV_CS_INFO SYNO_DS112jInfoBoardDeCsInfo[] =
{
       {0, N_A, BOARD_DEV_SPI_FLASH, 8}
};

#define SYNO_6702_1BAY_OE_LOW                    0x0
#define SYNO_6702_1BAY_OE_HIGH                   0x0
#define SYNO_6702_1BAY_OE_VAL_LOW                (BIT13|BIT17|BIT18|BIT19)
#define SYNO_6702_1BAY_OE_VAL_HIGH               0x0

MV_BOARD_INFO SYNO_6702_1BAY_INFO = {
    "Synology Disk Station",                      /* boardName[MAX_BOARD_NAME_LEN] */

    SYNO_6702_1BAY_BOARD_MPP_GROUP_TYPE_NUM,          /* numBoardMppGroupType */
    SYNO_6702_1BAYInfoBoardMppTypeInfo,

    SYNO_6702_1BAY_BOARD_MPP_CONFIG_NUM,              /* numBoardMppConfig */
    SYNO_6702_1BAY_InfoBoardMppConfigValue,

    0,                                            /* intsGppMaskLow */
    0,                                            /* intsGppMaskHigh */

    SYNO_DS109_BOARD_DEVICE_CONFIG_NUM,           /* numBoardDevIf */
    SYNO_DS112jInfoBoardDeCsInfo,

    SYNO_DS109_BOARD_TWSI_DEF_NUM,                /* numBoardTwsiDev */
    SYNO_DS109InfoBoardTwsiDev,

    DS211_BOARD_MAC_INFO_NUM,                /* numBoardMacInfo */
    DS211InfoBoardMacInfo,

    SYNO_6702_1BAY_BOARD_GPP_INFO_NUM,                /* numBoardGppInfo */
    SYNO_6702_1BAY_InfoBoardGppInfo,

    0,                                            /* activeLedsNumber */

    NULL,
    N_A,                                          /* ledsPolarity */

    SYNO_6702_1BAY_OE_LOW,                            /* gppOutEnLow */
    SYNO_6702_1BAY_OE_HIGH,                           /* gppOutEnHigh */
    SYNO_6702_1BAY_OE_VAL_LOW,                        /* gppOutValLow */
    SYNO_6702_1BAY_OE_VAL_HIGH,                       /* gppOutValHigh */
    0,
	0,
    NULL,                                          /* pSwitchInfo */
	0,
	0,
	0
};
#endif


MV_BOARD_INFO*	boardInfoTbl[] = 	{
                    &db88f6281AInfo,
                    &rd88f6281AInfo,
                    &db88f6192AInfo,
                    &rd88f6192AInfo,
                    &db88f6180AInfo,
                    &db88f6190AInfo,
                    &rd88f6190AInfo,
                    &rd88f6281APcacInfo,
                    &dbCustomerInfo,
                    &sheevaPlugInfo,
                    &db88f6280AInfo,
                    &db88f6282AInfo,
					&rd88f6282aInfo,
                    &db88f6701AInfo,
                    &db88f6702AInfo
#ifdef CONFIG_SYNO_MV88F6281
					,
					NULL,                           /* 0x0F */
					NULL,                           /* 0x10 */
					NULL,                           /* 0x11 */
					NULL,                           /* 0x12 */
					&SYNO_DS409_INFO,               /* SYNO_DS409_ID */
					&SYNO_DS409slim_INFO,           /* SYNO_DS409slim_ID */
					&SYNO_DS109_INFO,               /* SYNO_DS109_ID */
					NULL,                           /* reserve for usb station */
					&SYNO_DS211_INFO,               /* SYNO_DS211_ID */
					&SYNO_4BAY_INFO,                /* SYNO_DS411slim_ID */
					&SYNO_4BAY_INFO,				/* SYNO_6282_RS_ID */
					&SYNO_4BAY_INFO,				/* SYNO_DS411_ID */
					&SYNO_DS211_INFO,				/* SYNO_DS212_ID */
					&SYNO_6702_1BAY_INFO,           /* SYNO_DS112j_ID */
#endif
					};


