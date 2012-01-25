#include "StdAfx.h"
#include "CPropertyManager.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"
#include "env/DLLSHAREDATA.h"
#include "macro/CSMacroMgr.h"

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
		// note: 基本的にここで適用しないで、MYWM_CHANGESETTINGからたどって適用してください。
		// 自ウィンドウには最後に通知されます。大抵は、OnChangeSetting にあります。
		// ここでしか適用しないと、ほかのウィンドウが変更されません。
		
		CEditApp::Instance()->m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		if( bGroup != (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CAppNodeManager::Instance()->ResetGroupId();
		}

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );


		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
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
	STypeConfig& types = CDocTypeManager().GetTypeSetting(nSettingType);
	m_cPropTypes.SetTypeData( types );
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように
	//m_cPropTypes.m_CKeyWordSetMgr = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;

	/* プロパティシートの作成 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* 変更された設定値のコピー */
		int nTextWrapMethodOld = CEditWnd::Instance()->GetDocument().m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;
		m_cPropTypes.GetTypeData( types );

		// 2008.06.01 nasukoji	テキストの折り返し位置変更対応
		// タイプ別設定を呼び出したウィンドウについては、タイプ別設定が変更されたら
		// 折り返し方法の一時設定適用中を解除してタイプ別設定を有効とする。
		if( nTextWrapMethodOld != CEditWnd::Instance()->GetDocument().m_cDocType.GetDocumentAttribute().m_nTextWrapMethod )		// 設定が変更された
			CEditWnd::Instance()->GetDocument().m_bTextWrapMethodCurTemp = false;	// 一時設定適用中を解除

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( GetDllShareData().m_sHandles.m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CAppNodeGroupHandle(0).SendMessageToAllEditors(
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

