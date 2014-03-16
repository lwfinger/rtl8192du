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

/* HAL  Related Config */

#define DBG 0
