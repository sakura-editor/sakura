/*!	@file
	@brief 置換ダイアログ

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, Stonee, hor, YAZAKI
	Copyright (C) 2002, MIK, hor, novice, genta, aroka, YAZAKI
	Copyright (C) 2006, かろと, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <WindowsX.h>	//  ダイアログコントロール用マクロ定義
#include "dlg/CDlgReplace.h"
#include "view/CEditView.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//置換 CDlgReplace.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//11900
	IDC_BUTTON_SEARCHNEXT,			HIDC_REP_BUTTON_SEARCHNEXT,			//下検索
	IDC_BUTTON_SEARCHPREV,			HIDC_REP_BUTTON_SEARCHPREV,			//上検索
	IDC_BUTTON_REPALCE,				HIDC_REP_BUTTON_REPALCE,			//置換
	IDC_BUTTON_REPALCEALL,			HIDC_REP_BUTTON_REPALCEALL,			//全置換
	IDCANCEL,						HIDCANCEL_REP,						//キャンセル
	IDC_BUTTON_HELP,				HIDC_REP_BUTTON_HELP,				//ヘルプ
	IDC_CHK_PASTE,					HIDC_REP_CHK_PASTE,					//クリップボードから貼り付け
	IDC_CHK_WORD,					HIDC_REP_CHK_WORD,					//単語単位
	IDC_CHK_LOHICASE,				HIDC_REP_CHK_LOHICASE,				//大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_REP_CHK_REGULAREXP,			//正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_REP_CHECK_NOTIFYNOTFOUND,		//見つからないときに通知
	IDC_CHECK_bAutoCloseDlgReplace,	HIDC_REP_CHECK_bAutoCloseDlgReplace,	//自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_REP_COMBO_TEXT,				//置換前
	IDC_COMBO_TEXT2,				HIDC_REP_COMBO_TEXT2,				//置換後
	IDC_RADIO_REPLACE,				HIDC_REP_RADIO_REPLACE,				//置換対象：置換
	IDC_RADIO_INSERT,				HIDC_REP_RADIO_INSERT,				//置換対象：挿入
	IDC_RADIO_ADD,					HIDC_REP_RADIO_ADD,					//置換対象：追加
	IDC_RADIO_SELECTEDAREA,			HIDC_REP_RADIO_SELECTEDAREA,		//範囲：全体
	IDC_RADIO_ALLAREA,				HIDC_REP_RADIO_ALLAREA,				//範囲：選択範囲
	IDC_STATIC_JRE32VER,			HIDC_REP_STATIC_JRE32VER,			//正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			//2002.01.16 hor 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			//2002.01.26 hor 先頭（末尾）から再検索
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgReplace::CDlgReplace()
{
	m_sSearchOption.Reset();	// 検索オプション
	/*
	m_bLoHiCase = FALSE;		// 英大文字と英小文字を区別する
	m_bWordOnly = FALSE;		// 一致する単語のみ検索する
	m_bRegularExp = FALSE;		// 正規表現
	*/
	m_bConsecutiveAll = FALSE;	// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	m_bSelectedArea = FALSE;	// 選択範囲内置換
	m_szText[0] = L'\0';			// 検索文字列
	m_szText2[0] = L'\0';		// 置換後文字列
	m_nReplaceTarget = 0;		// 置換対象		// 2001.12.03 hor
	m_nPaste = FALSE;			// 貼り付ける？	// 2001.12.03 hor
	m_nReplaceCnt = 0;			//すべて置換の実行結果		// 2002.02.08 hor
	m_bCanceled = false;		//すべて置換を中断したか	// 2002.02.08 hor
	return;
}

/* モードレスダイアログの表示 */
HWND CDlgReplace::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, BOOL bSelected )
{
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;		// 検索オプション
	m_bConsecutiveAll = m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll;	// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	m_bSelectedArea = m_pShareData->m_Common.m_sSearch.m_bSelectedArea;		// 選択範囲内置換
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示
	m_bSelected = bSelected;
	m_ptEscCaretPos_PHY = ((CEditView*)lParam)->GetCaret().GetCaretLogicPos();	// 検索/置換開始時のカーソル位置退避
	((CEditView*)lParam)->m_bSearch = TRUE;							// 検索/置換開始位置の登録有無			02/07/28 ai
	return CDialog::DoModeless( hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW );
}

/* モードレス時：置換・検索対象となるビューの変更 */
void CDlgReplace::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}




/* ダイアログデータの設定 */
void CDlgReplace::SetData( void )
{
	// 検索文字列/置換後文字列リストの設定(関数化)	2010/5/26 Uchi
	SetCombosList();

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 N.Nakatani
	/* 単語単位で探す */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly );

	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
	::CheckDlgButton( GetHwnd(), IDC_CHECK_CONSECUTIVEALL, m_bConsecutiveAll );

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );

		/*「すべて置換」は置換の繰返し */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), FALSE );	// 2007.01.16 ryoji
	}
	// To Here Jun. 29, 2001 genta

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );


	/* 置換 ダイアログを自動的に閉じる */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgReplace, m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_sSearch.m_bSearchAll );

	// From Here 2001.12.03 hor
	// クリップボードから貼り付ける？
	::CheckDlgButton( GetHwnd(), IDC_CHK_PASTE, m_nPaste );
	// 置換対象
	if(m_nReplaceTarget==0){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_REPLACE, TRUE );
	}else
	if(m_nReplaceTarget==1){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_INSERT, TRUE );
	}else
	if(m_nReplaceTarget==2){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ADD, TRUE );
	}
	// To Here 2001.12.03 hor

	return;
}



// 検索文字列/置換後文字列リストの設定
//	2010/5/26 Uchi
void CDlgReplace::SetCombosList( void )
{
	int		i;
	HWND	hwndCombo;
	TCHAR	szBuff[_MAX_PATH+1];

	/* 検索文字列 */
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT );
	while (ComboBox_GetCount(hwndCombo) > 0) {
		ComboBox_DeleteString( hwndCombo, 0);
	}
	for (i = 0; i < m_pShareData->m_sSearchKeywords.m_aSearchKeys.size(); ++i) {
		Combo_AddString( hwndCombo, m_pShareData->m_sSearchKeywords.m_aSearchKeys[i] );
	}
	ComboBox_GetText( hwndCombo, szBuff, _MAX_PATH);
	if (auto_strcmp( to_wchar(szBuff), m_szText ) != 0) {
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szText );
	}

	/* 置換後文字列 */
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 );
	while (ComboBox_GetCount(hwndCombo) > 0) {
		ComboBox_DeleteString( hwndCombo, 0);
	}
	for (i = 0; i < m_pShareData->m_sSearchKeywords.m_aReplaceKeys.size(); ++i) {
		Combo_AddString( hwndCombo, m_pShareData->m_sSearchKeywords.m_aReplaceKeys[i] );
	}
	ComboBox_GetText( hwndCombo, szBuff, _MAX_PATH);
	if (auto_strcmp( to_wchar(szBuff), m_szText2 ) != 0) {
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT2, m_szText2 );
	}
}


/* ダイアログデータの取得 */
/* 0==条件未入力  0より大きい==正常   0より小さい==入力エラー */
int CDlgReplace::GetData( void )
{
	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	// 2001/06/23 N.Nakatani
	/* 単語単位で探す */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
	m_bConsecutiveAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_CONSECUTIVEALL );

	/* 正規表現 */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));
	/* 選択範囲内置換 */
	m_bSelectedArea = ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_SELECTEDAREA );
	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// 検索オプション
	m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = m_bConsecutiveAll;	// 1==「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	m_pShareData->m_Common.m_sSearch.m_bSelectedArea = m_bSelectedArea;		// 選択範囲内置換
	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示

	/* 検索文字列 */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, m_szText, _countof(m_szText));
	/* 置換後文字列 */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT2, m_szText2, _countof(m_szText2));

	/* 置換 ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgReplace );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_sSearch.m_bSearchAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL );

	if( 0 < wcslen( m_szText ) ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( m_sSearchOption.bRegularExp && !CheckRegexpSyntax( m_szText, GetHwnd(), true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()追加に伴う変更
		CSearchKeywordManager().AddToSearchKeyArr( m_szText );

		/* 置換後文字列 */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeyArr()追加に伴う変更
		CSearchKeywordManager().AddToReplaceKeyArr( m_szText2 );

		// From Here 2001.12.03 hor
		// クリップボードから貼り付ける？
		m_nPaste=IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), !m_nPaste );
		// 置換対象
		m_nReplaceTarget=0;
		if(::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_INSERT )){
			m_nReplaceTarget=1;
		}else
		if(::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ADD )){
			m_nReplaceTarget=2;
		}
		// To Here 2001.12.03 hor

		// 検索文字列/置換後文字列リストの設定	2010/5/26 Uchi
		if (!m_bModal) {
			SetCombosList();
		}
		return 1;
	}else{
		return 0;
	}
}




BOOL CDlgReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	//	Jun. 26, 2001 genta
	//	この位置で正規表現の初期化をする必要はない
	//	他との一貫性を保つため削除

	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	ComboBox_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), (WPARAM)_MAX_PATH - 1 );
	ComboBox_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), (WPARAM)_MAX_PATH - 1 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	ComboBox_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), TRUE );
	ComboBox_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );


	/* テキスト選択中か */
	if( m_bSelected ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );	// 2001.12.03 hor コメント解除
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );	// 2001.12.03 hor コメント解除
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );		// 2001.12.03 hor コメント解除
		::CheckDlgButton( GetHwnd(), IDC_RADIO_SELECTEDAREA, TRUE );
//		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALLAREA, FALSE );						// 2001.12.03 hor コメント
	}else{
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );	// 2001.12.03 hor コメント
//		::CheckDlgButton( GetHwnd(), IDC_RADIO_SELECTEDAREA, FALSE );					// 2001.12.03 hor コメント
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALLAREA, TRUE );
	}
	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}




BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;

	switch( wID ){
	case IDC_CHK_PASTE:
		/* テキストの貼り付け */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE ) &&
			!pcEditView->m_pcEditDoc->m_cDocEditor.IsEnablePaste() ){
			OkMessage( GetHwnd(), _T("クリップボードに有効なデータがありません！") );
			::CheckDlgButton( GetHwnd(), IDC_CHK_PASTE, FALSE );
		}
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT2 ), !(::IsDlgButtonChecked( GetHwnd(), IDC_CHK_PASTE)) );
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		/* 範囲範囲 */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		/* ファイル全体 */
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		/* 「置換」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//	case IDC_CHK_LOHICASE:	/* 大文字と小文字を区別する */
//		MYTRACE_A( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
//	case IDC_CHK_WORDONLY:	/* 一致する単語のみ検索 */
//		MYTRACE_A( "IDC_CHK_WORDONLY\n" );
//		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE_A( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* 英大文字と英小文字を区別する */
				//	Jan. 31, 2002 genta
				//	大文字・小文字の区別は正規表現の設定に関わらず保存する
				//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
				//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani
				/* 単語単位で探す */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );

				/*「すべて置換」は置換の繰返し */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), TRUE );	// 2007.01.16 ryoji
			}
		}else{
			/* 英大文字と英小文字を区別する */
			//::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	大文字・小文字の区別は正規表現の設定に関わらず保存する
			//::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 N.Nakatani
			/* 単語単位で探す */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), TRUE );

			/*「すべて置換」は置換の繰返し */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_CONSECUTIVEALL ), FALSE );	// 2007.01.16 ryoji
		}
		return TRUE;
//	case IDOK:			/* 下検索 */
//		/* ダイアログデータの取得 */
//		nRet = GetData();
//		if( 0 < nRet ){
//			::EndDialog( hwndDlg, 2 );
//		}else
//		if( 0 == nRet ){
//			::EndDialog( hwndDlg, 0 );
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:	/* 上検索 */
		if( 0 < GetData() ){

			// 検索開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* コマンドコードによる処理振り分け */
			/* 前を検索 */
			pcEditView->GetCommander().HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* 再描画（0文字幅マッチでキャレットを表示するため） */
			pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else{
			OkMessage( GetHwnd(), _T("文字列を指定してください。") );
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	/* 下検索 */
		if( 0 < GetData() ){

			// 検索開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* コマンドコードによる処理振り分け */
			/* 次を検索 */
			pcEditView->GetCommander().HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* 再描画（0文字幅マッチでキャレットを表示するため） */
			pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else{
			OkMessage( GetHwnd(), _T("文字列を指定してください。") );
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	//2002.01.16 hor 該当行マーク
		if( 0 < GetData() ){
			pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
			::SendMessage(GetHwnd(),WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(GetHwnd(),IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* 置換 */
		if( 0 < GetData() ){

			// 置換開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				/*
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				*/
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* 置換 */
			//@@@ 2002.2.2 YAZAKI 置換コマンドをCEditViewに新設
			//@@@ 2002/04/08 YAZAKI 親ウィンドウのハンドルを渡すように変更。
			pcEditView->GetCommander().HandleCommand( F_REPLACE, TRUE, (LPARAM)GetHwnd(), 0, 0, 0 );
			/* 再描画 */
			pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			OkMessage( GetHwnd(), _T("文字列を指定してください。") );
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	/* すべて置換 */
		if( 0 < GetData() ){
			// 置換開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* すべて行置換時の処置は「すべて置換」は置換の繰返しオプションOFFの場合にして削除 2007.01.16 ryoji */
			pcEditView->GetCommander().HandleCommand( F_REPLACE_ALL, TRUE, 0, 0, 0, 0 );
			pcEditView->GetCommander().HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* アクティブにする */
			ActivateFrameWindow( GetHwnd() );

			TopOkMessage( GetHwnd(), _T("%d箇所を置換しました。"), m_nReplaceCnt);

			if( !m_bCanceled ){
				if( m_bModal ){		/* モーダルダイアログか */
					/* 置換ダイアログを閉じる */
					::EndDialog( GetHwnd(), 0 );
				}else{
					/* 置換 ダイアログを自動的に閉じる */
					if( m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace ){
						::DestroyWindow( GetHwnd() );
					}
				}
			}
			return TRUE;
		}else{
			OkMessage( GetHwnd(), _T("置換条件を指定してください。") );
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog( hwndDlg, 0 );
//		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgReplace::OnActivate( WPARAM wParam, LPARAM lParam )
{
	// 0文字幅マッチ描画のON/OFF	// 2009.11.29 ryoji
	CEditView*	pcEditView = (CEditView*)m_lParam;
	CLayoutRange cRangeSel = pcEditView->GetCommander().GetSelect();
	if( cRangeSel.IsValid() && cRangeSel.IsLineOne() && cRangeSel.IsOne() )
		pcEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return CDialog::OnActivate(wParam, lParam);
}

//@@@ 2002.01.18 add start
LPVOID CDlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


