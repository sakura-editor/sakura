#include "stdafx.h"
#include "CPropertyManager.h"
#include "mymessage.h"

CPropertyManager::CPropertyManager()
{
	/* 設定プロパティシートの初期化１ */
	m_cPropCommon.Create( CEditWnd::Instance()->GetHwnd(), &CEditApp::Instance()->GetIcons(), CEditApp::Instance()->m_pcSMacroMgr, &CEditWnd::Instance()->GetMenuDrawer() );
	m_cPropTypes.Create( G_AppInstance(), CEditWnd::Instance()->GetHwnd() );
}

/*! 共通設定 プロパティシート */
BOOL CPropertyManager::OpenPropertySheet( int nPageNum )
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
		m_cPropCommon.ApplyData();
		CEditApp::Instance()->m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* フォントが変わった */
		for( int i = 0; i < 4; ++i ){
			CEditApp::Instance()->m_pcEditWnd->m_pcEditViewArr[i]->m_cTipWnd.ChangeFont( &(GetDllShareData().m_Common.m_sHelper.m_lf_kh) );
		}

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! タイプ別設定 プロパティシート */
BOOL CPropertyManager::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	STypeConfig& types = GetDllShareData().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように
	//m_cPropTypes.m_CKeyWordSetMgr = GetDllShareData().m_CKeyWordSetMgr;

	/* プロパティシートの作成 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* 変更された設定値のコピー */
		m_cPropTypes.GetTypeData( types );

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)CEditWnd::Instance()->GetHwnd(),
			CEditWnd::Instance()->GetHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}

