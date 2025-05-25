/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CPropertyManager.h"
#include "env/DLLSHAREDATA.h"
#include "env/CDocTypeManager.h"
#include "apiwrap/StdApi.h"
#include <memory>
#include "config/system_constants.h"

void CPropertyManager::Create( HWND hwndOwner, CImageListMgr* pImageList, CMenuDrawer* pMenuDrawer )
{
	m_hwndOwner = hwndOwner;
	m_pImageList = pImageList;
	m_pMenuDrawer = pMenuDrawer;

	m_nPropComPageNum = -1;
	m_nPropTypePageNum = -1;
}

/*! 共通設定 プロパティシート */
bool CPropertyManager::OpenPropertySheet( HWND hWnd, int nPageNum, bool bTrayProc )
{
	bool bRet;
	CPropCommon* pcPropCommon = new CPropCommon();
	pcPropCommon->Create( m_hwndOwner, m_pImageList, m_pMenuDrawer );

	// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
	// 共通設定の一時設定領域にSharaDataをコピーする
	pcPropCommon->InitData();

	if( nPageNum != -1 ){
		m_nPropComPageNum = nPageNum;
	}

	/* プロパティシートの作成 */
	if( pcPropCommon->DoPropertySheet( m_nPropComPageNum, bTrayProc ) ){

		// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		// 2007.06.20 ryoji グループ化に変更があったときはグループIDをリセットする
		BOOL bGroup = (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin);

		// 印刷中にキーワードを上書きしないように
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( pcPropCommon->m_hwndParent, &pLock );

		pcPropCommon->ApplyData();
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
		bRet = true;
	}else{
		bRet = false;
	}

	// 最後にアクセスしたシートを覚えておく
	m_nPropComPageNum = pcPropCommon->GetPageNum();

	delete pcPropCommon;

	return bRet;
}

/*! タイプ別設定 プロパティシート */
bool CPropertyManager::OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType )
{
	bool bRet;
	CPropTypes* pcPropTypes = new CPropTypes();
	pcPropTypes->Create( G_AppInstance(), m_hwndOwner );

	auto pType = std::make_unique<STypeConfig>();
	CDocTypeManager().GetTypeConfig(nSettingType, *pType);
	pcPropTypes->SetTypeData(*pType);
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように

	if( nPageNum != -1 ){
		m_nPropTypePageNum = nPageNum;
	}

	/* プロパティシートの作成 */
	if( pcPropTypes->DoPropertySheet( m_nPropTypePageNum ) ){
		// 2013.06.10 Moca 印刷終了まで待機する
		CShareDataLockCounter* pLock = NULL;
		CShareDataLockCounter::WaitLock( pcPropTypes->GetHwndParent(), &pLock );

		pcPropTypes->GetTypeData(*pType);

		CDocTypeManager().SetTypeConfig(nSettingType, *pType);

		/* アクセラレータテーブルの再作成 */
		// ::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nSettingType.GetIndex(),
			(LPARAM)PM_CHANGESETTING_TYPE,
			hWnd
		);
		if( pcPropTypes->GetChangeKeyWordSet() ){
			CAppNodeGroupHandle(0).SendMessageToAllEditors(
				WM_COMMAND,
				(WPARAM)MAKELONG( F_REDRAW, 0 ),
				(LPARAM)0,
				hWnd
			);
		}

		delete pLock;
		bRet = true;
	}else{
		bRet = false;
	}

	// 最後にアクセスしたシートを覚えておく
	m_nPropTypePageNum = pcPropTypes->GetPageNum();

	delete pcPropTypes;

	return bRet;
}
