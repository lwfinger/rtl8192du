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

#include "drv_types.h"
#include "rtl8192d_hal.h"

/*  */
/*	Constant. */
/*  */

/*  */
/*  Default LED behavior. */
/*  */
#define LED_BLINK_NORMAL_INTERVAL	100
#define LED_BLINK_SLOWLY_INTERVAL	200
#define LED_BLINK_LONG_INTERVAL	400

#define LED_BLINK_NO_LINK_INTERVAL_ALPHA		1000
#define LED_BLINK_LINK_INTERVAL_ALPHA			500		/* 500 */
#define LED_BLINK_SCAN_INTERVAL_ALPHA		180	/* 150 */
#define LED_BLINK_FASTER_INTERVAL_ALPHA		50
#define LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA	5000

/*  */
/*  LED object. */
/*  */

/*  */
/*	Prototype of protected function. */
/*  */

static void blinktimerCallback(unsigned long data);

static void
BlinkWorkItemCallback(
	struct work_struct *work
	);

static void
ResetLedStatus(struct LED_871X *	pled) {
	pled->currledstate = RTW_LED_OFF; /*  Current LED state. */
	pled->led_on = false; /*  true if LED is ON, false if LED is OFF. */

	pled->blink_in_prog = false; /*  true if it is blinking, false o.w.. */
	pled->nolink_blink_in_prog = false;
	pled->link_blink_in_prog = false;
	pled->start_link_blink_in_prog = false;
	pled->scan_blink_in_prog = false;
	pled->wps_blink_in_prog = false;
	pled->blinktimes = 0; /*  Number of times to toggle led state for blinking. */
	pled->blinkingledstate = LED_UNKNOWN; /*  Next state for blinking, either RTW_LED_ON or RTW_LED_OFF are. */
}

/*  */
/*  LED_819xUsb routines. */
/*  */

/*  */
/*	Description: */
/*		Initialize an struct LED_871X object. */
/*  */

static void InitLed871x(struct rtw_adapter *padapter,
	struct LED_871X *		pled,
	enum LED_PIN_871X	LedPin
	)
{
	pled->padapter = padapter;

	pled->LedPin = LedPin;

	pled->currledstate = RTW_LED_OFF;
	pled->led_on = false;

	pled->blink_in_prog = false;
	pled->blinktimes = 0;
	pled->blinkingledstate = LED_UNKNOWN;

	_init_timer(&(pled->blinktimer), padapter->pnetdev, blinktimerCallback, pled);

	INIT_WORK(&(pled->BlinkWorkItem), BlinkWorkItemCallback);
}

/*  */
/*	Description: */
/*		DeInitialize an struct LED_871X object. */
/*  */
static void DeInitLed871x(struct LED_871X *pled)
{
	/* call _cancel_workitem_sync(&(pled->BlinkWorkItem)) */
    /* before _cancel_timer_ex(&(pled->blinktimer)) to */
    /* avoid led timer restarting when driver is removed */

	_cancel_workitem_sync(&(pled->BlinkWorkItem));
	_cancel_timer_ex(&(pled->blinktimer));
	/*  We should reset blink_in_prog if we cancel the LedControlTimer, 2005.03.10, by rcnjko. */
	ResetLedStatus(pled);
}

/*  */
/*	Description: */
/*		Turn on LED according to LedPin specified. */
/*  */

static void SwLedOn(struct rtw_adapter *padapter, struct LED_871X *pled)
{
	u8	LedCfg;

	if ((padapter->bSurpriseRemoved == true) || (padapter->bDriverStopped == true))
	{
		return;
	}

	switch (pled->LedPin)
	{
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			LedCfg = rtw_read8(padapter, REG_LEDCFG2);
			rtw_write8(padapter, REG_LEDCFG2, LedCfg&0xf0); /*  SW control led0 on. */

			break;

		case LED_PIN_LED1:
			LedCfg = rtw_read8(padapter, (REG_LEDCFG2 + 1));
			rtw_write8(padapter, (REG_LEDCFG2 + 1), LedCfg&0x0f); /*  SW control led1 on. */

			break;

		default:
			break;
	}

	pled->led_on = true;
}

/*  */
/*	Description: */
/*		Turn off LED according to LedPin specified. */
/*  */
static void SwLedOff(struct rtw_adapter *padapter, struct LED_871X *pled)
{
	u8	LedCfg;

	if ((padapter->bSurpriseRemoved == true) || (padapter->bDriverStopped == true))
	{
             return;
	}

	switch (pled->LedPin)
	{
		case LED_PIN_GPIO0:
			break;

		case LED_PIN_LED0:
			LedCfg = rtw_read8(padapter, REG_LEDCFG2);
			LedCfg &= 0xf0; /*  Set to software control. */
			rtw_write8(padapter, REG_LEDCFG2, (LedCfg|BIT3));

			break;

		case LED_PIN_LED1:
			LedCfg = rtw_read8(padapter, (REG_LEDCFG2+1));
			LedCfg &= 0x0f; /*  Set to software control. */
			rtw_write8(padapter, (REG_LEDCFG2+1), (LedCfg|BIT3));

			break;

		default:
			break;
	}

	pled->led_on = false;
}

/*  */
/*	Description: */
/*		Implementation of LED blinking behavior. */
/*		It toggle off LED and schedule corresponding timer if necessary. */
/*  */
static void SwLedBlink(struct LED_871X *pled)
{
	struct rtw_adapter			*padapter = pled->padapter;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	u8				bStopBlinking = false;

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	/*  Determine if we shall change LED state again. */
	pled->blinktimes--;
	switch (pled->currledstate)
	{

	case LED_BLINK_NORMAL:
		if (pled->blinktimes == 0)
		{
			bStopBlinking = true;
		}
		break;

	case LED_BLINK_StartToBlink:
		if (check_fwstate(pmlmepriv, _FW_LINKED) && check_fwstate(pmlmepriv, WIFI_STATION_STATE))
		{
			bStopBlinking = true;
		}
		if (check_fwstate(pmlmepriv, _FW_LINKED) &&
			(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) || check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)))
		{
			bStopBlinking = true;
		}
		else if (pled->blinktimes == 0)
		{
			bStopBlinking = true;
		}
		break;

	case LED_BLINK_WPS:
		if (pled->blinktimes == 0)
		{
			bStopBlinking = true;
		}
		break;

	default:
		bStopBlinking = true;
		break;

	}

	if (bStopBlinking)
	{
		if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
		{
			SwLedOff(padapter, pled);
		}
		else if ((check_fwstate(pmlmepriv, _FW_LINKED)== true) && (pled->led_on == false))
		{
			SwLedOn(padapter, pled);
		}
		else if ((check_fwstate(pmlmepriv, _FW_LINKED)== true) &&  pled->led_on == true)
		{
			SwLedOff(padapter, pled);
		}

		pled->blinktimes = 0;
		pled->blink_in_prog = false;
	}
	else
	{
		/*  Assign LED state to toggle. */
		if (pled->blinkingledstate == RTW_LED_ON)
			pled->blinkingledstate = RTW_LED_OFF;
		else
			pled->blinkingledstate = RTW_LED_ON;

		/*  Schedule a timer to toggle LED state. */
		switch (pled->currledstate)
		{
		case LED_BLINK_NORMAL:
			_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
			break;

		case LED_BLINK_SLOWLY:
		case LED_BLINK_StartToBlink:
			_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
			break;

		case LED_BLINK_WPS:
			{
				if (pled->blinkingledstate == RTW_LED_ON)
					_set_timer(&(pled->blinktimer), LED_BLINK_LONG_INTERVAL);
				else
					_set_timer(&(pled->blinktimer), LED_BLINK_LONG_INTERVAL);
			}
			break;

		default:
			_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
			break;
		}
	}
}

static void SwLedBlink1(struct LED_871X *pled)
{
	struct rtw_adapter				*padapter = pled->padapter;
	struct hal_data_8192du		*pHalData = GET_HAL_DATA(padapter);
	struct led_priv		*ledpriv = &(padapter->ledpriv);
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	struct LED_871X *			pled1 = &(ledpriv->SwLed1);
	u8					bStopBlinking = false;

	if (pHalData->CustomerID == RT_CID_819x_CAMEO)
		pled = &(ledpriv->SwLed1);

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	if (pHalData->CustomerID == RT_CID_DEFAULT)
	{
		if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
		{
			if (!pled1->bSWLedCtrl)
			{
				SwLedOn(padapter, pled1);
				pled1->bSWLedCtrl = true;
			}
			else if (!pled1->led_on)
				SwLedOn(padapter, pled1);
			RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (): turn on pled1\n"));
		}
		else
		{
			if (!pled1->bSWLedCtrl)
			{
				SwLedOff(padapter, pled1);
				pled1->bSWLedCtrl = true;
			}
			else if (pled1->led_on)
				SwLedOff(padapter, pled1);
			RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (): turn off pled1\n"));
		}
	}

	switch (pled->currledstate)
	{
		case LED_BLINK_SLOWLY:
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			break;

		case LED_BLINK_NORMAL:
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			break;

		case LED_SCAN_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->link_blink_in_prog = true;
					pled->currledstate = LED_BLINK_NORMAL;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));

				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->nolink_blink_in_prog = true;
					pled->currledstate = LED_BLINK_SLOWLY;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->scan_blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_TXRX_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}
			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->link_blink_in_prog = true;
					pled->currledstate = LED_BLINK_NORMAL;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->nolink_blink_in_prog = true;
					pled->currledstate = LED_BLINK_SLOWLY;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->blinktimes = 0;
				pled->blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_BLINK_WPS:
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			break;

		case LED_BLINK_WPS_STOP:	/* WPS success */
			if (pled->blinkingledstate == RTW_LED_ON)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
				bStopBlinking = false;
			}
			else
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					pled->link_blink_in_prog = true;
					pled->currledstate = LED_BLINK_NORMAL;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->wps_blink_in_prog = false;
			}
			break;

		default:
			break;
	}
}

static void SwLedBlink2(struct LED_871X *pled)
{
	struct rtw_adapter				*padapter = pled->padapter;
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	u8					bStopBlinking = false;

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	switch (pled->currledstate)
	{
		case LED_SCAN_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;
					SwLedOn(padapter, pled);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("stop scan blink currledstate %d\n", pled->currledstate));

				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;
					SwLedOff(padapter, pled);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("stop scan blink currledstate %d\n", pled->currledstate));
				}
				pled->scan_blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_TXRX_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}
			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;
					SwLedOn(padapter, pled);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("stop currledstate %d\n", pled->currledstate));

				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;
					SwLedOff(padapter, pled);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("stop currledstate %d\n", pled->currledstate));
				}
				pled->blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}
			}
			break;

		default:
			break;
	}
}

static void SwLedBlink3(struct LED_871X *pled)
{
	struct rtw_adapter			*padapter = pled->padapter;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	u8				bStopBlinking = false;

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		if (pled->currledstate != LED_BLINK_WPS_STOP)
			SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	switch (pled->currledstate)
	{
		case LED_SCAN_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;
					if (!pled->led_on)
						SwLedOn(padapter, pled);

					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;
					if (pled->led_on)
						SwLedOff(padapter, pled);

					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->scan_blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_TXRX_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}
			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== true)
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;

					if (!pled->led_on)
						SwLedOn(padapter, pled);

					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				else if (check_fwstate(pmlmepriv, _FW_LINKED)== false)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;

					if (pled->led_on)
						SwLedOff(padapter, pled);

					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_BLINK_WPS:
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			break;

		case LED_BLINK_WPS_STOP:	/* WPS success */
			if (pled->blinkingledstate == RTW_LED_ON)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
				bStopBlinking = false;
			}
			else
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;
					SwLedOn(padapter, pled);
					RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
				}
				pled->wps_blink_in_prog = false;
			}
			break;

		default:
			break;
	}
}

static void SwLedBlink4(struct LED_871X *pled)
{
	struct rtw_adapter			*padapter = pled->padapter;
	struct led_priv	*ledpriv = &(padapter->ledpriv);
	struct LED_871X *		pled1 = &(ledpriv->SwLed1);
	u8				bStopBlinking = false;

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	if (!pled1->wps_blink_in_prog && pled1->blinkingledstate == LED_UNKNOWN)
	{
		pled1->blinkingledstate = RTW_LED_OFF;
		pled1->currledstate = RTW_LED_OFF;
		SwLedOff(padapter, pled1);
	}

	switch (pled->currledstate)
	{
		case LED_BLINK_SLOWLY:
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			break;

		case LED_BLINK_StartToBlink:
			if (pled->led_on)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
			}
			else
			{
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
			}
			break;

		case LED_SCAN_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = false;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					pled->nolink_blink_in_prog = false;
					pled->currledstate = LED_BLINK_SLOWLY;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
				}
				pled->scan_blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_TXRX_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}
			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					pled->nolink_blink_in_prog = true;
					pled->currledstate = LED_BLINK_SLOWLY;
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
				}
				pled->blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_BLINK_WPS:
			if (pled->led_on)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
			}
			else
			{
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
			}
			break;

		case LED_BLINK_WPS_STOP:	/* WPS authentication fail */
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;

			_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
			break;

		case LED_BLINK_WPS_STOP_OVERLAP:	/* WPS session overlap */
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				if (pled->led_on)
				{
					pled->blinktimes = 1;
				}
				else
				{
					bStopBlinking = true;
				}
			}

			if (bStopBlinking)
			{
				pled->blinktimes = 10;
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			}
			else
			{
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;

				_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
			}
			break;

		default:
			break;
	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("SwLedBlink4 currledstate %d\n", pled->currledstate));

}

static void SwLedBlink5(struct LED_871X *pled)
{
	struct rtw_adapter			*padapter = pled->padapter;
	u8				bStopBlinking = false;

	/*  Change LED according to blinkingledstate specified. */
	if (pled->blinkingledstate == RTW_LED_ON)
	{
		SwLedOn(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn on\n", pled->blinktimes));
	}
	else
	{
		SwLedOff(padapter, pled);
		RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Blinktimes (%d): turn off\n", pled->blinktimes));
	}

	switch (pled->currledstate)
	{
		case LED_SCAN_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;
					if (pled->led_on)
						SwLedOff(padapter, pled);
				}
				else
				{		pled->currledstate = RTW_LED_ON;
						pled->blinkingledstate = RTW_LED_ON;
						if (!pled->led_on)
							_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}

				pled->scan_blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
				}
			}
			break;

		case LED_TXRX_BLINK:
			pled->blinktimes--;
			if (pled->blinktimes == 0)
			{
				bStopBlinking = true;
			}

			if (bStopBlinking)
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					pled->currledstate = RTW_LED_OFF;
					pled->blinkingledstate = RTW_LED_OFF;
					if (pled->led_on)
						SwLedOff(padapter, pled);
				}
				else
				{
					pled->currledstate = RTW_LED_ON;
					pled->blinkingledstate = RTW_LED_ON;
					if (!pled->led_on)
						_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}

				pled->blink_in_prog = false;
			}
			else
			{
				if (padapter->pwrctrlpriv.rf_pwrstate != rf_on && padapter->pwrctrlpriv.rfoff_reason > RF_CHANGE_BY_PS)
				{
					SwLedOff(padapter, pled);
				}
				else
				{
					 if (pled->led_on)
						pled->blinkingledstate = RTW_LED_OFF;
					else
						pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
				}
			}
			break;

		default:
			break;
	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("SwLedBlink5 currledstate %d\n", pled->currledstate));

}

/*  */
/*	Description: */
/*		Callback function of LED blinktimer, */
/*		it just schedules to corresponding BlinkWorkItem. */
/*  */
static void
blinktimerCallback(
	unsigned long data
	)
{
	struct LED_871X *	 pled = (struct LED_871X *)data;
	struct rtw_adapter		*padapter = pled->padapter;

	 if ((padapter->bSurpriseRemoved == true) || (padapter->bDriverStopped == true))
       {
             return;
       }

	schedule_work(&(pled->BlinkWorkItem));
}

/*  */
/*	Description: */
/*		Callback function of LED BlinkWorkItem. */
/*		We dispatch acture LED blink action according to LedStrategy. */
/*  */
static void BlinkWorkItemCallback(struct work_struct *work)
{
	struct LED_871X *	 pled = container_of(work, struct LED_871X, BlinkWorkItem);
	struct led_priv	*ledpriv = &(pled->padapter->ledpriv);
	struct rtw_adapter		*padapter = pled->padapter;

	 if ((padapter->bSurpriseRemoved == true) || (padapter->bDriverStopped == true))
       {
             return;
       }

	switch (ledpriv->LedStrategy)
	{
		case SW_LED_MODE0:
			SwLedBlink(pled);
			break;

		case SW_LED_MODE1:
			SwLedBlink1(pled);
			break;

		case SW_LED_MODE2:
			SwLedBlink2(pled);
			break;

		case SW_LED_MODE3:
			SwLedBlink3(pled);
			break;

		case SW_LED_MODE4:
			SwLedBlink4(pled);
			break;

		case SW_LED_MODE5:
			SwLedBlink5(pled);
			break;

		default:
			SwLedBlink(pled);
			break;
	}
}

/*  */
/*  Default LED behavior. */
/*  */

/*  */
/*	Description: */
/*		Implement each led action for SW_LED_MODE0. */
/*		This is default strategy. */
/*  */
static void SwLedControlMode0(
	struct rtw_adapter		*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);
	struct LED_871X *	pled = &(ledpriv->SwLed1);

	/*  Decide led state */
	switch (LedAction)
	{
	case LED_CTL_TX:
	case LED_CTL_RX:
		if (pled->blink_in_prog == false)
		{
			pled->blink_in_prog = true;

			pled->currledstate = LED_BLINK_NORMAL;
			pled->blinktimes = 2;

			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
		}
		break;

	case LED_CTL_START_TO_LINK:
		if (pled->blink_in_prog == false)
		{
			pled->blink_in_prog = true;

			pled->currledstate = LED_BLINK_StartToBlink;
			pled->blinktimes = 24;

			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
		}
		else
		{
			pled->currledstate = LED_BLINK_StartToBlink;
		}
		break;

	case LED_CTL_LINK:
		pled->currledstate = RTW_LED_ON;
		if (pled->blink_in_prog == false)
		{
			SwLedOn(padapter, pled);
		}
		break;

	case LED_CTL_NO_LINK:
		pled->currledstate = RTW_LED_OFF;
		if (pled->blink_in_prog == false)
		{
			SwLedOff(padapter, pled);
		}
		break;

	case LED_CTL_POWER_OFF:
		pled->currledstate = RTW_LED_OFF;
		pled->blinkingledstate = RTW_LED_OFF;

		if (pled->blink_in_prog)
		{
			_cancel_timer_ex(&(pled->blinktimer));
			pled->blink_in_prog = false;
		}
		SwLedOff(padapter, pled);
		break;

	case LED_CTL_START_WPS:
		if (pled->blink_in_prog == false || pled->currledstate == RTW_LED_ON)
		{
			pled->blink_in_prog = true;

			pled->currledstate = LED_BLINK_WPS;
			pled->blinktimes = 20;

			if (pled->led_on)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_LONG_INTERVAL);
			}
			else
			{
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_LONG_INTERVAL);
			}
		}
		break;

	case LED_CTL_STOP_WPS:
		if (pled->blink_in_prog)
		{
			pled->currledstate = RTW_LED_OFF;
			_cancel_timer_ex(&(pled->blinktimer));
			pled->blink_in_prog = false;
		}
		break;

	default:
		break;
	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Led %d\n", pled->currledstate));
}

 /* ALPHA, added by chiyoko, 20090106 */
static void SwLedControlMode1(
	struct rtw_adapter		*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct hal_data_8192du		*pHalData = GET_HAL_DATA(padapter);
	struct led_priv		*ledpriv = &(padapter->ledpriv);
	struct LED_871X *			pled = &(ledpriv->SwLed0);
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);

	if (pHalData->CustomerID == RT_CID_819x_CAMEO)
		pled = &(ledpriv->SwLed1);

	switch (LedAction)
	{
		case LED_CTL_START_TO_LINK:
		case LED_CTL_NO_LINK:
			if (pled->nolink_blink_in_prog == false)
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}
				if (pled->link_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->link_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}

				pled->nolink_blink_in_prog = true;
				pled->currledstate = LED_BLINK_SLOWLY;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_LINK:
			if (pled->link_blink_in_prog == false)
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}
				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->link_blink_in_prog = true;
				pled->currledstate = LED_BLINK_NORMAL;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_LINK_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_SITE_SURVEY:
			 if ((pmlmepriv->LinkDetectInfo.bBusyTraffic) && (check_fwstate(pmlmepriv, _FW_LINKED)== true))
				;
			 else if (pled->scan_blink_in_prog ==false)
			 {
				if (IS_LED_WPS_BLINKING(pled))
					return;

				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				if (pled->link_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					 pled->link_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->scan_blink_in_prog = true;
				pled->currledstate = LED_SCAN_BLINK;
				pled->blinktimes = 24;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			 }
			break;

		case LED_CTL_TX:
		case LED_CTL_RX:
			if (pled->blink_in_prog ==false)
			{
                            if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
                            {
					return;
                            }
                            if (pled->nolink_blink_in_prog == true)
                            {
                                _cancel_timer_ex(&(pled->blinktimer));
                                pled->nolink_blink_in_prog = false;
                            }
                            if (pled->link_blink_in_prog == true)
                            {
                                _cancel_timer_ex(&(pled->blinktimer));
                                pled->link_blink_in_prog = false;
                            }
                            pled->blink_in_prog = true;
                            pled->currledstate = LED_TXRX_BLINK;
                            pled->blinktimes = 2;
                            if (pled->led_on)
                                pled->blinkingledstate = RTW_LED_OFF;
                            else
                                pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_START_WPS: /* wait until xinpin finish */
		case LED_CTL_START_WPS_BOTTON:
			 if (pled->wps_blink_in_prog ==false)
			 {
				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				if (pled->link_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					 pled->link_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				if (pled->scan_blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->scan_blink_in_prog = false;
				}
				pled->wps_blink_in_prog = true;
				pled->currledstate = LED_BLINK_WPS;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			 }
			break;

		case LED_CTL_STOP_WPS:
			if (pled->nolink_blink_in_prog == true)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->nolink_blink_in_prog = false;
			}
			if (pled->link_blink_in_prog == true)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				 pled->link_blink_in_prog = false;
			}
			if (pled->blink_in_prog ==true)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog ==true)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
			}
			else
			{
				pled->wps_blink_in_prog = true;
			}

			pled->currledstate = LED_BLINK_WPS_STOP;
			if (pled->led_on)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
			}
			else
			{
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), 0);
			}
			break;

		case LED_CTL_STOP_WPS_FAIL:
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			pled->nolink_blink_in_prog = true;
			pled->currledstate = LED_BLINK_SLOWLY;
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			break;

		case LED_CTL_POWER_OFF:
			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;

			if (pled->nolink_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->nolink_blink_in_prog = false;
			}
			if (pled->link_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->link_blink_in_prog = false;
			}
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}

			SwLedOff(padapter, pled);
			break;

		default:
			break;

	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Led %d\n", pled->currledstate));
}

 /* Arcadyan/Sitecom , added by chiyoko, 20090216 */
static void SwLedControlMode2(
	struct rtw_adapter				*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct led_priv	 *ledpriv = &(padapter->ledpriv);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct LED_871X *		pled = &(ledpriv->SwLed0);

	switch (LedAction)
	{
		case LED_CTL_SITE_SURVEY:
			 if (pmlmepriv->LinkDetectInfo.bBusyTraffic)
				;
			 else if (pled->scan_blink_in_prog ==false)
			 {
				if (IS_LED_WPS_BLINKING(pled))
					return;

				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->scan_blink_in_prog = true;
				pled->currledstate = LED_SCAN_BLINK;
				pled->blinktimes = 24;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			 }
			break;

		case LED_CTL_TX:
		case LED_CTL_RX:
			if ((pled->blink_in_prog ==false) && (check_fwstate(pmlmepriv, _FW_LINKED)== true))
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}

				pled->blink_in_prog = true;
				pled->currledstate = LED_TXRX_BLINK;
				pled->blinktimes = 2;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_LINK:
			pled->currledstate = RTW_LED_ON;
			pled->blinkingledstate = RTW_LED_ON;
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}

			_set_timer(&(pled->blinktimer), 0);
			break;

		case LED_CTL_START_WPS: /* wait until xinpin finish */
		case LED_CTL_START_WPS_BOTTON:
			if (pled->wps_blink_in_prog ==false)
			{
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				if (pled->scan_blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->scan_blink_in_prog = false;
				}
				pled->wps_blink_in_prog = true;
				pled->currledstate = RTW_LED_ON;
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), 0);
			 }
			break;

		case LED_CTL_STOP_WPS:
			pled->wps_blink_in_prog = false;
			if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
			{
				SwLedOff(padapter, pled);
			}
			else
			{
				pled->currledstate = RTW_LED_ON;
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), 0);
				RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
			}
			break;

		case LED_CTL_STOP_WPS_FAIL:
			pled->wps_blink_in_prog = false;
			if (padapter->pwrctrlpriv.rf_pwrstate != rf_on)
			{
				SwLedOff(padapter, pled);
			}
			else
			{
				pled->currledstate = RTW_LED_OFF;
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), 0);
				RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
			}
			break;

		case LED_CTL_START_TO_LINK:
		case LED_CTL_NO_LINK:
			if (!IS_LED_BLINKING(pled))
			{
				pled->currledstate = RTW_LED_OFF;
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), 0);
			}
			break;

		case LED_CTL_POWER_OFF:
			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			_set_timer(&(pled->blinktimer), 0);
			break;

		default:
			break;

	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
}

  /* COREGA, added by chiyoko, 20090316 */
static void SwLedControlMode3(
	struct rtw_adapter				*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct LED_871X *		pled = &(ledpriv->SwLed0);

	switch (LedAction)
	{
		case LED_CTL_SITE_SURVEY:
			if (pmlmepriv->LinkDetectInfo.bBusyTraffic)
				;
			else if (pled->scan_blink_in_prog ==false)
			{
				if (IS_LED_WPS_BLINKING(pled))
					return;

				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->scan_blink_in_prog = true;
				pled->currledstate = LED_SCAN_BLINK;
				pled->blinktimes = 24;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_TX:
		case LED_CTL_RX:
			if ((pled->blink_in_prog ==false) && (check_fwstate(pmlmepriv, _FW_LINKED)== true))
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}

				pled->blink_in_prog = true;
				pled->currledstate = LED_TXRX_BLINK;
				pled->blinktimes = 2;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_LINK:
			if (IS_LED_WPS_BLINKING(pled))
				return;

			pled->currledstate = RTW_LED_ON;
			pled->blinkingledstate = RTW_LED_ON;
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}

			_set_timer(&(pled->blinktimer), 0);
			break;

		case LED_CTL_START_WPS: /* wait until xinpin finish */
		case LED_CTL_START_WPS_BOTTON:
			if (pled->wps_blink_in_prog ==false)
			{
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				if (pled->scan_blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->scan_blink_in_prog = false;
				}
				pled->wps_blink_in_prog = true;
				pled->currledstate = LED_BLINK_WPS;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_STOP_WPS:
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}
			else
			{
				pled->wps_blink_in_prog = true;
			}

			pled->currledstate = LED_BLINK_WPS_STOP;
			if (pled->led_on)
			{
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), LED_BLINK_WPS_SUCESS_INTERVAL_ALPHA);
			}
			else
			{
				pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), 0);
			}

			break;

		case LED_CTL_STOP_WPS_FAIL:
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;
			_set_timer(&(pled->blinktimer), 0);
			break;

		case LED_CTL_START_TO_LINK:
		case LED_CTL_NO_LINK:
			if (!IS_LED_BLINKING(pled))
			{
				pled->currledstate = RTW_LED_OFF;
				pled->blinkingledstate = RTW_LED_OFF;
				_set_timer(&(pled->blinktimer), 0);
			}
			break;

		case LED_CTL_POWER_OFF:
			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			_set_timer(&(pled->blinktimer), 0);
			break;

		default:
			break;

	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("currledstate %d\n", pled->currledstate));
}

 /* Edimax-Belkin, added by chiyoko, 20090413 */
static void SwLedControlMode4(
	struct rtw_adapter				*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct LED_871X *		pled = &(ledpriv->SwLed0);
	struct LED_871X *		pled1 = &(ledpriv->SwLed1);

	switch (LedAction)
	{
		case LED_CTL_START_TO_LINK:
			if (pled1->wps_blink_in_prog)
			{
				pled1->wps_blink_in_prog = false;
				_cancel_timer_ex(&(pled1->blinktimer));

				pled1->blinkingledstate = RTW_LED_OFF;
				pled1->currledstate = RTW_LED_OFF;

				if (pled1->led_on)
					_set_timer(&(pled->blinktimer), 0);
			}

			if (pled->start_link_blink_in_prog == false)
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				if (pled->nolink_blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}

				pled->start_link_blink_in_prog = true;
				pled->currledstate = LED_BLINK_StartToBlink;
				if (pled->led_on)
				{
					pled->blinkingledstate = RTW_LED_OFF;
					_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
				}
				else
				{
					pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
				}
			}
			break;

		case LED_CTL_LINK:
		case LED_CTL_NO_LINK:
			/* LED1 settings */
			if (LedAction == LED_CTL_LINK)
			{
				if (pled1->wps_blink_in_prog)
				{
					pled1->wps_blink_in_prog = false;
					_cancel_timer_ex(&(pled1->blinktimer));

					pled1->blinkingledstate = RTW_LED_OFF;
					pled1->currledstate = RTW_LED_OFF;

					if (pled1->led_on)
						_set_timer(&(pled->blinktimer), 0);
				}
			}

			if (pled->nolink_blink_in_prog == false)
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}

				pled->nolink_blink_in_prog = true;
				pled->currledstate = LED_BLINK_SLOWLY;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_SITE_SURVEY:
			if ((pmlmepriv->LinkDetectInfo.bBusyTraffic) && (check_fwstate(pmlmepriv, _FW_LINKED)== true))
				;
			else if (pled->scan_blink_in_prog ==false)
			{
				if (IS_LED_WPS_BLINKING(pled))
					return;

				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->scan_blink_in_prog = true;
				pled->currledstate = LED_SCAN_BLINK;
				pled->blinktimes = 24;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_TX:
		case LED_CTL_RX:
			if (pled->blink_in_prog ==false)
			{
				if (pled->currledstate == LED_SCAN_BLINK || IS_LED_WPS_BLINKING(pled))
				{
					return;
				}
				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				pled->blink_in_prog = true;
				pled->currledstate = LED_TXRX_BLINK;
				pled->blinktimes = 2;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_START_WPS: /* wait until xinpin finish */
		case LED_CTL_START_WPS_BOTTON:
			if (pled1->wps_blink_in_prog)
			{
				pled1->wps_blink_in_prog = false;
				_cancel_timer_ex(&(pled1->blinktimer));

				pled1->blinkingledstate = RTW_LED_OFF;
				pled1->currledstate = RTW_LED_OFF;

				if (pled1->led_on)
					_set_timer(&(pled->blinktimer), 0);
			}

			if (pled->wps_blink_in_prog ==false)
			{
				if (pled->nolink_blink_in_prog == true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->nolink_blink_in_prog = false;
				}
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				if (pled->scan_blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->scan_blink_in_prog = false;
				}
				pled->wps_blink_in_prog = true;
				pled->currledstate = LED_BLINK_WPS;
				if (pled->led_on)
				{
					pled->blinkingledstate = RTW_LED_OFF;
					_set_timer(&(pled->blinktimer), LED_BLINK_SLOWLY_INTERVAL);
				}
				else
				{
					pled->blinkingledstate = RTW_LED_ON;
					_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);
				}
			}
			break;

		case LED_CTL_STOP_WPS:	/* WPS connect success */
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			pled->nolink_blink_in_prog = true;
			pled->currledstate = LED_BLINK_SLOWLY;
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);

			break;

		case LED_CTL_STOP_WPS_FAIL:		/* WPS authentication fail */
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			pled->nolink_blink_in_prog = true;
			pled->currledstate = LED_BLINK_SLOWLY;
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);

			/* LED1 settings */
			if (pled1->wps_blink_in_prog)
				_cancel_timer_ex(&(pled1->blinktimer));
			else
				pled1->wps_blink_in_prog = true;

			pled1->currledstate = LED_BLINK_WPS_STOP;
			if (pled1->led_on)
				pled1->blinkingledstate = RTW_LED_OFF;
			else
				pled1->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);

			break;

		case LED_CTL_STOP_WPS_FAIL_OVERLAP:	/* WPS session overlap */
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}

			pled->nolink_blink_in_prog = true;
			pled->currledstate = LED_BLINK_SLOWLY;
			if (pled->led_on)
				pled->blinkingledstate = RTW_LED_OFF;
			else
				pled->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NO_LINK_INTERVAL_ALPHA);

			/* LED1 settings */
			if (pled1->wps_blink_in_prog)
				_cancel_timer_ex(&(pled1->blinktimer));
			else
				pled1->wps_blink_in_prog = true;

			pled1->currledstate = LED_BLINK_WPS_STOP_OVERLAP;
			pled1->blinktimes = 10;
			if (pled1->led_on)
				pled1->blinkingledstate = RTW_LED_OFF;
			else
				pled1->blinkingledstate = RTW_LED_ON;
			_set_timer(&(pled->blinktimer), LED_BLINK_NORMAL_INTERVAL);

			break;

		case LED_CTL_POWER_OFF:
			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;

			if (pled->nolink_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->nolink_blink_in_prog = false;
			}
			if (pled->link_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->link_blink_in_prog = false;
			}
			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}
			if (pled->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->wps_blink_in_prog = false;
			}
			if (pled->scan_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->scan_blink_in_prog = false;
			}
			if (pled->start_link_blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->start_link_blink_in_prog = false;
			}

			if (pled1->wps_blink_in_prog)
			{
				_cancel_timer_ex(&(pled1->blinktimer));
				pled1->wps_blink_in_prog = false;
			}

			pled1->blinkingledstate = LED_UNKNOWN;
			SwLedOff(padapter, pled);
			SwLedOff(padapter, pled1);
			break;

		default:
			break;

	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Led %d\n", pled->currledstate));
}

/* Sercomm-Belkin, added by chiyoko, 20090415 */
static void SwLedControlMode5(
	struct rtw_adapter				*padapter,
	enum LED_CTL_MODE		LedAction
)
{
	struct hal_data_8192du	*pHalData = GET_HAL_DATA(padapter);
	struct led_priv	*ledpriv = &(padapter->ledpriv);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct LED_871X *		pled = &(ledpriv->SwLed0);

	if (pHalData->EEPROMCustomerID == RT_CID_819x_CAMEO)
		pled = &(ledpriv->SwLed1);

	switch (LedAction)
	{
		case LED_CTL_POWER_ON:
		case LED_CTL_NO_LINK:
		case LED_CTL_LINK:	/* solid blue */
			pled->currledstate = RTW_LED_ON;
			pled->blinkingledstate = RTW_LED_ON;

			_set_timer(&(pled->blinktimer), 0);
			break;

		case LED_CTL_SITE_SURVEY:
			if ((pmlmepriv->LinkDetectInfo.bBusyTraffic) && (check_fwstate(pmlmepriv, _FW_LINKED)== true))
				;
			else if (pled->scan_blink_in_prog ==false)
			{
				if (pled->blink_in_prog ==true)
				{
					_cancel_timer_ex(&(pled->blinktimer));
					pled->blink_in_prog = false;
				}
				pled->scan_blink_in_prog = true;
				pled->currledstate = LED_SCAN_BLINK;
				pled->blinktimes = 24;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_SCAN_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_TX:
		case LED_CTL_RX:
			if (pled->blink_in_prog ==false)
			{
				if (pled->currledstate == LED_SCAN_BLINK)
				{
					return;
				}
				pled->blink_in_prog = true;
				pled->currledstate = LED_TXRX_BLINK;
				pled->blinktimes = 2;
				if (pled->led_on)
					pled->blinkingledstate = RTW_LED_OFF;
				else
					pled->blinkingledstate = RTW_LED_ON;
				_set_timer(&(pled->blinktimer), LED_BLINK_FASTER_INTERVAL_ALPHA);
			}
			break;

		case LED_CTL_POWER_OFF:
			pled->currledstate = RTW_LED_OFF;
			pled->blinkingledstate = RTW_LED_OFF;

			if (pled->blink_in_prog)
			{
				_cancel_timer_ex(&(pled->blinktimer));
				pled->blink_in_prog = false;
			}

			SwLedOff(padapter, pled);
			break;

		default:
			break;

	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("Led %d\n", pled->currledstate));
}

/*  */
/*	Description: */
/*		Dispatch LED action according to pHalData->LedStrategy. */
/*  */
static void LedControl871x(
	struct rtw_adapter				*padapter,
	enum LED_CTL_MODE		LedAction
	)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);

       if ((padapter->bSurpriseRemoved == true) || (padapter->bDriverStopped == true))
       {
             return;
       }

	if (ledpriv->bRegUseLed == false)
		return;

	if (	padapter->pwrctrlpriv.rf_pwrstate != rf_on &&
		(LedAction == LED_CTL_TX || LedAction == LED_CTL_RX ||
		 LedAction == LED_CTL_SITE_SURVEY ||
		 LedAction == LED_CTL_LINK ||
		 LedAction == LED_CTL_NO_LINK ||
		 LedAction == LED_CTL_POWER_ON))
	{
		return;
	}

	switch (ledpriv->LedStrategy)
	{
		case SW_LED_MODE0:
			break;

		case SW_LED_MODE1:
			SwLedControlMode1(padapter, LedAction);
			break;

		case SW_LED_MODE2:
			SwLedControlMode2(padapter, LedAction);
			break;

		case SW_LED_MODE3:
			SwLedControlMode3(padapter, LedAction);
			break;

		case SW_LED_MODE4:
			SwLedControlMode4(padapter, LedAction);
			break;

		case SW_LED_MODE5:
			SwLedControlMode5(padapter, LedAction);
			break;

		default:
			break;
	}

	RT_TRACE(_module_rtl8712_led_c_,_drv_info_,("LedStrategy:%d, LedAction %d\n", ledpriv->LedStrategy,LedAction));
}

/*  */
/*  Interface to manipulate LED objects. */
/*  */

/*  */
/*	Description: */
/*		Initialize all struct LED_871X objects. */
/*  */
void rtl8192du_InitSwLeds(struct rtw_adapter	*padapter)
{
	struct led_priv *pledpriv = &(padapter->ledpriv);

	pledpriv->LedControlHandler = LedControl871x;

	InitLed871x(padapter, &(pledpriv->SwLed0), LED_PIN_LED0);

	InitLed871x(padapter,&(pledpriv->SwLed1), LED_PIN_LED1);
}

/*  */
/*	Description: */
/*		DeInitialize all LED_819xUsb objects. */
/*  */
void rtl8192du_DeInitSwLeds(struct rtw_adapter *padapter)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);

	DeInitLed871x(&(ledpriv->SwLed0));
	DeInitLed871x(&(ledpriv->SwLed1));
}
