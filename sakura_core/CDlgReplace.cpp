//	$Id$
/*!	@file
	@brief 置換ダイアログ

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, Stonee, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgReplace.h"
#include "debug.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "global.h"
#include "CWaitCursor.h"
#include "funccode.h"		// Stonee, 2001/03/12

//置換 CDlgReplace.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
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
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgReplace::CDlgReplace()
{
	m_bLoHiCase = FALSE;		/* 英大文字と英小文字を区別する */
	m_bWordOnly = FALSE;		/* 一致する単語のみ検索する */
	m_bRegularExp = FALSE;		/* 正規表現 */
	m_bSelectedArea = FALSE;	/* 選択範囲内置換 */
	m_szText[0] = '\0';			/* 検索文字列 */
	m_szText2[0] = '\0';		/* 置換後文字列 */
	m_nReplaceTarget=0;			/* 置換対象 */		// 2001.12.03 hor
	m_nPaste=FALSE;				/* 貼り付ける？ */	// 2001.12.03 hor
	return;
}

/* モードレスダイアログの表示 */
HWND CDlgReplace::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, BOOL bSelected )
{
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;			/* 1==正規表現 */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;				/* 1==英大文字小文字の区別 */
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;				/* 1==単語のみ検索 */
	m_bSelectedArea = m_pShareData->m_Common.m_bSelectedArea;		/* 選択範囲内置換 */
	m_bNOTIFYNOTFOUND = m_pShareData->m_Common.m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
	m_bSelected = bSelected;
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
	int		i;
	HWND	hwndCombo;
//	m_hWnd = hwndDlg;	/* このダイアログのハンドル */

	/* 検索文字列 */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}

	/* 置換後文字列 */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2 );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 );
	for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szREPLACEKEYArr[i] );
	}

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 N.Nakatani
	/* 単語単位で探す */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );

// From Here 2001.12.03 hor
//	/* 選択範囲内置換 */
//	if( m_pShareData->m_Common.m_bSelectedArea ){
//		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
//	}else{
//		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
//	}
// To Here 2001.12.03 hor

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	/* 検索／置換  見つからないときメッセージを表示 */
	::CheckDlgButton( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND );


	/* 置換 ダイアログを自動的に閉じる */
	::CheckDlgButton( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace, m_pShareData->m_Common.m_bAutoCloseDlgReplace );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	::CheckDlgButton( m_hWnd, IDC_CHECK_SEARCHALL, m_pShareData->m_Common.m_bSearchAll );

	// From Here 2001.12.03 hor
	// クリップボードから貼り付ける？
	::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, m_nPaste );
	// 置換対象
	if(m_nReplaceTarget==0){
		::CheckDlgButton( m_hWnd, IDC_RADIO_REPLACE, TRUE );
	}else
	if(m_nReplaceTarget==1){
		::CheckDlgButton( m_hWnd, IDC_RADIO_INSERT, TRUE );
	}else
	if(m_nReplaceTarget==2){
		::CheckDlgButton( m_hWnd, IDC_RADIO_ADD, TRUE );
	}
	// To Here 2001.12.03 hor

	return;
}




/* ダイアログデータの取得 */
/* 0==条件未入力  0より大きい==正常   0より小さい==入力エラー */
int CDlgReplace::GetData( void )
{
	int			i;
	int			j;
	CMemory*	pcmWork;

	/* 英大文字と英小文字を区別する */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	// 2001/06/23 N.Nakatani
	/* 単語単位で探す */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* 正規表現 */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );
	/* 選択範囲内置換 */
	m_bSelectedArea = ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_SELECTEDAREA );
	/* 検索／置換  見つからないときメッセージを表示 */
	m_bNOTIFYNOTFOUND = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_NOTIFYNOTFOUND );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;			/* 1==正規表現 */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;				/* 1==英大文字小文字の区別 */
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;				/* 1==単語のみ検索 */
	m_pShareData->m_Common.m_bSelectedArea = m_bSelectedArea;		/* 選択範囲内置換 */
	m_pShareData->m_Common.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */

	/* 検索文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );
	/* 置換後文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2, _MAX_PATH - 1 );

	/* 置換 ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		pcmWork = new CMemory( m_szText, lstrlen( m_szText ) );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText, m_pShareData->m_szSEARCHKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nSEARCHKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
			++m_pShareData->m_nSEARCHKEYArrNum;
			if( m_pShareData->m_nSEARCHKEYArrNum > MAX_SEARCHKEY ){
				m_pShareData->m_nSEARCHKEYArrNum = MAX_SEARCHKEY;
			}
		}
		strcpy( m_pShareData->m_szSEARCHKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;

		/* 置換後文字列 */
		pcmWork = new CMemory( m_szText2, lstrlen( m_szText2 ) );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			if( 0 == strcmp( m_szText2, m_pShareData->m_szREPLACEKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nREPLACEKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_REPLACEKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[j], m_pShareData->m_szREPLACEKEYArr[j - 1] );
			}
			++m_pShareData->m_nREPLACEKEYArrNum;
			if( m_pShareData->m_nREPLACEKEYArrNum > MAX_REPLACEKEY ){
				m_pShareData->m_nREPLACEKEYArrNum = MAX_REPLACEKEY;
			}
		}
		strcpy( m_pShareData->m_szREPLACEKEYArr[0], pcmWork->GetPtr( NULL ) );
		delete pcmWork;

		// From Here 2001.12.03 hor
		// クリップボードから貼り付ける？
		m_nPaste=IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), !m_nPaste );
		// 置換対象
		m_nReplaceTarget=0;
		if(::IsDlgButtonChecked( m_hWnd, IDC_RADIO_INSERT )){
			m_nReplaceTarget=1;
		}else
		if(::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ADD )){
			m_nReplaceTarget=2;
		}
		// To Here 2001.12.03 hor

		return 1;
	}else{
		return 0;
	}
}




BOOL CDlgReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	//	Jun. 26, 2001 genta
	//	この位置で正規表現の初期化をする必要はない
	//	他との一貫性を保つため削除

	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	/* テキスト選択中か */
	if( m_bSelected ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHPREV ), FALSE );	// 2001.12.03 hor コメント解除
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SEARCHNEXT ), FALSE );	// 2001.12.03 hor コメント解除
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_REPALCE ), FALSE );		// 2001.12.03 hor コメント解除
		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, TRUE );
//		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, FALSE );						// 2001.12.03 hor コメント
	}else{
//		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_SELECTEDAREA ), FALSE );	// 2001.12.03 hor コメント
//		::CheckDlgButton( m_hWnd, IDC_RADIO_SELECTEDAREA, FALSE );					// 2001.12.03 hor コメント
		::CheckDlgButton( m_hWnd, IDC_RADIO_ALLAREA, TRUE );
	}
	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}




BOOL CDlgReplace::OnBnClicked( int wID )
{
	CEditView*	pcEditView = (CEditView*)m_lParam;
	int			nNewPos;
	int			nReplaceNum;
	char		szLabel[64];
	int			nAllLineNum;
	HWND		hwndProgress;
	HWND		hwndStatic;
//	MSG			msg;

// From Here 2001.12.03 hor
	int			colFrom;		//選択範囲開始桁
	int			linFrom;		//選択範囲開始行
	int			colTo,colToP;	//選択範囲終了桁
	int			linTo,linToP;	//選択範囲終了行
	int			colDif = 0;		//置換後の桁調整
	int			linDif = 0;		//置換後の行調整
	int			colOld = 0;		//検索後の選択範囲次桁
	int			linOld = 0;		//検索後の行
	int			lineCnt;		//置換前の行数
	int			linPrev = 0;	//前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
	int			linNext;		//次回の検索行(矩形)
	int			colTmp,linTmp,colLast,linLast;
	int			bBeginBoxSelect; // 矩形選択？
	const char*	pLine;
	int			nLineLen;
	const CLayout* pcLayout;
	int			bLineOffset=FALSE;
	int			bLineChecked=FALSE;

	switch( wID ){
	case IDC_CHK_PASTE:
		/* テキストの貼り付け */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE ) &&
			!pcEditView->m_pcEditDoc->IsEnablePaste() ){
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"クリップボードに有効なデータがありません！");
			::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
		}
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), !(::IsDlgButtonChecked( m_hWnd, IDC_CHK_PASTE)) );
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		/* 範囲範囲 */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		/* ファイル全体 */
		if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_ALLAREA ) ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHPREV ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_SEARCHNEXT ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_REPALCE ), FALSE );
		}
		return TRUE;
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		/* 「置換」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE) );
		return TRUE;
//	case IDC_CHK_LOHICASE:	/* 大文字と小文字を区別する */
//		MYTRACE( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
//	case IDC_CHK_WORDONLY:	/* 一致する単語のみ検索 */
//		MYTRACE( "IDC_CHK_WORDONLY\n" );
//		break;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
			// To Here Jun. 26, 2001 genta

				/* 英大文字と英小文字を区別する */
				//	Jan. 31, 2002 genta
				//	大文字・小文字の区別は正規表現の設定に関わらず保存する
				//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani
				/* 単語単位で探す */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );

			}
		}else{
			/* 英大文字と英小文字を区別する */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
			//	Jan. 31, 2002 genta
			//	大文字・小文字の区別は正規表現の設定に関わらず保存する
			//::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );

			// 2001/06/23 N.Nakatani
			/* 単語単位で探す */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );

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
			/* コマンドコードによる処理振り分け */
			/* 前を検索 */
			pcEditView->HandleCommand( F_SEARCH_PREV, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	/* 下検索 */
		if( 0 < GetData() ){
			/* コマンドコードによる処理振り分け */
			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			// 2001.12.03 hor
			//	ダイアログを閉じないとき、IDC_COMBO_TEXT 上で Enter した場合に
			//	キャレットが表示されなくなるのを回避する
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
			// To Here 2001.12.03 hor
               
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	//2002.01.16 hor 該当行マーク
		if( 0 < GetData() ){
			pcEditView->HandleCommand( F_BOOKMARK_PATTERN, NULL, 0, 0, 0, 0 );
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* 置換 */
		if( 0 < GetData() ){
			// From Here 2001.12.03 hor
			if( m_nPaste && !pcEditView->m_pcEditDoc->IsEnablePaste()){
				::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"クリップボードに有効なデータがありません！");
				::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
				return TRUE;
			}

			// 2002.01.09 hor
			// 選択エリアがあれば、その先頭にカーソルを移す
			if( pcEditView->IsTextSelected() ){
				if( pcEditView->m_bBeginBoxSelect ){
					pcEditView->MoveCursor( pcEditView->m_nSelectColmFrom,
											pcEditView->m_nSelectLineFrom,
											TRUE );
				} else {
					pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
				}
			}
			// To Here 2002.01.09 hor
			
			// 矩形選択？
//			bBeginBoxSelect = pcEditView->m_bBeginBoxSelect;

			/* カーソル左移動 */
			//pcEditView->HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//？？？
			// To Here 2001.12.03 hor

			/* テキスト選択解除 */
			/* 現在の選択範囲を非選択状態に戻す */
			pcEditView->DisableSelectArea( TRUE );

			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );

			/* テキストが選択されているか */
			if( pcEditView->IsTextSelected() ){
				// From Here 2001.12.03 hor
				if(m_nReplaceTarget==1){	//挿入位置へ移動
					colTmp = pcEditView->m_nSelectColmTo - pcEditView->m_nSelectColmFrom;
					linTmp = pcEditView->m_nSelectLineTo - pcEditView->m_nSelectLineFrom;
					pcEditView->m_nSelectColmFrom=-1;
					pcEditView->m_nSelectLineFrom=-1;
					pcEditView->m_nSelectColmTo	 =-1;
					pcEditView->m_nSelectLineTo	 =-1;
				}else
				if(m_nReplaceTarget==2){	//追加位置へ移動
					if(m_bRegularExp){
						//検索後の文字が改行やったら次の行の先頭へ移動
						pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							pcEditView->m_nSelectColmTo,
							pcEditView->m_nSelectLineTo,
							&colTmp,
							&linTmp
						);
						pLine = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr2( pcEditView->m_nSelectLineTo, &nLineLen, &pcLayout );
						if( NULL != pLine &&
							colTmp >= nLineLen - (pcLayout->m_cEol.GetLen()) ){
							pcEditView->m_nSelectColmTo=0;
							pcEditView->m_nSelectLineTo++;
						}
					}
					pcEditView->m_nCaretPosX = pcEditView->m_nSelectColmTo;
					pcEditView->m_nCaretPosY = pcEditView->m_nSelectLineTo;
					pcEditView->m_nSelectColmFrom=-1;
					pcEditView->m_nSelectLineFrom=-1;
					pcEditView->m_nSelectColmTo	 =-1;
					pcEditView->m_nSelectLineTo	 =-1;
				}
				/* コマンドコードによる処理振り分け */
				/* テキストを貼り付け */
				//pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
				if(m_nPaste){
					pcEditView->HandleCommand( F_PASTE, 0, 0, 0, 0, 0 );
				}else{
					// 2002/01/19 novice 正規表現による文字列置換
					if( pcEditView->m_bCurSrchRegularExp ){ /* 検索／置換  1==正規表現 */
						CMemory cmemory;
						CBregexp cRegexp;
						char*	RegRepOut;

						if( !InitRegexp( m_hWnd, cRegexp, true ) ){
							return 0;
						}

						if( FALSE == pcEditView->GetSelectedData( cmemory, FALSE, NULL, FALSE /*, EOL_NONE 2002/1/26 novice */ ) ){
							::MessageBeep( MB_ICONHAND );
						}
						// 変換後の文字列を別の引数にしました 2002.01.26 hor
						if( cRegexp.Replace( m_szText, m_szText2, cmemory.m_pData, cmemory.m_nDataLen ,&RegRepOut) ){
							pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)RegRepOut, FALSE, 0, 0 );
							delete [] RegRepOut;
						}
					}else{
						pcEditView->HandleCommand( F_INSTEXT, FALSE, (LPARAM)m_szText2, FALSE, 0, 0 );
					}
				}
				// 挿入後の検索開始位置を調整
				if(m_nReplaceTarget==1){
					pcEditView->m_nCaretPosX+=colTmp;
					pcEditView->m_nCaretPosY+=linTmp;
				}
				// To Here 2001.12.03 hor
				/* 次を検索 */
				pcEditView->HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, (LPARAM)"最後まで置換しました。", 0, 0 );
			}
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	/* すべて置換 */
		if( 0 < GetData() ){

		// From Here 2001.12.03 hor
			if( m_nPaste && !pcEditView->m_pcEditDoc->IsEnablePaste() ){
				::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"クリップボードに有効なデータがありません！");
				::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
				return TRUE;
			}
			// 矩形選択？
			bBeginBoxSelect = pcEditView->m_bBeginBoxSelect;
		// To Here 2001.12.03 hor


			/* 表示処理ON/OFF */
			BOOL bDisplayUpdate = FALSE;


			pcEditView->m_bDrawSWITCH = bDisplayUpdate;
			CDlgCancel	cDlgCancel;
			HWND		hwndCancel;
			nAllLineNum = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

			/* 進捗表示&中止ダイアログの作成 */
			hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_REPLACERUNNING );
			::EnableWindow( m_hWnd, FALSE );
			::EnableWindow( ::GetParent( m_hWnd ), FALSE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), FALSE );

			/* プログレスバー初期化 */
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
			nNewPos = 0;
 			::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );

			/* 置換個数初期化 */
			nReplaceNum = 0;
			hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
//			wsprintf( szLabel, "%d", nReplaceNum );
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
//			::SetWindowText( hwndStatic, szLabel );

		// From Here 2001.12.03 hor
			if (m_bSelectedArea) {
				/* 選択範囲置換 */
				/* 選択範囲開始位置の取得 */
				colFrom = pcEditView->m_nSelectColmFrom;
				linFrom = pcEditView->m_nSelectLineFrom;
				colTo   = pcEditView->m_nSelectColmTo;
				linTo   = pcEditView->m_nSelectLineTo;
				pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					colTo,
					linTo,
					&colToP,
					&linToP
				);
				//選択範囲開始位置へ移動
				pcEditView->MoveCursor( colFrom, linFrom, bDisplayUpdate );
			}else{
				/* ファイル全体置換 */
				/* ファイルの先頭に移動 */
				pcEditView->HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
			}
			colLast=pcEditView->m_nCaretPosX;
			linLast=pcEditView->m_nCaretPosY;

			/* テキスト選択解除 */
			/* 現在の選択範囲を非選択状態に戻す */
			pcEditView->DisableSelectArea( bDisplayUpdate );
			/* 次を検索 */
			pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
		// To Here 2001.12.03 hor

			/* テキストが選択されているか */
			while( pcEditView->IsTextSelected() ){
				/* キャンセルされたか */
//				if( cDlgCancel.IsCanceled() ){
				if( cDlgCancel.m_bCANCEL ){
					break;
				}
//				if( 0 == ( nReplaceNum % 8 ) ){
					/* 処理中のユーザー操作を可能にする */
					if( !::BlockingHook( hwndCancel ) ){
						return -1;
					}
//					if( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
//						if( msg.message == WM_QUIT ){
//							return -1;
//						}
//						if( !IsDialogMessage (hwndCancel, &msg ) ){
//							::TranslateMessage( &msg );
//							::DispatchMessage( &msg );
//						}
//					}
//				}
				if( 0 == ( nReplaceNum % 8 ) ){


//					if( 0 < nAllLineNum ){
						nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
						::PostMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
//					}
					_itoa( nReplaceNum, szLabel, 10 );
					::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(const char*)szLabel );
				}

//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_INSTEXT" );
//					gm_ProfileOutput = FALSE;
//#endif

				// From Here 2001.12.03 hor
					/* 検索後の位置を確認 */
					if(m_bSelectedArea){
						if (bBeginBoxSelect) {
						// 矩形選択
						//	o レイアウト座標をチェックしながら置換する
						//	o 折り返しがあると変になるかも・・・
						//
							// 検索時の行数を記憶
							lineCnt=pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
							// 検索後の範囲終端
							colOld = pcEditView->m_nSelectColmTo;
							linOld = pcEditView->m_nSelectLineTo;
							// 前回の検索行と違う？
							if(linOld!=linPrev){
								colDif=0;
							}
							linPrev=linOld;
							// 行は範囲内？
							if ((linTo+linDif == linOld && colTo+colDif < colOld) ||
								(linTo+linDif <  linOld)) {
								break;
							}
							// 桁は範囲内？
							if(!((colFrom<=pcEditView->m_nSelectColmFrom)&&
							     (colOld<=colTo+colDif))){
								if(colOld<colTo+colDif){
									linNext=pcEditView->m_nSelectLineTo;
								}else{
									linNext=pcEditView->m_nSelectLineTo+1;
								}
								//次の検索開始位置へシフト
								pcEditView->m_nCaretPosX=colFrom;
								pcEditView->m_nCaretPosY=linNext;
								//pcEditView->DisableSelectArea( bDisplayUpdate );
								//pcEditView->MoveCursor( colFrom, linNext, bDisplayUpdate );
								pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
								colDif=0;
								continue;
							}
						}else{
						// 普通の選択
						//	o 物理座標をチェックしながら置換する
						//
							// 検索時の行数を記憶
							lineCnt=pcEditView->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
							// 検索後の範囲終端
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nSelectColmTo,
								pcEditView->m_nSelectLineTo,
								&colOld,
								&linOld
							);
							// 行は範囲内？
							if ((linToP+linDif == linOld && colToP+colDif < colOld) ||
								(linToP+linDif <  linOld)) {
								break;
							}
						}
					}

					if(m_nReplaceTarget==1){	//挿入位置セット
						colTmp = pcEditView->m_nSelectColmTo - pcEditView->m_nSelectColmFrom;
						linTmp = pcEditView->m_nSelectLineTo - pcEditView->m_nSelectLineFrom;
						pcEditView->m_nSelectColmFrom=-1;
						pcEditView->m_nSelectLineFrom=-1;
						pcEditView->m_nSelectColmTo	 =-1;
						pcEditView->m_nSelectLineTo	 =-1;
					}else
					if(m_nReplaceTarget==2){	//追加位置セット
						if(!bLineChecked){
							//検索後の位置が改行やったら次の行の先頭にオフセット
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nSelectColmTo,
								pcEditView->m_nSelectLineTo,
								&colTmp,
								&linTmp
							);
							if(m_bRegularExp){
								pLine = pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr2( pcEditView->m_nSelectLineTo, &nLineLen, &pcLayout );
								if( NULL != pLine &&
									colTmp >= nLineLen - (pcLayout->m_cEol.GetLen()) ){
									bLineOffset=TRUE;
								}
							}
							bLineChecked=TRUE;
						}
						if(bLineOffset){
							pcEditView->m_nCaretPosX = 0;
							pcEditView->m_nCaretPosY ++;
							pcEditView->m_nCaretPosX_PHY = 0;
							pcEditView->m_nCaretPosY_PHY ++;
						}else{
							pcEditView->m_nCaretPosX = pcEditView->m_nSelectColmTo;
							pcEditView->m_nCaretPosY = pcEditView->m_nSelectLineTo;
						}
						pcEditView->m_nSelectColmFrom=-1;
						pcEditView->m_nSelectLineFrom=-1;
						pcEditView->m_nSelectColmTo	 =-1;
						pcEditView->m_nSelectLineTo	 =-1;
					}

					/* コマンドコードによる処理振り分け */
					/* テキストを貼り付け */
					//pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
					if(m_nPaste){
						pcEditView->HandleCommand( F_PASTE, 0, 0, 0, 0, 0 );
					}else{
						// 2002/01/19 novice 正規表現による文字列置換
						if( pcEditView->m_bCurSrchRegularExp ){ /* 検索／置換  1==正規表現 */
							CMemory cmemory;
							CBregexp cRegexp;
							char*	RegRepOut;

							if( !InitRegexp( m_hWnd, cRegexp, true ) ){
								return 0;
							}

							if( FALSE == pcEditView->GetSelectedData( cmemory, FALSE, NULL, FALSE /*, EOL_NONE 2002/1/26 novice */ ) ){
								::MessageBeep( MB_ICONHAND );
							}

							// 変換後の文字列を別の引数にしました 2002.01.26 hor
							if( cRegexp.Replace( m_szText, m_szText2, cmemory.m_pData, cmemory.m_nDataLen ,&RegRepOut) ){
								pcEditView->HandleCommand( F_INSTEXT, TRUE, (LPARAM)RegRepOut, FALSE, 0, 0 );
								delete [] RegRepOut;
							}
						}else{
							pcEditView->HandleCommand( F_INSTEXT, bDisplayUpdate, (LPARAM)m_szText2, TRUE, 0, 0 );
						}
					}

					// 挿入後の位置調整
					if(m_nReplaceTarget==1){
						pcEditView->m_nCaretPosX+=colTmp;
						pcEditView->m_nCaretPosY+=linTmp;
						if (!bBeginBoxSelect) {
							pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
								pcEditView->m_nCaretPosX,
								pcEditView->m_nCaretPosY,
								&pcEditView->m_nCaretPosX_PHY,
								&pcEditView->m_nCaretPosY_PHY
							);
						}
					}

					// 最後に置換した位置を記憶
					colLast=pcEditView->m_nCaretPosX;
					linLast=pcEditView->m_nCaretPosY;

					/* 置換後の位置を確認 */
					if(m_bSelectedArea){
						// 検索→置換の行補正値取得
						if(bBeginBoxSelect){
							colDif += colLast - colOld;
							linDif += pcEditView->m_pcEditDoc->m_cLayoutMgr.GetLineCount() - lineCnt;
						}else{
							colTmp=pcEditView->m_nCaretPosX_PHY;
							linTmp=pcEditView->m_nCaretPosY_PHY;
							linDif += pcEditView->m_pcEditDoc->m_cDocLineMgr.GetLineCount() - lineCnt;
							if(linToP+linDif==linTmp){
								colDif += colTmp - colOld;
							}
						}
					}
				// To Here 2001.12.03 hor

//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
				++nReplaceNum;

//#ifdef _DEBUG
//				{
//					CRunningTimer* pcRunningTimer = new CRunningTimer( (const char*)"F_SEARCH_NEXT" );
//					gm_ProfileOutput = FALSE;
//#endif

					/* 次を検索 */
					pcEditView->HandleCommand( F_SEARCH_NEXT, bDisplayUpdate, 0, 0, 0, 0 );
//#ifdef _DEBUG
//					gm_ProfileOutput = TRUE;
//					delete pcRunningTimer;
//					gm_ProfileOutput = FALSE;
//				}
//#endif
			}
			if( 0 < nAllLineNum ){
				nNewPos = (pcEditView->m_nSelectLineFrom * 100) / nAllLineNum;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
			_itoa( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

			if( !cDlgCancel.IsCanceled() ){
				nNewPos = 100;
				::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			}
//			cDlgCancel.Close();
			cDlgCancel.CloseDialog( 0 );
			::EnableWindow( m_hWnd, TRUE );
			::EnableWindow( ::GetParent( m_hWnd ), TRUE );
			::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );


		// From Here 2001.12.03 hor

			/* テキスト選択解除 */
			pcEditView->DisableSelectArea( TRUE );

			/* カーソル・選択範囲復元 */
			if((!m_bSelectedArea) ||			// ファイル全体置換
			   (cDlgCancel.IsCanceled())) {		// キャンセルされた
				// 最後に置換した文字列の右へ
				pcEditView->MoveCursor( colLast, linLast, TRUE );
			}else{
				if (bBeginBoxSelect) {
				// 矩形選択
					pcEditView->m_bBeginBoxSelect=bBeginBoxSelect;
					linTo+=linDif;
					if(linTo<0)linTo=0;
				}else{
				// 普通の選択
					colToP+=colDif;
					if(colToP<0)colToP=0;
					linToP+=linDif;
					if(linToP<0)linToP=0;
					pcEditView->m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
						colToP,
						linToP,
						&colTo,
						&linTo
					);
				}
				if(linFrom<linTo || colFrom<colTo){
					pcEditView->m_nSelectLineFrom = linFrom;
					pcEditView->m_nSelectColmFrom = colFrom;
					pcEditView->m_nSelectLineTo   = linTo;
					pcEditView->m_nSelectColmTo   = colTo;
				}
				pcEditView->MoveCursor( colTo, linTo, TRUE );
			}
		// To Here 2001.12.03 hor

			// 再描画
			pcEditView->m_bDrawSWITCH = TRUE;
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* アクティブにする */
			ActivateFrameWindow( m_hWnd );

			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
				"%d箇所を置換しました。", nReplaceNum
			);

//			nNewPos = 100;
// 			::SendMessage( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), PBM_SETPOS, nNewPos, 0 );

//			::ShowWindow( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), SW_HIDE );

			if( !cDlgCancel.IsCanceled() ){
				if( m_bModal ){		/* モーダルダイアログか */
					/* 置換ダイアログを閉じる */
					::EndDialog( m_hWnd, 0 );
				}else{
					/* 置換 ダイアログを自動的に閉じる */
					if( m_pShareData->m_Common.m_bAutoCloseDlgReplace ){
						::DestroyWindow( m_hWnd );
					}
				}
			}


			return TRUE;
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"置換条件を指定してください。"
			);
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog( hwndDlg, 0 );
//		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
