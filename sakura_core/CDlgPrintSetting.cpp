//	$Id$
/*!	@file
	@brief 印刷設定ダイアログ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <stdio.h>
#include "CDlgPrintSetting.h"
#include "CDlgInput1.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "etc_uty.h"		// Stonee, 2001/03/12
#include "sakura_rc.h"	// 2002/2/10 aroka
#include "debug.h"		// 2002/2/10 aroka

// 印刷設定 CDlgPrintSetting.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
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
	IDC_COMBO_PAPERORIENT,			HIDC_PS_COMBO_PAPERORIENT,	//用紙向き
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
	hwndComboFontHan = ::GetDlgItem( pCDlgPrintSetting->m_hWnd, IDC_COMBO_FONT_HAN );
	hwndComboFontZen = ::GetDlgItem( pCDlgPrintSetting->m_hWnd, IDC_COMBO_FONT_ZEN );

	/* LOGFONT */
	if( FIXED_PITCH & pelf->elfLogFont.lfPitchAndFamily ){
//		MYTRACE( pelf->elfLogFont.lfFaceName, "%s\n\n", pelf->elfLogFont.lfFaceName );
		::SendMessage( hwndComboFontHan, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)pelf->elfLogFont.lfFaceName  );
		::SendMessage( hwndComboFontZen, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)pelf->elfLogFont.lfFaceName  );
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

	nRet = CDialog::DoModal( hInstance, hwndParent, IDD_PRINTSETTING, NULL );
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
	m_hWnd = hwndDlg;
//	hwndComboSettingName = ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME );

//	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
//	::SendMessage( ::GetDlgItem( m_hWnd, IDC_EDIT_LINENUM ), EM_LIMITTEXT, (WPARAM)9, 0 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_HAN ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_ZEN ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_PAPER ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_PAPERORIENT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

	::SetTimer( m_hWnd, IDT_PRINTSETTING, 500, NULL );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}

BOOL CDlgPrintSetting::OnDestroy( void )
{
	::KillTimer( m_hWnd, IDT_PRINTSETTING );
	/* 基底クラスメンバ */
	return CDialog::OnDestroy();
}


BOOL CDlgPrintSetting::OnNotify( WPARAM wParam, LPARAM lParam )
{
//	WORD			wNotifyCode;
//	WORD			wID;
//	HWND			hwndCtl;
//	int				nRet;
//	int				nIndex;
//	char*			pszWork;
//	int				nWorkLine;
//	HWND			hwndComboSettingName;
	CDlgInput1		cDlgInput1;
//	HWND			hwndCtl;
	NMHDR*			pNMHDR;
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	BOOL			bSpinDown;
//	char			szWork[256];
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
	if( ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME ) == hwndCtl ){
		/* 設定のタイプが変わった */
		OnChangeSettingType( TRUE );
		return TRUE;
	}
	return FALSE;

}

BOOL CDlgPrintSetting::OnBnClicked( int wID )
{
	char			szWork[256];
	CDlgInput1		cDlgInput1;
	HWND			hwndComboSettingName;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「印刷ページ設定」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PAGESETUP) );
		return TRUE;
	case IDC_BUTTON_EDITSETTINGNAME:
		strcpy( szWork, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName );
		if( FALSE == cDlgInput1.DoModal( m_hInstance, m_hWnd, "設定名の変更", "設定の名称を入力してください。", sizeof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1, szWork ) ){
			return TRUE;
		}
		if( 0 < lstrlen( szWork ) ){
			strcpy( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName, szWork );
			/* 印刷設定名一覧 */
			hwndComboSettingName = ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME );
			::SendMessage( hwndComboSettingName, CB_RESETCONTENT, 0, 0 );
			int		nSelectIdx;
			int		i;
			int		nItemIdx;
			nSelectIdx = 0;
			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
				nItemIdx = ::SendMessage(
					hwndComboSettingName, CB_ADDSTRING, 0,
					(LPARAM)(LPCTSTR)m_PrintSettingArr[i].m_szPrintSettingName
				);
				::SendMessage( hwndComboSettingName, CB_SETITEMDATA, nItemIdx, (LPARAM)i );
				if( i == m_nCurrentPrintSetting ){
					nSelectIdx = nItemIdx;
				}
			}
			::SendMessage( hwndComboSettingName, CB_SETCURSEL, nSelectIdx, 0 );
		}
		return TRUE;
	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
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
	HWND	hwndComboPaperOrient;
	HWND	hwndComboSettingName;
	const char*	pszPaperNameArr[] = {
		"A4 (210 x 297 mm)",
		"A3 (297 x 420 mm)",
		"A4 small(210 x 297 mm)",
		"A5 (148 x 210 mm)",
		"B4 (250 x 354 mm)",
		"B5 (182 x 257 mm)",
		"Quarto(215 x 275 mm)",
		"DL Envelope(110 x 220 mm)",
		"C5 Envelope(162 x 229 mm)",
		"C3 Envelope(324 x 458 mm)",
		"C4 Envelope(229 x 324 mm)",
		"C6 Envelope(114 x 162 mm)",
		"C65 Envelope(114 x 229 mm)",
		"B4 Envelope(250 x 353 mm)",
		"B5 Envelope(176 x 250 mm)",
		"B6 Envelope(176 x 125 mm)",
		"Italy Envelope(110 x 230 mm)",
		"Letter、8 1/2x11 inch",
		"Legal、8 1/2x14 inch",
		"C sheet、17x22 inch",
		"D sheet、22x34 inch",
		"E sheet、34x44 inch",
		"Letter Small、8 1/2x11 inch",
		"Tabloid、11x17 inch",
		"Ledger、17x11 inch",
		"Statement、5 1/2x8 1/2 inch",
		"Executive、7 1/4x10 1/2 inch",
		"Folio、8 1/2x13 inch",
		"10x14 inch sheet",
		"11x17 inch sheet",
		"Note、 8 1/2x11 inch",
		"#9 Envelope、3 7/8x8 7/8 inch",
		"#10 Envelope、4 1/8x9 1/2 inch",
		"#11 Envelope、4 1/2x10 3/8 inch",
		"#12 Envelope、4 3/4x11 inch",
		"#14 Envelope、5x11 1/2 inch",
		"Monarch Envelope、3 7/8x7 1/2 inch",
		"6 3/4 Envelope、3 5/8x6 1/2 inch",
		"US Std Fanfold、14 7/8x11 inch",
		"German Std Fanfold、8 1/2x12 inch",
		"German Legal Fanfold、8 1/2x13 inch"
	};
	int		nPaperNameArrNum = sizeof( pszPaperNameArr ) / sizeof( pszPaperNameArr[0] );
	int		nPaperIdArr[] = {
		DMPAPER_A4		,
		DMPAPER_A3		,
		DMPAPER_A4SMALL	,
		DMPAPER_A5		,
		DMPAPER_B4		,
		DMPAPER_B5		,
		DMPAPER_QUARTO	,
		DMPAPER_ENV_DL	,
		DMPAPER_ENV_C5	,
		DMPAPER_ENV_C3	,
		DMPAPER_ENV_C4	,
		DMPAPER_ENV_C6	,
		DMPAPER_ENV_C65	,
		DMPAPER_ENV_B4	,
		DMPAPER_ENV_B5	,
		DMPAPER_ENV_B6	,
		DMPAPER_ENV_ITALY,
		DMPAPER_LETTER,
		DMPAPER_LEGAL,
		DMPAPER_CSHEET,
		DMPAPER_DSHEET,
		DMPAPER_ESHEET,
		DMPAPER_LETTERSMALL,
		DMPAPER_TABLOID,
		DMPAPER_LEDGER,
		DMPAPER_STATEMENT,
		DMPAPER_EXECUTIVE,
		DMPAPER_FOLIO,
		DMPAPER_10X14,
		DMPAPER_11X17,
		DMPAPER_NOTE,
		DMPAPER_ENV_9,
		DMPAPER_ENV_10,
		DMPAPER_ENV_11,
		DMPAPER_ENV_12,
		DMPAPER_ENV_14,
		DMPAPER_ENV_MONARCH,
		DMPAPER_ENV_PERSONAL,
		DMPAPER_FANFOLD_US,
		DMPAPER_FANFOLD_STD_GERMAN,
		DMPAPER_FANFOLD_LGL_GERMAN
	};
	int	i;
	int	nItemIdx;
	int	nSelectIdx;


	/* フォント一覧 */
	hdc = ::GetDC( m_hwndParent );
	hwndComboFont = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_HAN );
	::SendMessage( hwndComboFont, CB_RESETCONTENT, 0, 0 );
	hwndComboFont = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_ZEN );
	::SendMessage( hwndComboFont, CB_RESETCONTENT, 0, 0 );
	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)SetData_EnumFontFamProc,
		(LPARAM)this
	);
	::ReleaseDC( m_hwndParent, hdc );

	/* 用紙サイズ一覧 */
	hwndComboPaper = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPER );
	::SendMessage( hwndComboPaper, CB_RESETCONTENT, 0, 0 );
	for( i = 0; i < nPaperNameArrNum; ++i ){
		nItemIdx = ::SendMessage( hwndComboPaper, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)pszPaperNameArr[i] );
		::SendMessage( hwndComboPaper, CB_SETITEMDATA, nItemIdx, (LPARAM)nPaperIdArr[i] );
	}

	/* 用紙方向一覧 */
	hwndComboPaperOrient = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPERORIENT );
	::SendMessage( hwndComboPaperOrient, CB_RESETCONTENT, 0, 0 );
	nItemIdx = ::SendMessage( hwndComboPaperOrient, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"縦↑" );
	::SendMessage( hwndComboPaperOrient, CB_SETITEMDATA, nItemIdx, (LPARAM)DMORIENT_PORTRAIT );
	nItemIdx = ::SendMessage( hwndComboPaperOrient, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"横→" );
	::SendMessage( hwndComboPaperOrient, CB_SETITEMDATA, nItemIdx, (LPARAM)DMORIENT_LANDSCAPE );

/*
ｷ	DMPAPER_A4   A4 sheet、210 × 297 mm
ｷ	DMPAPER_A3   A3 sheet、297 × 420 mm
ｷ	DMPAPER_A4SMALL   A4 small sheet、210 × 297 mm
ｷ	DMPAPER_A5   A5 sheet、148 × 210 mm
ｷ	DMPAPER_B4   B4 sheet、250 × 354 mm
ｷ	DMPAPER_B5   B5 sheet、182 × 257 mm
ｷ	DMPAPER_QUARTO   Quarto、215 × 275 mm
ｷ	DMPAPER_ENV_DL   DL Envelope、110 × 220 mm
ｷ	DMPAPER_ENV_C5   C5 Envelope、162 × 229 mm
ｷ	DMPAPER_ENV_C3   C3 Envelope、324 × 458 mm
ｷ	DMPAPER_ENV_C4   C4 Envelope、229 × 324 mm
ｷ	DMPAPER_ENV_C6   C6 Envelope、114 × 162 mm
ｷ	DMPAPER_ENV_C65   C65 Envelope、114 × 229 mm
ｷ	DMPAPER_ENV_B4   B4 Envelope、250 × 353 mm
ｷ	DMPAPER_ENV_B5   B5 Envelope、176 × 250 mm
ｷ	DMPAPER_ENV_B6   B6 Envelope、176 × 125 mm
ｷ	DMPAPER_ENV_ITALY   Italy Envelope、110 × 230 mm

ｷ	DMPAPER_LETTER   Letter、8 1/2 × 11 inch
ｷ	MPAPER_LEGAL   Legal、8 1/2 × 14 inch
ｷ	DMPAPER_CSHEET   C sheet、17 × 22 inch
ｷ	DMPAPER_DSHEET   D sheet、22 × 34 inch
ｷ	DMPAPER_ESHEET   E sheet、34 × 44 inch
ｷ	DMPAPER_LETTERSMALL   Letter Small、8 1/2 × 11 inch
ｷ	DMPAPER_TABLOID   Tabloid、11 × 17 inch
ｷ	DMPAPER_LEDGER   Ledger、17 × 11 inch
ｷ	DMPAPER_STATEMENT   Statement、5 1/2 × 8 1/2 inch
ｷ	DMPAPER_EXECUTIVE   Executive、7 1/4 × 10 1/2 inch
ｷ	DMPAPER_FOLIO   Folio、8 1/2 × 13 inch
ｷ	DMPAPER_10X14   10 × 14 inch sheet
ｷ	DMPAPER_11X17   11 × 17 inch sheet
ｷ	DMPAPER_NOTE   Note、 8 1/2 × 11 inch
ｷ	DMPAPER_ENV_9   #9 Envelope、3 7/8 × 8 7/8 inch
ｷ	DMPAPER_ENV_10   #10 Envelope、4 1/8 × 9 1/2 inch
ｷ	DMPAPER_ENV_11   #11 Envelope、4 1/2 × 10 3/8 inch
ｷ	DMPAPER_ENV_12   #12 Envelope、4 3/4 × 11 inch
ｷ	DMPAPER_ENV_14   #14 Envelope、5 × 11 1/2 inch
ｷ	DMPAPER_ENV_MONARCH   Monarch Envelope、3 7/8 × 7 1/2 inch
ｷ	DMPAPER_ENV_PERSONAL   6 3/4 Envelope、3 5/8 × 6 1/2 inch
ｷ	DMPAPER_FANFOLD_US   US Std Fanfold、14 7/8 × 11 inch
ｷ	DMPAPER_FANFOLD_STD_GERMAN   German Std Fanfold、8 1/2 × 12 inch
ｷ	DMPA PER_FANFOLD_LGL_GERMAN   German Legal Fanfold、8 1/2 × 13 inch

*/
	/* 印刷設定名一覧 */
	hwndComboSettingName = ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME );
	::SendMessage( hwndComboSettingName, CB_RESETCONTENT, 0, 0 );
	nSelectIdx = 0;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		nItemIdx = ::SendMessage( hwndComboSettingName, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)m_PrintSettingArr[i].m_szPrintSettingName );
		::SendMessage( hwndComboSettingName, CB_SETITEMDATA, nItemIdx, (LPARAM)i );
		if( i == m_nCurrentPrintSetting ){
			nSelectIdx = nItemIdx;
		}
	}
	::SendMessage( hwndComboSettingName, CB_SETCURSEL, nSelectIdx, 0 );
//	::SendMessage( m_hWnd, WM_COMMAND, MAKELONG( IDC_COMBO_SETTINGNAME, CBN_SELCHANGE ), (LPARAM)hwndComboSettingName );

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
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_HAN );
	nIdx1 = ::SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
	::SendMessage( hwndCtrl, CB_GETLBTEXT, nIdx1,
		(LPARAM)m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan
	);
	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_ZEN );
	nIdx1 = ::SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
	::SendMessage( hwndCtrl, CB_GETLBTEXT, nIdx1,
		(LPARAM)m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen
	);

	/* 用紙サイズ一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPER );
	nIdx1 = ::SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize =
		::SendMessage( hwndCtrl, CB_GETITEMDATA, nIdx1, 0 );

	/* 用紙方向一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPERORIENT );
	nIdx1 = ::SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation =
		::SendMessage( hwndCtrl, CB_GETITEMDATA, nIdx1, 0 );


	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = ::GetDlgItemInt( m_hWnd, IDC_EDIT_FONTWIDTH, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = ::GetDlgItemInt( m_hWnd, IDC_EDIT_LINESPACE, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = ::GetDlgItemInt( m_hWnd, IDC_EDIT_DANSUU, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = ::GetDlgItemInt( m_hWnd, IDC_EDIT_DANSPACE, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = ::GetDlgItemInt( m_hWnd, IDC_EDIT_MARGINTY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = ::GetDlgItemInt( m_hWnd, IDC_EDIT_MARGINBY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = ::GetDlgItemInt( m_hWnd, IDC_EDIT_MARGINLX, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = ::GetDlgItemInt( m_hWnd, IDC_EDIT_MARGINRX, NULL, FALSE ) * 10;

	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_WORDWRAP ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = FALSE;
	}
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_LINENUMBER ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = FALSE;
	}


	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = nWork;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	}

	nWork = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = nWork;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = nWork;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = nWork * 10;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = nWork * 10;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = nWork * 10;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = nWork * 10;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = nWork * 10;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );
	}


//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;

	//@@@ 2002.2.4 YAZAKI
	/* ヘッダー */
	::GetDlgItemText( m_hWnd, IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::GetDlgItemText( m_hWnd, IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::GetDlgItemText( m_hWnd, IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	/* フッター */
	::GetDlgItemText( m_hWnd, IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[0], HEADER_MAX );	//	100文字で制限しないと。。。
	::GetDlgItemText( m_hWnd, IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[1], HEADER_MAX );	//	100文字で制限しないと。。。
	::GetDlgItemText( m_hWnd, IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[2], HEADER_MAX );	//	100文字で制限しないと。。。

	//行頭禁則	//@@@ 2002.04.09 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PS_KINSOKUHEAD ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = FALSE;
	}
	//行末禁則	//@@@ 2002.04.09 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PS_KINSOKUTAIL ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = FALSE;
	}
	//改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PS_KINSOKURET ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = FALSE;
	}
	//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PS_KINSOKUKUTO ) ){
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

	hwndComboSettingName = ::GetDlgItem( m_hWnd, IDC_COMBO_SETTINGNAME );
	nIdx1 = ::SendMessage( hwndComboSettingName, CB_GETCURSEL, 0, 0 );
	if( CB_ERR == nIdx1 ){
		return;
	}
	m_nCurrentPrintSetting = ::SendMessage( hwndComboSettingName, CB_GETITEMDATA, nIdx1, 0 );

	::SetDlgItemInt( m_hWnd, IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	/* ヘッダー */
	::SetDlgItemText( m_hWnd, IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::SetDlgItemText( m_hWnd, IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::SetDlgItemText( m_hWnd, IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	/* フッター */
	::SetDlgItemText( m_hWnd, IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_LEFT] );	//	100文字で制限しないと。。。
	::SetDlgItemText( m_hWnd, IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_CENTER] );	//	100文字で制限しないと。。。
	::SetDlgItemText( m_hWnd, IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_RIGHT] );	//	100文字で制限しないと。。。

	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_WORDWRAP, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_WORDWRAP, BST_UNCHECKED );
	}
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_LINENUMBER, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_LINENUMBER, BST_UNCHECKED );
	}



	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_HAN );
	nIdx1 = ::SendMessage( hwndCtrl, CB_FINDSTRING, 0, (LPARAM)m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
	::SendMessage( hwndCtrl, CB_SETCURSEL, nIdx1, 0 );

	/* フォント一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_FONT_ZEN );
	nIdx1 = ::SendMessage( hwndCtrl, CB_FINDSTRING, 0, (LPARAM)m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen );
	::SendMessage( hwndCtrl, CB_SETCURSEL, nIdx1, 0 );

	/* 用紙サイズ一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPER );
	nItemNum = ::SendMessage( hwndCtrl, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < nItemNum; ++i ){
		nItemData = ::SendMessage( hwndCtrl, CB_GETITEMDATA, i, 0 );
		if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize == nItemData ){
			::SendMessage( hwndCtrl, CB_SETCURSEL, i, 0 );
			break;
		}
	}

	/* 用紙方向一覧 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_PAPERORIENT );
	nItemNum = ::SendMessage( hwndCtrl, CB_GETCOUNT, 0, 0 );
	for( i = 0; i < nItemNum; ++i ){
		nItemData = ::SendMessage( hwndCtrl, CB_GETITEMDATA, i, 0 );
		if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation == nItemData ){
			::SendMessage( hwndCtrl, CB_SETCURSEL, i, 0 );
			break;
		}
	}

	// 行頭禁則	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUHEAD, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUHEAD, BST_UNCHECKED );
	}
	// 行末禁則	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUTAIL, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUTAIL, BST_UNCHECKED );
	}
	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKURET, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKURET, BST_UNCHECKED );
	}
	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto ){
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUKUTO, BST_CHECKED );
	}else{
		::CheckDlgButton( m_hWnd, IDC_CHECK_PS_KINSOKUKUTO, BST_UNCHECKED );
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
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=1;
		}else{
			nData+=1;
		}
		break;
	case IDC_SPIN_LINESPACE:
		nCtrlIdEDIT = IDC_EDIT_LINESPACE;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=10;
		}else{
			nData+=10;
		}
		break;
	case IDC_SPIN_DANSUU:
		nCtrlIdEDIT = IDC_EDIT_DANSUU;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_DANSPACE:
		nCtrlIdEDIT = IDC_EDIT_DANSPACE;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINTY:
		nCtrlIdEDIT = IDC_EDIT_MARGINTY;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINBY:
		nCtrlIdEDIT = IDC_EDIT_MARGINBY;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINLX:
		nCtrlIdEDIT = IDC_EDIT_MARGINLX;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINRX:
		nCtrlIdEDIT = IDC_EDIT_MARGINRX;
		nData = ::GetDlgItemInt( m_hWnd, nCtrlIdEDIT, NULL, FALSE );
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
		::SetDlgItemInt( m_hWnd, nCtrlIdEDIT, nData, FALSE );
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
	nTimer = wParam;

	if( nTimer != IDT_PRINTSETTING ){
		return FALSE;
	}
	/* ダイアログデータの取得 */
	GetData();
//	SetData( m_hWnd );
	pPS = &m_PrintSettingArr[m_nCurrentPrintSetting];

	dmDummy.dmFields = DM_PAPERSIZE | DMORIENT_LANDSCAPE;
	dmDummy.dmPaperSize = pPS->m_nPrintPaperSize;
	dmDummy.dmOrientation = pPS->m_nPrintPaperOrientation;
	/* 用紙の幅、高さ */
	if( FALSE == CPrint::GetPaperSize(
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

	::SetDlgItemInt( m_hWnd, IDC_STATIC_ENABLECOLMNS, nEnableColmns, FALSE );
	::SetDlgItemInt( m_hWnd, IDC_STATIC_ENABLELINES, nEnableLines, FALSE );
//	::SetDlgItemInt( m_hWnd, IDC_STATIC_ENABLELINESALL, nEnableLinesAll, FALSE );
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgPrintSetting::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
