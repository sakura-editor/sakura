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
#include "recent/CRecentSearch.h"
#include "util/window.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


/*!
 * @brief コンストラクタ
 */
CDlgFind::CDlgFind() noexcept
	: CDialog( false, true )		// 基底クラスは「リサイズ不可、ShareDataチェックあり」。
	, m_strText( L"", 0 )			// 検索文字列
	, m_sSearchOption()				// 検索オプション
	, m_bNotifyNotFound( false )	// 検索／置換  見つからないときメッセージを表示
	, m_bAutoClose( false )			// 検索ダイアログを自動的に閉じる
	, m_bSearchAll( false )			// 先頭（末尾）から再検索
	, m_hFont( NULL )				// ドキュメント設定から作成したフォント
	, m_ptEscCaretPos_PHY()			// 検索開始時のカーソル位置退避エリア
	, m_pcEditView( (CEditView*&) CDialog::m_lParam )
	, m_threadAutoCount()
{
	//ダイアログ表示時に初期化するので、ここでは何もしない。
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
 * @param [in] wParam 初期フォーカスを設定されるコントロールのハンドル
 * @param [in] lParam 検索対象となるビュー
 * @return TRUE or FALSE(WM_INITDIALOGと同じ)
 *
 * @date 2012/11/27 Uchi フォント設定
 */
BOOL CDlgFind::OnInitDialog( HWND wParam, LPARAM lParam )
{
	// タイトルをバックアップ
	if ( size_t cchTitle = ::GetWindowTextLengthW( GetHwnd() ) ) {
		auto titleBuf = std::make_unique<WCHAR[]>( cchTitle + 1 );
		::GetWindowTextW( GetHwnd(), titleBuf.get(), cchTitle + 1 );
		m_strOriginalTitle.assign( titleBuf.get(), cchTitle );
	}

	// コピー元の共有メモリを別名定義する
	const auto &s_sSearch = m_pShareData->m_Common.m_sSearch;

	// 共有メモリから設定をコピーする
	m_sSearchOption = s_sSearch.m_sSearchOption;			// 検索オプション
	m_bNotifyNotFound = s_sSearch.m_bNOTIFYNOTFOUND != 0;	// 検索／置換  見つからないときメッセージを表示
	m_bAutoClose = s_sSearch.m_bAutoCloseDlgFind != 0;		// 検索ダイアログを自動的に閉じる
	m_bSearchAll = s_sSearch.m_bSearchAll != 0;				// 先頭（末尾）から再検索

	if ( HWND hwndComboText = GetItemHwnd( IDC_COMBO_TEXT ) ) {
		/* コンボボックスを拡張UIにする */
		Combo_SetExtendedUI( hwndComboText, TRUE );

		// フォント設定
		m_hFont = SetMainFont( hwndComboText );
	}

	// 正規表現DLLが使えない場合、正規表現のフラグを落とす
	if ( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false ) ) {
		m_sSearchOption.bRegularExp = false;
	}

	// 検索開始時のカーソル位置をクリアする
	m_ptEscCaretPos_PHY.Set( CLogicInt( -1 ), CLogicInt( -1 ) );

	return TRUE;
}


/*!
 * @brief ウインドウ破棄メッセージハンドラ
 *
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 */
BOOL CDlgFind::OnDestroy()
{
	// 自動カウントを止める
	StopAutoCounter();

	return CDialog::OnDestroy();
}


/*!
 * @brief ウインドウ破棄後処理メッセージハンドラ
 * （ウインドウ破棄シーケンスの最後に呼ばれる）
 */
void CDlgFind::OnNcDestroy() noexcept
{
	if ( m_hFont != NULL ) {
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	CDialog::OnNcDestroy();
}


/*!
 * @brief ダイアログデータの設定
 * （CDlgFindメンバ変数 ⇒ ダイアログの同期）
 *
 * @date 2001/06/23 Norio Nakatani　単語単位で検索
 * @date Jun. 29, 2001 genta 正規表現ライブラリの差し替えに伴う処理の見直し
 * @date 2002.01.26 hor 先頭（末尾）から再検索
 */
void CDlgFind::SetData( void ) const noexcept
{
//	MYTRACE( _T("CDlgFind::SetData()") );

	/* 検索文字列 */
	::SetDlgItemTextW( GetHwnd(), IDC_COMBO_TEXT, m_strText.c_str() );

	/* 単語単位で検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly ? BST_CHECKED : BST_UNCHECKED );

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase ? BST_CHECKED : BST_UNCHECKED );

	/* 正規表現 */
	::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, m_sSearchOption.bRegularExp ? BST_CHECKED : BST_UNCHECKED );

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND, m_bNotifyNotFound ? BST_CHECKED : BST_UNCHECKED );

	/* 検索ダイアログを自動的に閉じる */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind, m_bAutoClose ? BST_CHECKED : BST_UNCHECKED );

	/* 先頭（末尾）から再検索 */
	::CheckDlgButton( GetHwnd(), IDC_CHECK_SEARCHALL, m_bSearchAll ? BST_CHECKED : BST_UNCHECKED );

	// 正規表現DLLが使えない場合、正規表現のチェックボックスを淡色表示にする
	bool checkResult = CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false);
	/* 正規表現 */
	::EnableWindow( GetItemHwnd( IDC_CHK_REGULAREXP ), checkResult ? TRUE : FALSE );

	// 正規表現を使う場合、単語単位で探すのチェックボックスを淡色表示にする
	::EnableWindow( GetItemHwnd( IDC_CHK_WORD ), !m_sSearchOption.bRegularExp ? TRUE : FALSE );

	return;
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
	if ( cchText != ::GetDlgItemTextW( GetHwnd(), IDC_COMBO_TEXT, textBuf.get(), cchText + 1 ) ) {
		ErrorBeep();
		return -1;
	}
	m_strText.assign( textBuf.get(), cchText );

	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ) == BST_CHECKED );

	/* 単語単位で検索 */
	m_sSearchOption.bWordOnly = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ) == BST_CHECKED );

	/* 正規表現 */
	m_sSearchOption.bRegularExp = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) == BST_CHECKED );

	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNotifyNotFound = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_NOTIFYNOTFOUND ) == BST_CHECKED );

	/* 検索ダイアログを自動的に閉じる */
	m_bAutoClose = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_bAutoCloseDlgFind ) == BST_CHECKED );

	/* 先頭（末尾）から再検索 */
	m_bSearchAll = ( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SEARCHALL ) == BST_CHECKED );

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
	case IDC_BUTTON_SEARCHNEXT:
	case IDC_BUTTON_SEARCHPREV:
		if ( nRet < 0 ) return FALSE;
		if ( nRet == 0 ) {
			// 検索条件を指定してください。
			OkMessage( GetHwnd(), LS(STR_DLGFIND1) );
			return FALSE;
		}
		DoSearch( wID == IDC_BUTTON_SEARCHNEXT ? SEARCH_FORWARD : SEARCH_BACKWARD );
		return TRUE;

	case IDC_BUTTON_SETMARK:
		if ( nRet <= 0 ) return FALSE;
		DoSetMark();
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


/*!
 * @brief コンボドロップダウンメッセージハンドラ
 * （WM_COMMANDのうち、コンボドロップダウン分を処理させるために呼ばれる）
 *
 * @param [in] wID 編集されたコンボのID
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 *
 * @date 2013.03.24 novice 新規作成
 */
BOOL CDlgFind::OnCbnDropDown( HWND hwndCtl, int wID )
{
	switch ( wID ) {
	case IDC_COMBO_TEXT:
		if ( Combo_GetCount( hwndCtl ) == 0 ) {
			CRecentSearch cRecentSearch;
			size_t cItems = cRecentSearch.GetItemCount();
			for ( size_t n = 0; n < cItems; ++n ) {
				Combo_AddString( hwndCtl, cRecentSearch.GetItemText( n ) );
			}
		}
		return FALSE;
	default:
		DEBUG_TRACE( _T("%ls(%d): %ls\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
		break;
	}
	return CDialog::OnCbnDropDown( hwndCtl, wID );
}


/*!
 * @brief コンボ編集メッセージハンドラ
 * （WM_COMMANDのうち、コンボ編集分を処理させるために呼ばれる）
 *
 * @param [in] wID 編集されたコンボのID
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 */
BOOL CDlgFind::OnCbnEditChange( HWND hwndCtl, int wID )
{
	DEBUG_TRACE(_T("%ls(%d): %ls\n"), __FILEW__, __LINE__, __FUNCTIONW__);

	// 自動カウントを止める
	StopAutoCounter();

	// 自動カウントのタイマーを起動する
	::SetTimer( GetHwnd(), IDT_AUTO_COUNT, TIMESPAN_AUTO_COUNT, NULL );

	// 検索開始時のカーソル位置をクリアする
	m_ptEscCaretPos_PHY.Set( CLogicInt( -1 ), CLogicInt( -1 ) );

	return FALSE;
}


/*!
 * @brief コンボ選択メッセージハンドラ
 * （WM_COMMANDのうち、コンボ選択分を処理させるために呼ばれる）
 *
 * @param [in] wID 編集されたコンボのID
 * @return TRUE or FALSE(FALSE推奨、OSには無視される)
 */
BOOL CDlgFind::OnCbnSelChange( HWND hwndCtl, int wID )
{
	DEBUG_TRACE(_T("%ls(%d): %ls\n"), __FILEW__, __LINE__, __FUNCTIONW__);

	// 自動カウントを止める
	StopAutoCounter();

	// 検索開始時のカーソル位置をクリアする
	m_ptEscCaretPos_PHY.Set( CLogicInt( -1 ), CLogicInt( -1 ) );

	// 自動カウントを開始する
	StartAutoCounter();

	return FALSE;
}


/*!
 * @brief タイマーイベントハンドラ
 */
BOOL CDlgFind::OnTimer( WPARAM nTimerId )
{
	switch( nTimerId )
	{
	case IDT_AUTO_COUNT:
		// 自動カウントを開始する
		StartAutoCounter();
		break;
	}
	return FALSE;
}


/*!
 * @brief アクティブ化／非アクティブ化メッセージハンドラ
 * （WM_ACTIVATEのを処理させるために呼ばれる）
 *
 * @date 2009.11.29 ryoji 0文字幅マッチ描画のON/OFF
 */
BOOL CDlgFind::OnActivate( WPARAM wParam, LPARAM lParam )
{
	WORD fActive = LOWORD( wParam );
	if ( fActive == WA_INACTIVE ) {
		// 自動カウントを止める
		StopAutoCounter();
	}

	// 0文字幅マッチ描画のON/OFF
	CLayoutRange cRangeSel = m_pcEditView->GetSelectionInfo().m_sSelect;
	if( cRangeSel.IsValid() && cRangeSel.IsLineOne() && cRangeSel.IsOne() )
		m_pcEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return CDialog::OnActivate(wParam, lParam);
}


/*!
 * @brief 検索キーを共有メモリに書き込む
 */
inline void CDlgFind::ApplySharedSearchKey() noexcept
{
	// 適用先の共有メモリを別名定義する
	auto &s_sSearch = m_pShareData->m_Common.m_sSearch;

	// 検索オプションを共有メモリに転送する
	s_sSearch.m_sSearchOption = m_sSearchOption;

	// 検索オプション(検索ダイアログ用拡張定義分)を共有メモリに転送する
	s_sSearch.m_bNOTIFYNOTFOUND = m_bNotifyNotFound ? TRUE : FALSE;
	s_sSearch.m_bAutoCloseDlgFind = m_bAutoClose ? TRUE : FALSE;
	s_sSearch.m_bSearchAll = m_bSearchAll ? TRUE : FALSE;

	// 検索キーを共有メモリに転送する
	const decltype(((const SShare_SearchKeywords*)NULL)->m_aSearchKeys)::ElementType keyType;
	if ( m_strText.length() < keyType.BUFFER_COUNT ) {
		// 履歴登録は格納できる長さの場合にのみ行う
		CSearchKeywordManager().AddToSearchKeyArr( m_strText.c_str() );
	}

	// 検索キーか検索オプションがビューの設定と異なる場合
	if ( m_pcEditView->m_strCurSearchKey != m_strText
		|| m_pcEditView->m_sCurSearchOption != m_sSearchOption ) {
		// 検索キーと検索オプションをビューに転送する
		m_pcEditView->m_strCurSearchKey = m_strText;
		m_pcEditView->m_sCurSearchOption = m_sSearchOption;
		m_pcEditView->m_nCurSearchKeySequence = s_sSearch.m_nSearchKeySequence;
		m_pcEditView->m_bCurSearchUpdate = true;
	}

}


 /*!
 * @brief 検索
 *
 * @param [in] direction 検索方向
 */
void CDlgFind::DoSearch( ESearchDirection direction ) noexcept
{
	EFunctionCode eFuncId;
	if ( direction == ESearchDirection::SEARCH_FORWARD ) {
		/* 次を検索 */
		eFuncId = F_SEARCH_NEXT;
	} else {
		/* 前を検索 */
		eFuncId = F_SEARCH_PREV;
	}

	// 検索キーを共有メモリに転送する
	ApplySharedSearchKey();

	if ( m_ptEscCaretPos_PHY.HasNegative() ) {
		// 検索開始時のカーソル位置を退避する
		m_ptEscCaretPos_PHY = m_pcEditView->GetCaret().GetCaretLogicPos();

		// 検索開始位置の登録有無を更新
		m_pcEditView->m_bSearch = TRUE;
	}

	m_pcEditView->GetCommander().HandleCommand( eFuncId, true, (LPARAM)GetHwnd(), 0, 0, 0 );

	/* 再描画 2005.04.06 zenryaku 0文字幅マッチでキャレットを表示するため */
	m_pcEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

	// 検索開始位置を登録
	if ( m_pcEditView->m_bSearch != FALSE ) {
		// 検索開始時のカーソル位置登録条件変更 02/07/28 ai start
		m_pcEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
		m_pcEditView->m_bSearch = FALSE;
	}

	/* 検索ダイアログを自動的に閉じる */
	if ( m_bAutoClose ) {
		CloseDialog( 0 );
	}
}

/*!
 * @brief 該当行をマーク
 *
 * @date 2002.01.16 hor 該当行マーク
 */
void CDlgFind::DoSetMark( void ) noexcept
{
	// 検索キーを共有メモリに転送する
	ApplySharedSearchKey();

	/* 検索して該当行をマーク */
	m_pcEditView->GetCommander().HandleCommand( F_BOOKMARK_PATTERN, false, 0, 0, 0, 0 );

	/* 検索ダイアログを自動的に閉じる */
	if( m_bAutoClose ) {
		CloseDialog( 0 );
	} else {
		::SendMessage( GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE );
	}
}


/*!
 * @brief 自動カウントを開始する
 */
void CDlgFind::StartAutoCounter() noexcept
{
	DEBUG_TRACE( _T("%ls(%d): %ls\n"), __FILEW__, __LINE__, __FUNCTIONW__ );

	// 自動カウントのタイマーを止める
	::KillTimer( GetHwnd(), IDT_AUTO_COUNT );

	// カウントスレッドが有効な場合、停止処理を走らせる
	if ( m_threadAutoCount.joinable() ) {
		// 自動カウントを止める
		StopAutoCounter();
	}

	// ダイアログデータの取得
	// ※検索できない状態ならスレッドを生成せずに抜ける
	auto nRet = GetData();
	if ( nRet <= 0 ) {
		DEBUG_TRACE( _T("%ls(%d): %ls aborted.\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
		return;
	}

	// 検索キーを共有メモリに転送する
	ApplySharedSearchKey();

	// ビューの検索キーを更新（≒コンパイル）する
	if ( !m_pcEditView->ChangeCurRegexp( false ) ) {
		// GetDataでチェック済みなのでここは絶対通らないはず。
		DEBUG_TRACE( _T("%ls(%d): %ls failed.\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
		return;
	}

	// ダイアログのタイトルを変える
	CNativeW strTitle;
	strTitle.AppendStringF( LSW(STR_AUTO_COUNT_PROCESSING), m_strOriginalTitle.c_str());
	::SetWindowTextW( GetHwnd(), strTitle.GetStringPtr() );

	// 新たなカウントスレッドを生成する
	m_threadAutoCount = std::thread( [this] { CountMatches(); } );
}


/*!
 * @brief 自動カウントを停止する
 */
void CDlgFind::StopAutoCounter() noexcept
{
	DEBUG_TRACE( _T("%ls(%d): %ls\n"), __FILEW__, __LINE__, __FUNCTIONW__ );

	// 自動カウントのタイマーを止める
	::KillTimer( GetHwnd(), IDT_AUTO_COUNT );

	// カウントスレッドが有効な場合、強制停止する
	if ( m_threadAutoCount.joinable() ) {
		DEBUG_TRACE( _T("%ls(%d): %ls thread is joinable.\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
		HANDLE hThread = (HANDLE) m_threadAutoCount.native_handle();
		::TerminateThread( hThread, -1 );
		m_threadAutoCount.detach();
	} else {
		DEBUG_TRACE( _T("%ls(%d): %ls thread is not joinable.\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
	}

	::SetWindowTextW( GetHwnd(), m_strOriginalTitle.c_str() );
}


/*!
 * @brief 条件に一致する文字列を数える
 *
 * @note CBookmarkManager::MarkSearchWordを改造して作成
 */
void CDlgFind::CountMatches() const noexcept
{
	DEBUG_TRACE(_T("%ls(%d): %ls start\n"), __FILEW__, __LINE__, __FUNCTIONW__);

	auto pcDocLineMgr = &m_pcEditView->GetDocument()->m_cDocLineMgr;
	auto &pattern = m_pcEditView->m_sSearchPattern;
	const SSearchOption& sSearchOption = pattern.GetSearchOption();

	// マッチャーの定義
	struct Matcher
	{
		virtual bool Match( const wchar_t* pchLine, size_t cchLine ) const = 0;
	};
	struct RegexMatcher : Matcher
	{
		RegexMatcher( const CSearchStringPattern& pattern )
			: pRegexp( pattern.GetRegexp() )
		{
		}
		bool Match( const wchar_t* pchLine, size_t cchLine ) const override
		{
			return pRegexp->Match( pchLine, cchLine, 0 );
		}
		CBregexp* pRegexp;
	};
	struct WordsMatcher : Matcher
	{
		WordsMatcher( const CSearchStringPattern& pattern, const SSearchOption& sSearchOption)
			: bLoHiCase( sSearchOption.bLoHiCase )
			, searchWords()
		{
			const wchar_t* pszPattern = pattern.GetKey();
			size_t nPatternLen = pattern.GetLen();
			CSearchAgent::CreateWordList( searchWords, pszPattern, nPatternLen );
		}
		bool Match( const wchar_t* pchLine, size_t cchLine ) const override
		{
			int nMatchLen;
			return CSearchAgent::SearchStringWord( pchLine, cchLine, 0,
				searchWords, bLoHiCase, &nMatchLen);
		}
		bool bLoHiCase;
		std::vector<std::pair<const wchar_t*, CLogicInt>> searchWords; // 単語の開始位置と長さの配列。
	};
	struct SimpleMatcher : Matcher
	{
		SimpleMatcher( const CSearchStringPattern& _pattern )
			: pattern( _pattern )
		{
		}
		bool Match( const wchar_t* pchLine, size_t cchLine ) const override
		{
			return CSearchAgent::SearchString( pchLine, cchLine, 0, pattern );
		}
		const CSearchStringPattern& pattern;
	};

	// マッチャーの生成
	std::unique_ptr<Matcher> matcher;

	/* 1==正規表現 */
	if ( sSearchOption.bRegularExp ) {
		matcher = std::unique_ptr<Matcher>( new RegexMatcher( pattern ) );
	}
	/* 1==単語のみ検索 */
	else if ( sSearchOption.bWordOnly ) {
		matcher = std::unique_ptr<Matcher>( new WordsMatcher( pattern, sSearchOption) );
	}
	/* その他 */
	else {
		matcher = std::unique_ptr<Matcher>( new SimpleMatcher( pattern ) );
	}

	// 一致件数のカウント
	size_t cMatched = 0;
	CDocLine* pDocLine = pcDocLineMgr->GetLine( CLogicInt(0) );
	while ( pDocLine != NULL ) {
		int nLineLen;
		const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
		if ( matcher->Match( pLine, nLineLen ) ) {
			cMatched++;
		}
		pDocLine = pDocLine->GetNextLine();
	}

	// 結果を表示
	DEBUG_TRACE( _T("%ls(%d): %ls %d matched\n"), __FILEW__, __LINE__, __FUNCTIONW__, cMatched );
	CNativeW strTitle;
	strTitle.AppendStringF( LSW(STR_AUTO_COUNT_DONE), m_strOriginalTitle.c_str(), cMatched );
	::SetWindowTextW( GetHwnd(), strTitle.GetStringPtr() );

	DEBUG_TRACE( _T("%ls(%d): %ls end\n"), __FILEW__, __LINE__, __FUNCTIONW__ );
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
