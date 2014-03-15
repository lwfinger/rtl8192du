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
#ifndef __RTW_LED_H_
#define __RTW_LED_H_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#define MSECS(t)	(HZ * ((t) / 1000) + (HZ * ((t) % 1000)) / 1000)

enum LED_CTL_MODE {
	LED_CTL_POWER_ON = 1,
	LED_CTL_LINK = 2,
	LED_CTL_NO_LINK = 3,
	LED_CTL_TX = 4,
	LED_CTL_RX = 5,
	LED_CTL_SITE_SURVEY = 6,
	LED_CTL_POWER_OFF = 7,
	LED_CTL_START_TO_LINK = 8,
	LED_CTL_START_WPS = 9,
	LED_CTL_STOP_WPS = 10,
	LED_CTL_START_WPS_BOTTON = 11, /* added for runtop */
	LED_CTL_STOP_WPS_FAIL = 12, /* added for ALPHA */
	LED_CTL_STOP_WPS_FAIL_OVERLAP = 13, /* added for BELKIN */
};


/*  */
/*  LED object. */
/*  */

enum LED_STATE_871X {
	LED_UNKNOWN = 0,
	RTW_LED_ON = 1,
	RTW_LED_OFF = 2,
	LED_BLINK_NORMAL = 3,
	LED_BLINK_SLOWLY = 4,
	LED_POWER_ON_BLINK = 5,
	LED_SCAN_BLINK = 6, /*  LED is blinking during scanning period, the # of times to blink is depend on time for scanning. */
	LED_NO_LINK_BLINK = 7, /*  LED is blinking during no link state. */
	LED_BLINK_StartToBlink = 8,/*  Customzied for Sercomm Printer Server case */
	LED_BLINK_WPS = 9,	/*  LED is blinkg during WPS communication */
	LED_TXRX_BLINK = 10,
	LED_BLINK_WPS_STOP = 11,	/* for ALPHA */
	LED_BLINK_WPS_STOP_OVERLAP = 12,	/* for BELKIN */
};

#define IS_LED_WPS_BLINKING(_LED_871X)	(((struct LED_871X *)_LED_871X)->currledstate==LED_BLINK_WPS \
					|| ((struct LED_871X *)_LED_871X)->currledstate==LED_BLINK_WPS_STOP \
					|| ((struct LED_871X *)_LED_871X)->wps_blink_in_prog)

#define IS_LED_BLINKING(_LED_871X)	(((struct LED_871X *)_LED_871X)->wps_blink_in_prog \
					||((struct LED_871X *)_LED_871X)->scan_blink_in_prog)

enum LED_PIN_871X {
	LED_PIN_GPIO0,
	LED_PIN_LED0,
	LED_PIN_LED1
};

struct LED_871X {
	struct rtw_adapter				*padapter;
	enum LED_PIN_871X		LedPin;	/*  Identify how to implement this SW led. */
	enum LED_STATE_871X		currledstate; /*  Current LED state. */
	u8					led_on; /*  true if LED is ON, false if LED is OFF. */

	u8					bSWLedCtrl;

	u8					blink_in_prog; /*  true if it is blinking, false o.w.. */
	/*  ALPHA, added by chiyoko, 20090106 */
	u8					nolink_blink_in_prog;
	u8					link_blink_in_prog;
	u8					start_link_blink_in_prog;
	u8					scan_blink_in_prog;
	u8					wps_blink_in_prog;

	u32					blinktimes; /*  Number of times to toggle led state for blinking. */
	enum LED_STATE_871X		blinkingledstate; /*  Next state for blinking, either RTW_LED_ON or RTW_LED_OFF are. */

	struct timer_list		blinktimer; /*  Timer object for led blinking. */
	struct work_struct BlinkWorkItem; /*  Workitem used by blinktimer to manipulate H/W to blink LED. */
};


/*  LED customization. */

enum LED_STRATEGY_871X {
	SW_LED_MODE0, /*  SW control 1 LED via GPIO0. It is default option. */
	SW_LED_MODE1, /*  2 LEDs, through LED0 and LED1. For ALPHA. */
	SW_LED_MODE2, /*  SW control 1 LED via GPIO0, customized for AzWave 8187 minicard. */
	SW_LED_MODE3, /*  SW control 1 LED via GPIO0, customized for Sercomm Printer Server case. */
	SW_LED_MODE4, /* for Edimax / Belkin */
	SW_LED_MODE5, /* for Sercomm / Belkin */
	SW_LED_MODE6, /* for 88CU minicard, porting from ce SW_LED_MODE7 */
	HW_LED, /*  HW control 2 LEDs, LED0 and LED1 (there are 4 different control modes, see MAC.CONFIG1 for details.) */
};

struct led_priv{
	/* add for led controll */
	struct LED_871X			SwLed0;
	struct LED_871X			SwLed1;
	enum LED_STRATEGY_871X	LedStrategy;
	u8	bRegUseLed;
	void (*LedControlHandler)(struct rtw_adapter *padapter, enum LED_CTL_MODE _ledaction);
	/* add for led controll */
};

#define rtw_led_control(adapter, _ledaction) \
	do { \
		if ((adapter)->ledpriv.LedControlHandler) \
			(adapter)->ledpriv.LedControlHandler((adapter),	\
							     (_ledaction)); \
	} while (0)

extern void BlinkHandler(struct LED_871X *pled);

#endif /* __RTW_LED_H_ */
