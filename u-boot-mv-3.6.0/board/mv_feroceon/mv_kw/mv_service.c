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

#include "mvBoardEnvSpec.h"
#include "mvBoardEnvLib.h"

/*********************************************************** 
* Init the PHY or Switch of the board 			   *
 ***********************************************************/
void mvBoardEgigaPhySwitchInit(void) 
{
	if (
#ifdef CONFIG_SYNO_MV88F6281
		(SYNO_DS409_ID == mvBoardIdGet()) ||
		(SYNO_DS109_ID == mvBoardIdGet()) ||
		(SYNO_DS409slim_ID == mvBoardIdGet()) ||
		(SYNO_DS211_ID == mvBoardIdGet()) ||
		(SYNO_DS411slim_ID == mvBoardIdGet()) ||
		(SYNO_6282_RS_ID == mvBoardIdGet()) ||
		(SYNO_DS411_ID == mvBoardIdGet()) ||
		(SYNO_DS212_ID == mvBoardIdGet()) ||
		(SYNO_6702_1BAY_ID == mvBoardIdGet()) ||
#endif
		(DB_88F6281A_BP_ID == mvBoardIdGet()) ||
        (DB_88F6282A_BP_ID == mvBoardIdGet()) ||
        (DB_88F6701A_BP_ID == mvBoardIdGet()) ||
        (DB_88F6702A_BP_ID == mvBoardIdGet()) ||
		(DB_88F6192A_BP_ID == mvBoardIdGet()))
	{
		if ( !(mvBoardIsPortInGmii()))
		{
#if defined (SYNO_GPIO_FOR_6282_DS) ||  defined (SYNO_OPTION_6282_PHY_WOL)
			//if it is DS411, DS212, DS211pr1, or DS212p (with 1318  Phy)
			mvEthE1310PhyBasicInit(0);
			mvEthE1310PhyBasicInit(1);
#else
			mvEthE1116PhyBasicInit(0);
			mvEthE1116PhyBasicInit(1);
#endif
		}
	}
	else if ((DB_88F6190A_BP_ID == mvBoardIdGet()) ||
            (DB_88F6180A_BP_ID == mvBoardIdGet()) ||
             (DB_88F6280A_BP_ID == mvBoardIdGet()) ||
            (RD_88F6192A_ID == mvBoardIdGet()) ||
            (RD_88F6190A_ID == mvBoardIdGet()) ||
             (RD_88F6281A_PCAC_ID == mvBoardIdGet()) ||
             (SHEEVA_PLUG_ID == mvBoardIdGet()))
	{
		mvEthE1116PhyBasicInit(0);
	}
	else if (RD_88F6281A_ID == mvBoardIdGet())
	{
		mvEthE6161SwitchBasicInit(0);
        mvEthE1116PhyBasicInit(1);
	}
	else if (RD_88F6282A_ID == mvBoardIdGet())
	{
        	mvEthE1310PhyBasicInit(0);
		mvEthE6171SwitchBasicInit(1);
	}
	else if (DB_CUSTOMER_ID == mvBoardIdGet())
	{
	}
}

#include "mv_service.h"
