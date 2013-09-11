/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CPropertyManager.h"
#include "CShareData.h"
#include "mymessage.h"

void CPropertyManager::Create( HINSTANCE hInstance, HWND hwndOwner, CImageListMgr* pImageList, CMenuDrawer* menu )
{
	/* 設定プロパティシートの初期化１ */
	m_cPropCommon.Create( hInstance, hwndOwner, pImageList, menu );
	m_cPropTypes.Create( hInstance, hwndOwner );
}

/*! 共通設定 プロパティシート */
bool CPropertyManager::OpenPropertySheet( HWND hWnd, int nPageNum )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
	// 共通設定の一時設定領域にSharaDataをコピーする
	m_cPropCommon.InitData();
	
	/* プロパティシートの作成 */
	if( m_cPropCommon.DoPropertySheet( nPageNum ) ){

		// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		// 2007.06.20 ryoji グループ化に変更があったときはグループIDをリセットする
		BOOL bGroup = (pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin);

		m_cPropCommon.ApplyData();
		// note: 基本的にここで適用しないで、MYWM_CHANGESETTINGからたどって適用してください。
		// 自ウィンドウには最後に通知されます。大抵は、OnChangeSetting にあります。
		// ここでしか適用しないと、ほかのウィンドウが変更されません。
		
		if( bGroup != (pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* アクセラレータテーブルの再作成 */
		::SendMessage( pShareData->m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		return true;
	}else{
		return false;
	}
}



/*! タイプ別設定 プロパティシート */
bool CPropertyManager::OpenPropertySheetTypes( HWND hWnd, int nPageNum, int nSettingType )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	STypeConfig& types = pShareData->m_Types[nSettingType];
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように

	/* プロパティシートの作成 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){

		m_cPropTypes.GetTypeData( types );

		/* アクセラレータテーブルの再作成 */
		::SendMessage( pShareData->m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		return true;
	}else{
		return false;
	}
}

