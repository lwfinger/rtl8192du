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
/*-------------------------------------------------------------------------------

	For type defines and data structure defines

--------------------------------------------------------------------------------*/


#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__

#include <drv_conf.h>
#include <osdep_service.h>
#include <wlan_bssdef.h>

#include <drv_types_linux.h>

enum _NIC_VERSION {

	RTL8711_NIC,
	RTL8712_NIC,
	RTL8713_NIC,
	RTL8716_NIC

};

enum{
	UP_LINK,
	DOWN_LINK,
};
typedef struct _ADAPTER _adapter, ADAPTER,*PADAPTER;

#ifdef CONFIG_80211N_HT
#include <rtw_ht.h>
#endif

#include <rtw_cmd.h>
#include <wlan_bssdef.h>
#include <rtw_security.h>
#include <rtw_xmit.h>
#include <rtw_recv.h>
#include <hal_intf.h>
#include <hal_com.h>
#include <rtw_qos.h>
#include <rtw_pwrctrl.h>
#include <rtw_io.h>
#include <rtw_eeprom.h>
#include <sta_info.h>
#include <rtw_mlme.h>
#include <rtw_debug.h>
#include <rtw_rf.h>
#include <rtw_event.h>
#include <rtw_led.h>
#include <rtw_mlme_ext.h>
#include <rtw_p2p.h>
#include <rtw_tdls.h>
#include <rtw_ap.h>

#include "../hal/dm.h"

#ifdef CONFIG_DRVEXT_MODULE
#include <drvext_api.h>
#endif

#ifdef CONFIG_MP_INCLUDED
#include <rtw_mp.h>
#endif

#ifdef CONFIG_BR_EXT
#include <rtw_br_ext.h>
#endif	// CONFIG_BR_EXT

#ifdef CONFIG_IOCTL_CFG80211
	#include "ioctl_cfg80211.h"
#endif //CONFIG_IOCTL_CFG80211

#define SPEC_DEV_ID_NONE BIT(0)
#define SPEC_DEV_ID_DISABLE_HT BIT(1)
#define SPEC_DEV_ID_ENABLE_PS BIT(2)
#define SPEC_DEV_ID_RF_CONFIG_1T1R BIT(3)
#define SPEC_DEV_ID_RF_CONFIG_2T2R BIT(4)
#define SPEC_DEV_ID_ASSIGN_IFNAME BIT(5)

struct specific_device_id{

	u32		flags;

	u16		idVendor;
	u16		idProduct;

};

struct registry_priv
{
	u8	chip_version;
	u8	rfintfs;
	u8	lbkmode;
	u8	hci;
	NDIS_802_11_SSID	ssid;
	u8	network_mode;	//infra, ad-hoc, auto
	u8	channel;//ad-hoc support requirement
	u8	wireless_mode;//A, B, G, auto
	u8	scan_mode;//active, passive
	u8	radio_enable;
	u8	preamble;//long, short, auto
	u8	vrtl_carrier_sense;//Enable, Disable, Auto
	u8	vcs_type;//RTS/CTS, CTS-to-self
	u16	rts_thresh;
	u16  frag_thresh;
	u8	adhoc_tx_pwr;
	u8	soft_ap;
	u8	power_mgnt;
	u8	ips_mode;
	u8	smart_ps;
	u8	long_retry_lmt;
	u8	short_retry_lmt;
	u16	busy_thresh;
	u8	ack_policy;
	u8	mp_mode;
	u8	software_encrypt;
	u8	software_decrypt;

	u8	acm_method;
	  //UAPSD
	u8	wmm_enable;
	u8	uapsd_enable;
	u8	uapsd_max_sp;
	u8	uapsd_acbk_en;
	u8	uapsd_acbe_en;
	u8	uapsd_acvi_en;
	u8	uapsd_acvo_en;

	WLAN_BSSID_EX    dev_network;

#ifdef CONFIG_80211N_HT
	u8	ht_enable;
	u8	cbw40_enable;
	u8	ampdu_enable;//for tx
	u8	rx_stbc;
	u8	ampdu_amsdu;//A-MPDU Supports A-MSDU is permitted
#endif
	u8	lowrate_two_xmit;

	u8	rf_config ;
	u8	low_power ;

	u8	wifi_spec;// !turbo_mode

	u8	channel_plan;
#ifdef CONFIG_BT_COEXIST
	u8	bt_iso;
	u8	bt_sco;
	u8	bt_ampdu;
#endif
	BOOLEAN	bAcceptAddbaReq;

	u8	antdiv_cfg;

	u8	usbss_enable;//0:disable,1:enable
	u8	hwpdn_mode;//0:disable,1:enable,2:decide by EFUSE config
	u8	hwpwrp_detect;//0:disable,1:enable

	u8	hw_wps_pbc;//0:disable,1:enable

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	char	adaptor_info_caching_file_path[PATH_LENGTH_MAX];
#endif

#ifdef CONFIG_LAYER2_ROAMING
	u8	max_roaming_times; // the max number driver will try to roaming
#endif

#ifdef CONFIG_IOL
	bool force_iol; //enable iol without other concern
#endif
	u8  special_rf_path; //0: 2T2R ,1: only turn on path A 1T1R, 2: only turn on path B 1T1R
	u8	mac_phy_mode; //0:by efuse, 1:smsp, 2:dmdp, 3:dmsp.

#ifdef CONFIG_80211D
	u8 enable80211d;
#endif

	u8 ifname[16];
	u8 if2name[16];

	u8 notch_filter;

#ifdef CONFIG_MULTI_VIR_IFACES
	u8 ext_iface_num;//primary/secondary iface is excluded
#endif

	u8 hiq_filter;
};


//For registry parameters
#define RGTRY_OFT(field) ((ULONG)FIELD_OFFSET(struct registry_priv,field))
#define RGTRY_SZ(field)   sizeof(((struct registry_priv*) 0)->field)
#define BSSID_OFT(field) ((ULONG)FIELD_OFFSET(WLAN_BSSID_EX,field))
#define BSSID_SZ(field)   sizeof(((PWLAN_BSSID_EX) 0)->field)

#define MAX_CONTINUAL_URB_ERR 4

#ifdef CONFIG_CONCURRENT_MODE
#define is_primary_adapter(adapter) (adapter->adapter_type == PRIMARY_ADAPTER)
#define get_iface_type(adapter) (adapter->iface_type)
#else
#define is_primary_adapter(adapter) (1)
#define get_iface_type(adapter) (IFACE_PORT0)
#endif
#define GET_PRIMARY_ADAPTER(padapter) (((_adapter *)padapter)->dvobj->if1)
#define GET_IFACE_NUMS(padapter) (((_adapter *)padapter)->dvobj->iface_nums)
#define GET_ADAPTER(padapter, iface_id) (((_adapter *)padapter)->dvobj->padapters[iface_id])

struct debug_priv {
	u64 dbg_rx_fifo_last_overflow;
	u64 dbg_rx_fifo_curr_overflow;
	u64 dbg_rx_fifo_diff_overflow;
	u64 dbg_rx_ampdu_drop_count;
	u64 dbg_rx_ampdu_forced_indicate_count;
	u64 dbg_rx_ampdu_loss_count;
	u64 dbg_rx_ampdu_window_shift_cnt;
	u64 dbg_rx_dup_mgt_frame_drop_count;};

enum _IFACE_ID {
	IFACE_ID0, //maping to PRIMARY_ADAPTER
	IFACE_ID1, //maping to SECONDARY_ADAPTER
	IFACE_ID2,
	IFACE_ID3,
	IFACE_ID_MAX,
};

struct cam_ctl_t {
	_lock lock;
	u64 bitmap;
};

struct cam_entry_cache {
	u16 ctrl;
	u8 mac[ETH_ALEN];
	u8 key[16];
};

#define KEY_FMT "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define KEY_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5], \
	((u8*)(x))[6],((u8*)(x))[7],((u8*)(x))[8],((u8*)(x))[9],((u8*)(x))[10],((u8*)(x))[11], \
	((u8*)(x))[12],((u8*)(x))[13],((u8*)(x))[14],((u8*)(x))[15]

struct dvobj_priv
{
	_adapter *if1; //PRIMARY_ADAPTER
	_adapter *if2; //SECONDARY_ADAPTER

	s32 processing_dev_remove;
	struct debug_priv drv_dbg;
	//for local/global synchronization
	_mutex hw_init_mutex;
	_mutex h2c_fwcmd_mutex;
	_mutex setch_mutex;
	_mutex setbw_mutex;

	unsigned char	oper_channel; //saved channel info when call set_channel_bw
	unsigned char	oper_bwmode;
	unsigned char	oper_ch_offset;//PRIME_CHNL_OFFSET
	u32 on_oper_ch_time;

	//extend to support mulitu interface
	//padapters[IFACE_ID0] == if1
	//padapters[IFACE_ID1] == if2
	_adapter *padapters[IFACE_ID_MAX];
	u8 iface_nums; // total number of ifaces used runtime

	struct cam_ctl_t cam_ctl;
	struct cam_entry_cache cam_cache[32];

	//For 92D, DMDP have 2 interface.
	u8	InterfaceNumber;
	u8	NumInterfaces;
	u8	DualMacMode;
	u8	irq_alloc;

/*-------- below is for SDIO INTERFACE --------*/

#ifdef INTF_DATA
	INTF_DATA intf_data;
#endif

/*-------- below is for USB INTERFACE --------*/


	u8	nr_endpoint;
	u8	ishighspeed;
	u8	RtNumInPipes;
	u8	RtNumOutPipes;
	int	ep_num[5]; //endpoint number

	int	RegUsbSS;

	_sema	usb_suspend_sema;

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	_mutex  usb_vendor_req_mutex;
#endif

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	u8 * usb_alloc_vendor_req_buf;
	u8 * usb_vendor_req_buf;
#endif

	struct usb_interface *pusbintf;
	struct usb_device *pusbdev;

	ATOMIC_T continual_urb_error;

	DM_ODM_T odmpriv;
};

static struct device *dvobj_to_dev(struct dvobj_priv *dvobj)
{
	/* todo: get interface type from dvobj and the return the dev accordingly */

	return &dvobj->pusbintf->dev;
}

enum _IFACE_TYPE {
	IFACE_PORT0, //mapping to port0 for C/D series chips
	IFACE_PORT1, //mapping to port1 for C/D series chip
	MAX_IFACE_PORT,
};

enum _ADAPTER_TYPE {
	PRIMARY_ADAPTER,
	SECONDARY_ADAPTER,
	MAX_ADAPTER = 0xFF,
};

typedef enum _DRIVER_STATE{
	DRIVER_NORMAL = 0,
	DRIVER_DISAPPEAR = 1,
	DRIVER_REPLACE_DONGLE = 2,
}DRIVER_STATE;

#ifdef CONFIG_INTEL_PROXIM
struct proxim {
	bool proxim_support;
	bool proxim_on;

	void *proximity_priv;
	int (*proxim_rx)(_adapter *padapter,
		union recv_frame *precv_frame);
	u8	(*proxim_get_var)(_adapter* padapter, u8 type);
};
#endif	//CONFIG_INTEL_PROXIM

#ifdef RTL8723A_SDIO_LOOPBACK
typedef struct loopbackdata
{
	_sema	sema;
	_thread_hdl_ lbkthread;
	u8 bstop;
	u32 cnt;
	u16 size;
	u16 txsize;
	u8 txbuf[0x8000];
	u16 rxsize;
	u8 rxbuf[0x8000];
	u8 msg[100];

}LOOPBACKDATA, *PLOOPBACKDATA;
#endif

struct _ADAPTER{
	int	DriverState;// for disable driver using module, use dongle to replace module.
	int	pid[3];//process id from UI, 0:wps, 1:hostapd, 2:dhcpcd
	int	bDongle;//build-in module or external dongle
	u16	chip_type;
	u16	HardwareType;
	u16	interface_type;//USB,SDIO,PCI

	struct dvobj_priv *dvobj;
	struct	mlme_priv mlmepriv;
	struct	mlme_ext_priv mlmeextpriv;
	struct	cmd_priv	cmdpriv;
	struct	evt_priv	evtpriv;
	struct	workqueue_struct *regd_queue;
	//struct	io_queue	*pio_queue;
	struct	io_priv	iopriv;
	struct	xmit_priv	xmitpriv;
	struct	recv_priv	recvpriv;
	struct	sta_priv	stapriv;
	struct	security_priv	securitypriv;
	_lock   security_key_mutex; // add for CONFIG_IEEE80211W, none 11w also can use
	struct	registry_priv	registrypriv;
	struct	pwrctrl_priv	pwrctrlpriv;
	struct	eeprom_priv eeprompriv;
	struct	led_priv	ledpriv;

#ifdef CONFIG_MP_INCLUDED
       struct	mp_priv	mppriv;
#endif

#ifdef CONFIG_DRVEXT_MODULE
	struct	drvext_priv	drvextpriv;
#endif

#ifdef CONFIG_AP_MODE
	struct	hostapd_priv	*phostapdpriv;
#endif

#ifdef CONFIG_IOCTL_CFG80211
#ifdef CONFIG_P2P
	struct cfg80211_wifidirect_info	cfg80211_wdinfo;
#endif //CONFIG_P2P
#endif //CONFIG_IOCTL_CFG80211
	u32	setband;
#ifdef CONFIG_P2P
	struct wifidirect_info	wdinfo;
#endif //CONFIG_P2P

#ifdef CONFIG_TDLS
	struct tdls_info	tdlsinfo;
#endif //CONFIG_TDLS

#ifdef CONFIG_WFD
	struct wifi_display_info wfd_info;
#endif //CONFIG_WFD

	PVOID			HalData;
	u32 hal_data_sz;
	struct hal_ops	HalFunc;

#ifdef CONFIG_BT_COEXIST
	//struct	btcoexist_priv	bt_coexist;
#endif
	s32	bDriverStopped;
	s32	bSurpriseRemoved;
	s32  bCardDisableWOHSM;

	u32	IsrContent;
	u32	ImrContent;

	u8	EepromAddressSize;
	u8	hw_init_completed;
	u8	bDriverIsGoingToUnload;
	u8	init_adpt_in_progress;
	u8	bHaltInProgress;

	_thread_hdl_ cmdThread;
	_thread_hdl_ evtThread;
	_thread_hdl_ xmitThread;
	_thread_hdl_ recvThread;

	void (*intf_start)(_adapter * adapter);
	void (*intf_stop)(_adapter * adapter);

	_nic_hdl pnetdev;
	char old_ifname[IFNAMSIZ];

	// used by rtw_rereg_nd_name related function
	struct rereg_nd_name_data {
		_nic_hdl old_pnetdev;
		char old_ifname[IFNAMSIZ];
		u8 old_ips_mode;
		u8 old_bRegUseLed;
	} rereg_nd_name_priv;

	int bup;
	struct net_device_stats stats;
	struct iw_statistics iwstats;
	struct proc_dir_entry *dir_dev;// for proc directory
	struct proc_dir_entry *dir_dm;

#ifdef CONFIG_IOCTL_CFG80211
	struct wireless_dev *rtw_wdev;
	struct rtw_wdev_priv wdev_data;
#endif //CONFIG_IOCTL_CFG80211

	int net_closed;

	u8 bFWReady;
	u8 bReadPortCancel;
	u8 bWritePortCancel;
	u8 bRxRSSIDisplay;
	//	Added by Albert 2012/07/26
	//	The driver will write the initial gain everytime when running in the DM_Write_DIG function.
	u8 bForceWriteInitGain;
	//	Added by Albert 2012/10/26
	//	The driver will show up the desired channel number when this flag is 1.
	u8 bNotifyChannelChange;
#ifdef CONFIG_P2P
	//	Added by Albert 2012/12/06
	//	The driver will show the current P2P status when the upper application reads it.
	u8 bShowGetP2PState;
#endif
#ifdef CONFIG_AUTOSUSPEND
	u8	bDisableAutosuspend;
#endif

	//pbuddy_adapter is used only in  two inteface case, (iface_nums=2 in struct dvobj_priv)
	//PRIMARY_ADAPTER's buddy is SECONDARY_ADAPTER
	//SECONDARY_ADAPTER's buddy is PRIMARY_ADAPTER
	//for iface_id > SECONDARY_ADAPTER(IFACE_ID1), refer to padapters[iface_id]  in struct dvobj_priv
	//and their pbuddy_adapter is PRIMARY_ADAPTER.
	//for PRIMARY_ADAPTER(IFACE_ID0) can directly refer to if1 in struct dvobj_priv
	_adapter *pbuddy_adapter;

#if defined(CONFIG_CONCURRENT_MODE) || defined(CONFIG_DUALMAC_CONCURRENT)
	u8 isprimary; //is primary adapter or not
	//notes:
	// if isprimary is true, the adapter_type value is 0, iface_id is IFACE_ID0 for PRIMARY_ADAPTER
	// if isprimary is false, the adapter_type value is 1, iface_id is IFACE_ID1 for SECONDARY_ADAPTER
	// refer to iface_id if iface_nums>2 and isprimary is false and the adapter_type value is 0xff.
	u8 adapter_type;//used only in  two inteface case(PRIMARY_ADAPTER and SECONDARY_ADAPTER) .
	u8 iface_type; //interface port type, it depends on HW port
#endif

	//extend to support multi interface
       //IFACE_ID0 is equals to PRIMARY_ADAPTER
       //IFACE_ID1 is equals to SECONDARY_ADAPTER
	u8 iface_id;

#ifdef CONFIG_DUALMAC_CONCURRENT
	u8 DualMacConcurrent; // 1: DMSP 0:DMDP
#endif

#ifdef CONFIG_BR_EXT
	_lock					br_ext_lock;
	//unsigned int			macclone_completed;
	struct nat25_network_db_entry	*nethash[NAT25_HASH_SIZE];
	int				pppoe_connection_in_progress;
	unsigned char			pppoe_addr[MACADDRLEN];
	unsigned char			scdb_mac[MACADDRLEN];
	unsigned char			scdb_ip[4];
	struct nat25_network_db_entry	*scdb_entry;
	unsigned char			br_mac[MACADDRLEN];
	unsigned char			br_ip[4];

	struct br_ext_info		ethBrExtInfo;
#endif	// CONFIG_BR_EXT

#ifdef CONFIG_INTEL_PROXIM
	/* intel Proximity, should be alloc mem
	 * in intel Proximity module and can only
	 * be used in intel Proximity mode */
	struct proxim proximity;
#endif	//CONFIG_INTEL_PROXIM

#ifdef RTL8723A_SDIO_LOOPBACK
	PLOOPBACKDATA ploopback;
#endif

	u8 fix_rate;
};

#define adapter_to_dvobj(adapter) (adapter->dvobj)
#define adapter_to_odm(adapter) (&(adapter->dvobj->odmpriv))
#define adapter_wdev_data(adapter) (&((adapter)->wdev_data))

int rtw_handle_dualmac(_adapter *adapter, bool init);

__inline static u8 *myid(struct eeprom_priv *peepriv)
{
	return (peepriv->mac_addr);
}


#endif //__DRV_TYPES_H__
