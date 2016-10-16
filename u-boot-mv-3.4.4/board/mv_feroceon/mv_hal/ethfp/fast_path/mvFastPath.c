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

/*******************************************************************************
* mvFastPath.c - Marvell Network Fast Processing (Routing and NAT)
*
* DESCRIPTION:
*       
*       Supported Features:
*       - OS independent. 
*
*******************************************************************************/

/* includes */
#include "mvOs.h"
#include "mvDebug.h"
#include "ethfp/fast_path/mvFastPath.h"
#include "ethfp/mvEth.h"

MV_U32  ip_hash_depth[ETH_FP_MAX_HASH_DEPTH+1];
MV_U32  nat_hash_depth[ETH_FP_MAX_HASH_DEPTH+1];


static struct ruleHashBucket    *ruleDb;
static MV_U32                   ruleDbSize;

static MV_U32                   nfpRuleSetCount;
static MV_U32                   nfpRuleUpdateCount;
static MV_U32                   nfpRuleDeleteCount;

MV_U32   fp_ip_jhash_iv = 0;


MV_STATUS   mvFpInit(void)
{
    memset(nat_hash_depth, 0, (ETH_FP_MAX_HASH_DEPTH+1)*sizeof(MV_U32));
    memset(ip_hash_depth, 0, (ETH_FP_MAX_HASH_DEPTH+1)*sizeof(MV_U32));
    fp_ip_jhash_iv = mvOsRand();

    return MV_OK;
}


/* Initialize NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbInit(MV_U32 dbSize)
{
    ruleDb = (struct ruleHashBucket *)mvOsMalloc(sizeof(struct ruleHashBucket)*dbSize);
    if (ruleDb == NULL) {
	    mvOsPrintf("NFP Rule DB: Not Enough Memory\n");
	    return MV_NO_RESOURCE;
    }
    ruleDbSize = dbSize;
    memset(ruleDb, 0, sizeof(struct ruleHashBucket)*ruleDbSize);
    nfpRuleSetCount = nfpRuleUpdateCount = nfpRuleDeleteCount = 0;

    mvOsPrintf("mvFpRuleDb (%p): %d entries, %d bytes\n", 
                ruleDb, ruleDbSize, sizeof(struct ruleHashBucket)*ruleDbSize);

    return MV_OK;
}

/* Clear NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbClear(void)
{
	MV_U32 i = 0;
	MV_FP_RULE *currRule;
	MV_FP_RULE *tmpRule;

	if (ruleDb == NULL) 
		return MV_NOT_INITIALIZED;

	for (i = 0; i < ruleDbSize; i++) {
		currRule = ruleDb[i].ruleChain;
		while (currRule != NULL) {
			tmpRule = currRule;
			currRule = currRule->next;
			mvOsFree(tmpRule);
		}
        	ruleDb[i].ruleChain = NULL;
	}
	return MV_OK;
}


/* Free Rule Database memory */
void mvFpRuleDbDestroy(void)
{
	if (ruleDb != NULL)
		mvOsFree(ruleDb);
}

/* Print rule action type. Assume rule is not NULL. */
static void    mvFpActionTypePrint(const MV_FP_RULE *rule)
{
	switch(rule->mgmtInfo.actionType) {
		case MV_FP_ROUTE_CMD:
			mvOsPrintf("A=Route, ");
			break;
		case MV_FP_DROP_CMD:
			mvOsPrintf("A=Drop, ");
			break;
		case MV_FP_TO_STACK_CMD:
			mvOsPrintf("A=Stack, ");
			break;
		default:
			mvOsPrintf("A=Unknown (%d), ", rule->mgmtInfo.actionType);
			break;
	}
}

/* Print rule type (static or dynamic). Assume rule is not NULL. */
static void    mvFpRuleTypePrint(const MV_FP_RULE *rule)
{
	switch(rule->mgmtInfo.ruleType) {
		case MV_FP_STATIC_RULE:
			mvOsPrintf("T=Static");
			break;
		case MV_FP_DYNAMIC_RULE:
			mvOsPrintf("T=Dynamic");
			break;
		default:
			mvOsPrintf("T=Unknown");
			break;
	}
}

/* Print a NFP Rule */
void    mvFpRulePrint(const MV_FP_RULE *rule)
{
	mvFpActionTypePrint(rule);
	mvFpRuleTypePrint(rule);
	mvOsPrintf(", SIP=");
	mvDebugPrintIpAddr(MV_32BIT_BE(rule->routingInfo.srcIp));
	mvOsPrintf(", DIP=");
	mvDebugPrintIpAddr(MV_32BIT_BE(rule->routingInfo.dstIp));
	mvOsPrintf(", GTW=");
	mvDebugPrintIpAddr(MV_32BIT_BE(rule->routingInfo.defGtwIp));
	mvOsPrintf(", DA=");
	mvDebugPrintMacAddr(rule->routingInfo.dstMac);
	mvOsPrintf(", SA=");
	mvDebugPrintMacAddr(rule->routingInfo.srcMac);
	mvOsPrintf(", inIf=%d", rule->routingInfo.inIfIndex);
	mvOsPrintf(", outIf=%d", rule->routingInfo.outIfIndex);

#if defined(CONFIG_MV_GTW_HEADER_MODE)
    mvOsPrintf(", mvHeader=0x%X", rule->routingInfo.pktHdr.mvHeader.header);
#elif defined (CONFIG_MV_GTW_DSA_TAG_MODE)
    mvOsPrintf(", dsaTag=0x%X", rule->routingInfo.pktHdr.dsaTag);
#endif /* CONFIG_MV_GTW_HEADER_MODE || CONFIG_MV_GTW_EXT_DSA_TAG_MODE || CONFIG_MV_GTW_EXT_DSA_TAG_MODE */

  	mvOsPrintf(", count=%d, aware_flags=0x%X", rule->mgmtInfo.new_count, rule->routingInfo.aware_flags);
	mvOsPrintf("\n");
}

/* Print NFP Rule Database (Routing + ARP information table) */
MV_STATUS   mvFpRuleDbPrint(void)
{
	MV_U32 count, i;
	MV_FP_RULE *currRule;

	mvOsPrintf("\nPrinting NFP Rule Database\n");
    count = 0;

	for(i=0; i<ruleDbSize; i++) 
    {
		currRule = ruleDb[i].ruleChain;

        if(currRule != NULL)
    	    mvOsPrintf("\n%03u: FP DB hash=0x%x", count, i);

		while (currRule != NULL) {
          	mvOsPrintf("\n%03u: Rule=%p: ", count, currRule);
	        mvFpRulePrint(currRule);
	        currRule = currRule->next;
            count++;
        }
	}
	return MV_OK;
}


/* Copy all the information from src_rule to new_rule */
/* Warning - doesn't perform any checks on memory, just copies */
/* count is set to zero in new_rule */
/* Note: the next pointer is not updated . */
void mvFpRuleCopy(MV_FP_RULE *dstRule, const MV_FP_RULE *srcRule)
{
	dstRule->mgmtInfo.actionType = srcRule->mgmtInfo.actionType;
	dstRule->mgmtInfo.new_count = srcRule->mgmtInfo.new_count;
	dstRule->mgmtInfo.old_count = srcRule->mgmtInfo.old_count;
	dstRule->mgmtInfo.ruleType = srcRule->mgmtInfo.ruleType;
	dstRule->mgmtInfo.snat_aware_refcnt = srcRule->mgmtInfo.snat_aware_refcnt;
	dstRule->mgmtInfo.dnat_aware_refcnt = srcRule->mgmtInfo.dnat_aware_refcnt;

	dstRule->routingInfo.aware_flags = srcRule->routingInfo.aware_flags;
	dstRule->routingInfo.srcIp = srcRule->routingInfo.srcIp;
	dstRule->routingInfo.dstIp = srcRule->routingInfo.dstIp;
	dstRule->routingInfo.defGtwIp = srcRule->routingInfo.defGtwIp;
	memcpy(dstRule->routingInfo.srcMac, srcRule->routingInfo.srcMac, MV_MAC_ADDR_SIZE);
	memcpy(dstRule->routingInfo.dstMac, srcRule->routingInfo.dstMac, MV_MAC_ADDR_SIZE);
	dstRule->routingInfo.inIfIndex = srcRule->routingInfo.inIfIndex;
	dstRule->routingInfo.outIfIndex = srcRule->routingInfo.outIfIndex;
#ifdef CONFIG_MV_GATEWAY
	dstRule->routingInfo.pktHdr = srcRule->routingInfo.pktHdr;
#endif /* CONFIG_MV_GATEWAY */

}

/* Get the count value for a rule that matches the given SIP, DIP */
MV_U32 mvFpRouteCountGet(MV_U32 srcIp, MV_U32 dstIp)
{
    MV_U32      hash, hash_tr;
    MV_FP_RULE  *pRule;

    hash = mv_jhash_3words(dstIp, srcIp, (MV_U32)0, fp_ip_jhash_iv);
    hash_tr = hash & (ruleDbSize - 1);

    pRule = ruleDb[hash_tr].ruleChain;    
    while(pRule)
    {
        /* look for a matching rule */
        if( (pRule->routingInfo.dstIp == dstIp) && 
            (pRule->routingInfo.srcIp == srcIp) )
        {
	        return pRule->mgmtInfo.new_count;
        }
        pRule = pRule->next;
    }
    return 0;
}

MV_STATUS mvFpRuleAwareSet(MV_FP_RULE *pSetRule)
{
    MV_U32      hash, hash_tr;
    MV_FP_RULE  *pRule;

    hash = mv_jhash_3words(pSetRule->routingInfo.dstIp, pSetRule->routingInfo.srcIp, 
                            (MV_U32)0, fp_ip_jhash_iv);
    hash_tr = hash & (ruleDbSize - 1);

    pRule = ruleDb[hash_tr].ruleChain;    
    while(pRule)
    {
	    if( (pRule->routingInfo.srcIp == pSetRule->routingInfo.srcIp) &&
	        (pRule->routingInfo.dstIp == pSetRule->routingInfo.dstIp) ) {

	        
            pRule->routingInfo.aware_flags = pSetRule->routingInfo.aware_flags;
#ifdef MV_FP_DEBUG
	        mvOsPrintf("Update FP aware: DIP=%u.%u.%u.%u, SIP=%u.%u.%u.%u, hash0x%x, flags=0x%x\n",
                	    MV_IP_QUAD(pSetRule->routingInfo.dstIp), 
                        MV_IP_QUAD(pSetRule->routingInfo.srcIp), 
                	    hash_tr, pSetRule->routingInfo.aware_flags);
#endif
	        return MV_OK;
	    }
        pRule = pRule->next;
    }
#ifdef MV_FP_DEBUG
    mvOsPrintf("FP aware NOT found: DIP=%u.%u.%u.%u, SIP=%u.%u.%u.%u, hash=0x%x, flags=0x%x\n",
                	    MV_IP_QUAD(pSetRule->routingInfo.dstIp), 
                        MV_IP_QUAD(pSetRule->routingInfo.srcIp), 
                	    hash_tr, pSetRule->routingInfo.aware_flags);
#endif
    return MV_NOT_FOUND;
}

/* Set a Routing Rule: create a new rule or update an existing rule  */
/* in the Routing + ARP information table */
MV_STATUS mvFpRuleSet(MV_FP_RULE *pSetRule)
{
    MV_U32      hash, hash_tr;
    MV_FP_RULE  *pRule, *pNewRule;

    hash = mv_jhash_3words(pSetRule->routingInfo.dstIp, pSetRule->routingInfo.srcIp, 
                            (MV_U32)0, fp_ip_jhash_iv);
    hash_tr = hash & (ruleDbSize - 1);

    pRule = ruleDb[hash_tr].ruleChain;    
    while(pRule)
    {
	    if( (pRule->routingInfo.srcIp == pSetRule->routingInfo.srcIp &&
	         pRule->routingInfo.dstIp == pSetRule->routingInfo.dstIp) ) {

	        mvFpRuleCopy(pRule, pSetRule);
            nfpRuleUpdateCount++;

#ifdef MV_FP_DEBUG
	        mvOsPrintf("UpdNFP_%03u: DIP=%u.%u.%u.%u, SIP=%u.%u.%u.%u, hash=0x%04x\n",
                	    nfpRuleUpdateCount, MV_IP_QUAD(pSetRule->routingInfo.dstIp), 
                        MV_IP_QUAD(pSetRule->routingInfo.srcIp), hash_tr);
#endif
	        return MV_OK;
	    }
        pRule = pRule->next;
    }

    /* Allocate new entry */
    pNewRule = mvOsMalloc(sizeof(MV_FP_RULE));
    if(pNewRule == NULL)
    {
        mvOsPrintf("mvFpRuleSet: Can't allocate new rule\n");
        return MV_FAIL;
    }

    mvFpRuleCopy(pNewRule, pSetRule);
    pNewRule->next = NULL;

    if(ruleDb[hash_tr].ruleChain == NULL)
        ruleDb[hash_tr].ruleChain = pNewRule;
    else {
	    pRule = ruleDb[hash_tr].ruleChain;
        while (pRule->next != NULL)
	        pRule = pRule->next;	    
	    pRule->next = pNewRule;
    }
    nfpRuleSetCount++;

#ifdef MV_FP_DEBUG
    mvOsPrintf("SetNFP_%03u: DIP=%u.%u.%u.%u, SIP=%u.%u.%u.%u, hash=0x%04x, aware=0x%02x\n",
                nfpRuleSetCount, MV_IP_QUAD(pSetRule->routingInfo.dstIp), 
                MV_IP_QUAD(pSetRule->routingInfo.srcIp), 
                hash_tr, pSetRule->routingInfo.aware_flags);
#endif
    return MV_OK;
}

/* Delete a specified rule from the Routing + ARP information table */
MV_STATUS mvFpRuleDelete(MV_FP_RULE *rule)
{
    MV_U32      hash, hash_tr;
    MV_FP_RULE  *currRule, *prevRule;

    nfpRuleDeleteCount++;
    hash = mv_jhash_3words(rule->routingInfo.dstIp, rule->routingInfo.srcIp, 
                            (MV_U32)0, fp_ip_jhash_iv);
    hash_tr = hash & (ruleDbSize - 1);

    prevRule = NULL;
    for (currRule = ruleDb[hash_tr].ruleChain; 
	     currRule != NULL; 
	     prevRule = currRule, currRule = currRule->next) 
    {
	    if( (currRule->routingInfo.srcIp == rule->routingInfo.srcIp) && 
		    (currRule->routingInfo.dstIp == rule->routingInfo.dstIp) ) 
        {		
		    if (prevRule == NULL)
			    ruleDb[hash_tr].ruleChain = currRule->next;
		    else
			    prevRule->next = currRule->next;
#ifdef MV_FP_DEBUG
    		mvOsPrintf("DelNFP_%03u: DIP=%u.%u.%u.%u, SIP=%u.%u.%u.%u, hash=0x%04x\n",
              	        nfpRuleDeleteCount, MV_IP_QUAD(currRule->routingInfo.dstIp), 
                        MV_IP_QUAD(currRule->routingInfo.srcIp), hash_tr);
#endif	
		    mvOsFree(currRule);	
		    return MV_OK;
	    }
    }
    return MV_NOT_FOUND;
}

/* Find and return the first matching rule in the Routing + ARP information table */
static INLINE MV_FP_RULE* mvFpRuleFind(MV_U32 dstIp, MV_U32 srcIp)
{
    MV_U32      hash, hash_tr;
    MV_FP_RULE* pRule;
    int         count = 0;

    hash = mv_jhash_3words(dstIp, srcIp, (MV_U32)0, fp_ip_jhash_iv);
    hash_tr = hash & (ruleDbSize - 1);

    pRule = ruleDb[hash_tr].ruleChain;

    while(pRule)
    {
        /* look for a matching rule */
        if( (pRule->routingInfo.dstIp == dstIp) && 
            (pRule->routingInfo.srcIp == srcIp) )
        {
            FASTPATH_STAT( ip_hash_depth[count]++);
	        pRule->mgmtInfo.new_count++;
            return pRule;
        }
        pRule = pRule->next;
        count++;
    }
    return NULL;
}

int mvFpProcess(MV_U8* pData, MV_IP_HEADER* pIpHdr, MV_FP_STATS* pFpStats)
{
    MV_FP_RULE      *pFpRoute; 
    MV_U32          dip, sip;
#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    MV_U8           proto;
    MV_U16          srcPort, dstPort;
    MV_FP_NAT_RULE  *pDnatRule, *pSnatRule;
#endif
    MV_U8*	    pEthHeader = pData + ETH_MV_HEADER_SIZE;

    FASTPATH_STAT( pFpStats->process++ );

    /* Check MAC address: 
     *   WAN - non-promiscous mode. 
     *       Unicast packets - NFP, 
     *       Multicast, Broadcast - Linux 
     *   LAN - Promiscous mode. 
     *       LAN Unicast MAC - NFP, 
     *       Multicast, Broadcast, Unknown Unicast - Linux 
     */
    if(pEthHeader[0] & 0x1)
    {
        /* Go to Linux IP stack */   
        FASTPATH_STAT( pFpStats->multicast++);
        return -1;
    }

    /* Check TTL value */
    if(pIpHdr->ttl <= 1)
    {
        /* TTL expired */
        FASTPATH_STAT( pFpStats->ip_ttl_expired++ );
        return -1;
    }

    dip = pIpHdr->dstIP;
    sip = pIpHdr->srcIP;
#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    proto = mvFpNatPortsGet(pIpHdr, &dstPort, &srcPort);
    if(proto == MV_IP_PROTO_NULL)
    {
        /* NAT not supported for this protocol */
        FASTPATH_STAT( pFpStats->nat_bad_proto++ );
        pDnatRule = NULL;
    }
    else
    {
        /* Lookup NAT database accordingly with 5 tuple key */
        pDnatRule = mvFpNatRuleFind(dip, sip, proto, dstPort, srcPort);
    }
    if(pDnatRule != NULL) 
    {
        if(pDnatRule->flags & MV_FP_DIP_CMD_MAP)
        {
            dip = pDnatRule->newIp;
        }
        if(pDnatRule->flags & MV_FP_DPORT_CMD_MAP)
        {
            dstPort = pDnatRule->newPort;
        }
    }
    else
    {
        FASTPATH_STAT( pFpStats->dnat_not_found++ );
    }

#endif /* CONFIG_MV_ETH_FP_NAT_SUPPORT */

    pFpRoute = mvFpRuleFind(dip, sip);
    if(pFpRoute == NULL)
    {
        /* IP Routing rule is not found: go to Linux IP stack */
        FASTPATH_STAT( pFpStats->ip_not_found++ );
        return -1;
    }
    FASTPATH_STAT(pFpStats->ip_found++);

#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    if( (pDnatRule != NULL) && (pDnatRule->flags & MV_FP_DNAT_CMD_MAP) )
    {
        FASTPATH_STAT( pFpStats->dnat_found++ );
        pSnatRule = mvFpNatRuleFind(dip, sip, proto, dstPort, srcPort);
    }
    else
    {
        pSnatRule = pDnatRule;
    }
    
    if( (pSnatRule != NULL) && (pSnatRule->flags & MV_FP_SNAT_CMD_MAP) )
    {
        FASTPATH_STAT( pFpStats->snat_found++ );
    }
    else
    {
        FASTPATH_STAT( pFpStats->snat_not_found++ );
    }
    
    /* Check IP awareness */
    if( (pFpRoute->routingInfo.aware_flags & MV_FP_DIP_CMD_MAP) &&
        (pDnatRule == NULL) )
    {
        FASTPATH_STAT( pFpStats->dnat_aware++ );
        return -1;
    }

    if( (pFpRoute->routingInfo.aware_flags & MV_FP_SIP_CMD_MAP) &&
        (pSnatRule == NULL) )
    {
        FASTPATH_STAT( pFpStats->snat_aware++ );
        return -1;
    }

    /* Update packet accordingly with NAT rules */
    if( (pDnatRule != NULL) || (pSnatRule != NULL) )
    {
        mvFpNatPktUpdate(pIpHdr, pDnatRule, pSnatRule);
    }
#endif /* CONFIG_MV_ETH_FP_NAT_SUPPORT */

    if( (unsigned int)pEthHeader & 0x2)
    {
        *((MV_U16*)(pEthHeader)) = *(MV_U16*)(&pFpRoute->routingInfo.dstMac[0]);
        *((MV_U32*)(pEthHeader + 2)) = *(MV_U32*)(&pFpRoute->routingInfo.dstMac[2]);
        *((MV_U32*)(pEthHeader + 2 + 4)) = *(MV_U32*)(&pFpRoute->routingInfo.srcMac[0]);
        *((MV_U16*)(pEthHeader + 2 + 4 + 4)) = *(MV_U16*)(&pFpRoute->routingInfo.srcMac[4]);
    }   
    else
    {
        memcpy(pEthHeader, pFpRoute->routingInfo.dstMac, MV_MAC_ADDR_SIZE*2);
    }
    pIpHdr->ttl--;
#if defined(CONFIG_MV_GTW_HEADER_MODE)
    memcpy(pData, &(pFpRoute->routingInfo.pktHdr.mvHeader.header), ETH_MV_HEADER_SIZE);
#endif
    mvOsCacheLineFlushInv(pPortCtrl->osHandle, pData);	
    mvOsCacheLineFlushInv(pPortCtrl->osHandle, pData + CPU_D_CACHE_LINE_SIZE);
    
    return pFpRoute->routingInfo.outIfIndex;
}

void        mvFpStatsPrint(MV_FP_STATS *pFpStats)
{
#ifdef MV_FP_STATISTICS
    int         i = 0;

    mvOsPrintf( "\n====================================================\n" );
    mvOsPrintf( "fast path statistics");
    mvOsPrintf( "\n-------------------------------\n" );

    mvOsPrintf( "fast_path_parsing.............%10u\n", pFpStats->parsing );
    mvOsPrintf( "fast_path_process.............%10u\n", pFpStats->process );
    mvOsPrintf( "fast_path_multicast...........%10u\n", pFpStats->multicast );
    mvOsPrintf( "fast_path_non_ip..............%10u\n", pFpStats->non_ip );
    mvOsPrintf( "fast_path_vlan_tagged.........%10u\n", pFpStats->vlan_tagged );
    mvOsPrintf( "fast_path_ip_not_found........%10u\n", pFpStats->ip_not_found );
    mvOsPrintf( "fast_path_ttl_expired.........%10u\n", pFpStats->ip_ttl_expired );
    mvOsPrintf( "fast_path_ip_found............%10u\n", pFpStats->ip_found );

#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    mvOsPrintf( "fp_nat_bad_proto..............%10u\n", pFpStats->nat_bad_proto);
    mvOsPrintf( "fp_nat_dnat_found.............%10u\n", pFpStats->dnat_found);
    mvOsPrintf( "fp_nat_snat_found.............%10u\n", pFpStats->snat_found);
    mvOsPrintf( "fp_nat_dnat_not_found.........%10u\n", pFpStats->dnat_not_found);
    mvOsPrintf( "fp_nat_snat_not_found.........%10u\n", pFpStats->snat_not_found);
    mvOsPrintf( "fp_nat_dnat_aware.............%10u\n", pFpStats->dnat_aware);
    mvOsPrintf( "fp_nat_snat_aware.............%10u\n", pFpStats->snat_aware);
#endif /* CONFIG_MV_ETH_FP_NAT_SUPPORT */

    mvOsPrintf("\nHASH Depth IP stats\n");
    for(i=0; i<=ETH_FP_MAX_HASH_DEPTH; i++)
    { 
        if(ip_hash_depth[i] != 0)
        {
            mvOsPrintf("%d depth - %u times\n", i, ip_hash_depth[i]);
            ip_hash_depth[i] = 0;
        }
    } 

#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    mvOsPrintf("\nHASH Depth NAT stats\n");
    for(i=0; i<=ETH_FP_MAX_HASH_DEPTH; i++)
    { 
        if(nat_hash_depth[i] != 0)
        {
            mvOsPrintf("%d depth - %u times\n", i, nat_hash_depth[i]);
            nat_hash_depth[i] = 0;
        }
    } 
#endif /* CONFIG_MV_ETH_FP_NAT_SUPPORT */

    memset(pFpStats, 0, sizeof(MV_FP_STATS));
#endif /* MV_FP_STATISTICS */
    mvOsPrintf("\n");
    mvOsPrintf("nfpRuleSetCount=%u, nfpRuleUpdateCount=%u, nfpRuleDeleteCount=%u\n",
                nfpRuleSetCount, nfpRuleUpdateCount, nfpRuleDeleteCount);

#ifdef CONFIG_MV_ETH_FP_NAT_SUPPORT
    mvOsPrintf("\n");
    mvOsPrintf("natRuleSetCount=%u, natRuleUpdateCount=%u, natRuleDeleteCount=%u\n",
                natRuleSetCount, natRuleUpdateCount, natRuleDeleteCount);
#endif /* CONFIG_MV_ETH_FP_NAT_SUPPORT */
}

