/*!	@file
	@brief 印刷設定ダイアログ

	@author Norio Nakatani
	
	@date 2006.08.14 Moca 用紙方向コンボボックスを廃止し、ボタンを有効化．
		用紙名一覧の重複削除．
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, Stonee
	Copyright (C) 2002, MIK, aroka, YAZAKI
	Copyright (C) 2003, かろと
	Copyright (C) 2006, ryoji

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
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgInput1.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"	// 2002/2/10 aroka
#include "sakura.hh"

// 印刷設定 CDlgPrintSetting.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12500
	IDC_COMBO_SETTINGNAME,			HIDC_PS_COMBO_SETTINGNAME,	//ページ設定
	IDC_BUTTON_EDITSETTINGNAME,		HIDC_PS_BUTTON_EDITSETTINGNAME,	//設定名変更
	IDC_COMBO_FONT_HAN,				HIDC_PS_COMBO_FONT_HAN,		//半角フォント
	IDC_COMBO_FONT_ZEN,				HIDC_PS_COMBO_FONT_ZEN,		//全角フォント
	IDC_EDIT_FONTHEIGHT,			HIDC_PS_EDIT_FONTHEIGHT,	//フォント高
	IDC_SPIN_FONTHEIGHT,			HIDC_PS_EDIT_FONTHEIGHT,	//12570,
	IDC_SPIN_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//12571,
	IDC_EDIT_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//行送り
	IDC_EDIT_DANSUU,				HIDC_PS_EDIT_DANSUU,		//段数
	IDC_SPIN_DANSUU,				HIDC_PS_EDIT_DANSUU,		//12572,
	IDC_EDIT_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//段の隙間
	IDC_SPIN_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//12573,
	IDC_COMBO_PAPER,				HIDC_PS_COMBO_PAPER,		//用紙サイズ
	IDC_RADIO_PORTRAIT,				HIDC_PS_STATIC_PAPERORIENT,	//横向き
	IDC_RADIO_LANDSCAPE,			HIDC_PS_STATIC_PAPERORIENT,	//縦向き
	IDC_EDIT_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//余白上
	IDC_SPIN_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//12574,
	IDC_EDIT_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//余白下
	IDC_SPIN_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//12575,
	IDC_EDIT_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//余白左
	IDC_SPIN_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//12576,
	IDC_EDIT_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//余白右
	IDC_SPIN_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//12577,
	IDC_CHECK_WORDWRAP,				HIDC_PS_CHECK_WORDWRAP,		//ワードラップ
	IDC_CHECK_LINENUMBER,			HIDC_PS_CHECK_LINENUMBER,	//行番号
	IDC_CHECK_PS_KINSOKUHEAD,		HIDC_PS_CHECK_KINSOKUHEAD,	//行頭禁則	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKUTAIL,		HIDC_PS_CHECK_KINSOKUTAIL,	//行末禁則	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKURET,		HIDC_PS_CHECK_KINSOKURET,	//改行文字をぶら下げる	//@@@ 2002.04.14 MIK
	IDC_CHECK_PS_KINSOKUKUTO,		HIDC_PS_CHECK_KINSOKUKUTO,	//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	IDC_CHECK_COLORPRINT,			HIDC_PS_CHECK_COLORPRINT,	//カラー印刷			// 2013/4/26 Uchi
	IDC_EDIT_HEAD1,					HIDC_PS_EDIT_HEAD1,			//ヘッダー(左寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_HEAD2,					HIDC_PS_EDIT_HEAD2,			//ヘッダー(中央寄せ)	// 2006.10.11 ryoji
	IDC_EDIT_HEAD3,					HIDC_PS_EDIT_HEAD3,			//ヘッダー(右寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT1,					HIDC_PS_EDIT_FOOT1,			//フッター(左寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT2,					HIDC_PS_EDIT_FOOT2,			//フッター(中央寄せ)	// 2006.10.11 ryoji
	IDC_EDIT_FOOT3,					HIDC_PS_EDIT_FOOT3,			//フッター(右寄せ)		// 2006.10.11 ryoji
	IDC_CHECK_USE_FONT_HEAD,		HIDC_PS_FONT_HEAD,			//ヘッダー(フォント)	// 2013.05.16 Uchi
	IDC_BUTTON_FONT_HEAD,			HIDC_PS_FONT_HEAD,			//ヘッダー(フォント)	// 2013.05.16 Uchi
	IDC_CHECK_USE_FONT_FOOT,		HIDC_PS_FONT_FOOT,			//フッター(フォント)	// 2013/5/16 Uchi
	IDC_BUTTON_FONT_FOOT,			HIDC_PS_FONT_FOOT,			//フッター(フォント)	// 2013/5/16 Uchi
	IDOK,							HIDOK_PS,					//OK
	IDCANCEL,						HIDCANCEL_PS,				//キャンセル
	IDC_BUTTON_HELP,				HIDC_PS_BUTTON_HELP,		//ヘルプ
	0, 0
};	//@@@ 2002.01.07 add end MIK

#define IDT_PRINTSETTING 1467

int CALLBACK SetData_EnumFontFamProc(
	ENUMLOGFONT*	pelf,	// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,	// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 	// address of application-defined data
)
{
	CDlgPrintSetting*	pCDlgPrintSetting;
	HWND				hwndComboFontHan;
	HWND				hwndComboFontZen;
	pCDlgPrintSetting = (CDlgPrintSetting*)lParam;
	hwndComboFontHan = ::GetDlgItem( pCDlgPrintSetting->GetHwnd(), IDC_COMBO_FONT_HAN );
	hwndComboFontZen = ::GetDlgItem( pCDlgPrintSetting->GetHwnd(), IDC_COMBO_FONT_ZEN );

	/* LOGFONT */
	Combo_AddString( hwndComboFontHan, pelf->elfLogFont.lfFaceName );
	Combo_AddString( hwndComboFontZen, pelf->elfLogFont.lfFaceName );
	return 1;
}

/* モーダルダイアログの表示 */
int CDlgPrintSetting::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	int*			pnCurrentPrintSetting,
	PRINTSETTING*	pPrintSettingArr,
	int				nLineNumberColumns
)
{
	int		nRet;
	int		i;
	m_nCurrentPrintSetting = *pnCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		m_PrintSettingArr[i] = pPrintSettingArr[i];
	}
	m_nLineNumberColumns = nLineNumberColumns;

	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTSETTING, (LPARAM)NULL );
	if( FALSE != nRet ){
		*pnCurrentPrintSetting = m_nCurrentPrintSetting;
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			pPrintSettingArr[i] = m_PrintSettingArr[i];
		}
	}
	return nRet;
}

BOOL CDlgPrintSetting::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER ), TRUE );

	// タイマーでの更新をやめて、能動的に更新要求する 2013.5.5 aroka
	// CDialog::OnInitDialogの奥でOnChangeSettingTypeが呼ばれるのでここでは更新要求しない
	//	::SetTimer( GetHwnd(), IDT_PRINTSETTING, 500, NULL );
	//UpdatePrintableLineAndColumn();

	// ダイアログのフォントの取得
	m_hFontDlg = (HFONT)::SendMessage( GetHwnd(), WM_GETFONT, 0, 0 );	// ダイアログのフォント
	LOGFONT	lf;
	::GetObject(m_hFontDlg, sizeof(LOGFONT), &lf);
	m_nFontHeight = lf.lfHeight;		// フォントサイズ

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgPrintSetting::OnDestroy( void )
{
	::KillTimer( GetHwnd(), IDT_PRINTSETTING );

	// フォントの破棄
	HFONT	hFontOld;
	hFontOld = (HFONT)::SendMessage(::GetDlgItem( GetHwnd(), IDC_STATIC_FONT_HEAD ), WM_GETFONT, 0, 0 );
	if (m_hFontDlg != hFontOld) {
		::DeleteObject( hFontOld );
	}
	hFontOld = (HFONT)::SendMessage(::GetDlgItem( GetHwnd(), IDC_STATIC_FONT_FOOT ), WM_GETFONT, 0, 0 );
	if (m_hFontDlg != hFontOld) {
		::DeleteObject( hFontOld );
	}

	/* 基底クラスメンバ */
	return CDialog::OnDestroy();
}


BOOL CDlgPrintSetting::OnNotify( WPARAM wParam, LPARAM lParam )
{
	CDlgInput1		cDlgInput1;
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	BOOL			bSpinDown;
	idCtrl = (int)wParam;
	pMNUD  = (NM_UPDOWN*)lParam;
	if( pMNUD->iDelta < 0 ){
		bSpinDown = FALSE;
	}else{
		bSpinDown = TRUE;
	}
	switch( idCtrl ){
	case IDC_SPIN_FONTHEIGHT:
	case IDC_SPIN_LINESPACE:
	case IDC_SPIN_DANSUU:
	case IDC_SPIN_DANSPACE:
	case IDC_SPIN_MARGINTY:
	case IDC_SPIN_MARGINBY:
	case IDC_SPIN_MARGINLX:
	case IDC_SPIN_MARGINRX:
		/* スピンコントロールの処理 */
		OnSpin( idCtrl, bSpinDown );
		UpdatePrintableLineAndColumn();
		break;
	}
	return TRUE;
}

BOOL CDlgPrintSetting::OnCbnSelChange( HWND hwndCtl, int wID )
{
//	if( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ) == hwndCtl ){
	switch( wID ){
	case IDC_COMBO_SETTINGNAME:
		/* 設定のタイプが変わった */
		OnChangeSettingType( TRUE );
		return TRUE;
	case IDC_COMBO_FONT_HAN:
	case IDC_COMBO_FONT_ZEN:
	case IDC_COMBO_PAPER:
		UpdatePrintableLineAndColumn();
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	}
	return FALSE;
}


BOOL CDlgPrintSetting::OnBnClicked( int wID )
{
	TCHAR			szWork[256];
	CDlgInput1		cDlgInput1;
	HWND			hwndComboSettingName;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「印刷ページ設定」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PAGESETUP) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDC_BUTTON_EDITSETTINGNAME:
		_tcscpy( szWork, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName );
		{
			BOOL bDlgInputResult=cDlgInput1.DoModal(
				m_hInstance,
				GetHwnd(),
				LS(STR_DLGPRNST1),
				LS(STR_DLGPRNST2),
				_countof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1,
				szWork
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		if( szWork[0] != _T('\0') ){
			int		size = _countof(m_PrintSettingArr[0].m_szPrintSettingName) - 1;
			_tcsncpy( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName, szWork, size);
			m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName[size] = _T('\0');
			/* 印刷設定名一覧 */
			hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
			Combo_ResetContent( hwndComboSettingName );
			int		nSelectIdx;
			int		i;
			int		nItemIdx;
			nSelectIdx = 0;
			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
				nItemIdx = Combo_AddString( 
					hwndComboSettingName,
					m_PrintSettingArr[i].m_szPrintSettingName
				);
				Combo_SetItemData( hwndComboSettingName, nItemIdx, i );
				if( i == m_nCurrentPrintSetting ){
					nSelectIdx = nItemIdx;
				}
			}
			Combo_SetCurSel( hwndComboSettingName, nSelectIdx );
		}
		return TRUE;
	case IDC_BUTTON_FONT_HEAD:
		{
			LOGFONT	lf = m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader;
			INT		nPointSize;

			if (lf.lfFaceName[0] == _T('\0')) {
				// 半角フォントを設定
				auto_strcpy( lf.lfFaceName, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
				// 1/10mm→画面ドット数
				lf.lfHeight = -( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight * 
					::GetDeviceCaps ( ::GetDC( m_hwndParent ), LOGPIXELSY ) / 254 );
			}

			if (MySelectFont( &lf, &nPointSize, GetHwnd(), false)) {
				m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader = lf;
				m_PrintSettingArr[m_nCurrentPrintSetting].m_nHeaderPointSize = nPointSize;
				SetFontName( IDC_STATIC_FONT_HEAD, IDC_CHECK_USE_FONT_HEAD,
					m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader,
					m_PrintSettingArr[m_nCurrentPrintSetting].m_nHeaderPointSize );
				UpdatePrintableLineAndColumn();
			}
		}
		return TRUE;
	case IDC_BUTTON_FONT_FOOT:
		{
			LOGFONT	lf = m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter;
			INT		nPointSize;

			if (lf.lfFaceName[0] == _T('\0')) {
				// 半角フォントを設定
				auto_strcpy( lf.lfFaceName, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
				// 1/10mm→画面ドット数
				lf.lfHeight = -( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight * 
					::GetDeviceCaps ( ::GetDC( m_hwndParent ), LOGPIXELSY ) / 254 );
			}

			if (MySelectFont( &lf, &nPointSize, GetHwnd(), false)) {
				m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter = lf;
				m_PrintSettingArr[m_nCurrentPrintSetting].m_nFooterPointSize = nPointSize;
				SetFontName( IDC_STATIC_FONT_FOOT, IDC_CHECK_USE_FONT_FOOT,
					m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter,
					m_PrintSettingArr[m_nCurrentPrintSetting].m_nFooterPointSize );
				UpdatePrintableLineAndColumn();
			}
		}
		return TRUE;
	case IDC_CHECK_USE_FONT_HEAD:
		if (m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader.lfFaceName[0] != _T('\0')) {
			memset( &m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader, 0, sizeof(LOGFONT) );
			m_PrintSettingArr[m_nCurrentPrintSetting].m_nHeaderPointSize = 0;
			SetFontName( IDC_STATIC_FONT_HEAD, IDC_CHECK_USE_FONT_HEAD,
				m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader,
				m_PrintSettingArr[m_nCurrentPrintSetting].m_nHeaderPointSize );
		}
		UpdatePrintableLineAndColumn();
		return TRUE;
	case IDC_CHECK_USE_FONT_FOOT:
		if (m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter.lfFaceName[0] != _T('\0')) {
			memset( &m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter, 0, sizeof(LOGFONT) );
			m_PrintSettingArr[m_nCurrentPrintSetting].m_nFooterPointSize = 0;
			SetFontName( IDC_STATIC_FONT_FOOT, IDC_CHECK_USE_FONT_FOOT,
				m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter,
				m_PrintSettingArr[m_nCurrentPrintSetting].m_nFooterPointSize );
		}
		UpdatePrintableLineAndColumn();
		return TRUE;
	case IDOK:
		if( CalcPrintableLineAndColumn() ){
			/* ダイアログデータの取得 */
			::EndDialog( GetHwnd(), GetData() );
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	case IDC_RADIO_PORTRAIT:
	case IDC_RADIO_LANDSCAPE:
		UpdatePrintableLineAndColumn();
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	case IDC_CHECK_LINENUMBER:
		UpdatePrintableLineAndColumn();
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}


BOOL CDlgPrintSetting::OnStnClicked( int wID )
{
	switch( wID ){
	case IDC_STATIC_ENABLECOLUMNS:
	case IDC_STATIC_ENABLELINES:
		// 現状クリックは受け付けていないが、メッセージ処理したいのでここに配置 2013.5.5 aroka
		// メッセージが連続して送られたときは一回だけ対応する 2013.5.5 aroka
		if( m_bPrintableLinesAndColumnInvalid ){
			m_bPrintableLinesAndColumnInvalid = false;
			CalcPrintableLineAndColumn();
		}
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnStnClicked( wID );
}


BOOL CDlgPrintSetting::OnEnChange( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_EDIT_FONTHEIGHT:	// フォント幅の最小値が非０のため'12'と入力すると'1'のところで蹴られてしまう 2013.5.5 aroka
		if( ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FONTHEIGHT, NULL, FALSE ) >=10 ){	// 二桁以上の場合は領域チェック 2013.5.20 aroka
			UpdatePrintableLineAndColumn();
		}
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	case IDC_EDIT_LINESPACE:
	case IDC_EDIT_DANSUU:
	case IDC_EDIT_DANSPACE:
	case IDC_EDIT_MARGINTY:
	case IDC_EDIT_MARGINBY:
	case IDC_EDIT_MARGINLX:
	case IDC_EDIT_MARGINRX:
		UpdatePrintableLineAndColumn();
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	}
	/* 基底クラスメンバ */
	return CDialog::OnEnChange( hwndCtl, wID );
}


BOOL CDlgPrintSetting::OnEnKillFocus( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_EDIT_FONTHEIGHT:
	//case IDC_EDIT_LINESPACE:	// EN_CHANGE で計算しているので冗長かな、と思いコメントアウト 2013.5.5 aroka
	//case IDC_EDIT_DANSUU:
	//case IDC_EDIT_DANSPACE:
	//case IDC_EDIT_MARGINTY:
	//case IDC_EDIT_MARGINBY:
	//case IDC_EDIT_MARGINLX:
	//case IDC_EDIT_MARGINRX:
	case IDC_EDIT_HEAD1:	// テキスト編集のたびにチェックすると遅いのでフォーカス移動時のみ 2013.5.12 aroka
	case IDC_EDIT_HEAD2:
	case IDC_EDIT_HEAD3:
	case IDC_EDIT_FOOT1:
	case IDC_EDIT_FOOT2:
	case IDC_EDIT_FOOT3:
		UpdatePrintableLineAndColumn();
		break;	// ここでは行と桁の更新要求のみ。後の処理はCDialogに任せる。
	}
	/* 基底クラスメンバ */
	return CDialog::OnEnKillFocus( hwndCtl, wID );
}


/* ダイアログデータの設定 */
void CDlgPrintSetting::SetData( void )
{
	HDC		hdc;
	HWND	hwndComboFont;
	HWND	hwndComboPaper;
	HWND	hwndComboSettingName;
	int	i;
	int	nItemIdx;
	int	nSelectIdx;


	/* フォント一覧 */
	hdc = ::GetDC( m_hwndParent );
	hwndComboFont = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	Combo_ResetContent( hwndComboFont );
	hwndComboFont = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	Combo_ResetContent( hwndComboFont );
	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)SetData_EnumFontFamProc,
		(LPARAM)this
	);
	::ReleaseDC( m_hwndParent, hdc );

	/* 用紙サイズ一覧 */
	hwndComboPaper = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	Combo_ResetContent( hwndComboPaper );
	// 2006.08.14 Moca 用紙名一覧の重複削除
	for( i = 0; i < CPrint::m_nPaperInfoArrNum; ++i ){
		nItemIdx = Combo_AddString( hwndComboPaper, CPrint::m_paperInfoArr[i].m_pszName );
		Combo_SetItemData( hwndComboPaper, nItemIdx, CPrint::m_paperInfoArr[i].m_nId );
	}


	/* 印刷設定名一覧 */
	hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
	Combo_ResetContent( hwndComboSettingName );
	nSelectIdx = 0;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		nItemIdx = Combo_AddString( hwndComboSettingName, m_PrintSettingArr[i].m_szPrintSettingName );
		Combo_SetItemData( hwndComboSettingName, nItemIdx, i );
		if( i == m_nCurrentPrintSetting ){
			nSelectIdx = nItemIdx;
		}
	}
	Combo_SetCurSel( hwndComboSettingName, nSelectIdx );

	/* 設定のタイプが変わった */
	OnChangeSettingType( FALSE );

	return;
}




/* ダイアログデータの取得 */
/* TRUE==正常 FALSE==入力エラー */
int CDlgPrintSetting::GetData( void )
{
	HWND	hwndCtrl;
	int		nIdx1;
	int		nWork;

	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	Combo_GetLBText( hwndCtrl, nIdx1,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan
	);
	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	Combo_GetLBText( hwndCtrl, nIdx1,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen
	);

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FONTHEIGHT, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, NULL, FALSE ) * 10;

	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	nWork = DataCheckAndCorrect( IDC_EDIT_FONTHEIGHT, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTHEIGHT, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight, FALSE );
	}
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = ( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight+1 ) / 2;

	nWork = DataCheckAndCorrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	}

	/* 用紙サイズ一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize =
		(short)Combo_GetItemData( hwndCtrl, nIdx1 );

	// 用紙の向き
	// 2006.08.14 Moca 用紙方向コンボボックスを廃止し、ボタンを有効化
	if( IsDlgButtonCheckedBool( GetHwnd(), IDC_RADIO_PORTRAIT ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_LANDSCAPE;
	}

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, NULL, FALSE ) * 10;

	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );
	}

	// 行番号を印刷
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_LINENUMBER );
	// 英文ワードラップ
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_WORDWRAP );

	//行頭禁則	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD );
	//行末禁則	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL );
	//改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKURET );
	//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO );

	// カラー印刷
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bColorPrint =
		( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_COLORPRINT ) );

	//@@@ 2002.2.4 YAZAKI
	/* ヘッダー */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	/* フッター */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	// ヘッダフォント
	if (!IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_USE_FONT_HEAD )) {
		memset( &m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader, 0, sizeof(LOGFONT) );
	}
	// フッタフォント
	if (!IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_USE_FONT_FOOT )) {
		memset( &m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter, 0, sizeof(LOGFONT) );
	}

	return TRUE;
}


/* 設定のタイプが変わった */
void CDlgPrintSetting::OnChangeSettingType( BOOL bGetData )
{
	HWND	hwndComboSettingName;
	HWND	hwndCtrl;
	int		nIdx1;
	int		nItemNum;
	int		nItemData;
	int		i;

	if( bGetData ){
		GetData();
	}

	hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
	nIdx1 = Combo_GetCurSel( hwndComboSettingName );
	if( CB_ERR == nIdx1 ){
		return;
	}
	m_nCurrentPrintSetting = Combo_GetItemData( hwndComboSettingName, nIdx1 );

	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTHEIGHT, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );

	/* 用紙サイズ一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nItemNum = Combo_GetCount( hwndCtrl );
	for( i = 0; i < nItemNum; ++i ){
		nItemData = Combo_GetItemData( hwndCtrl, i );
		if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize == nItemData ){
			Combo_SetCurSel( hwndCtrl, i );
			break;
		}
	}

	// 用紙の向き
	// 2006.08.14 Moca 用紙方向コンボボックスを廃止し、ボタンを有効化
	bool bIsPortrait = ( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation == DMORIENT_PORTRAIT );
	CheckDlgButtonBool( GetHwnd(), IDC_RADIO_PORTRAIT, bIsPortrait );
	CheckDlgButtonBool( GetHwnd(), IDC_RADIO_LANDSCAPE, !bIsPortrait );

	// 余白
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	// 行番号を印刷
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_LINENUMBER, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber );
	// 英文ワードラップ
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_WORDWRAP, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap );

	// 行頭禁則	//@@@ 2002.04.09 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead );
	// 行末禁則	//@@@ 2002.04.09 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail );

	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKURET, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet );
	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto );

	// カラー印刷
	::CheckDlgButton( GetHwnd(), IDC_CHECK_COLORPRINT, 
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bColorPrint ? BST_CHECKED : BST_UNCHECKED);

	/* ヘッダー */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	/* フッター */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	// ヘッダフォント
	SetFontName( IDC_STATIC_FONT_HEAD, IDC_CHECK_USE_FONT_HEAD,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_lfHeader,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nHeaderPointSize );
	// フッタフォント
	SetFontName( IDC_STATIC_FONT_FOOT, IDC_CHECK_USE_FONT_FOOT,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_lfFooter,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nFooterPointSize );

	UpdatePrintableLineAndColumn();
	return;
}


const struct {
	int ctrlid;
	int minval;
	int maxval;
} sDataRange[] = {
	{ IDC_EDIT_FONTHEIGHT,	7,	200},	//!< 1/10mm
	{ IDC_EDIT_LINESPACE,	0,	150},	//!< %
	{ IDC_EDIT_DANSUU,		1,	4  },
	{ IDC_EDIT_DANSPACE,	0,	30 },	//!< mm
	{ IDC_EDIT_MARGINTY,	0,	50 },	//!< mm
	{ IDC_EDIT_MARGINBY,	0,	50 },	//!< mm
	{ IDC_EDIT_MARGINLX,	0,	50 },	//!< mm
	{ IDC_EDIT_MARGINRX,	0,	50 },	//!< mm
};

/* スピンコントロールの処理 */
void CDlgPrintSetting::OnSpin( int nCtrlId, BOOL bDown )
{
	int		nData = 0;
	int		nCtrlIdEDIT = 0;
	int		nDiff = 1;
	int		nIdx = -1;
	switch( nCtrlId ){
	case IDC_SPIN_FONTHEIGHT:	nIdx = 0;				break;
	case IDC_SPIN_LINESPACE:	nIdx = 1;	nDiff=10;	break;
	case IDC_SPIN_DANSUU:		nIdx = 2;				break;
	case IDC_SPIN_DANSPACE:		nIdx = 3;				break;
	case IDC_SPIN_MARGINTY:		nIdx = 4;				break;
	case IDC_SPIN_MARGINBY:		nIdx = 5;				break;
	case IDC_SPIN_MARGINLX:		nIdx = 6;				break;
	case IDC_SPIN_MARGINRX:		nIdx = 7;				break;
	}
	if( nIdx >= 0 ){
		nCtrlIdEDIT = sDataRange[nIdx].ctrlid;
 		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
 		if( bDown ){
			nData -= nDiff;
 		}else{
			nData += nDiff;
 		}
		/* 入力値(数値)のエラーチェックをして正しい値を返す */
		nData = DataCheckAndCorrect( nCtrlIdEDIT, nData );
		::SetDlgItemInt( GetHwnd(), nCtrlIdEDIT, nData, FALSE );
	}
}


/* 入力値(数値)のエラーチェックをして正しい値を返す */
int CDlgPrintSetting::DataCheckAndCorrect( int nCtrlId, int nData )
{
	int nIdx = -1;
	switch( nCtrlId ){
	case IDC_EDIT_FONTHEIGHT:	nIdx = 0;		break;
	case IDC_EDIT_LINESPACE:	nIdx = 1;		break;
	case IDC_EDIT_DANSUU:		nIdx = 2;		break;
	case IDC_EDIT_DANSPACE:		nIdx = 3;		break;
	case IDC_EDIT_MARGINTY:		nIdx = 4;		break;
	case IDC_EDIT_MARGINBY:		nIdx = 5;		break;
	case IDC_EDIT_MARGINLX:		nIdx = 6;		break;
	case IDC_EDIT_MARGINRX:		nIdx = 7;		break;
	}
	if( nIdx >= 0 ){
		if( nData <= sDataRange[nIdx].minval ){
			nData = sDataRange[nIdx].minval;
 		}
		if( nData > sDataRange[nIdx].maxval ){
			nData = sDataRange[nIdx].maxval;
 		}
	}
	return nData;
}


/*!
	印字可能行数と桁数を計算
	@date 2013.05.05 aroka OnTimerから移動
	@retval 印字可能領域があれば TRUE  // 2013.05.20 aroka
*/
BOOL CDlgPrintSetting::CalcPrintableLineAndColumn()
{
	int				nEnableColumns;		/* 行あたりの文字数 */
	int				nEnableLines;		/* 縦方向の行数 */
	MYDEVMODE		dmDummy;			// 2003.05.18 かろと 型変更
	short			nPaperAllWidth;		/* 用紙幅 */
	short			nPaperAllHeight;	/* 用紙高さ */
	PRINTSETTING*	pPS;

	/* ダイアログデータの取得 */
	GetData();
	pPS = &m_PrintSettingArr[m_nCurrentPrintSetting];

	dmDummy.dmFields = DM_PAPERSIZE | DMORIENT_LANDSCAPE;
	dmDummy.dmPaperSize = pPS->m_nPrintPaperSize;
	dmDummy.dmOrientation = pPS->m_nPrintPaperOrientation;
	/* 用紙の幅、高さ */
	if( !CPrint::GetPaperSize(
		&nPaperAllWidth,
		&nPaperAllHeight,
		&dmDummy
	) ){
	// 2001.12.21 hor GetPaperSize失敗時はそのまま終了
	//	nPaperAllWidth = 210 * 10;		/* 用紙幅 */
	//	nPaperAllHeight = 297 * 10;		/* 用紙高さ */
		return FALSE;
	}
	/* 行あたりの文字数(行番号込み) */
	nEnableColumns = CPrint::CalculatePrintableColumns( pPS, nPaperAllWidth, pPS->m_bPrintLineNumber?m_nLineNumberColumns:0 );	/* 印字可能桁数/ページ */
	/* 縦方向の行数 */
	nEnableLines = CPrint::CalculatePrintableLines( pPS, nPaperAllHeight );			/* 印字可能行数/ページ */

	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLECOLUMNS, nEnableColumns, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINES, nEnableLines, FALSE );

	// フォントのポイント数	2013/5/9 Uchi
	// 1pt = 1/72in = 25.4/72mm
	int		nFontPoints = pPS->m_nPrintFontHeight * 720 / 254;
	TCHAR	szFontPoints[20];
	auto_sprintf_s( szFontPoints, _countof(szFontPoints), _T("%d.%dpt"), nFontPoints/10, nFontPoints%10 );
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FONTSIZE, szFontPoints );

	// 印字可能領域がない場合は OK を押せなくする 2013.5.10 aroka
	if( nEnableColumns == 0 || nEnableLines == 0 ){
		::EnableWindow( GetDlgItem( GetHwnd(), IDOK ), FALSE );
		return FALSE;
	}else{
		::EnableWindow( GetDlgItem( GetHwnd(), IDOK ), TRUE );
		return TRUE;
	}
}


// 行数と桁数の更新を要求（メッセージキューにポストする）
// ダイアログ初期化の途中で EN_CHANGE に反応すると計算がおかしくなるため、関数呼び出しではなくPostMessageで処理 2013.5.5 aroka
void CDlgPrintSetting::UpdatePrintableLineAndColumn()
{
	m_bPrintableLinesAndColumnInvalid = true;
	::PostMessageA( GetHwnd(), WM_COMMAND, MAKELONG( IDC_STATIC_ENABLECOLUMNS, STN_CLICKED ), (LPARAM)::GetDlgItem( GetHwnd(), IDC_STATIC_ENABLECOLUMNS ) );
}


//@@@ 2002.01.18 add start
LPVOID CDlgPrintSetting::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


// フォント名/使用ボタンの設定
void CDlgPrintSetting::SetFontName( int idTxt, int idUse, LOGFONT& lf, int nPointSize )
{
	TCHAR	szName[100];
	bool	bUseFont = lf.lfFaceName[0] != _T('\0');

	CheckDlgButtonBool( GetHwnd(), idUse, bUseFont);
	::EnableWindow( ::GetDlgItem( GetHwnd(), idUse ), bUseFont );
	if (bUseFont) {
		LOGFONT	lft;
		lft = lf;
		lft.lfHeight = m_nFontHeight;		// フォントサイズをダイアログに合せる

		HFONT	hFontOld = (HFONT)::SendMessage(::GetDlgItem( GetHwnd(), idTxt ), WM_GETFONT, 0, 0 );

		// 論理フォントを作成
		HFONT	hFont = ::CreateFontIndirect( &lft );
		if (hFont) {
			// フォントの設定
			::SendMessage( ::GetDlgItem( GetHwnd(), idTxt ), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0) );
		}
		if (m_hFontDlg != hFontOld) {
			// 古いフォントの破棄
			::DeleteObject( hFontOld );
		}

		// フォント名/サイズの作成
		int		nMM = MulDiv( nPointSize, 254, 720 );	// フォントサイズ計算(pt->1/10mm)
		auto_sprintf(szName, nPointSize%10 ? _T("%.32s(%.1fpt/%d.%dmm)") : _T("%.32s(%.0fpt/%d.%dmm)"),
					lf.lfFaceName,
					double(nPointSize)/10,
					nMM/10, nMM/10);
	}
	else {
		szName[0] = _T('\0');
	}
	::DlgItem_SetText( GetHwnd(), idTxt, szName );
}
