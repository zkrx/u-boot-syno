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

#include "mv_service.h"

/*********************************************************** 
* Init the PHY or Switch of the board 			   *
 ***********************************************************/
MV_VOID mvEgigaPhySwitchInit(void) 
{
	if (RD_88F6183_AP == mvBoardIdGet())
	{
		mvEthE6161SwitchBasicInit();
	}
	else if ((DB_88F5181_5281_DDR2 == mvBoardIdGet())||
	    (DB_88F5X81_DDR1 == mvBoardIdGet()) ||
	    (DB_88F5181_DDR1_PEXPCI == mvBoardIdGet()) ||
	    (RD_88F5181_POS_NAS == mvBoardIdGet()) ||
	    (DB_88F5182_DDR2 == mvBoardIdGet())||
	    (DB_88F5182_DDR2_A == mvBoardIdGet())||
	    (DB_88F5082_DDR2 == mvBoardIdGet())||
	    (RD_88F5182_2XSATA == mvBoardIdGet())||
	    (DB_88F5181L_DDR2_2XTDM == mvBoardIdGet()) ||
	    (DB_88F5180N_DDR1 == mvBoardIdGet()) ||
	    (DB_88F5180N_DDR2 == mvBoardIdGet()) ||
	    (DB_88W8660_DDR2 == mvBoardIdGet()) )
	{
		mvEthE1111PhyBasicInit(0);
	
	}else if ((RD_88F5182_2XSATA3 == mvBoardIdGet()) ||
			(RD_88F5182_2XSATA == mvBoardIdGet()) ||
			(RD_88F5082_2XSATA == mvBoardIdGet()) ||
			(RD_88F5082_2XSATA3 == mvBoardIdGet()) ||
	    		(DB_88F5X81_DDR2 == mvBoardIdGet()) ||
			(RD_88F6183_GP == mvBoardIdGet()) ||
			(DB_88F6183_BP == mvBoardIdGet()))
	{
		/* 1118 is identical to 1116 */
		mvEthE1116PhyBasicInit(0);

	}else if (DB_88F5181_5281_DDR1 == mvBoardIdGet())
	{
		mvEthE1011PhyBasicInit(0);

	}else if ((RD_88F5181_VOIP == mvBoardIdGet()) || (RD_88F5181L_VOIP_FE == mvBoardIdGet()) ||
			(RD_88F5181_GTW_FE == mvBoardIdGet()))
	{
		mvEthE6063SwitchBasicInit(0);

	}else if ((RD_88W8660_DDR1 == mvBoardIdGet()) || (RD_88W8660_AP82S_DDR1 == mvBoardIdGet()))
	{
		mvEthE6065_61SwitchBasicInit(0);
	
	}else if ( (RD_88F5181L_VOIP_GE == mvBoardIdGet()) || 
			(RD_88F5181_GTW_GE == mvBoardIdGet()) ||
			(RD_88F5181L_VOIP_FXO_GE == mvBoardIdGet()))
	{
		mvEthE6131SwitchBasicInit(0);

	}else if (RD_88F6082_MICRO_DAS_NAS == mvBoardIdGet())
	{
		mvEthE1112PhyBasicInit(0);
	}
	else if ((DB_88F6082_BP == mvBoardIdGet()) ||
		(DB_88F6082L_BP == mvBoardIdGet()))
	{
		mvEthE1112PhyBasicInit(0);
		mvEthE1112PhyBasicInit(1);
	}
	else if (RD_88F6082_DX243_24G == mvBoardIdGet())
	{
		mvEthE1112PhyBasicInit(0);
		mvEthE1111PhyBasicInit(1);
	}
	else if (RD_88F6082_NAS == mvBoardIdGet())
	{
		/* Power down phy in case we enter power down mode */
		mvEthE1112PhyPowerDown(0);
		mvEthE1112PhyPowerDown(1);
	}
	else if (RD_88F6183_AP == mvBoardIdGet())
	{
		mvEthE6161SwitchBasicInit();
	}
}
