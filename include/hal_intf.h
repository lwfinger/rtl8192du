/******************************************************************************
 *
 *Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 *This program is free software; you can redistribute it and/or modify it
 *under the terms of version 2 of the GNU General Public License as
 *published by the Free Software Foundation.
 *
 *This program is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *more details.
 *
 *
 ******************************************************************************/
#ifndef __HAL_INTF_H__
#define __HAL_INTF_H__

#include <autoconf.h>
#include <osdep_service.h>
#include <drv_types.h>

/* CCK */
#define RATE_1M						BIT(0)
#define RATE_2M						BIT(1)
#define RATE_5_5M					BIT(2)
#define RATE_11M					BIT(3)
/* OFDM */
#define RATE_6M						BIT(4)
#define RATE_9M						BIT(5)
#define RATE_12M					BIT(6)
#define RATE_18M					BIT(7)
#define RATE_24M					BIT(8)
#define RATE_36M					BIT(9)
#define RATE_48M					BIT(10)
#define RATE_54M					BIT(11)
/* MCS 1 Spatial Stream */
#define RATE_MCS0					BIT(12)
#define RATE_MCS1					BIT(13)
#define RATE_MCS2					BIT(14)
#define RATE_MCS3					BIT(15)
#define RATE_MCS4					BIT(16)
#define RATE_MCS5					BIT(17)
#define RATE_MCS6					BIT(18)
#define RATE_MCS7					BIT(19)
/* MCS 2 Spatial Stream */
#define RATE_MCS8					BIT(20)
#define RATE_MCS9					BIT(21)
#define RATE_MCS10					BIT(22)
#define RATE_MCS11					BIT(23)
#define RATE_MCS12					BIT(24)
#define RATE_MCS13					BIT(25)
#define RATE_MCS14					BIT(26)
#define RATE_MCS15					BIT(27)

/*  ALL CCK Rate */
#define	RATE_ALL_CCK		(RATR_1M|RATR_2M|RATR_55M|RATR_11M)
#define	RATE_ALL_OFDM_AG	(RATR_6M|RATR_9M|RATR_12M|RATR_18M|RATR_24M|\
				RATR_36M|RATR_48M|RATR_54M)
#define	RATE_ALL_OFDM_1SS	(RATR_MCS0|RATR_MCS1|RATR_MCS2|RATR_MCS3 |\
				RATR_MCS4|RATR_MCS5|RATR_MCS6|RATR_MCS7)
#define	RATE_ALL_OFDM_2SS	(RATR_MCS8|RATR_MCS9|RATR_MCS10|RATR_MCS11|\
				RATR_MCS12|RATR_MCS13|RATR_MCS14|RATR_MCS15)

/*------------------------------ Tx Desc definition Macro ------------------*/
#define DESC_RATE1M				0x00
#define DESC_RATE2M				0x01
#define DESC_RATE5_5M				0x02
#define DESC_RATE11M				0x03

/*  OFDM Rates, TxHT = 0 */
#define DESC_RATE6M				0x04
#define DESC_RATE9M				0x05
#define DESC_RATE12M				0x06
#define DESC_RATE18M				0x07
#define DESC_RATE24M				0x08
#define DESC_RATE36M				0x09
#define DESC_RATE48M				0x0a
#define DESC_RATE54M				0x0b

/*  MCS Rates, TxHT = 1 */
#define DESC_RATEMCS0				0x0c
#define DESC_RATEMCS1				0x0d
#define DESC_RATEMCS2				0x0e
#define DESC_RATEMCS3				0x0f
#define DESC_RATEMCS4				0x10
#define DESC_RATEMCS5				0x11
#define DESC_RATEMCS6				0x12
#define DESC_RATEMCS7				0x13
#define DESC_RATEMCS8				0x14
#define DESC_RATEMCS9				0x15
#define DESC_RATEMCS10				0x16
#define DESC_RATEMCS11				0x17
#define DESC_RATEMCS12				0x18
#define DESC_RATEMCS13				0x19
#define DESC_RATEMCS14				0x1a
#define DESC_RATEMCS15				0x1b
#define DESC_RATEMCS15_SG			0x1c
#define DESC_RATEMCS32				0x20

/*  Global var */
#define	OFDM_TABLE_SIZE_92C	37
#define	OFDM_TABLE_SIZE_92D	43
#define	CCK_TABLE_SIZE		33

extern u32 OFDMSwingTable[OFDM_TABLE_SIZE_92D] ;

extern u8 CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8];

extern u8 CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8];

extern int rtw_ht_enable;

enum RTL871X_HCI_TYPE {

	RTW_SDIO,
	RTW_USB,
	RTW_PCIE
};

enum _CHIP_TYPE {

	NULL_CHIP_TYPE,
	RTL8712_8188S_8191S_8192S,
	RTL8188C_8192C,
	RTL8192D,
	RTL8723A,
	RTL8188E,
	MAX_CHIP_TYPE
};


enum HW_VARIABLES {
	HW_VAR_MEDIA_STATUS,
	HW_VAR_MEDIA_STATUS_SUCCESS,
	HW_VAR_SET_OPMODE,
	HW_VAR_MAC_ADDR,
	HW_VAR_BSSID,
	HW_VAR_INIT_RTS_RATE,
	HW_VAR_INIT_DATA_RATE,
	HW_VAR_BASIC_RATE,
	HW_VAR_TXPAUSE,
	HW_VAR_BCN_FUNC,
	HW_VAR_CORRECT_TSF,
	HW_VAR_CHECK_BSSID,
	HW_VAR_MLME_DISCONNECT,
	HW_VAR_MLME_SITESURVEY,
	HW_VAR_MLME_JOIN,
	HW_VAR_ON_RCR_AM,
	HW_VAR_OFF_RCR_AM,
	HW_VAR_BEACON_INTERVAL,
	HW_VAR_SLOT_TIME,
	HW_VAR_RESP_SIFS,
	HW_VAR_ACK_PREAMBLE,
	HW_VAR_SEC_CFG,
	HW_VAR_BCN_VALID,
	HW_VAR_RF_TYPE,
	HW_VAR_DM_FLAG,
	HW_VAR_DM_FUNC_OP,
	HW_VAR_DM_FUNC_SET,
	HW_VAR_DM_FUNC_CLR,
	HW_VAR_DM_INIT_PWDB,
	HW_VAR_CAM_EMPTY_ENTRY,
	HW_VAR_CAM_INVALID_ALL,
	HW_VAR_CAM_WRITE,
	HW_VAR_CAM_READ,
	HW_VAR_AC_PARAM_VO,
	HW_VAR_AC_PARAM_VI,
	HW_VAR_AC_PARAM_BE,
	HW_VAR_AC_PARAM_BK,
	HW_VAR_ACM_CTRL,
	HW_VAR_AMPDU_MIN_SPACE,
	HW_VAR_AMPDU_FACTOR,
	HW_VAR_RXDMA_AGG_PG_TH,
	HW_VAR_SET_RPWM,
	HW_VAR_H2C_FW_PWRMODE,
	HW_VAR_H2C_FW_JOINBSSRPT,
	HW_VAR_FWLPS_RF_ON,
	HW_VAR_H2C_FW_P2P_PS_OFFLOAD,
	HW_VAR_TDLS_WRCR,
	HW_VAR_TDLS_INIT_CH_SEN,
	HW_VAR_TDLS_RS_RCR,
	HW_VAR_TDLS_DONE_CH_SEN,
	HW_VAR_INITIAL_GAIN,
	HW_VAR_TRIGGER_GPIO_0,
	HW_VAR_BT_SET_COEXIST,
	HW_VAR_BT_ISSUE_DELBA,
	HW_VAR_CURRENT_ANTENNA,
	HW_VAR_ANTENNA_DIVERSITY_LINK,
	HW_VAR_ANTENNA_DIVERSITY_SELECT,
	HW_VAR_SWITCH_EPHY_WoWLAN,
	HW_VAR_EFUSE_BYTES,
	HW_VAR_FIFO_CLEARN_UP,
	HW_VAR_CHECK_TXBUF,
	HW_VAR_APFM_ON_MAC, /* Auto FSM to Turn On, include clock, isolation, power control for MAC only */
	HW_VAR_WOWLAN,
	HW_VAR_VID,
	HW_VAR_PID,
	HW_VAR_MBSSID_CAM_WRITE,
	HW_VAR_MBSSID_CAM_CLEAR,
	HW_VAR_RCR_MBSSID_EN,
};

enum HAL_DEF_VARIABLE {
	HAL_DEF_UNDERCORATEDSMOOTHEDPWDB,
	HAL_DEF_IS_SUPPORT_ANT_DIV,
	HAL_DEF_CURRENT_ANTENNA,
	HAL_DEF_DRVINFO_SZ,
	HAL_DEF_MAX_RECVBUF_SZ,
	HAL_DEF_RX_PACKET_OFFSET,
	HAL_DEF_DBG_DUMP_RXPKT,/* for dbg */
	HAL_DEF_DBG_DM_FUNC,/* for dbg */

};

enum HAL_INTF_PS_FUNC {
	HAL_USB_SELECT_SUSPEND,
	HAL_MAX_ID,
};

typedef s32 (*c2h_id_filter)(u8 id);

struct hal_ops {
	u32 (*hal_init)(struct rtw_adapter *adapter);
	u32 (*hal_deinit)(struct rtw_adapter *adapter);

	void (*free_hal_data)(struct rtw_adapter *adapter);

	u32 (*inirp_init)(struct rtw_adapter *adapter);
	u32 (*inirp_deinit)(struct rtw_adapter *adapter);

	s32 (*init_xmit_priv)(struct rtw_adapter *adapter);
	void (*free_xmit_priv)(struct rtw_adapter *adapter);

	s32 (*init_recv_priv)(struct rtw_adapter *adapter);
	void (*free_recv_priv)(struct rtw_adapter *adapter);

	void (*InitSwLeds)(struct rtw_adapter *adapter);
	void (*DeInitSwLeds)(struct rtw_adapter *adapter);

	void (*dm_init)(struct rtw_adapter *adapter);
	void (*dm_deinit)(struct rtw_adapter *adapter);
	void (*read_chip_version)(struct rtw_adapter *adapter);

	void (*init_default_value)(struct rtw_adapter *adapter);

	void (*intf_chip_configure)(struct rtw_adapter *adapter);

	void (*read_adapter_info)(struct rtw_adapter *adapter);

	void (*enable_interrupt)(struct rtw_adapter *adapter);
	void (*disable_interrupt)(struct rtw_adapter *adapter);
	s32 (*interrupt_handler)(struct rtw_adapter *adapter);

	void (*set_bwmode_handler)(struct rtw_adapter *adapter,
				      enum HT_CHANNEL_WIDTH Bandwidth,
				      u8 Offset);
	void (*set_channel_handler)(struct rtw_adapter *adapter, u8 channel);

	void (*hal_dm_watchdog)(struct rtw_adapter *adapter);

	void (*SetHwRegHandler)(struct rtw_adapter *adapter, u8  variable,
				u8 *val);
	void (*GetHwRegHandler)(struct rtw_adapter *adapter, u8  variable,
				u8 *val);

	u8  (*GetHalDefVarHandler)(struct rtw_adapter *adapter,
				   enum HAL_DEF_VARIABLE eVariable,
				   void *pValue);
	u8  (*SetHalDefVarHandler)(struct rtw_adapter *adapter,
				   enum HAL_DEF_VARIABLE eVariable,
				   void *pValue);

	void (*UpdateRAMaskHandler)(struct rtw_adapter *adapter, u32 mac_id);
	void (*SetBeaconRelatedRegistersHandler)(struct rtw_adapter *adapter);

	void (*Add_RateATid)(struct rtw_adapter *adapter, u32 bitmap, u8 arg);

	u8  (*interface_ps_func)(struct rtw_adapter *adapter,
				 enum HAL_INTF_PS_FUNC efunc_id, u8 *val);

	s32 (*hal_xmit)(struct rtw_adapter *adapter,
			struct xmit_frame *pxmitframe);
	s32 (*mgnt_xmit)(struct rtw_adapter *adapter,
			 struct xmit_frame *pmgntframe);

	u32 (*read_bbreg)(struct rtw_adapter *adapter, u32 RegAddr,
			  u32 BitMask);
	void (*write_bbreg)(struct rtw_adapter *adapter, u32 RegAddr,
			    u32 BitMask, u32 Data);
	u32 (*read_rfreg)(struct rtw_adapter *adapter,
			  enum RF_RADIO_PATH_E eRFPath, u32 RegAddr,
			  u32 BitMask);
	void (*write_rfreg)(struct rtw_adapter *adapter,
			    enum RF_RADIO_PATH_E eRFPath, u32 RegAddr,
			    u32 BitMask, u32 Data);
	void (*EfusePowerSwitch)(struct rtw_adapter *adapter, u8 bWrite,
				 u8 PwrState);
	void (*ReadEFuse)(struct rtw_adapter *adapter, u8 efuseType,
			  u16 _offset, u16 _size_byte, u8 *pbuf,
			  bool bPseudoTest);
	void (*EFUSEGetEfuseDefinition)(struct rtw_adapter *adapter,
					u8 efuseType, u8 type, void **pOut,
					bool bPseudoTest);
	u16 (*EfuseGetCurrentSize)(struct rtw_adapter *adapter, u8 efuseType,
				   bool bPseudoTest);
	int (*Efuse_PgPacketRead)(struct rtw_adapter *adapter, u8 offset,
				  u8 *data, bool bPseudoTest);
	int (*Efuse_PgPacketWrite)(struct rtw_adapter *adapter, u8 offset,
				   u8 word_en, u8 *data, bool bPseudoTest);
	u8  (*Efuse_WordEnableDataWrite)(struct rtw_adapter *adapter,
					 u16 efuse_addr, u8 word_en, u8 *data,
					 bool bPseudoTest);

	void (*hal_notch_filter)(struct rtw_adapter *adapter, bool enable);
	void (*hal_reset_security_engine)(struct rtw_adapter *adapter);

	s32 (*c2h_handler)(struct rtw_adapter *padapter,
			   struct c2h_evt_hdr *c2h_evt);
	c2h_id_filter c2h_id_filter_ccx;
};

enum RT_EEPROM_TYPE {
	EEPROM_93C46,
	EEPROM_93C56,
	EEPROM_BOOT_EFUSE,
};

#define USB_HIGH_SPEED_BULK_SIZE	512
#define USB_FULL_SPEED_BULK_SIZE	64

#define RF_CHANGE_BY_INIT	0
#define RF_CHANGE_BY_IPS	BIT28
#define RF_CHANGE_BY_PS		BIT29
#define RF_CHANGE_BY_HW		BIT30
#define RF_CHANGE_BY_SW		BIT31

enum HARDWARE_TYPE {
	HARDWARE_TYPE_RTL8180,
	HARDWARE_TYPE_RTL8185,
	HARDWARE_TYPE_RTL8187,
	HARDWARE_TYPE_RTL8188,
	HARDWARE_TYPE_RTL8190P,
	HARDWARE_TYPE_RTL8192E,
	HARDWARE_TYPE_RTL819xU,
	HARDWARE_TYPE_RTL8192SE,
	HARDWARE_TYPE_RTL8192SU,
	HARDWARE_TYPE_RTL8192CE,
	HARDWARE_TYPE_RTL8192CU,
	HARDWARE_TYPE_RTL8192DE,
	HARDWARE_TYPE_RTL8192DU,
	HARDWARE_TYPE_RTL8723AE,
	HARDWARE_TYPE_RTL8723AU,
	HARDWARE_TYPE_RTL8723AS,
	HARDWARE_TYPE_RTL8188EE,
	HARDWARE_TYPE_RTL8188EU,
	HARDWARE_TYPE_RTL8188ES,
	HARDWARE_TYPE_MAX,
};

/*  RTL8192D Series */
#define IS_HARDWARE_TYPE_8192DU(_adapter)			\
	((_adapter)->HardwareType == HARDWARE_TYPE_RTL8192DU)
#define	IS_HARDWARE_TYPE_8192D(_adapter)			\
	(IS_HARDWARE_TYPE_8192DU(_adapter))

#define GET_EEPROM_EFUSE_PRIV(priv)	(&priv->eeprompriv)

#ifdef CONFIG_WAKE_ON_WLAN
enum wowlan_subcode {
	WOWLAN_PATTERN_MATCH	= 1,
	WOWLAN_MAGIC_PACKET	= 2,
	WOWLAN_UNICAST		= 3,
	WOWLAN_SET_PATTERN	= 4,
	WOWLAN_DUMP_REG		= 5,
	WOWLAN_ENABLE		= 6,
	WOWLAN_DISABLE		= 7,
	WOWLAN_STATUS		= 8,
	WOWLAN_DEBUG_RELOAD_FW	= 9,
	WOWLAN_DEBUG_1		=10,
	WOWLAN_DEBUG_2		=11
};

struct wowlan_ioctl_param{
	unsigned int subcode;
	unsigned int subcode_value;
	unsigned int wakeup_reason;
	unsigned int len;
	unsigned char pattern[0];
};

#define Rx_Pairwisekey				BIT(0)
#define Rx_GTK					BIT(1)
#define Rx_DisAssoc				BIT(2)
#define Rx_DeAuth				BIT(3)
#define FWDecisionDisconnect			BIT(4)
#define Rx_MagicPkt				BIT(5)
#define FinishBtFwPatch				BIT(7)

#endif /*  CONFIG_WAKE_ON_WLAN */

void rtw_hal_def_value_init(struct rtw_adapter *padapter);
void rtw_hal_free_data(struct rtw_adapter *padapter);
void rtw_hal_dm_init(struct rtw_adapter *padapter);
void rtw_hal_dm_deinit(struct rtw_adapter *padapter);
void rtw_hal_sw_led_init(struct rtw_adapter *padapter);
void rtw_hal_sw_led_deinit(struct rtw_adapter *padapter);
uint rtw_hal_init(struct rtw_adapter *padapter);
uint rtw_hal_deinit(struct rtw_adapter *padapter);
void rtw_hal_stop(struct rtw_adapter *padapter);
void rtw_hal_set_hwreg(struct rtw_adapter *padapter, u8 variable, u8 *val);
void rtw_hal_get_hwreg(struct rtw_adapter *padapter, u8 variable, u8 *val);
void rtw_hal_chip_configure(struct rtw_adapter *padapter);
void rtw_hal_read_chip_info(struct rtw_adapter *padapter);
void rtw_hal_read_chip_version(struct rtw_adapter *padapter);
u8 rtw_hal_set_def_var(struct rtw_adapter *padapter,
		       enum HAL_DEF_VARIABLE eVariable, void *pValue);
u8 rtw_hal_get_def_var(struct rtw_adapter *padapter,
		       enum HAL_DEF_VARIABLE eVariable, void *pValue);
void rtw_hal_enable_interrupt(struct rtw_adapter *padapter);
void rtw_hal_disable_interrupt(struct rtw_adapter *padapter);
u32 rtw_hal_inirp_init(struct rtw_adapter *padapter);
u32 rtw_hal_inirp_deinit(struct rtw_adapter *padapter);
u8 rtw_hal_intf_ps_func(struct rtw_adapter *padapter,
			enum HAL_INTF_PS_FUNC efunc_id, u8 *val);
s32 rtw_hal_xmit(struct rtw_adapter *padapter, struct xmit_frame *pxmitframe);
s32 rtw_hal_mgnt_xmit(struct rtw_adapter *padapter,
		      struct xmit_frame *pmgntframe);
s32 rtw_hal_init_xmit_priv(struct rtw_adapter *padapter);
void rtw_hal_free_xmit_priv(struct rtw_adapter *padapter);
s32 rtw_hal_init_recv_priv(struct rtw_adapter *padapter);
void rtw_hal_free_recv_priv(struct rtw_adapter *padapter);
void rtw_hal_update_ra_mask(struct rtw_adapter *padapter, u32 mac_id);
void rtw_hal_add_ra_tid(struct rtw_adapter *padapter, u32 bitmap, u8 arg);
void rtw_hal_bcn_related_reg_setting(struct rtw_adapter *padapter);
u32 rtw_hal_read_bbreg(struct rtw_adapter *padapter, u32 RegAddr, u32 BitMask);
void rtw_hal_write_bbreg(struct rtw_adapter *padapter, u32 RegAddr, u32 BitMask,
			 u32 Data);
u32 rtw_hal_read_rfreg(struct rtw_adapter *padapter, u32 eRFPath, u32 RegAddr,
		       u32 BitMask);
void rtw_hal_write_rfreg(struct rtw_adapter *padapter, u32 eRFPath, u32 RegAddr,
			 u32 BitMask, u32 Data);
s32 rtw_hal_interrupt_handler(struct rtw_adapter *padapter);
void rtw_hal_set_bwmode(struct rtw_adapter *padapter,
			enum HT_CHANNEL_WIDTH Bandwidth, u8 Offset);
void rtw_hal_set_chan(struct rtw_adapter *padapter, u8 channel);
void rtw_hal_dm_watchdog(struct rtw_adapter *padapter);
void rtw_hal_notch_filter(struct rtw_adapter *adapter, bool enable);
void rtw_hal_reset_security_engine(struct rtw_adapter *adapter);
s32 rtw_hal_c2h_handler(struct rtw_adapter *adapter,
			struct c2h_evt_hdr *c2h_evt);
c2h_id_filter rtw_hal_c2h_id_filter_ccx(struct rtw_adapter *adapter);
/* return the final channel plan decision */
u8 hal_com_get_channel_plan(struct rtw_adapter *padapter, u8 hw_channel_plan,
			    u8 sw_channel_plan, u8 def_channel_plan,
			    bool AutoLoadFail);
void halsetbratecfg(struct rtw_adapter *adapter, u8 *mBratesOS, u16 *pBrateCfg);
u8 ratetohwrate(u8 rate);
void hal_init_macaddr(struct rtw_adapter *adapter);
void c2h_evt_clear(struct rtw_adapter *adapter);
s32 c2h_evt_read(struct rtw_adapter *adapter, u8 *buf);

#endif /* __HAL_INTF_H__ */
