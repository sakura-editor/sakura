/*! @file
	@brief タイプ別設定 - カラー

	@date 2008.04.12 kobake CPropTypes.cppから分離
	@date 2009.02.22 ryoji
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, こおり, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "uiparts/CGraphics.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "prop/CPropCommon.h"

using namespace std;

//! カスタムカラー用の識別文字列
static const TCHAR* TSTR_PTRCUSTOMCOLORS = _T("ptrCustomColors");

WNDPROC	m_wpColorListProc;

static const DWORD p_helpids2[] = {	//11400
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				//色指定
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				//色分け表示
	IDC_CHECK_BOLD,					HIDC_CHECK_BOLD,				//太字
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			//下線
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			//文字色
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			//背景色
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		//文字色統一
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		//背景色統一
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		//エクスポート
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			//強調キーワード１セット名
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		//強調キーワード2～10	// 2006.08.06 ryoji
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	//ブロックコメント１開始
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		//ブロックコメント１終了
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	//ブロックコメント２開始
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		//ブロックコメント２終了
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			//行コメント１
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			//行コメント２
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			//行コメント３
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		//桁数１
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		//桁数２
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		//桁数３
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				//桁指定１
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				//桁指定２
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				//桁指定３
	IDC_COMBO_STRINGLITERAL,		HIDC_COMBO_STRINGLITERAL,		//文字列エスケープ
	IDC_CHECK_STRINGLINEONLY,		HIDC_CHECK_STRINGLINEONLY,		//文字列は行内のみ
	IDC_CHECK_STRINGENDLINE,		HIDC_CHECK_STRINGENDLINE,		//終了文字がない場合行末まで色分け
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				//縦線の桁指定	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};

TYPE_NAME_ID<EStringLiteralType> StringLitteralArr[] = {
	{ STRING_LITERAL_CPP,    STR_STRINGESC_CPP },
	{ STRING_LITERAL_PLSQL,  STR_STRINGESC_PLSQL },
	{ STRING_LITERAL_HTML,   STR_STRINGESC_HTML },
	{ STRING_LITERAL_CSHARP, STR_STRINGESC_CSHARP },
	{ STRING_LITERAL_PYTHON, STR_STRINGESC_PYTHON },
};


//	行コメントに関する情報
struct {
	int nEditID;
	int nCheckBoxID;
	int nTextID;
} const cLineComment[COMMENT_DELIMITER_NUM] = {
	{ IDC_EDIT_LINECOMMENT	, IDC_CHECK_LCPOS , IDC_EDIT_LINECOMMENTPOS },
	{ IDC_EDIT_LINECOMMENT2	, IDC_CHECK_LCPOS2, IDC_EDIT_LINECOMMENTPOS2},
	{ IDC_EDIT_LINECOMMENT3	, IDC_CHECK_LCPOS3, IDC_EDIT_LINECOMMENTPOS3}
};

/* 色の設定をインポート */
// 2010/4/23 Uchi Importの外出し
bool CPropTypesColor::Import( HWND hwndDlg )
{
	ColorInfo		ColorInfoArr[64];
	CImpExpColors	cImpExpColors( ColorInfoArr );

	/* 色設定 I/O */
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
		_tcscpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
	}

	// インポート
	if (!cImpExpColors.ImportUI(m_hInstance, hwndDlg)) {
		// インポートをしていない
		return false;
	}

	/* データのコピー */
	m_Types.m_nColorInfoArrNum = COLORIDX_LAST;
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		_tcscpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* ダイアログデータの設定 color */
	SetData( hwndDlg );

	return true;
}


/* 色の設定をエクスポート */
// 2010/4/23 Uchi Exportの外出し
bool CPropTypesColor::Export( HWND hwndDlg )
{
	CImpExpColors	cImpExpColors( m_Types.m_ColorInfoArr);

	// エクスポート
	return cImpExpColors.ExportUI(m_hInstance, hwndDlg);
}



LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int			xPos = 0;
	int			yPos;
	int			nIndex = -1;
	int			nItemNum;
	RECT		rcItem = {0,0,0,0};
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch( uMsg ){
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = List_GetCount( hwnd );
		for( i = 0; i < nItemNum; ++i ){
			List_GetItemRect( hwnd, i, &rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE( _T("hit at i==%d\n"), i );
//				MYTRACE( _T("\n") );
				nIndex = i;
				break;
			}
		}
		break;
	}
	switch( uMsg ){
	case WM_RBUTTONDOWN:

		if( -1 == nIndex ){
			break;
		}
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji 有効範囲の制限追加
			List_SetCurSel( hwnd, nIndex );
			::SendMessageCmd( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
			pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
			/* 下線 */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )	// 2006.12.18 ryoji フラグ利用で簡素化
			{
				pColorInfo->m_sFontAttr.m_bUnderLine = !pColorInfo->m_sFontAttr.m_bUnderLine; // toggle true/false
				::CheckDlgButtonBool( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, pColorInfo->m_sFontAttr.m_bUnderLine );
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji 有効範囲の制限追加
			pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
			/* 太字で表示 */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) )	// 2006.12.18 ryoji フラグ利用で簡素化
			{
				pColorInfo->m_sFontAttr.m_bBoldFont = !pColorInfo->m_sFontAttr.m_bBoldFont; // toggle true/false
				::CheckDlgButtonBool( ::GetParent( hwnd ), IDC_CHECK_BOLD, pColorInfo->m_sFontAttr.m_bBoldFont );
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
		/* 色分け/表示 する */
		if( 2 <= xPos && xPos <= 16
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_FORCE_DISP) )	// 2006.12.18 ryoji フラグ利用で簡素化
			)
		{
			if( pColorInfo->m_bDisp ){	/* 色分け/表示する */
				pColorInfo->m_bDisp = false;
			}else{
				pColorInfo->m_bDisp = true;
			}
			if( COLORIDX_GYOU == nIndex ){
				pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_TEXT) ) )
		{
			/* 色選択ダイアログ */
			// 2005.11.30 Moca カスタム色保持
			DWORD* pColors = (DWORD*)::GetProp( hwnd, TSTR_PTRCUSTOMCOLORS );
			if( CPropTypesColor::SelectColor( hwnd, &pColorInfo->m_sColorAttr.m_cTEXT, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
			}
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
			)
		{
			/* 色選択ダイアログ */
			// 2005.11.30 Moca カスタム色保持
			DWORD* pColors = (DWORD*)::GetProp( hwnd, TSTR_PTRCUSTOMCOLORS );
			if( CPropTypesColor::SelectColor( hwnd, &pColorInfo->m_sColorAttr.m_cBACK, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
			}
		}
		break;
	// 2005.11.30 Moca カスタム色保持
	case WM_DESTROY:
		if( ::GetProp( hwnd, TSTR_PTRCUSTOMCOLORS ) ){
			::RemoveProp( hwnd, TSTR_PTRCUSTOMCOLORS );
		}
		break;
	}
	return CallWindowProc( m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* color メッセージ処理 */
INT_PTR CPropTypesColor::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	int					nVal;
	int					nIndex;
	static HWND			hwndListColor;
	LPDRAWITEMSTRUCT	pDis;

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		hwndListColor = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );

		/* ダイアログデータの設定 color */
		SetData( hwndDlg );

		/* 色リストをフック */
		// Modified by KEITA for WIN64 2003.9.6
		m_wpColorListProc = (WNDPROC) ::SetWindowLongPtr( hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );
		// 2005.11.30 Moca カスタム色を保持
		::SetProp( hwndListColor, TSTR_PTRCUSTOMCOLORS, m_dwCustColors );
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		if( hwndListColor == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndListColor );
				m_nCurrentColorType = nIndex;		/* 現在選択されている色タイプ */

				{
					// 各種コントロールの有効／無効を切り替える	// 2006.12.18 ryoji フラグ利用で簡素化
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),			(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BOLD ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),		(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_MOZI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
				}

				/* 色分け/表示 をする */
				::CheckDlgButtonBool( hwndDlg, IDC_CHECK_DISP, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp );
				/* 太字で表示 */
				::CheckDlgButtonBool( hwndDlg, IDC_CHECK_BOLD, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sFontAttr.m_bBoldFont );
				/* 下線を表示 */
				::CheckDlgButtonBool( hwndDlg, IDC_CHECK_UNDERLINE, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sFontAttr.m_bUnderLine );

				::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				return TRUE;
			}
		}
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_SAMETEXTCOLOR: /* 文字色統一 */
				{
					// 2006.04.26 ryoji 文字色／背景色統一ダイアログを使う
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	/* 背景色統一 */
				{
					// 2006.04.26 ryoji 文字色／背景色統一ダイアログを使う
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cBACK;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cBACK, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_CHECK_DISP:	/* 色分け/表示 をする */
				m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_DISP );
				/* 現在選択されている色タイプ */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				m_Types.m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;
			case IDC_CHECK_BOLD:	/* 太字か */
				m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sFontAttr.m_bBoldFont = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BOLD );
				/* 現在選択されている色タイプ */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* 下線を表示 */
				m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sFontAttr.m_bUnderLine = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_UNDERLINE );
				/* 現在選択されている色タイプ */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* 色の設定をインポート */
				Import( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* 色の設定をエクスポート */
				Export( hwndDlg );
				return TRUE;

			//	From Here Jun. 6, 2001 genta
			//	行コメント開始桁指定のON/OFF
			case IDC_CHECK_LCPOS:
			case IDC_CHECK_LCPOS2:
			case IDC_CHECK_LCPOS3:
			//	To Here Jun. 6, 2001 genta
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			//強調キーワードの選択
			case IDC_BUTTON_KEYWORD_SELECT:
				{
					CDlgKeywordSelect cDlgKeywordSelect;
					//強調キーワード1を取得する。
					HWND hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
					int nIdx = Combo_GetCurSel( hwndCombo );
					if( CB_ERR == nIdx || 0 == nIdx ){
						m_nSet[ 0 ] = -1;
					}else{
						m_nSet[ 0 ] = nIdx - 1;
					}
					cDlgKeywordSelect.DoModal( ::GetModuleHandle(NULL), hwndDlg, m_nSet );
					RearrangeKeywordSet( hwndDlg );	//	Jan. 23, 2005 genta キーワードセット再配置
					//強調キーワード1を反映する。
					if( -1 == m_nSet[ 0 ] ){
						Combo_SetCurSel( hwndCombo, 0 );
					}else{
						Combo_SetCurSel( hwndCombo, m_nSet[ 0 ] + 1 );
					}
				}
				break;
			//強調キーワードの選択
			case IDC_BUTTON_EDITKEYWORD:
				{
					GetData( hwndDlg ); // Keywrod1取得
					CPropKeyword* pPropKeyword = new CPropKeyword;
					CPropCommon* pCommon = (CPropCommon*)pPropKeyword;
					pCommon->m_hwndParent = ::GetParent(hwndDlg);
					pCommon->InitData( m_nSet, m_Types.m_szTypeName, m_Types.m_szTypeExts );
					INT_PTR res = ::DialogBoxParam(
						CSelectLang::getLangRsrcInstance(),
						MAKEINTRESOURCE( IDD_PROP_KEYWORD ),
						hwndDlg,
						CPropKeyword::DlgProc_dialog,
						(LPARAM)pPropKeyword
					);
					if( res == IDOK ){
						CShareDataLockCounter::WaitLock( pCommon->m_hwndParent );
						pCommon->ApplyData( m_nSet );
						SetDataKeyword(hwndDlg);
						// 設定済みキーワードが削除されたかもしれないので再設定
						RearrangeKeywordSet( hwndDlg );
						m_bChangeKeyWordSet = true;
					}
					delete pPropKeyword;
					return TRUE;
				}
			case IDC_CHECK_STRINGLINEONLY:
				{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STRINGENDLINE),
						::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_STRINGLINEONLY ) );
					return TRUE;
				}
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		//	From Here May 21, 2001 genta activate spin control
		case IDC_SPIN_LCColNum:
			/* 行コメント桁位置 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LCColNum2:
			/* 行コメント桁位置 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, nVal, FALSE );
			return TRUE;
		//	To Here May 21, 2001 genta activate spin control

		//	From Here Jun. 01, 2001 JEPRO 3つ目を追加
		case IDC_SPIN_LCColNum3:
			/* 行コメント桁位置 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, nVal, FALSE );
			return TRUE;
		//	To Here Jun. 01, 2001
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//				OnHelp( hwndDlg, IDD_PROP1P3 );
				OnHelp( hwndDlg, IDD_PROP_COLOR );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( _T("color PSN_KILLACTIVE\n") );
				/* ダイアログデータの取得 color */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPTYPE_PAGENUM_COLOR;
				return TRUE;
			}
			break;	/* default */
		}
		break;	/* WM_NOTIFY */
	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;				/* コントロールのID */
		pDis = (LPDRAWITEMSTRUCT) lParam;	/* 項目描画情報 */
		switch( idCtrl ){

		case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cTEXT );
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_sColorAttr.m_cBACK );
			return TRUE;
		case IDC_LIST_COLORS:		/* 色種別リスト */
			DrawColorListItem( pDis );
			return TRUE;
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}


/* ダイアログデータの設定 color */
void CPropTypesColor::SetData( HWND hwndDlg )
{

	HWND	hwndWork;
	int		i;
	int		nItem;

	m_nCurrentColorType = 0;	/* 現在選択されている色タイプ */

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */	//@@@ 2002.09.22 YAZAKI
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT2 )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT3 )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );	//Jun. 01, 2001 JEPRO 追加
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM )	, BLOCKCOMMENT_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO )	, BLOCKCOMMENT_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 ), BLOCKCOMMENT_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, BLOCKCOMMENT_BUFFERSIZE - 1 );

	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_cBlockComments[0].getBlockCommentFrom() );	/* ブロックコメントデリミタ(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO	, m_Types.m_cBlockComments[0].getBlockCommentTo() );	/* ブロックコメントデリミタ(To) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_cBlockComments[1].getBlockCommentFrom() );	/* ブロックコメントデリミタ2(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_cBlockComments[1].getBlockCommentTo() );	/* ブロックコメントデリミタ2(To) */

	/* 行コメントデリミタ @@@ 2002.09.22 YAZAKI*/
	//	From Here May 12, 2001 genta
	//	行コメントの開始桁位置設定
	//	May 21, 2001 genta 桁位置を1から数えるように
	for ( i=0; i<COMMENT_DELIMITER_NUM; i++ ){
		//	テキスト
		::DlgItem_SetText( hwndDlg, cLineComment[i].nEditID, m_Types.m_cLineComment.getLineComment(i) );	

		//	桁数チェックと、数値
		int nPos = m_Types.m_cLineComment.getLineCommentPos(i);
		if( nPos >= 0 ){
			::CheckDlgButton( hwndDlg, cLineComment[i].nCheckBoxID, TRUE );
			::SetDlgItemInt( hwndDlg, cLineComment[i].nTextID, nPos + 1, FALSE );
		}
		else {
			::CheckDlgButton( hwndDlg, cLineComment[i].nCheckBoxID, FALSE );
			::SetDlgItemInt( hwndDlg, cLineComment[i].nTextID, (~nPos) + 1, FALSE );
		}
	}

	HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_STRINGLITERAL );
	Combo_ResetContent( hwndCombo );
	int		nSelPos = 0;
	for( i = 0; i < _countof( StringLitteralArr ); ++i ){
		Combo_InsertString( hwndCombo, i, LS(StringLitteralArr[i].nNameId) );
		if( StringLitteralArr[i].nMethod == m_Types.m_nStringType ){		// テキストの折り返し方法
			nSelPos = i;
		}
	}
	Combo_SetCurSel( hwndCombo, nSelPos );
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_STRINGLINEONLY, m_Types.m_bStringLineOnly );
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_STRINGENDLINE, m_Types.m_bStringEndLine );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STRINGENDLINE),
		::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_STRINGLINEONLY ) );

	//強調キーワード1～10の設定
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		m_nSet[ i ] = m_Types.m_nKeyWordSetIdx[i];
	}
	SetDataKeyword( hwndDlg ); // m_nSet

	/* 色をつける文字種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	List_ResetContent( hwndWork );  /* リストを空にする */
	// 2014.11.25 大きいフォント対応
	int nItemHeight = CTextWidthCalc(hwndWork).GetTextHeight();
	List_SetItemHeight(hwndWork, 0, nItemHeight + 4);
	for( i = 0; i < COLORIDX_LAST; ++i ){
		GetDefaultColorInfoName( &m_Types.m_ColorInfoArr[i], i );
		nItem = ::List_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
		List_SetItemData( hwndWork, nItem, &m_Types.m_ColorInfoArr[i] );
	}
	/* 現在選択されている色タイプ */
	List_SetCurSel( hwndWork, m_nCurrentColorType );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

	// from here 2005.11.30 Moca 指定位置縦線の設定
	WCHAR szVertLine[MAX_VERTLINES * 15] = L"";
	int offset = 0;
	for( i = 0; i < MAX_VERTLINES && m_Types.m_nVertLineIdx[i] != 0; i++ ){
		CKetaXInt nXCol = m_Types.m_nVertLineIdx[i];
		CKetaXInt nXColEnd = nXCol;
		CKetaXInt nXColAdd = CKetaXInt(1);
		if( nXCol < 0 ){
			if( i < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = m_Types.m_nVertLineIdx[++i];
				nXColAdd = m_Types.m_nVertLineIdx[++i];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				if(offset){
					szVertLine[offset] = ',';
					szVertLine[offset+1] = '\0';
					offset += 1;
				}
				offset += auto_sprintf( &szVertLine[offset], L"%d(%d,%d)", nXColAdd, nXCol, nXColEnd );
			}
		}
		else{
			if(offset){
				szVertLine[offset] = ',';
				szVertLine[offset+1] = '\0';
				offset += 1;
			}
			offset += auto_sprintf( &szVertLine[offset], L"%d", nXCol );
		}
	}
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_VERTLINE ), MAX_VERTLINES * 15 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine );
	// to here 2005.11.30 Moca 指定位置縦線の設定
	return;
}


/*! セット名コンボボックスの値セット
*/
void CPropTypesColor::SetDataKeyword( HWND hwndDlg )
{
	int i;

	HWND hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	Combo_ResetContent( hwndWork );  /* コンボボックスを空にする */
	/* 一行目は空白 */
	Combo_AddString( hwndWork, L" " );
	//	Mar. 31, 2003 genta KeyWordSetMgrをポインタに
	if( 0 < m_pCKeyWordSetMgr->m_nKeyWordSetNum ){
		const int* const set = m_nSet;
		for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_pCKeyWordSetMgr->GetTypeName( i ) );
		}
		if( -1 == set[0] ){
			/* セット名コンボボックスのデフォルト選択 */
			Combo_SetCurSel( hwndWork, 0 );
		}else{
			/* セット名コンボボックスのデフォルト選択 */
			Combo_SetCurSel( hwndWork, set[0] + 1 );
		}
	}
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* ダイアログデータの取得 color */
int CPropTypesColor::GetData( HWND hwndDlg )
{
	int		nIdx;
	HWND	hwndWork;

	//	From Here May 12, 2001 genta
	//	コメントの開始桁位置の取得
	//	May 21, 2001 genta 桁位置を1から数えるように
	wchar_t buffer[COMMENT_DELIMITER_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI LineCommentを取得するためのバッファ
	int pos;
	UINT en;
	BOOL bTranslated;

	int i;
	for( i=0; i<COMMENT_DELIMITER_NUM; i++ ){
		en = ::IsDlgButtonChecked( hwndDlg, cLineComment[i].nCheckBoxID );
		pos = ::GetDlgItemInt( hwndDlg, cLineComment[i].nTextID, &bTranslated, FALSE );
		if( bTranslated != TRUE ){
			en = 0;
			pos = 0;
		}
		//	pos == 0のときは無効扱い
		if( pos == 0 )	en = 0;
		else			--pos;
		//	無効のときは1の補数で格納

		::DlgItem_GetText( hwndDlg, cLineComment[i].nEditID		, buffer	, COMMENT_DELIMITER_BUFFERSIZE );		/* 行コメントデリミタ */
		m_Types.m_cLineComment.CopyTo( i, buffer, en ? pos : ~pos );
	}

	wchar_t szFromBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI
	wchar_t szToBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI

	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* ブロックコメントデリミタ(From) */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* ブロックコメントデリミタ(To) */
	m_Types.m_cBlockComments[0].SetBlockCommentRule( szFromBuffer, szToBuffer );

	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* ブロックコメントデリミタ(From) */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* ブロックコメントデリミタ(To) */
	m_Types.m_cBlockComments[1].SetBlockCommentRule( szFromBuffer, szToBuffer );

	/* 文字列区切り記号エスケープ方法 */
	int		nSelPos = Combo_GetCurSel( GetDlgItem(hwndDlg, IDC_COMBO_STRINGLITERAL) );
	if( nSelPos >= 0 ){
		m_Types.m_nStringType = StringLitteralArr[nSelPos].nMethod;
	}
	m_Types.m_bStringLineOnly = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_STRINGLINEONLY );
	m_Types.m_bStringEndLine = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_STRINGENDLINE );
	

	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	nIdx = Combo_GetCurSel( hwndWork );
	if( CB_ERR == nIdx ||
		0 == nIdx ){
		m_Types.m_nKeyWordSetIdx[0] = -1;
	}else{
		m_Types.m_nKeyWordSetIdx[0] = nIdx - 1;

	}
	m_nSet[0] = m_Types.m_nKeyWordSetIdx[0];

	//強調キーワード2～10の取得(1は別)
	for( nIdx = 1; nIdx < MAX_KEYWORDSET_PER_TYPE; nIdx++ ){
		m_Types.m_nKeyWordSetIdx[nIdx] = m_nSet[nIdx];
	}

	// from here 2005.11.30 Moca 指定位置縦線の設定
	WCHAR szVertLine[MAX_VERTLINES * 15];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine, MAX_VERTLINES * 15 );

	int offset = 0;
	i = 0;
	while( i < MAX_VERTLINES ){
		int value = 0;
		for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
			value = szVertLine[offset] - '0' + value * 10;
		}
		if( value <= 0 ){
			break;
		}
		if( szVertLine[offset] == '(' ){
			offset++;
			int valueBegin = 0;
			int valueEnd = 0;
			for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
				valueBegin = szVertLine[offset] - '0' + valueBegin * 10;
			}
			if( valueBegin <= 0 ){
				break;
			}
			if( szVertLine[offset] == ',' ){
				offset++;
			}else if( szVertLine[offset] != ')' ){
				break;
			}
			for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
				valueEnd = szVertLine[offset] - '0' + valueEnd * 10;
			}
			if( valueEnd <= 0 ){
				valueEnd = MAXLINEKETAS;
			}
			if( szVertLine[offset] != ')' ){
				break;
			}
			offset++;
			if(i + 2 < MAX_VERTLINES){
				m_Types.m_nVertLineIdx[i++] = CKetaXInt(-valueBegin);
				m_Types.m_nVertLineIdx[i++] = CKetaXInt(valueEnd);
				m_Types.m_nVertLineIdx[i++] = CKetaXInt(value);
			}
			else{
				break;
			}
		}
		else{
			m_Types.m_nVertLineIdx[i++] = CKetaXInt(value);
		}
		if( szVertLine[offset] != ',' ){
			break;
		}
		offset++;
	}
	if( i < MAX_VERTLINES ){
		m_Types.m_nVertLineIdx[i] = CKetaXInt(0);
	}
	// to here 2005.11.30 Moca 指定位置縦線の設定
	return TRUE;
}



/* 色ボタンの描画 */
void CPropTypesColor::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE( _T("pDis->itemAction = ") );

	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	RECT		rc;
	RECT		rcFocus;

	//描画対象
	CGraphics gr(pDis->hDC);

	/* ボタンの表面の色で塗りつぶす */
	gr.SetBrushColor( cBtnFace );
	gr.FillMyRect( pDis->rcItem );

	/* 枠の描画 */
	rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if( pDis->itemState & ODS_SELECTED ){

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 3, NULL );
		::LineTo( gr, 1, 1 );
		::LineTo( gr, pDis->rcItem.right - 2, 1 );

		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}
	else{
		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2 );
		::LineTo( gr, pDis->rcItem.right - 2, 0 );

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );
	}
	
	if((pDis->itemState & ODS_DISABLED)==0){
		/* 指定色で塗りつぶす */
		gr.SetBrushColor(cColor);
		gr.SetPen(cBtnShadow);
		::RoundRect( gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
	}

	/* フォーカスの長方形 */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( gr, &rcFocus );
	}
}



//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropTypesColor::EnableTypesPropInput( HWND hwndDlg )
{
	//	From Here Jun. 6, 2001 genta
	//	行コメント開始桁位置入力ボックスのEnable/Disable設定
	//	1つ目
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum ), FALSE );
	}
	//	2つ目
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum2 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum2 ), FALSE );
	}
	//	3つ目
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS3 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS3 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS3 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum3 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum3 ), FALSE );
	}
	//	To Here Jun. 6, 2001 genta
}
//	To Here Sept. 10, 2000



/*!	@brief キーワードセットの再配列

	キーワードセットの色分けでは未指定のキーワードセット以降はチェックを省略する．
	そのためセットの途中に未指定のものがある場合はそれ以降を前に詰めることで
	指定された全てのキーワードセットが有効になるようにする．
	その際，色分けの設定も同時に移動する．

	m_nSet, m_Types.m_ColorInfoArr[]が変更される．

	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル

	@author	genta 
	@date	2005.01.23 genta new

*/
void CPropTypesColor::RearrangeKeywordSet( HWND hwndDlg )
{
	int i, j;
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		if( m_nSet[ i ] != -1 )
			continue;

		//	未設定の場合
		for( j = i; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			if( m_nSet[ j ] != -1 ){
				//	後ろに設定済み項目があった場合
				m_nSet[ i ] = m_nSet[ j ];
				m_nSet[ j ] = -1;

				//	色設定を入れ替える
				//	構造体ごと入れ替えると名前が変わってしまうので注意
				ColorInfo &col1 = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ];
				ColorInfo &col2   = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + j ];

				std::swap( col1.m_bDisp, col2.m_bDisp );
				std::swap( col1.m_sFontAttr, col2.m_sFontAttr );
				std::swap( col1.m_sColorAttr, col2.m_sColorAttr );

				break;
			}
		}
		if( j == MAX_KEYWORDSET_PER_TYPE ){
			//	後ろには設定済み項目がなかった
			break;
		}
	}
	
	//	リストボックス及び色設定ボタンを再描画
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_LIST_COLORS ), NULL, TRUE );
}



/* 色種別リスト オーナー描画 */
void CPropTypesColor::DrawColorListItem( DRAWITEMSTRUCT* pDis )
{
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor( COLOR_3DSHADOW );

	if( pDis == NULL || pDis->itemData == 0 ) return;

	//描画対象
	CGraphics gr(pDis->hDC);

//	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
//	rc2 = pDis->rcItem;

	/* アイテムデータの取得 */
	pColorInfo = (ColorInfo*)pDis->itemData;

	/* アイテム矩形塗りつぶし */
	gr.SetBrushColor( ::GetSysColor( COLOR_WINDOW ) );
	gr.FillMyRect( pDis->rcItem );
	
	/* アイテムが選択されている */
	if( pDis->itemState & ODS_SELECTED ){
		gr.SetBrushColor( ::GetSysColor( COLOR_HIGHLIGHT ) );
		gr.SetTextForeColor( ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
	}else{
		gr.SetBrushColor( ::GetSysColor( COLOR_WINDOW ) );
		gr.SetTextForeColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
	}

	rc1.left+= (2 + 16);
	rc1.top += 2;
	rc1.right -= ( 2 + 27 );
	rc1.bottom -= 2;
	/* 選択ハイライト矩形 */
	gr.FillMyRect(rc1);
	/* テキスト */
	::SetBkMode( gr, TRANSPARENT );
	::TextOut( gr, rc1.left, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	if( pColorInfo->m_sFontAttr.m_bBoldFont ){	/* 太字か */
		::TextOut( gr, rc1.left + 1, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_sFontAttr.m_bUnderLine ){	/* 下線か */
		SIZE	sz;
		::GetTextExtentPoint32( gr, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ), &sz );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 2, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 2 );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 1, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 1 );
	}

	/* アイテムにフォーカスがある */	// 2006.05.01 ryoji 描画条件の不正を修正
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( gr, &pDis->rcItem );
	}

	/* 「色分け/表示する」のチェック */
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if( pColorInfo->m_bDisp ){	/* 色分け/表示する */
		// 2006.04.26 ryoji テキスト色を使う（「ハイコントラスト黒」のような設定でも見えるように）
		gr.SetPen( ::GetSysColor( COLOR_WINDOWTEXT ) );

		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
	}
//	return;


	// 2002/11/02 Moca 比較方法変更
//	if( 0 != strcmp( "カーソル行アンダーライン", pColorInfo->m_szName ) )
	if ( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
	{
		/* 背景色 見本矩形 */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		gr.SetBrushColor( pColorInfo->m_sColorAttr.m_cBACK );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}


	if( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_TEXT) )
	{
		/* 前景色 見本矩形 */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 27;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;
		gr.SetBrushColor( pColorInfo->m_sColorAttr.m_cTEXT );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}
}



/* 色選択ダイアログ */
BOOL CPropTypesColor::SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors )
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof_raw( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( !::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}

