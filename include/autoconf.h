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
 *
 ******************************************************************************/

/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define DRV_NAME "r8192du"
#define DRIVERVERSION	"v4.2.1_7122.20130408"

#define CONFIG_IOCTL_CFG80211 1

/*
 * Internal  General Config
 */
#define CONFIG_PWRCTRL	1

//#define CONFIG_WAKE_ON_WLAN

#ifdef CONFIG_WAKE_ON_WLAN
#define CONFIG_WOWLAN 1
#endif /* CONFIG_WAKE_ON_WLAN */
#define CONFIG_R871X_TEST	1

#define CONFIG_RECV_REORDERING_CTRL	1

#define CONFIG_92D_AP_MODE 1

#define CONFIG_P2P	1

#ifdef CONFIG_P2P
	#define CONFIG_P2P_REMOVE_GROUP_INFO
#endif

//#define CONFIG_IOCTL_CFG80211

#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable,*/

//#define CONFIG_CONCURRENT_MODE 1

/*
 * USB VENDOR REQ BUFFER ALLOCATION METHOD
 * if not set we'll use function local variable (stack memory)
 */

/* HAL  Related Config */

#define RTL8192C_RX_PACKET_NO_INCLUDE_CRC	1

#define CONFIG_ONLY_ONE_OUT_EP_TO_LOW	0

#define CONFIG_OUT_EP_WIFI_MODE	0

#define RTL8192CU_ASIC_VERIFICATION	0	/*  For ASIC verification. */

#define DISABLE_BB_RF	0

#define RTL8191C_FPGA_NETWORKTYPE_ADHOC 0

#define ANTENNA_SELECTION_STATIC_SETTING 0

#define TX_POWER_FOR_5G_BAND				1	/* For 5G band TX Power */

#define RTL8192D_EASY_SMART_CONCURRENT	0

#define RTL8192D_DUAL_MAC_MODE_SWITCH	0

#define FW_PROCESS_VENDOR_CMD 1

#define DBG 0

#define CONFIG_DEBUG_RTL819X
