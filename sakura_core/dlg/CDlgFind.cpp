/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/12 再作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, JEPRO, hor, Stonee
	Copyright (C) 2002, MIK, hor, YAZAKI, genta
	Copyright (C) 2005, zenryaku
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgFind.h"
#include "view/CEditView.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


/*!
 * @brief コンストラクタ
 */
CDlgFind::CDlgFind() noexcept
	: CDialog( false, true )	// 基底クラスは「リサイズ不可、ShareDataチェックあり」。
	, m_strText( L"", 0 )		// 検索文字列
	, m_sSearchOption()			// 検索オプション
	, m_bNOTIFYNOTFOUND( 0 )	// 検索／置換  見つからないときメッセージを表示
	, m_bAutoCloseDlgFind( 0 )	// 検索ダイアログを自動的に閉じる
	, m_bSearchAll( 0 )			// 先頭（末尾）から再検索
	, m_ptEscCaretPos_PHY()		// 検索開始時のカーソル位置退避エリア
	, m_cRecentSearch()			// 検索語の履歴にアクセスするためのもの
	, m_comboDel()				// コンボボックスに追加したアイテムを自動削除させるもの
	, m_cFontText()				// フォントを自動削除させるもの
	, m_pcEditView( (CEditView*&) CDialog::m_lParam )
{
	//ダイアログ表示時に初期化するので、ここでは何もしない。
}


/*!
	コンボボックスのドロップダウンメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
BOOL CDlgFind::OnCbnDropDown( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_COMBO_TEXT:
		if ( ::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_sSearchKeywords.m_aSearchKeys.size();
			for (int i = 0; i < nSize; ++i) {
				Combo_AddString( hwndCtl, m_pShareData->m_sSearchKeywords.m_aSearchKeys[i] );
			}
		}
		break;
	}
	return CDialog::OnCbnDropDown( hwndCtl, wID );
}

/*!
 * @brief モードレスダイアログの表示
 *
 * @param [in] pcEditView 検索対象となるビュー
 * @param [in] pszFind 検索文字列
 * @param [in] cchFind 検索文字列の長さ
 * @returns 検索ダイアログのウインドウハンドル
 */
HWND CDlgFind::DoModeless( CEditView* pcEditView, const WCHAR* pszFind, size_t cchFind )
{
	// 前提条件
	assert( pcEditView );
	assert( pszFind );

	/* 検索文字列を初期化 */
	if( 0 < cchFind ) {
		m_strText.assign( pszFind, cchFind );
	}

	/* ダイアログ表示済みならアクティブにする */
	if ( GetHwnd() != NULL ) {
		ActivateFrameWindow( GetHwnd() );
		SetData();
		return GetHwnd();
	}

	// 基底クラスのメソッドを呼び出してダイアログを表示する
	auto hwndParent = pcEditView->GetHwnd();
	auto hInstance = (HINSTANCE)::GetWindowLongPtr( hwndParent, GWLP_HINSTANCE );
	return CDialog::DoModeless( hInstance, hwndParent, IDD_FIND, (LPARAM)pcEditView, SW_SHOW );
}


/* モードレス時：検索対象となるビューの変更 */
void CDlgFind::ChangeView( CEditView* pcEditView )
{
	assert( pcEditView );
	m_pcEditView = pcEditView;
	return;
}


/*!
 * @brief ダイアログ初期表示メッセージハンドラ
 *
 * @return TRUE or FALSE(WM_INITDIALOGと同じ)
 */
BOOL CDlgFind::OnInitDialog( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	// 基底クラス呼び出し(WM_INITDIALOGの戻り値は「成功/失敗」ではない)
	BOOL bRet = CDialog::OnInitDialog( hwnd, wParam, lParam );

	// 共有メモリから設定をコピーする
	m_sSearchOption = m_pShareData->m_Common.m_sSearch.m_sSearchOption;			// 検索オプション
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND;		// 検索／置換  見つからないときメッセージを表示
	m_bAutoCloseDlgFind = m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind;	// 検索ダイアログを自動的に閉じる
	m_bSearchAll = m_pShareData->m_Common.m_sSearch.m_bSearchAll;				// 先頭（末尾）から再検索

	// 検索開始時のカーソル位置を退避する
	m_ptEscCaretPos_PHY = m_pcEditView->GetCaret().GetCaretLogicPos();

	// 検索開始位置の登録有無を更新
	m_pcEditView->m_bSearch = TRUE;

	// ユーティリティ初期化
	m_comboDel = SComboBoxItemDeleter();
	m_comboDel.pRecent = &m_cRecentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &m_comboDel);

	// フォント設定	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessageAny( GetItemHwnd( IDC_COMBO_TEXT ), WM_GETFONT, 0, 0 );
	HFONT hFont = SetMainFont( GetItemHwnd( IDC_COMBO_TEXT ) );
	m_cFontText.SetFont( hFontOld, hFont, GetItemHwnd( IDC_COMBO_TEXT ) );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	Combo_SetExtendedUI( GetItemHwnd( IDC_COMBO_TEXT ), TRUE );

	// 正規表現DLLが使えない場合、正規表現のフラグを落とす
	if ( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false ) ) {
		m_sSearchOption.bRegularExp = false;
	}

	// ダイアログデータの設定
	SetData();

	return bRet;
}


/*!
 * @brief ウインドウ破棄メッセージハンドラ
 *
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 */
BOOL CDlgFind::OnDestroy()
{
	m_cFontText.ReleaseOnDestroy();
	return CDialog::OnDestroy();
}



/*!
 * @brief ダイアログデータの設定
 * （CDlgFindメンバ変数 ⇒ ダイアログの同期）
 *
 * @date 2001/06/23 Norio Nakatani　単語単位で検索
 * @date Jun. 29, 2001 genta 正規表現ライブラリの差し替えに伴う処理の見直し
 * @date 2002.01.26 hor 先頭（末尾）から再検索
 * @date 2010/05/28 Uchi 検索文字列リストの設定(関数化)
 */
void CDlgFind::SetData( void ) const noexcept
{
//	MYTRACE( _T("CDlgFind::SetData()") );

	/* 検索文字列 */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_strText.c_str() );

	/* 単語単位で検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly ? BST_CHECKED : BST_UNCHECKED );

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase ? BST_CHECKED : BST_UNCHECKED );

	/* 正規表現 */
	::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, m_sSearchOption.bRegularExp ? BST_CHECKED : BST_UNCHECKED );

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND ? BST_CHECKED : BST_UNCHECKED );

	/* 検索ダイアログを自動的に閉じる */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind, m_bAutoCloseDlgFind ? BST_CHECKED : BST_UNCHECKED );

	/* 先頭（末尾）から再検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_bSearchAll ? BST_CHECKED : BST_UNCHECKED );

	// 正規表現DLLが使えない場合、正規表現のチェックボックスを淡色表示にする
	bool checkResult = CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false);
	/* 正規表現 */
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_REGULAREXP ), checkResult ? TRUE : FALSE );

	// 正規表現を使う場合、単語単位で探すのチェックボックスを淡色表示にする
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), !m_sSearchOption.bRegularExp ? TRUE : FALSE );

	return;
}


// 検索文字列リストの設定
//	2010/5/28 Uchi
void CDlgFind::SetCombosList( void )
{
	HWND	hwndCombo;

	/* 検索文字列 */
	hwndCombo = GetItemHwnd( IDC_COMBO_TEXT );
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString( hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	Combo_GetText( hwndCombo, &vText[0], nBufferSize );
	if (auto_strcmp( to_wchar(&vText[0]), m_strText.c_str() ) != 0) {
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_strText.c_str() );
	}
}


/*!
 * @brief ダイアログデータの取得
 * （ダイアログ ⇒ CDlgFindメンバ変数の同期）
 *
 * @retval 1 取込成功 (CDlgFindの実装では検索可能)
 * @retval 0 変更なし (CDlgFindの実装では検索不可)
 * @retval -1 取込失敗
 *
 * @date 2001/06/23 Norio Nakatani 単語単位で検索を追加
 * @date Jun. 26, 2001 genta 正規表現ライブラリの差し替えに伴う処理の見直し
 * @date 2002.01.26 hor 先頭（末尾）から再検索
 * @date 2002.2.2 YAZAKI CShareDataに移動
 */
int CDlgFind::GetData( void )
{
//	MYTRACE( _T("CDlgFind::GetData()") );

	/* 検索文字列 */
	size_t cchText = ::GetWindowTextLengthW( GetItemHwnd( IDC_COMBO_TEXT ) );
	if ( cchText == 0 ) {
		return 0;
	}

	/* 検索文字列 */
	auto textBuf = std::make_unique<WCHAR[]>( cchText + 1 );
	if ( cchText != ::GetWindowTextW( GetItemHwnd( IDC_COMBO_TEXT ), textBuf.get(), cchText + 1 ) ) {
		ErrorBeep();
		return -1;
	}
	m_strText.assign( textBuf.get(), cchText );

	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	/* 単語単位で検索 */
	m_sSearchOption.bWordOnly = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* 正規表現 */
	m_sSearchOption.bRegularExp = (0!=IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));

	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND );

	/* 検索ダイアログを自動的に閉じる */
	m_bAutoCloseDlgFind = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind );

	/* 先頭（末尾）から再検索 */
	m_bSearchAll = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL );

	/* 正規表現？ */
	if ( m_sSearchOption.bRegularExp ) {
		if ( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ) {
			ErrorBeep();
			m_sSearchOption.bRegularExp = false;
			return -1;
		}
		int nFlag = m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if ( !CheckRegexpSyntax( m_strText.c_str(), GetHwnd(), true, nFlag) ) {
			return -1;
		}
	}

	// 以下の処理は検索実行の直前に行うべき処理
	m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示
	m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = m_bAutoCloseDlgFind;
	m_pShareData->m_Common.m_sSearch.m_bSearchAll = m_bSearchAll;

	/* 検索文字列 */
	if ( cchText < _MAX_PATH) {
		CSearchKeywordManager().AddToSearchKeyArr( m_strText.c_str() );
		m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// 検索オプション
	}

	if ( m_pcEditView->m_strCurSearchKey != m_strText
		|| m_pcEditView->m_sCurSearchOption != m_sSearchOption) {
		m_pcEditView->m_strCurSearchKey = m_strText;
		m_pcEditView->m_sCurSearchOption = m_sSearchOption;
		m_pcEditView->m_bCurSearchUpdate = true;
		m_pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
	}

	/* ダイアログデータの設定 */
	SetCombosList();		//	コンボのみの初期化	2010/5/28 Uchi

	return 1;
}



/*!
 * @brief ボタン押下メッセージハンドラ
 * （WM_COMMANDのうち、ボタン押下分を処理させるために呼ばれる）
 *
 * @param [in] wID 押下されたボタンのID
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 *
 * @date Feb. 13, 2001 JEPRO ボタン名を[IDC_BUTTON1]→[IDC_BUTTON_SERACHPREV]に変更
 * @date Feb. 13, 2001 JEPRO ボタン名を[IDOK]→[IDC_BUTTON_SERACHNEXT]に変更
 * @date 2001/03/12 Stonee MyWinHelpの第四引数を、機能番号からヘルプトピック番号を調べるようにした
 * @date 2002.01.16 hor 該当行マーク
 */
BOOL CDlgFind::OnBnClicked( int wID )
{
	/* ダイアログデータの取得 */
	auto nRet = GetData();
	if ( nRet < 0 ) {
		SetData();
	}

	switch ( wID ) {
	case IDC_BUTTON_SEARCHPREV:
		if ( nRet < 0 ) return FALSE;
		if ( nRet == 0 ) {
			// 検索条件を指定してください。
			OkMessage( GetHwnd(), LS(STR_DLGFIND1) );
			return FALSE;
		}
		{
			/* 前を検索 */
			m_pcEditView->GetCommander().HandleCommand( F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0 );

			/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
			m_pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

			// 02/06/26 ai Start
			// 検索開始位置を登録
			if( FALSE != m_pcEditView->m_bSearch ){
				// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
				m_pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				m_pcEditView->m_bSearch = FALSE;
				// 02/07/28 ai end
			}//  02/06/26 ai End

			/* 検索ダイアログを自動的に閉じる */
			if ( m_bAutoCloseDlgFind ) {
				CloseDialog( 0 );
			}
		}
		return TRUE;

	case IDC_BUTTON_SEARCHNEXT:
		if ( nRet < 0 ) return FALSE;
		if ( nRet == 0 ) {
			// 検索条件を指定してください。
			OkMessage(GetHwnd(), LS(STR_DLGFIND1));
			return FALSE;
		}
		{
			/* 次を検索 */
			m_pcEditView->GetCommander().HandleCommand( F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0 );

			/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
			m_pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

			// 検索開始位置を登録
			if( FALSE != m_pcEditView->m_bSearch ){
				// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
				m_pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				m_pcEditView->m_bSearch = FALSE;
			}

			/* 検索ダイアログを自動的に閉じる */
			if ( m_bAutoCloseDlgFind ) {
				CloseDialog( 0 );
			}
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:
		if ( nRet <= 0 ) return FALSE;
		{
			/* 検索して該当行をマーク */
			m_pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, false, 0, 0, 0, 0 );

			/* 検索ダイアログを自動的に閉じる */
			if( m_bAutoCloseDlgFind ) {
				CloseDialog( 0 );
			} else {
				::SendMessage( GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE );
			}
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG) );
		break;

	case IDCANCEL:
		//既定動作をオーバーライドして0を返して閉じている
		CloseDialog( 0 );
		return TRUE;

	default:
		break;
	}

	// 基底クラス呼び出し
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFind::OnActivate( WPARAM wParam, LPARAM lParam )
{
	// 0文字幅マッチ描画のON/OFF	// 2009.11.29 ryoji
	CLayoutRange cRangeSel = m_pcEditView->GetSelectionInfo().m_sSelect;
	if( cRangeSel.IsValid() && cRangeSel.IsLineOne() && cRangeSel.IsOne() )
		m_pcEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return CDialog::OnActivate(wParam, lParam);
}

/*!
 * @brief コントロールのヘルプIDを返却する
 * @date 2002.01.18 MIK add
 */
LPVOID CDlgFind::GetHelpIdTable( void )
{
	//検索 CDlgFind
	static constexpr DWORD helpIdTable[] = {	//11800
		IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		//次を検索
		IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		//前を検索
		IDCANCEL,						HIDCANCEL_FIND,						//キャンセル
		IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				//ヘルプ
		IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					//単語単位
		IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				//大文字小文字
		IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			//正規表現
		IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		//見つからないときに通知
		IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	//自動的に閉じる
		IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				//検索文字列
		IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			//正規表現バージョン
		IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			//2002.01.16 hor 検索該当行をマーク
		IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			//2002.01.26 hor 先頭（末尾）から再検索
		0, 0
	};
	return (LPVOID)helpIdTable;
}


