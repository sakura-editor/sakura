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
#include "env/DLLSHAREDATA.h"
#include "env/CDocTypeManager.h"

void CPropertyManager::Create( HWND hwndOwner, CImageListMgr* pImageList, CMenuDrawer* menu )
{
	/* 設定プロパティシートの初期化１ */
	m_cPropCommon.Create( hwndOwner, pImageList, menu );
	m_cPropTypes.Create( G_AppInstance(), hwndOwner );
}

/*! 共通設定 プロパティシート */
bool CPropertyManager::OpenPropertySheet( HWND hWnd, int nPageNum )
{
	// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
	// 共通設定の一時設定領域にSharaDataをコピーする
	m_cPropCommon.InitData();

	/* プロパティシートの作成 */
	if( m_cPropCommon.DoPropertySheet( nPageNum ) ){

		// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		// 2007.06.20 ryoji グループ化に変更があったときはグループIDをリセットする
		BOOL bGroup = (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin);

		// 印刷中にキーワードを上書きしないように
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( m_cPropCommon.m_hwndParent, &pLock );

		m_cPropCommon.ApplyData();
		// note: 基本的にここで適用しないで、MYWM_CHANGESETTINGからたどって適用してください。
		// 自ウィンドウには最後に通知されます。大抵は、OnChangeSetting にあります。
		// ここでしか適用しないと、ほかのウィンドウが変更されません。

		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CAppNodeManager::getInstance()->ResetGroupId();
		}

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		return true;
	}else{
		return false;
	}
}



/*! タイプ別設定 プロパティシート */
bool CPropertyManager::OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType )
{
	STypeConfig& types = CDocTypeManager().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように

	/* プロパティシートの作成 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		// 2013.06.10 Moca 印刷終了まで待機する
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( m_cPropTypes.GetHwndParent(), &pLock );

		m_cPropTypes.GetTypeData( types );

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		return true;
	}else{
		return false;
	}
}

