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

#define _MLME_OSDEP_C_

#include <autoconf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <mlme_osdep.h>

void rtw_join_timeout_handler (void *FunctionContext)
{
	struct rtw_adapter *adapter = (struct rtw_adapter *)FunctionContext;
	_rtw_join_timeout_handler(adapter);
}

void _rtw_scan_timeout_handler (void *FunctionContext)
{
	struct rtw_adapter *adapter = (struct rtw_adapter *)FunctionContext;
	rtw_scan_timeout_handler(adapter);
}

static void _dynamic_check_timer_handlder (void *FunctionContext)
{
	struct rtw_adapter *adapter = (struct rtw_adapter *)FunctionContext;

	rtw_dynamic_check_timer_handlder(adapter);

	_set_timer(&adapter->mlmepriv.dynamic_chk_timer, 2000);
}

#ifdef CONFIG_IOCTL_CFG80211
static void _rtw_set_scan_deny_timer_hdl(void *FunctionContext)
{
	struct rtw_adapter *adapter = (struct rtw_adapter *)FunctionContext;
	rtw_set_scan_deny_timer_hdl(adapter);
}
#endif

void rtw_init_mlme_timer(struct rtw_adapter *padapter)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_init_timer(&(pmlmepriv->assoc_timer), padapter->pnetdev, rtw_join_timeout_handler, padapter);
	_init_timer(&(pmlmepriv->scan_to_timer), padapter->pnetdev, _rtw_scan_timeout_handler, padapter);

	_init_timer(&(pmlmepriv->dynamic_chk_timer), padapter->pnetdev, _dynamic_check_timer_handlder, padapter);

	#ifdef CONFIG_IOCTL_CFG80211
	_init_timer(&(pmlmepriv->set_scan_deny_timer), padapter->pnetdev, _rtw_set_scan_deny_timer_hdl, padapter);
	#endif
}

void rtw_os_indicate_connect(struct rtw_adapter *adapter)
{

#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_indicate_connect(adapter);
#endif /* CONFIG_IOCTL_CFG80211 */

	rtw_indicate_wx_assoc_event(adapter);
	netif_carrier_on(adapter->pnetdev);

	if (adapter->pid[2] !=0)
		rtw_signal_process(adapter->pid[2], SIGALRM);

}

void rtw_os_indicate_scan_done(struct rtw_adapter *padapter, bool aborted)
{
#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_indicate_scan_done(wdev_to_priv(padapter->rtw_wdev), aborted);
#endif
	indicate_wx_scan_complete_event(padapter);
}

static struct RT_PMKID_LIST   backupPMKIDList[NUM_PMKID_CACHE];
void rtw_reset_securitypriv(struct rtw_adapter *adapter)
{
	u8	backupPMKIDIndex = 0;
	u8	backupTKIPCountermeasure = 0x00;
	u32	backupTKIPcountermeasure_time = 0;

	if (adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)/* 802.1x */
	{
		/*  Added by Albert 2009/02/18 */
		/*  We have to backup the PMK information for WiFi PMK Caching test item. */
		/*  */
		/*  Backup the btkip_countermeasure information. */
		/*  When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds. */

		memset(&backupPMKIDList[0], 0x00, sizeof(struct RT_PMKID_LIST) * NUM_PMKID_CACHE);

		memcpy(&backupPMKIDList[0], &adapter->securitypriv.PMKIDList[0], sizeof(struct RT_PMKID_LIST) * NUM_PMKID_CACHE);
		backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
		backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;
		backupTKIPcountermeasure_time = adapter->securitypriv.btkip_countermeasure_time;

		memset((unsigned char *)&adapter->securitypriv, 0, sizeof (struct security_priv));

		/*  Added by Albert 2009/02/18 */
		/*  Restore the PMK information to securitypriv structure for the following connection. */
		memcpy(&adapter->securitypriv.PMKIDList[0], &backupPMKIDList[0], sizeof(struct RT_PMKID_LIST) * NUM_PMKID_CACHE);
		adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
		adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;
		adapter->securitypriv.btkip_countermeasure_time = backupTKIPcountermeasure_time;

		adapter->securitypriv.ndisauthtype = NDIS802_11AUTHMODEOPEN;
		adapter->securitypriv.ndisencryptstatus = NDIS802_11WEPDISABLED;

	}
	else /* reset values in securitypriv */
	{
		struct security_priv *psec_priv=&adapter->securitypriv;

		psec_priv->dot11AuthAlgrthm =dot11AuthAlgrthm_Open;  /* open system */
		psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psec_priv->dot11PrivacyKeyIndex = 0;

		psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
		psec_priv->dot118021XGrpKeyid = 1;

		psec_priv->ndisauthtype = NDIS802_11AUTHMODEOPEN;
		psec_priv->ndisencryptstatus = NDIS802_11WEPDISABLED;
		/*  */
	}
}

void rtw_os_indicate_disconnect(struct rtw_adapter *adapter)
{

	netif_carrier_off(adapter->pnetdev); /*  Do it first for tx broadcast pkt after disconnection issue! */

#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_indicate_disconnect(adapter);
#endif /* CONFIG_IOCTL_CFG80211 */

	rtw_indicate_wx_disassoc_event(adapter);

	 rtw_reset_securitypriv(adapter);

}

void rtw_report_sec_ie(struct rtw_adapter *adapter,u8 authmode,u8 *sec_ie)
{
	uint	len;
	u8	*buff,*p,i;
	union iwreq_data wrqu;

	RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("+rtw_report_sec_ie, authmode=%d\n", authmode));

	buff = NULL;
	if (authmode==_WPA_IE_ID_) {
		RT_TRACE(_module_mlme_osdep_c_,_drv_info_,("rtw_report_sec_ie, authmode=%d\n", authmode));

		buff = kmalloc(IW_CUSTOM_MAX, GFP_KERNEL);

		memset(buff,0,IW_CUSTOM_MAX);

		p=buff;

		p+=sprintf(p,"ASSOCINFO(ReqIEs=");

		len = sec_ie[1]+2;
		len =  (len < IW_CUSTOM_MAX) ? len:IW_CUSTOM_MAX;

		for (i=0;i<len;i++) {
			p+=sprintf(p,"%02x",sec_ie[i]);
		}

		p+=sprintf(p,")");

		memset(&wrqu,0,sizeof(wrqu));

		wrqu.data.length=p-buff;

		wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

		wireless_send_event(adapter->pnetdev,IWEVCUSTOM,&wrqu,buff);

		kfree(buff);

	}

}

static void _survey_timer_hdl (void *FunctionContext)
{
	struct rtw_adapter *padapter = (struct rtw_adapter *)FunctionContext;

	survey_timer_hdl(padapter);
}

static void _link_timer_hdl (void *FunctionContext)
{
	struct rtw_adapter *padapter = (struct rtw_adapter *)FunctionContext;
	link_timer_hdl(padapter);
}

static void _addba_timer_hdl(void *FunctionContext)
{
	struct sta_info *psta = (struct sta_info *)FunctionContext;
	addba_timer_hdl(psta);
}

void init_addba_retry_timer(struct rtw_adapter *padapter, struct sta_info *psta)
{

	_init_timer(&psta->addba_retry_timer, padapter->pnetdev, _addba_timer_hdl, psta);
}

/*
void _reauth_timer_hdl(void *FunctionContext)
{
	struct rtw_adapter *padapter = (struct rtw_adapter *)FunctionContext;
	reauth_timer_hdl(padapter);
}

void _reassoc_timer_hdl(void *FunctionContext)
{
	struct rtw_adapter *padapter = (struct rtw_adapter *)FunctionContext;
	reassoc_timer_hdl(padapter);
}
*/

void init_mlme_ext_timer(struct rtw_adapter *padapter)
{
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	_init_timer(&pmlmeext->survey_timer, padapter->pnetdev, _survey_timer_hdl, padapter);
	_init_timer(&pmlmeext->link_timer, padapter->pnetdev, _link_timer_hdl, padapter);
}

#ifdef CONFIG_92D_AP_MODE

void rtw_indicate_sta_assoc_event(struct rtw_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (psta==NULL)
		return;

	if (psta->aid > NUM_STA)
		return;

	if (pstapriv->sta_aid[psta->aid - 1] != psta)
		return;

	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_8192D("+rtw_indicate_sta_assoc_event\n");

	wireless_send_event(padapter->pnetdev, IWEVREGISTERED, &wrqu, NULL);
}

void rtw_indicate_sta_disassoc_event(struct rtw_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (psta==NULL)
		return;

	if (psta->aid > NUM_STA)
		return;

	if (pstapriv->sta_aid[psta->aid - 1] != psta)
		return;

	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	DBG_8192D("+rtw_indicate_sta_disassoc_event\n");

	wireless_send_event(padapter->pnetdev, IWEVEXPIRED, &wrqu, NULL);
}

#endif
