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
#define DB_88F6281A_BOARD_GPP_INFO_NUM		    0x3
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
	{BOARD_GPP_TSU_DIRCTION, 33},
	{BOARD_GPP_SDIO_DETECT, 38},
	{BOARD_GPP_USB_VBUS, 49}
	};

MV_DEV_CS_INFO db88f6281AInfoBoardDeCsInfo[] = 
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
	N_A,
	N_A,						/* ledsPolarity */		
	DB_88F6281A_OE_LOW,				/* gppOutEnLow */
	DB_88F6281A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6281A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6281A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL						/* pSwitchInfo */
};


#define RD_88F6281A_BOARD_PCI_IF_NUM		0x0
#define RD_88F6281A_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6281A_BOARD_MAC_INFO_NUM		0x2
#define RD_88F6281A_BOARD_GPP_INFO_NUM		0x5
#define RD_88F6281A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6281A_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6281A_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6281A_BOARD_DEBUG_LED_NUM		0x0

MV_BOARD_MAC_INFO rd88f6281AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0xa},
    {BOARD_MAC_SPEED_AUTO, 0xb}
	}; 

MV_BOARD_SWITCH_INFO rd88f6281AInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode, MV_32 switchOnPort} */
	{{38, {0, 1, 2, 3}, 5, 1, 0},
	 {-1, {-1}, -1, -1, -1}};

MV_BOARD_TWSI_INFO	rd88f6281AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6281AInfoBoardMppTypeInfo[] = 
	{{MV_BOARD_RGMII, MV_BOARD_TDM}
	}; 

MV_DEV_CS_INFO rd88f6281AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */
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
	N_A,						/* ledsPolarity */		
	RD_88F6281A_OE_LOW,				/* gppOutEnLow */
	RD_88F6281A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6281A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6281A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT6, 						/* gppPolarityValHigh */
	rd88f6281AInfoBoardSwitchInfo			/* pSwitchInfo */
};


#define DB_88F6192A_BOARD_PCI_IF_NUM            0x0
#define DB_88F6192A_BOARD_TWSI_DEF_NUM		    0x7
#define DB_88F6192A_BOARD_MAC_INFO_NUM		    0x2
#define DB_88F6192A_BOARD_GPP_INFO_NUM		    0x3
#define DB_88F6192A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6192A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6192A_BOARD_DEVICE_CONFIG_NUM	    0x1
#define DB_88F6192A_BOARD_DEBUG_LED_NUM		    0x0

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
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */
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
	N_A,
	N_A,						/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
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
	N_A,
	N_A,						/* ledsPolarity */		
	DB_88F6192A_OE_LOW,				/* gppOutEnLow */
	DB_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
};

#define RD_88F6192A_BOARD_PCI_IF_NUM		0x0
#define RD_88F6192A_BOARD_TWSI_DEF_NUM		0x0
#define RD_88F6192A_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6192A_BOARD_GPP_INFO_NUM		0xE
#define RD_88F6192A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6192A_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6192A_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6192A_BOARD_DEBUG_LED_NUM		0x3

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
	N_A,						/* ledsPolarity */		
	RD_88F6192A_OE_LOW,				/* gppOutEnLow */
	RD_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
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
	N_A,						/* ledsPolarity */		
	RD_88F6192A_OE_LOW,				/* gppOutEnLow */
	RD_88F6192A_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6192A_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6192A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
};

#define DB_88F6180A_BOARD_PCI_IF_NUM		0x0
#define DB_88F6180A_BOARD_TWSI_DEF_NUM		0x2
#define DB_88F6180A_BOARD_MAC_INFO_NUM		0x1
#define DB_88F6180A_BOARD_GPP_INFO_NUM		0x0
#define DB_88F6180A_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6180A_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6180A_BOARD_DEVICE_CONFIG_NUM	0x1
#define DB_88F6180A_BOARD_DEBUG_LED_NUM		0x2

MV_U8	db88f6180AInfoBoardDebugLedIf[] =
	{7, 10};

MV_BOARD_TWSI_INFO	db88f6180AInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6180AInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_GPP_INFO db88f6180AInfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_USB_VBUS, 6}
	};

MV_BOARD_MPP_TYPE_INFO db88f6180AInfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_DEV_CS_INFO db88f6180AInfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */

MV_BOARD_MPP_INFO	db88f6180AInfoBoardMppConfigValue[] = 
	{{{
	DB_88F6180A_MPP0_7,		
	DB_88F6180A_MPP8_15,
    DB_88F6180A_MPP16_19,
    DB_88F6180A_MPP35_39,
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
	db88f6180AInfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6180A_OE_LOW,				/* gppOutEnLow */
	DB_88F6180A_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6180A_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6180A_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
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

#define SYNO_DS109_BOARD_GPP_INFO_NUM                        0x7
MV_BOARD_GPP_INFO SYNO_DS109InfoBoardGppInfo[] =
{
        {SYNO_DS109_GPP_HDD2_FAIL_LED, 22},
        {SYNO_DS109_GPP_HDD1_FAIL_LED, 23},
        {SYNO_DS109_GPP_HDD2_POWER, 31},                  
        {SYNO_DS109_GPP_FAN_RESISTER_A, 32},
        {SYNO_DS109_GPP_FAN_RESISTER_B, 33},
        {SYNO_DS109_GPP_FAN_RESISTER_C, 34},
        {SYNO_DS109_GPP_FAN1_FAIL, 35},
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
    NULL,
	NULL,
    NULL                                          /* pSwitchInfo */
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

#define SYNO_DS409slim_BOARD_GPP_INFO_NUM                      13
MV_BOARD_GPP_INFO SYNO_DS409slimInfoBoardGppInfo[] =
{
        {SYNO_DS409slim_GPP_HDD1_LED_0, 20},
        {SYNO_DS409slim_GPP_HDD1_LED_1, 21},
        {SYNO_DS409slim_GPP_HDD2_LED_0, 22},
        {SYNO_DS409slim_GPP_HDD2_LED_1, 23},
        {SYNO_DS409slim_GPP_HDD3_LED_0, 24},
        {SYNO_DS409slim_GPP_HDD3_LED_1, 25},
        {SYNO_DS409slim_GPP_HDD4_LED_0, 26},
        {SYNO_DS409slim_GPP_HDD4_LED_1, 27},
        {SYNO_DS409slim_GPP_BP_LOCK_OUT, 31},
        {SYNO_DS409slim_GPP_FAN_RESISTER_A, 32},
        {SYNO_DS409slim_GPP_FAN_RESISTER_B, 33},
        {SYNO_DS409slim_GPP_FAN_RESISTER_C, 34},
        {SYNO_DS409slim_GPP_FAN1_FAIL, 35},
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
	NULL,
	NULL,
    NULL                                              /* pSwitchInfo */
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

#define SYNO_DS409_BOARD_GPP_INFO_NUM                      22
MV_BOARD_GPP_INFO SYNO_DS409InfoBoardGppInfo[] =
{
        {SYNO_DS409_GPP_ALERM_LED, 12},
        {SYNO_DS409_GPP_FAN_150, 15},
        {SYNO_DS409_GPP_FAN_120, 16},
        {SYNO_DS409_GPP_FAN_100, 17},
		{SYNO_DS409_GPP_FAN_SENSE, 18},
		{SYNO_DS409_GPP_INTER_LOCK, 19},		
        {SYNO_DS409_GPP_MODEL_ID_0, 28},
        {SYNO_DS409_GPP_MODEL_ID_1, 29},        
		{SYNO_DS409_GPP_HDD1_LED_0, 36},
		{SYNO_DS409_GPP_HDD1_LED_1, 37},
		{SYNO_DS409_GPP_HDD2_LED_0, 38},
		{SYNO_DS409_GPP_HDD2_LED_1, 39},
		{SYNO_DS409_GPP_HDD3_LED_0, 40},
		{SYNO_DS409_GPP_HDD3_LED_1, 41},
		{SYNO_DS409_GPP_HDD4_LED_0, 42},
		{SYNO_DS409_GPP_HDD4_LED_1, 43},
		{SYNO_DS409_GPP_HDD5_LED_0, 44},
		{SYNO_DS409_GPP_HDD5_LED_1, 45},
		{SYNO_DS409_GPP_BUZZER_MUTE_REQ, 46},
		{SYNO_DS409_GPP_BUZZER_MUTE_ACK, 47},
		{SYNO_DS409_GPP_RPS1_ON, 48},
		{SYNO_DS409_GPP_RPS2_ON, 49},
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

    NULL,                						/* numBoardTwsiDev */
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
	NULL,
	NULL,
    NULL                                              /* pSwitchInfo */
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
#ifdef CONFIG_SYNO_MV88F6281
		    NULL,                           /* 0x07 */
		    NULL,                           /* 0x08 */
		    NULL,                           /* 0x09 */
		    NULL,                           /* 0x0A */
		    NULL,                           /* 0x0B */
		    NULL,                           /* 0x0C */
		    NULL,                           /* 0x0D */
		    NULL,                           /* 0x0E */
		    NULL,                           /* 0x0F */
		    NULL,                           /* 0x10 */
		    NULL,                           /* 0x11 */
		    NULL,                           /* 0x12 */
		    &SYNO_DS409_INFO,               /* SYNO_DS409_ID */
		    &SYNO_DS409slim_INFO,           /* SYNO_DS409slim_ID */
		    &SYNO_DS109_INFO                /* SYNO_DS109_ID */
#endif
					};


