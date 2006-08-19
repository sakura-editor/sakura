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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CDlgReplace.h"
#include "debug.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "CLayout.h"/// 2002/2/3 aroka
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理

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
	IDC_BUTTON_REPLACEALL_LINE,		HIDC_REP_BUTTON_REPLACEALL_LINE,	//すべて行置換	// 2006.08.06 ryoji
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
	m_nReplaceTarget = 0;		/* 置換対象 */		// 2001.12.03 hor
	m_nPaste = FALSE;			/* 貼り付ける？ */	// 2001.12.03 hor
	m_nReplaceCnt = 0;			//すべて置換の実行結果		// 2002.02.08 hor
	m_bCanceled = false;		//すべて置換を中断したか	// 2002.02.08 hor
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
	m_nEscCaretPosX_PHY = ((CEditView*)lParam)->m_nCaretPosX_PHY;	/* 検索/置換開始時のカーソル位置退避	02/07/28 ai */
	m_nEscCaretPosY_PHY = ((CEditView*)lParam)->m_nCaretPosY_PHY;	/* 検索/置換開始時のカーソル位置退避	02/07/28 ai */
	((CEditView*)lParam)->m_bSearch = TRUE;							/* 検索/置換開始位置の登録有無			02/07/28 ai */
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
		//	正規表現のときも選択できるように。
//		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

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
//	int			i;
//	int			j;
//	CMemory*	pcmWork;

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
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, sizeof( m_szText ));
	/* 置換後文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT2, m_szText2, sizeof( m_szText2 ));

	/* 置換 ダイアログを自動的に閉じる */
	m_pShareData->m_Common.m_bAutoCloseDlgReplace = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_bAutoCloseDlgReplace );

	/* 先頭（末尾）から再検索 2002.01.26 hor */
	m_pShareData->m_Common.m_bSearchAll = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_SEARCHALL );

	if( 0 < lstrlen( m_szText ) ){
		/* 正規表現？ */
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag ) ){
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		/* 検索文字列 */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()追加に伴う変更
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );

		/* 置換後文字列 */
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeyArr()追加に伴う変更
		CShareData::getInstance()->AddToReplaceKeyArr( m_szText2 );

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
//	int			nNewPos;
//	int			nReplaceNum;
//	char		szLabel[64];
//	int			nAllLineNum;
//	HWND		hwndProgress;
//	HWND		hwndStatic;
//	MSG			msg;

// From Here 2001.12.03 hor
//	int			colFrom;		//選択範囲開始桁
//	int			linFrom;		//選択範囲開始行
//	int			colTo,colToP;	//選択範囲終了桁
//	int			linTo,linToP;	//選択範囲終了行
//	int			colDif = 0;		//置換後の桁調整
//	int			linDif = 0;		//置換後の行調整
//	int			colOld = 0;		//検索後の選択範囲次桁
//	int			linOld = 0;		//検索後の行
//	int			lineCnt;		//置換前の行数
//	int			linPrev = 0;	//前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
//	int			linNext;		//次回の検索行(矩形)
//	int			colTmp,linTmp,colLast,linLast;
//	int			bBeginBoxSelect; // 矩形選択？
//	const char*	pLine;
//	int			nLineLen;
//	const CLayout* pcLayout;
//	int			bLineOffset=FALSE;
//	int			bLineChecked=FALSE;

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
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG) );
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
				//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				// 2001/06/23 N.Nakatani
				/* 単語単位で探す */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );

			}
		}else{
			/* 英大文字と英小文字を区別する */
			//::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
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

			// 検索開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

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

			// 検索開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

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
			pcEditView->HandleCommand( F_BOOKMARK_PATTERN, FALSE, 0, 0, 0, 0 );
			::SendMessage(m_hWnd,WM_NEXTDLGCTL,(WPARAM)::GetDlgItem(m_hWnd,IDC_COMBO_TEXT ),TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	/* 置換 */
		if( 0 < GetData() ){

			// 置換開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* 置換 */
			//@@@ 2002.2.2 YAZAKI 置換コマンドをCEditViewに新設
			//@@@ 2002/04/08 YAZAKI 親ウィンドウのハンドルを渡すように変更。
			pcEditView->HandleCommand( F_REPLACE, TRUE, (LPARAM)m_hWnd, 0, 0, 0 );
			/* 再描画 */
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"文字列を指定してください。"
			);
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	/* すべて置換 */
	case IDC_BUTTON_REPLACEALL_LINE: /* すべて行置換 2006.01.22 かろと */
		if( 0 < GetData() ){
			TCHAR *szUnit = _T("");	// 置換数表示の単位 2006.04.02 かろと
			// 置換開始位置を登録 02/07/28 ai start
			if( TRUE == pcEditView->m_bSearch ){
				pcEditView->m_nSrchStartPosX_PHY = m_nEscCaretPosX_PHY;
				pcEditView->m_nSrchStartPosY_PHY = m_nEscCaretPosY_PHY;
				pcEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

			/* すべて行置換時の処置追加 2006.01.22 かろと */
			if ( wID == IDC_BUTTON_REPLACEALL_LINE ) {
				pcEditView->HandleCommand( F_REPLACE_ALL_LINE, TRUE, 0, 0, 0, 0 );
				szUnit = _T("行");
			} else {
				pcEditView->HandleCommand( F_REPLACE_ALL, TRUE, 0, 0, 0, 0 );
				szUnit = _T("箇所");
			}
			pcEditView->HandleCommand( F_REDRAW, TRUE, 0, 0, 0, 0 );

			/* アクティブにする */
			ActivateFrameWindow( m_hWnd );

			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
				"%d%sを置換しました。", m_nReplaceCnt, szUnit);

//			nNewPos = 100;
// 			::SendMessage( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), PBM_SETPOS, nNewPos, 0 );

//			::ShowWindow( ::GetDlgItem( m_hWnd, IDC_PROGRESS_REPLACE ), SW_HIDE );

//			if( !cDlgCancel.IsCanceled() ){
			if( !m_bCanceled ){
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
