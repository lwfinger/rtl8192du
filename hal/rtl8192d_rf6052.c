/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
/******************************************************************************
 *
 *
 * Module:	HalRf6052.c	( Source C File)
 *
 * Note:	Provide RF 6052 series relative API.
 *
 * Function:
 *
 * Export:
 *
 * Abbrev:
 *
 * History:
 * Data			Who		Remark
 *
 * 09/25/2008	MHC		Create initial version.
 * 11/05/2008	MHC		Add API for tw power setting.
 *
 *
******************************************************************************/

#define _RTL8192D_RF6052_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>
#include <rtl8192d_hal.h>

/*---------------------------Define Local Constant---------------------------*/
// Define local structure for debug!!!!!
struct rf_shadow_compare_map {
	// Shadow register value
	u32		Value;
	// Compare or not flag
	u8		Compare;
	// Record If it had ever modified unpredicted
	u8		ErrorOrNot;
	// Recorver Flag
	u8		Recorver;
	//
	u8		Driver_Write;
};
/*---------------------------Define Local Constant---------------------------*/


/*------------------------Define global variable-----------------------------*/
/*------------------------Define global variable-----------------------------*/


/*------------------------Define local variable------------------------------*/
// 2008/11/20 MH For Debug only, RF
static	struct rf_shadow_compare_map RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG];
/*------------------------Define local variable------------------------------*/


/*-----------------------------------------------------------------------------
 * Function:	RF_ChangeTxPath
 *
 * Overview:	For RL6052, we must change some RF settign for 1T or 2T.
 *
 * Input:		u2Byte DataRate		// 0x80-8f, 0x90-9f
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/25/2008	MHC		Create Version 0.
 *						Firmwaer support the utility later.
 *
 *---------------------------------------------------------------------------*/
void rtl8192d_RF_ChangeTxPath(	struct rtw_adapter *	Adapter,
										u16		DataRate)
{
// We do not support gain table change inACUT now !!!! Delete later !!!

}	/* RF_ChangeTxPath */


/*-----------------------------------------------------------------------------
 * Function:    PHY_RF6052SetBandwidth()
 *
 * Overview:    This function is called by SetBWModeCallback8190Pci() only
 *
 * Input:       struct rtw_adapter *				Adapter
 *			WIRELESS_BANDWIDTH_E	Bandwidth	//20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		For RF type 0222D
 *---------------------------------------------------------------------------*/
void
rtl8192d_PHY_RF6052SetBandwidth(
	struct rtw_adapter *				Adapter,
	enum HT_CHANNEL_WIDTH		Bandwidth)	//20M or 40M
{
	u8			eRFPath;
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);

	switch (Bandwidth)
	{
		case HT_CHANNEL_WIDTH_20:
			for (eRFPath=0;eRFPath<pHalData->NumTotalRFPath;eRFPath++)
			{
				pHalData->RfRegChnlVal[eRFPath] = ((pHalData->RfRegChnlVal[eRFPath] & 0xfffff3ff) | 0x0400);
				PHY_SetRFReg(Adapter, (enum RF_RADIO_PATH_E)eRFPath, RF_CHNLBW, BIT10|BIT11, 0x01);

			}
			break;

		case HT_CHANNEL_WIDTH_40:
			for (eRFPath=0;eRFPath<pHalData->NumTotalRFPath;eRFPath++)
			{
				pHalData->RfRegChnlVal[eRFPath] = ((pHalData->RfRegChnlVal[eRFPath] & 0xfffff3ff));
				//PHY_SetRFReg(Adapter, eRFPath, RF_CHNLBW, bRFRegOffsetMask, pHalData->RfRegChnlVal[eRFPath]);
				PHY_SetRFReg(Adapter, (enum RF_RADIO_PATH_E)eRFPath, RF_CHNLBW, BIT10|BIT11, 0x00);
				//RT_TRACE(COMP_RF, DBG_LOUD, ("PHY_RF6052SetBandwidth 40M RF 0x18 = 0x%x interface index %d\n",pHalData->RfRegChnlVal[eRFPath],  Adapter->interfaceIndex));
			}
			break;

		default:
			//RT_TRACE(COMP_DBG, DBG_LOUD, ("PHY_SetRF8225Bandwidth(): unknown Bandwidth: %#X\n",Bandwidth ));
			break;
	}
	//RT_TRACE(COMP_MLME,DBG_LOUD,("<==== PHY_RF6052SetBandwidth()Bandwidth:%d \n",Bandwidth));
}


/*-----------------------------------------------------------------------------
 * Function:	PHY_RF6052SetCckTxPower
 *
 * Overview:
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/05/2008	MHC		Simulate 8192series..
 *
 *---------------------------------------------------------------------------*/

void
rtl8192d_PHY_RF6052SetCckTxPower(
	struct rtw_adapter *		Adapter,
	u8*			pPowerlevel)
{
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
	u32			TxAGC[2]={0, 0}, tmpval=0;
	bool		TurboScanOff = false;
	u8			idx1, idx2;
	u8*			ptr;

	if (pHalData->EEPROMRegulatory != 0)
		TurboScanOff = true;

	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS)
	{
		TxAGC[RF_PATH_A] = 0x3f3f3f3f;
		TxAGC[RF_PATH_B] = 0x3f3f3f3f;

		TurboScanOff =  true;//disable Turbo scan

		if (TurboScanOff)
		{
			for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
			}
		}
	}
	else
	{
//vivi merge from 92c, pass win7 DTM item: performance_ext
// 20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism.
// Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism.
// In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder.
		{
			for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
			}

			if (pHalData->EEPROMRegulatory==0)
			{
				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][6]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][7]<<8);
				TxAGC[RF_PATH_A] += tmpval;

				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][14]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][15]<<24);
				TxAGC[RF_PATH_B] += tmpval;
			}
		}
	}

	for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
	{
		ptr = (u8 *)(&(TxAGC[idx1]));
		for (idx2=0; idx2<4; idx2++)
		{
			if (*ptr > RF6052_MAX_TX_PWR)
				*ptr = RF6052_MAX_TX_PWR;
			ptr++;
		}
	}

	// rf-A cck tx power
	tmpval = TxAGC[RF_PATH_A]&0xff;
	PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, bMaskByte1, tmpval);
	//RTPRINT(FPHY, PHY_TXPWR, ("CCK PWR 1M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_A_CCK1_Mcs32));
	tmpval = TxAGC[RF_PATH_A]>>8;
	PHY_SetBBReg(Adapter, rTxAGC_B_CCK11_A_CCK2_11, 0xffffff00, tmpval);
	//RTPRINT(FPHY, PHY_TXPWR, ("CCK PWR 2~11M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11));

	// rf-B cck tx power
	tmpval = TxAGC[RF_PATH_B]>>24;
	PHY_SetBBReg(Adapter, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte0, tmpval);
	//RTPRINT(FPHY, PHY_TXPWR, ("CCK PWR 11M (rf-B) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11));
	tmpval = TxAGC[RF_PATH_B]&0x00ffffff;
	PHY_SetBBReg(Adapter, rTxAGC_B_CCK1_55_Mcs32, 0xffffff00, tmpval);
	//RTPRINT(FPHY, PHY_TXPWR, ("CCK PWR 1~5.5M (rf-B) = 0x%x (reg 0x%x)\n",
	//	tmpval, rTxAGC_B_CCK1_55_Mcs32));

}	/* PHY_RF6052SetCckTxPower */

//
// powerbase0 for OFDM rates
// powerbase1 for HT MCS rates
//
static void getPowerBase(
	struct rtw_adapter *	Adapter,
	u8		*pPowerLevel,
	u8		Channel,
	u32	*OfdmBase,
	u32	*MCSBase
	)
{
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);
	u32	powerBase0, powerBase1;
	u8	Legacy_pwrdiff=0;
	s8	HT20_pwrdiff=0;
	u8	i, powerlevel[2];

	for (i=0; i<2; i++)
	{
		powerlevel[i] = pPowerLevel[i];
		Legacy_pwrdiff = pHalData->TxPwrLegacyHtDiff[i][Channel-1];
		powerBase0 = powerlevel[i] + Legacy_pwrdiff;

		powerBase0 = (powerBase0<<24) | (powerBase0<<16) |(powerBase0<<8) |powerBase0;
		*(OfdmBase+i) = powerBase0;
		//RTPRINT(FPHY, PHY_TXPWR, (" [OFDM power base index rf(%c) = 0x%x]\n", ((i==0)?'A':'B'), *(OfdmBase+i)));
	}

	for (i=0; i<2; i++)
	{
		//Check HT20 to HT40 diff
		if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
		{
			HT20_pwrdiff = pHalData->TxPwrHt20Diff[i][Channel-1];
			powerlevel[i] += HT20_pwrdiff;
		}
		powerBase1 = powerlevel[i];
		powerBase1 = (powerBase1<<24) | (powerBase1<<16) |(powerBase1<<8) |powerBase1;
		*(MCSBase+i) = powerBase1;
		//RTPRINT(FPHY, PHY_TXPWR, (" [MCS power base index rf(%c) = 0x%x]\n", ((i==0)?'A':'B'), *(MCSBase+i)));
	}
}

/*
Decide the group value according to the PHY_REG_PG.txt with 2G band and 5G band.
*/
static u8 getChnlGroupByPG(u8 chnlindex)
{
	u8	group=0;
	u8	channel_info[59] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,149,151,153,155,157,159,161,163,165};

	if (channel_info[chnlindex] <= 3)			// Cjanel 1-3
		group = 0;
	else if (channel_info[chnlindex] <= 9)		// Channel 4-9
		group = 1;
	else if (channel_info[chnlindex] <=14)				// Channel 10-14
		group = 2;
	else if (channel_info[chnlindex] <= 64)
		group = 6;
	else if (channel_info[chnlindex] <= 140)
		group = 7;
	else
		group = 8;

	return group;
}

static void getTxPowerWriteValByRegulatory(
		struct rtw_adapter *	Adapter,
		u8*			pPowerLevel,
		u8			Channel,
		u8			index,
		u32*		powerBase0,
		u32*		powerBase1,
		u32*		pOutWriteVal
	)
{
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	u8	i, chnlGroup=0, pwr_diff_limit[4], customer_pwr_limit;
	s8	pwr_diff=0;
	u32	writeVal, customer_limit, rf;

	//
	// Index 0 & 1= legacy OFDM, 2-5=HT_MCS rate
	//
	for (rf=0; rf<2; rf++)
	{
		switch (pHalData->EEPROMRegulatory)
		{
			case 0:	// Realtek better performance
					// increase power diff defined by Realtek for large power
				chnlGroup = 0;
				//RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%lx\n",
				//	chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]));
				writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
					((index<2)?powerBase0[rf]:powerBase1[rf]);
				//RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal(%c) = 0x%lx\n", ((rf==0)?'A':'B'), writeVal));
				break;
			case 1:	// Realtek regulatory
					// increase power diff defined by Realtek for regulatory
					if (pHalData->pwrGroupCnt == 1)
						chnlGroup = 0;
					if (pHalData->pwrGroupCnt >= MAX_PG_GROUP)
					{
						chnlGroup = getChnlGroupByPG(Channel-1);

						if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
							chnlGroup++;
						else	   // 40M BW
							chnlGroup += 4;
					}
					//RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%lx\n",
					//chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]));
					writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
							((index<2)?powerBase0[rf]:powerBase1[rf]);
					//RTPRINT(FPHY, PHY_TXPWR, ("Realtek regulatory, 20MHz, writeVal(%c) = 0x%lx\n", ((rf==0)?'A':'B'), writeVal));

				break;
			case 2:	// Better regulatory
					// don't increase any power diff
				writeVal = ((index<2)?powerBase0[rf]:powerBase1[rf]);
				//RTPRINT(FPHY, PHY_TXPWR, ("Better regulatory, writeVal(%c) = 0x%lx\n", ((rf==0)?'A':'B'), writeVal));
				break;
			case 3:	// Customer defined power diff.
					// increase power diff defined by customer.
				chnlGroup = 0;
				//RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%lx\n",
				//	chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]));

				//if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_40)
				//{
					//RTPRINT(FPHY, PHY_TXPWR, ("customer's limit, 40MHz rf(%c) = 0x%x\n",
					//	((rf==0)?'A':'B'), pHalData->PwrGroupHT40[rf][Channel-1]));
				//}
				//else
				//{
					//RTPRINT(FPHY, PHY_TXPWR, ("customer's limit, 20MHz rf(%c) = 0x%x\n",
					//	((rf==0)?'A':'B'), pHalData->PwrGroupHT20[rf][Channel-1]));
				//}

				if (index < 2)
					pwr_diff = pHalData->TxPwrLegacyHtDiff[rf][Channel-1];
				else if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
					pwr_diff = pHalData->TxPwrHt20Diff[rf][Channel-1];

				//RTPRINT(FPHY, PHY_TXPWR, ("power diff rf(%c) = 0x%x\n", ((rf==0)?'A':'B'), pwr_diff));

				if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_40)
					customer_pwr_limit = pHalData->PwrGroupHT40[rf][Channel-1];
				else
					customer_pwr_limit = pHalData->PwrGroupHT20[rf][Channel-1];

				//RTPRINT(FPHY, PHY_TXPWR, ("customer pwr limit  rf(%c) = 0x%x\n", ((rf==0)?'A':'B'), customer_pwr_limit));

				if (pwr_diff >= customer_pwr_limit)
					pwr_diff = 0;
				else
					pwr_diff = customer_pwr_limit - pwr_diff;

				for (i=0; i<4; i++)
				{
					pwr_diff_limit[i] = (u8)((pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]&(0x7f<<(i*8)))>>(i*8));

					if (pwr_diff_limit[i] > pwr_diff)
						pwr_diff_limit[i] = pwr_diff;
				}

				customer_limit = (pwr_diff_limit[3]<<24) | (pwr_diff_limit[2]<<16) |
								(pwr_diff_limit[1]<<8) | (pwr_diff_limit[0]);
				//RTPRINT(FPHY, PHY_TXPWR, ("Customer's limit rf(%c) = 0x%lx\n", ((rf==0)?'A':'B'), customer_limit));

				writeVal = customer_limit + ((index<2)?powerBase0[rf]:powerBase1[rf]);
				//RTPRINT(FPHY, PHY_TXPWR, ("Customer, writeVal rf(%c)= 0x%lx\n", ((rf==0)?'A':'B'), writeVal));
				break;
			default:
				chnlGroup = 0;
				writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
						((index<2)?powerBase0[rf]:powerBase1[rf]);
				//RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal rf(%c) = 0x%lx\n", ((rf==0)?'A':'B'), writeVal));
				break;
		}

		// 20100628 Joseph: High power mode for BT-Coexist mechanism.
		// This mechanism is only applied when Driver-Highpower-Mechanism is OFF.
		if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_BT1)
		{
			//RTPRINT(FBT, BT_TRACE, ("Tx Power (-6)\n"));
			writeVal = writeVal - 0x06060606;
		}
		else if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_BT2)
		{
			//RTPRINT(FBT, BT_TRACE, ("Tx Power (-0)\n"));
			writeVal = writeVal ;
		}

		//if (pMgntInfo->bDisableTXPowerByRate)
		//{
			// add for  OID_RT_11N_TX_POWER_BY_RATE ,disable tx powre change by rate
		//	writeVal = 0x2c2c2c2c;
		//}
		*(pOutWriteVal+rf) = writeVal;
	}
}

static void writeOFDMPowerReg(
		struct rtw_adapter *	Adapter,
		u8			index,
		u32			*pValue
	)
{
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);
	u16	RegOffset_A[6] = {rTxAGC_A_Rate18_06, rTxAGC_A_Rate54_24,
				  rTxAGC_A_Mcs03_Mcs00, rTxAGC_A_Mcs07_Mcs04,
				  rTxAGC_A_Mcs11_Mcs08, rTxAGC_A_Mcs15_Mcs12};
	u16	RegOffset_B[6] = {rTxAGC_B_Rate18_06, rTxAGC_B_Rate54_24,
				  rTxAGC_B_Mcs03_Mcs00, rTxAGC_B_Mcs07_Mcs04,
				  rTxAGC_B_Mcs11_Mcs08, rTxAGC_B_Mcs15_Mcs12};
	u8	i, rf, pwr_val[4];
	u32	writeVal;
	u16	RegOffset;

	for (rf=0; rf<2; rf++)
	{
		writeVal = pValue[rf];
		for (i=0; i<4; i++)
		{
			pwr_val[i] = (u8)((writeVal & (0x7f<<(i*8)))>>(i*8));
			if (pwr_val[i]  > RF6052_MAX_TX_PWR)
				pwr_val[i]  = RF6052_MAX_TX_PWR;
		}
		writeVal = (pwr_val[3]<<24) | (pwr_val[2]<<16) |(pwr_val[1]<<8) |pwr_val[0];

		if (rf == 0)
			RegOffset = RegOffset_A[index];
		else
			RegOffset = RegOffset_B[index];
		PHY_SetBBReg(Adapter, RegOffset, bMaskDWord, writeVal);
		//RTPRINT(FPHY, PHY_TXPWR, ("Set 0x%x = %08x\n", RegOffset, writeVal));

		// 201005115 Joseph: Set Tx Power diff for Tx power training mechanism.
		if (((pHalData->rf_type == RF_2T2R) &&
				(RegOffset == rTxAGC_A_Mcs15_Mcs12 || RegOffset == rTxAGC_B_Mcs15_Mcs12))||
		     ((pHalData->rf_type != RF_2T2R) &&
				(RegOffset == rTxAGC_A_Mcs07_Mcs04 || RegOffset == rTxAGC_B_Mcs07_Mcs04))	)
		{
			writeVal = pwr_val[3];
			if (RegOffset == rTxAGC_A_Mcs15_Mcs12 || RegOffset == rTxAGC_A_Mcs07_Mcs04)
				RegOffset = 0xc90;
			if (RegOffset == rTxAGC_B_Mcs15_Mcs12 || RegOffset == rTxAGC_B_Mcs07_Mcs04)
				RegOffset = 0xc98;
			for (i=0; i<3; i++)
			{
				if (i!=2)
					writeVal = (writeVal>8)?(writeVal-8):0;
				else
					writeVal = (writeVal>6)?(writeVal-6):0;
				rtw_write8(Adapter, (u32)(RegOffset+i), (u8)writeVal);
			}
		}
	}
}
/*-----------------------------------------------------------------------------
 * Function:	PHY_RF6052SetOFDMTxPower
 *
 * Overview:	For legacy and HY OFDM, we must read EEPROM TX power index for
 *			different channel and read original value in TX power register area from
 *			0xe00. We increase offset and original value to be correct tx pwr.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/05/2008	MHC		Simulate 8192 series method.
 * 01/06/2009	MHC		1. Prevent Path B tx power overflow or underflow dure to
 *						A/B pwr difference or legacy/HT pwr diff.
 *						2. We concern with path B legacy/HT OFDM difference.
 * 01/22/2009	MHC		Support new EPRO format from SD3.
 *
 *---------------------------------------------------------------------------*/
void
rtl8192d_PHY_RF6052SetOFDMTxPower(
	struct rtw_adapter *	Adapter,
	u8*		pPowerLevel,
	u8		Channel)
{
	u32	writeVal[2], powerBase0[2], powerBase1[2];
	u8	index = 0;

	//DBG_871X("PHY_RF6052SetOFDMTxPower, channel(%d) \n", Channel);

	getPowerBase(Adapter, pPowerLevel, Channel, &powerBase0[0], &powerBase1[0]);

	for (index=0; index<6; index++)
	{
		getTxPowerWriteValByRegulatory(Adapter, pPowerLevel, Channel, index,
			&powerBase0[0], &powerBase1[0], &writeVal[0]);

		writeOFDMPowerReg(Adapter, index, &writeVal[0]);
	}
}

bool
rtl8192d_PHY_EnableAnotherPHY(
	struct rtw_adapter *		Adapter,
	bool			bMac0
	)
{
	u8			u1bTmp;
	u8			MAC_REG = bMac0==true?REG_MAC1:REG_MAC0;
	u8			MAC_ON_BIT = bMac0==true?MAC1_ON:MAC0_ON;
	bool		bResult = true;	//true: need to enable BB/RF power
	u32			MaskForPHYSet = 0;

	//MAC0 Need PHY1 load radio_b.txt . Driver use DBI to write.
	u1bTmp = rtw_read8(Adapter, MAC_REG);

	if (!(u1bTmp&MAC_ON_BIT))
	{
		// Enable BB and RF power
		if (bMac0)
			MaskForPHYSet = MAC0_ACCESS_PHY1;
		else
			MaskForPHYSet = MAC1_ACCESS_PHY0;
		rtw_write16(Adapter, REG_SYS_FUNC_EN|MaskForPHYSet, rtw_read16(Adapter, REG_SYS_FUNC_EN|MaskForPHYSet)&0xFFFC);
		rtw_write16(Adapter, REG_SYS_FUNC_EN|MaskForPHYSet, rtw_read16(Adapter, REG_SYS_FUNC_EN|MaskForPHYSet)|BIT13|BIT0|BIT1);
	} else {
		// We think if MAC1 is ON,then radio_a.txt and radio_b.txt has been load.
		bResult = false;
	}
	return bResult;
}

void
rtl8192d_PHY_PowerDownAnotherPHY(
	struct rtw_adapter *		Adapter,
	bool			bMac0
	)
{
	u8	u1bTmp;
	u8	MAC_REG = bMac0==true?REG_MAC1:REG_MAC0;
	u8	MAC_ON_BIT = bMac0==true?MAC1_ON:MAC0_ON;
	u32	MaskforPhySet = 0;

	// check MAC0 enable or not again now, if enabled, not power down radio A.
	u1bTmp = rtw_read8(Adapter, MAC_REG);

	if (!(u1bTmp&MAC_ON_BIT)) {
		// power down RF radio A according to YuNan's advice.
		if (bMac0)
			MaskforPhySet = MAC0_ACCESS_PHY1;
		else
			MaskforPhySet = MAC1_ACCESS_PHY0;
		  rtw_write32(Adapter, rFPGA0_XA_LSSIParameter|MaskforPhySet, 0x00000000);
	}

	//RT_TRACE(COMP_RF, DBG_LOUD, ("<====PHY_PowerDownAnotherPHY\n"));
}

static int
phy_RF6052_Config_ParaFile(
	struct rtw_adapter *		Adapter
	)
{
	u32	u4RegValue=0;
	u8	eRFPath;
	struct bb_register_def *pPhyReg;
	int	rtStatus = _SUCCESS;
	struct hal_data_8192du	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	u8	*pszRadioAFile = NULL, *pszRadioBFile = NULL;
	static s8		sz92DRadioAFile[] = RTL8192D_PHY_RADIO_A;
	static s8		sz92DRadioBFile[] = RTL8192D_PHY_RADIO_B;
	static s8		sz92DRadioAintPAFile[] = RTL8192D_PHY_RADIO_A_intPA;
	static s8		sz92DRadioBintPAFile[] = RTL8192D_PHY_RADIO_B_intPA;
	bool		bMac1NeedInitRadioAFirst = false,bMac0NeedInitRadioBFirst = false;
	bool		bNeedPowerDownRadioA = false,bNeedPowerDownRadioB = false;
	bool		bTrueBPath = false;//vivi added this for read parameter from header, 20100908
	u32	MaskforPhySet = 0; //For 92d PHY cross access, 88c must set value 0.


	pszRadioAFile = sz92DRadioAFile;
	pszRadioBFile = sz92DRadioBFile;

	if (pHalData->InternalPA5G[0])
		pszRadioAFile = sz92DRadioAintPAFile;
	if (pHalData->InternalPA5G[1])
		pszRadioBFile = sz92DRadioBintPAFile;

	//DMDP,MAC0 on G band,MAC1 on A band.
	if (pHalData->MacPhyMode92D==DUALMAC_DUALPHY)
	{
		if (pHalData->CurrentBandType92D == BAND_ON_2_4G && pHalData->interfaceIndex == 0)
		{
			//MAC0 Need PHY1 load radio_b.txt . Driver use DBI to write.
			if (rtl8192d_PHY_EnableAnotherPHY(Adapter, true))
			{
				pHalData->NumTotalRFPath = 2;
				bMac0NeedInitRadioBFirst = true;
			}
			else
			{
				// We think if MAC1 is ON,then radio_a.txt and radio_b.txt has been load.
				return rtStatus;
			}
		}
		else if (pHalData->CurrentBandType92D == BAND_ON_5G && pHalData->interfaceIndex == 1)
		{
			//MAC1 Need PHY0 load radio_a.txt . Driver use DBI to write.
			if (rtl8192d_PHY_EnableAnotherPHY(Adapter, false))
			{
				pHalData->NumTotalRFPath = 2;
				bMac1NeedInitRadioAFirst = true;
			}
			else
			{
				// We think if MAC0 is ON,then radio_a.txt and radio_b.txt has been load.
				return rtStatus;
			}
		}
		else if (pHalData->interfaceIndex == 1)
		{
			// MAC0 enabled, only init radia B.
			pszRadioAFile = pszRadioBFile;
			bTrueBPath = true;  //vivi added this for read parameter from header, 20100909
		}
	}

	//RT_TRACE(COMP_INIT, DBG_LOUD, (" ===> phy_RF6052_Config_ParaFile() Radio_A:%s\n",pszRadioAFile));
	//RT_TRACE(COMP_INIT, DBG_LOUD, (" ===> phy_RF6052_Config_ParaFile() Radio_B:%s\n",pszRadioBFile));

	//3//-----------------------------------------------------------------
	//3// <2> Initialize RF
	//3//-----------------------------------------------------------------
	for (eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{
		if (bMac1NeedInitRadioAFirst) //Mac1 use PHY0 write
		{
			if (eRFPath == RF_PATH_A)
			{
				bNeedPowerDownRadioA = true;
				MaskforPhySet = MAC1_ACCESS_PHY0;
			}
			else if (eRFPath == RF_PATH_B)
			{
				MaskforPhySet = 0;
				bMac1NeedInitRadioAFirst = false;
				eRFPath = RF_PATH_A;
				bTrueBPath = true;
				pszRadioAFile = pszRadioBFile;
				pHalData->NumTotalRFPath = 1;
			}
		}
		else  if (bMac0NeedInitRadioBFirst) //Mac0 use PHY1 write
		{
			if (eRFPath == RF_PATH_A)
			{
				MaskforPhySet = 0;
			}

			if (eRFPath == RF_PATH_B)
			{
				MaskforPhySet = MAC0_ACCESS_PHY1;
				bMac0NeedInitRadioBFirst = false;
				bNeedPowerDownRadioB = true;
				eRFPath = RF_PATH_A;
				bTrueBPath = true;
				pszRadioAFile = pszRadioBFile;
				pHalData->NumTotalRFPath = 1;
			}
		}

		pPhyReg = &pHalData->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath)
		{
			case RF_PATH_A:
			case RF_PATH_C:
				u4RegValue = PHY_QueryBBReg(Adapter, pPhyReg->rfintfs|MaskforPhySet, bRFSI_RFENV);
				break;
			case RF_PATH_B :
			case RF_PATH_D:
				u4RegValue = PHY_QueryBBReg(Adapter, pPhyReg->rfintfs|MaskforPhySet, bRFSI_RFENV<<16);
				break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(Adapter, pPhyReg->rfintfe|MaskforPhySet, bRFSI_RFENV<<16, 0x1);
		rtw_udelay_os(1);//PlatformStallExecution(1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(Adapter, pPhyReg->rfintfo|MaskforPhySet, bRFSI_RFENV, 0x1);
		rtw_udelay_os(1);//PlatformStallExecution(1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(Adapter, pPhyReg->rfHSSIPara2|MaskforPhySet, b3WireAddressLength, 0x0);	// Set 1 to 4 bits for 8255
		rtw_udelay_os(1);//PlatformStallExecution(1);

		PHY_SetBBReg(Adapter, pPhyReg->rfHSSIPara2|MaskforPhySet, b3WireDataLength, 0x0);	// Set 0 to 12  bits for 8255
		rtw_udelay_os(1);//PlatformStallExecution(1);

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath)
		{
			case RF_PATH_A:
#ifdef CONFIG_EMBEDDED_FWIMG
				//vivi added this for read parameter from header, 20100908
				if (bTrueBPath == true)
					rtStatus = rtl8192d_PHY_ConfigRFWithHeaderFile(Adapter,radiob_txt|MaskforPhySet, (enum RF_RADIO_PATH_E)eRFPath);
				else
					rtStatus = rtl8192d_PHY_ConfigRFWithHeaderFile(Adapter,radioa_txt|MaskforPhySet, (enum RF_RADIO_PATH_E)eRFPath);
#else
				rtStatus = rtl8192d_PHY_ConfigRFWithParaFile(Adapter, pszRadioAFile, (enum RF_RADIO_PATH_E)eRFPath);
#endif
				break;
			case RF_PATH_B:
#ifdef CONFIG_EMBEDDED_FWIMG
			rtStatus = rtl8192d_PHY_ConfigRFWithHeaderFile(Adapter,radiob_txt, (enum RF_RADIO_PATH_E)eRFPath);
#else
			rtStatus = rtl8192d_PHY_ConfigRFWithParaFile(Adapter, pszRadioBFile, (enum RF_RADIO_PATH_E)eRFPath);
#endif
				break;
			case RF_PATH_C:
				break;
			case RF_PATH_D:
				break;
		}

		/*----Restore RFENV control type----*/;
		switch (eRFPath)
		{
			case RF_PATH_A:
			case RF_PATH_C:
				PHY_SetBBReg(Adapter, pPhyReg->rfintfs|MaskforPhySet, bRFSI_RFENV, u4RegValue);
				break;
			case RF_PATH_B :
			case RF_PATH_D:
				PHY_SetBBReg(Adapter, pPhyReg->rfintfs|MaskforPhySet, bRFSI_RFENV<<16, u4RegValue);
				break;
		}

		if (rtStatus != _SUCCESS){
			//RT_TRACE(COMP_FPGA, DBG_LOUD, ("phy_RF6052_Config_ParaFile():Radio[%d] Fail!!", eRFPath));
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}

	if (bNeedPowerDownRadioA)
	{
		// check MAC0 enable or not again now, if enabled, not power down radio A.
		rtl8192d_PHY_PowerDownAnotherPHY(Adapter, false);
	}
	else  if (bNeedPowerDownRadioB)
	{
		// check MAC1 enable or not again now, if enabled, not power down radio B.
		rtl8192d_PHY_PowerDownAnotherPHY(Adapter, true);
	}

	for (eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{
#if MP_DRIVER == 1
		PHY_SetRFReg(Adapter, eRFPath, RF_RXRF_A3, bRFRegOffsetMask, 0xff456);
#endif
		pdmpriv->RegRF3C[eRFPath] = PHY_QueryRFReg(Adapter, eRFPath, RF_RXRF_A3, bRFRegOffsetMask);
	}

	//RT_TRACE(COMP_INIT, DBG_LOUD, ("<---phy_RF6052_Config_ParaFile()\n"));
	return rtStatus;

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}

int
PHY_RF6052_Config8192D(
	struct rtw_adapter *		Adapter)
{
	struct hal_data_8192du *pHalData = GET_HAL_DATA(Adapter);
	int rtStatus = _SUCCESS;

	//
	// Initialize general global value
	//
	// TODO: Extend RF_PATH_C and RF_PATH_D in the future
	if (pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

#ifdef CONFIG_DUALMAC_CONCURRENT
	if (pHalData->bSlaveOfDMSP)
	{
		DBG_871X(("PHY_RF6052_Config() skip configuration RF \n"));
		return rtStatus;
	}
#endif

	//
	// Config BB and RF
	//
	rtStatus = phy_RF6052_Config_ParaFile(Adapter);

	return rtStatus;

}


//
// ==> RF shadow Operation API Code Section!!!
//
/*-----------------------------------------------------------------------------
 * Function:	PHY_RFShadowRead
 *				PHY_RFShadowWrite
 *				PHY_RFShadowCompare
 *				PHY_RFShadowRecorver
 *				PHY_RFShadowCompareAll
 *				PHY_RFShadowRecorverAll
 *				PHY_RFShadowCompareFlagSet
 *				PHY_RFShadowRecorverFlagSet
 *
 * Overview:	When we set RF register, we must write shadow at first.
 *			When we are running, we must compare shadow abd locate error addr.
 *			Decide to recorver or not.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/20/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/

u32
PHY_RFShadowRead(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset)
{
	return	RF_Shadow[eRFPath][Offset].Value;

}	/* PHY_RFShadowRead */


void
PHY_RFShadowWrite(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset,
	u32				Data)
{
	RF_Shadow[eRFPath][Offset].Value = (Data & bRFRegOffsetMask);
	RF_Shadow[eRFPath][Offset].Driver_Write = true;

}	/* PHY_RFShadowWrite */

bool
PHY_RFShadowCompare(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset);
bool
PHY_RFShadowCompare(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset)
{
	u32	reg;
	// Check if we need to check the register
	if (RF_Shadow[eRFPath][Offset].Compare == true)
	{
		reg = PHY_QueryRFReg(Adapter, eRFPath, Offset, bRFRegOffsetMask);
		// Compare shadow and real rf register for 20bits!!
		if (RF_Shadow[eRFPath][Offset].Value != reg)
		{
			// Locate error position.
			RF_Shadow[eRFPath][Offset].ErrorOrNot = true;
			//RT_TRACE(COMP_INIT, DBG_LOUD,
			//("PHY_RFShadowCompare RF-%d Addr%02lx Err = %05lx\n",
			//eRFPath, Offset, reg));
		}
		return RF_Shadow[eRFPath][Offset].ErrorOrNot ;
	}
	return false;
}	/* PHY_RFShadowCompare */
void
PHY_RFShadowRecorver(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset);
void
PHY_RFShadowRecorver(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset)
{
	// Check if the address is error
	if (RF_Shadow[eRFPath][Offset].ErrorOrNot == true)
	{
		// Check if we need to recorver the register.
		if (RF_Shadow[eRFPath][Offset].Recorver == true)
		{
			PHY_SetRFReg(Adapter, eRFPath, Offset, bRFRegOffsetMask,
							RF_Shadow[eRFPath][Offset].Value);
			//RT_TRACE(COMP_INIT, DBG_LOUD,
			//("PHY_RFShadowRecorver RF-%d Addr%02lx=%05lx",
			//eRFPath, Offset, RF_Shadow[eRFPath][Offset].Value));
		}
	}

}	/* PHY_RFShadowRecorver */
void
PHY_RFShadowCompareAll(
	struct rtw_adapter *			Adapter);
void
PHY_RFShadowCompareAll(
	struct rtw_adapter *			Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			PHY_RFShadowCompare(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset);
		}
	}

}	/* PHY_RFShadowCompareAll */
void
PHY_RFShadowRecorverAll(
	struct rtw_adapter *			Adapter);
void
PHY_RFShadowRecorverAll(
	struct rtw_adapter *			Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			PHY_RFShadowRecorver(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset);
		}
	}

}	/* PHY_RFShadowRecorverAll */
void
PHY_RFShadowCompareFlagSet(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset,
	u8				Type);
void
PHY_RFShadowCompareFlagSet(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset,
	u8				Type)
{
	// Set True or False!!!
	RF_Shadow[eRFPath][Offset].Compare = Type;

}	/* PHY_RFShadowCompareFlagSet */
void
PHY_RFShadowRecorverFlagSet(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset,
	u8				Type);
void
PHY_RFShadowRecorverFlagSet(
	struct rtw_adapter *			Adapter,
	enum RF_RADIO_PATH_E	eRFPath,
	u32				Offset,
	u8				Type)
{
	// Set True or False!!!
	RF_Shadow[eRFPath][Offset].Recorver= Type;

}	/* PHY_RFShadowRecorverFlagSet */
void
PHY_RFShadowCompareFlagSetAll(
	struct rtw_adapter *			Adapter);
void
PHY_RFShadowCompareFlagSetAll(
	struct rtw_adapter *			Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			// 2008/11/20 MH For S3S4 test, we only check reg 26/27 now!!!!
			if (Offset != 0x26 && Offset != 0x27)
				PHY_RFShadowCompareFlagSet(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset, false);
			else
				PHY_RFShadowCompareFlagSet(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset, true);
		}
	}

}	/* PHY_RFShadowCompareFlagSetAll */
void
PHY_RFShadowRecorverFlagSetAll(
	struct rtw_adapter *			Adapter);
void
PHY_RFShadowRecorverFlagSetAll(
	struct rtw_adapter *			Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			// 2008/11/20 MH For S3S4 test, we only check reg 26/27 now!!!!
			if (Offset != 0x26 && Offset != 0x27)
				PHY_RFShadowRecorverFlagSet(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset, false);
			else
				PHY_RFShadowRecorverFlagSet(Adapter, (enum RF_RADIO_PATH_E)eRFPath, Offset, true);
		}
	}

}	/* PHY_RFShadowCompareFlagSetAll */
void
PHY_RFShadowRefresh(
	struct rtw_adapter *			Adapter);
void
PHY_RFShadowRefresh(
	struct rtw_adapter *			Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			RF_Shadow[eRFPath][Offset].Value = 0;
			RF_Shadow[eRFPath][Offset].Compare = false;
			RF_Shadow[eRFPath][Offset].Recorver  = false;
			RF_Shadow[eRFPath][Offset].ErrorOrNot = false;
			RF_Shadow[eRFPath][Offset].Driver_Write = false;
		}
	}

}	/* PHY_RFShadowRead */

/* End of HalRf6052.c */
