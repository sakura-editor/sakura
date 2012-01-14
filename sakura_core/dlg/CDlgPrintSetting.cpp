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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgInput1.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "sakura_rc.h"	// 2002/2/10 aroka
#include "sakura.hh"

// 印刷設定 CDlgPrintSetting.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12500
	IDC_BUTTON_EDITSETTINGNAME,		HIDC_PS_BUTTON_EDITSETTINGNAME,	//設定名変更
	IDOK,							HIDOK_PS,					//OK
	IDCANCEL,						HIDCANCEL_PS,				//キャンセル
	IDC_BUTTON_HELP,				HIDC_PS_BUTTON_HELP,		//ヘルプ
	IDC_CHECK_WORDWRAP,				HIDC_PS_CHECK_WORDWRAP,		//ワードラップ
	IDC_CHECK_LINENUMBER,			HIDC_PS_CHECK_LINENUMBER,	//行番号
	IDC_COMBO_FONT_HAN,				HIDC_PS_COMBO_FONT_HAN,		//半角フォント
	IDC_COMBO_FONT_ZEN,				HIDC_PS_COMBO_FONT_ZEN,		//全角フォント
	IDC_COMBO_SETTINGNAME,			HIDC_PS_COMBO_SETTINGNAME,	//ページ設定
	IDC_COMBO_PAPER,				HIDC_PS_COMBO_PAPER,		//用紙サイズ
	IDC_EDIT_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//フォント幅
	IDC_EDIT_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//行送り
	IDC_EDIT_DANSUU,				HIDC_PS_EDIT_DANSUU,		//段数
	IDC_EDIT_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//段の隙間
	IDC_EDIT_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//余白上
	IDC_EDIT_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//余白下
	IDC_EDIT_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//余白左
	IDC_EDIT_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//余白右
	IDC_SPIN_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//12570,
	IDC_SPIN_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//12571,
	IDC_SPIN_DANSUU,				HIDC_PS_EDIT_DANSUU,		//12572,
	IDC_SPIN_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//12573,
	IDC_SPIN_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//12574,
	IDC_SPIN_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//12575,
	IDC_SPIN_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//12576,
	IDC_SPIN_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//12577,
	IDC_CHECK_PS_KINSOKUHEAD,		HIDC_PS_CHECK_KINSOKUHEAD,	//行頭禁則	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKUTAIL,		HIDC_PS_CHECK_KINSOKUTAIL,	//行末禁則	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKURET,		HIDC_PS_CHECK_KINSOKURET,	//改行文字をぶら下げる	//@@@ 2002.04.14 MIK
	IDC_CHECK_PS_KINSOKUKUTO,		HIDC_PS_CHECK_KINSOKUKUTO,	//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	IDC_EDIT_HEAD1,					HIDC_PS_EDIT_HEAD1,			//ヘッダー(左寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_HEAD2,					HIDC_PS_EDIT_HEAD2,			//ヘッダー(中央寄せ)	// 2006.10.11 ryoji
	IDC_EDIT_HEAD3,					HIDC_PS_EDIT_HEAD3,			//ヘッダー(右寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT1,					HIDC_PS_EDIT_FOOT1,			//フッター(左寄せ)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT2,					HIDC_PS_EDIT_FOOT2,			//フッター(中央寄せ)	// 2006.10.11 ryoji
	IDC_EDIT_FOOT3,					HIDC_PS_EDIT_FOOT3,			//フッター(右寄せ)		// 2006.10.11 ryoji
//	IDC_STATIC,						-1,
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
	if( FIXED_PITCH & pelf->elfLogFont.lfPitchAndFamily ){
//		MYTRACE_A( pelf->elfLogFont.lfFaceName, "%ls\n\n", pelf->elfLogFont.lfFaceName );
		Combo_AddString( hwndComboFontHan, pelf->elfLogFont.lfFaceName  );
		Combo_AddString( hwndComboFontZen, pelf->elfLogFont.lfFaceName  );
	}
	return 1;
}

/* モーダルダイアログの表示 */
int CDlgPrintSetting::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	int*			pnCurrentPrintSetting,
	PRINTSETTING*	pPrintSettingArr
)
{
	int		nRet;
	int		i;
	m_nCurrentPrintSetting = *pnCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		m_PrintSettingArr[i] = pPrintSettingArr[i];
	}

	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTSETTING, NULL );
//	nRet = ::DialogBoxParam(
//		m_hInstance,
//		MAKEINTRESOURCE( IDD_PRINTSETTING ),
//		m_hwndParent,
//		(DLGPROC)PrintSettingDialogProc,
//		(LPARAM)this
//	);
	if( TRUE == nRet ){
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

	::SetTimer( GetHwnd(), IDT_PRINTSETTING, 500, NULL );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgPrintSetting::OnDestroy( void )
{
	::KillTimer( GetHwnd(), IDT_PRINTSETTING );
	/* 基底クラスメンバ */
	return CDialog::OnDestroy();
}


BOOL CDlgPrintSetting::OnNotify( WPARAM wParam, LPARAM lParam )
{
	CDlgInput1		cDlgInput1;
	NMHDR*			pNMHDR;
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	BOOL			bSpinDown;
	idCtrl = (int)wParam;
	pNMHDR = (NMHDR*)lParam;
	pMNUD  = (NM_UPDOWN*)lParam;
	if( pMNUD->iDelta < 0 ){
		bSpinDown = FALSE;
	}else{
		bSpinDown = TRUE;
	}
	switch( idCtrl ){
	case IDC_SPIN_FONTWIDTH:
	case IDC_SPIN_LINESPACE:
	case IDC_SPIN_DANSUU:
	case IDC_SPIN_DANSPACE:
	case IDC_SPIN_MARGINTY:
	case IDC_SPIN_MARGINBY:
	case IDC_SPIN_MARGINLX:
	case IDC_SPIN_MARGINRX:
		/* スピンコントロールの処理 */
		OnSpin( idCtrl, bSpinDown );
		break;
	}
	return TRUE;
}

BOOL CDlgPrintSetting::OnCbnSelChange( HWND hwndCtl, int wID )
{
	if( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ) == hwndCtl ){
		/* 設定のタイプが変わった */
		OnChangeSettingType( TRUE );
		return TRUE;
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
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PAGESETUP) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDC_BUTTON_EDITSETTINGNAME:
		_tcscpy( szWork, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName );
		{
			BOOL bDlgInputResult=cDlgInput1.DoModal(
				m_hInstance,
				GetHwnd(),
				_T("設定名の変更"),
				_T("設定の名称を入力してください。"),
				_countof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1,
				szWork
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		if( 0 < _tcslen( szWork ) ){
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
	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
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

	/* 用紙サイズ一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize =
		Combo_GetItemData( hwndCtrl, nIdx1 );

	// 用紙の向き
	// 2006.08.14 Moca 用紙方向コンボボックスを廃止し、ボタンを有効化
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_PORTRAIT ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_LANDSCAPE;
	}

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, NULL, FALSE ) * 10;

	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_WORDWRAP ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = FALSE;
	}
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_LINENUMBER ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = FALSE;
	}


	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	}

	nWork = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );
	}


//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;

	//@@@ 2002.2.4 YAZAKI
	/* ヘッダー */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	/* フッター */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	//行頭禁則	//@@@ 2002.04.09 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = FALSE;
	}
	//行末禁則	//@@@ 2002.04.09 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = FALSE;
	}
	//改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKURET ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = FALSE;
	}
	//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto = FALSE;
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

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	/* ヘッダー */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	/* フッター */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_WORDWRAP, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_WORDWRAP, BST_UNCHECKED );
	}
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_LINENUMBER, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_LINENUMBER, BST_UNCHECKED );
	}



	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

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
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation == DMORIENT_PORTRAIT ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_PORTRAIT, BST_CHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LANDSCAPE, BST_UNCHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_PORTRAIT, BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LANDSCAPE, BST_CHECKED );
	}

	// 行頭禁則	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, BST_UNCHECKED );
	}
	// 行末禁則	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, BST_UNCHECKED );
	}
	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKURET, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKURET, BST_UNCHECKED );
	}
	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, BST_UNCHECKED );
	}

	return;
}

/* スピンコントロールの処理 */
void CDlgPrintSetting::OnSpin( int nCtrlId, BOOL bDown )
{
	int		nData;
	BOOL	bUnknown;
	int		nCtrlIdEDIT;
	bUnknown = FALSE;
	switch( nCtrlId ){
	case IDC_SPIN_FONTWIDTH:
		nCtrlIdEDIT = IDC_EDIT_FONTWIDTH;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=1;
		}else{
			nData+=1;
		}
		break;
	case IDC_SPIN_LINESPACE:
		nCtrlIdEDIT = IDC_EDIT_LINESPACE;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=10;
		}else{
			nData+=10;
		}
		break;
	case IDC_SPIN_DANSUU:
		nCtrlIdEDIT = IDC_EDIT_DANSUU;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_DANSPACE:
		nCtrlIdEDIT = IDC_EDIT_DANSPACE;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINTY:
		nCtrlIdEDIT = IDC_EDIT_MARGINTY;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINBY:
		nCtrlIdEDIT = IDC_EDIT_MARGINBY;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINLX:
		nCtrlIdEDIT = IDC_EDIT_MARGINLX;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINRX:
		nCtrlIdEDIT = IDC_EDIT_MARGINRX;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	default:
		bUnknown = TRUE;
		break;
	}
	if( !bUnknown ){
		/* 入力値(数値)のエラーチェックをして正しい値を返す */
		nData = DataCheckAndCrrect( nCtrlIdEDIT, nData );
		::SetDlgItemInt( GetHwnd(), nCtrlIdEDIT, nData, FALSE );
	}
	return;
}


/* 入力値(数値)のエラーチェックをして正しい値を返す */
int CDlgPrintSetting::DataCheckAndCrrect( int nCtrlId, int nData )
{
//	int		nData;
	switch( nCtrlId ){
	case IDC_EDIT_FONTWIDTH:
		if( 7 >= nData ){
			nData = 7;
		}
		if( 100 < nData ){
			nData = 100;
		}
		break;
	case IDC_EDIT_LINESPACE:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 150 < nData ){
			nData = 150;
		}
		break;
	case IDC_EDIT_DANSUU:
		if( 1 >= nData ){
			nData = 1;
		}
		if( 4 < nData ){
			nData = 4;
		}
		break;
	case IDC_EDIT_DANSPACE:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 30 < nData ){
			nData = 30;
		}
		break;
	case IDC_EDIT_MARGINTY:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINBY:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINLX:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINRX:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	}
	return nData;
}

/* タイマー処理 */
BOOL CDlgPrintSetting::OnTimer( WPARAM wParam )
{
	int nTimer;
	int				nEnableColmns;		/* 行あたりの文字数 */
	int				nEnableLines;		/* 縦方向の行数 */
//	int				nEnableLinesAll;	/* ページあたりの行数 */
	MYDEVMODE		dmDummy;			// 2003.05.18 かろと 型変更
	int				nPaperAllWidth;		/* 用紙幅 */
	int				nPaperAllHeight;	/* 用紙高さ */
	PRINTSETTING*	pPS;
	nTimer = (int)wParam;

	if( nTimer != IDT_PRINTSETTING ){
		return FALSE;
	}
	/* ダイアログデータの取得 */
	GetData();
//	SetData( GetHwnd() );
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
	nEnableColmns =
		( nPaperAllWidth - pPS->m_nPrintMarginLX - pPS->m_nPrintMarginRX
		- ( pPS->m_nPrintDansuu - 1 ) * pPS->m_nPrintDanSpace
//		- ( pPS->m_nPrintDansuu ) *  ( ( m_nPreview_LineNumberColmns /*+ (m_nPreview_LineNumberColmns?1:0)*/ ) * pPS->m_nPrintFontWidth )
		) / pPS->m_nPrintFontWidth / pPS->m_nPrintDansuu;	/* 印字可能桁数/ページ */
	/* 縦方向の行数 */
	nEnableLines =
		( nPaperAllHeight - pPS->m_nPrintMarginTY - pPS->m_nPrintMarginBY ) /
		( pPS->m_nPrintFontHeight + ( pPS->m_nPrintFontHeight * pPS->m_nPrintLineSpacing / 100 ) ) - 4;	/* 印字可能行数/ページ */
//	/* ページあたりの行数 */
//	nEnableLinesAll = nEnableLines * pPS->m_nPrintDansuu;

	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLECOLMNS, nEnableColmns, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINES, nEnableLines, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINESALL, nEnableLinesAll, FALSE );
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgPrintSetting::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


