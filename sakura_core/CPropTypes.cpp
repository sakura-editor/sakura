//	$Id$
/*!	@file
	@brief タイプ別設定ダイアログボックス

	@author Norio Nakatani
	@date 1998/12/24  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "sakura_rc.h"
#include "CPropTypes.h"
#include "debug.h"
#include <windows.h>
//#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "funccode.h"	//Stonee, 2001/05/18

struct TYPE_NAME {
	int		nMethod;
	char*	pszName;
};
TYPE_NAME OlmArr[] = {
//	{ OUTLINE_C,		"C" },
	{ OUTLINE_CPP,	"C/C++" },
	{ OUTLINE_PLSQL,"PL/SQL" },
	{ OUTLINE_JAVA,	"Java" },
	{ OUTLINE_COBOL,"COBOL" },
	{ OUTLINE_PERL,	"Perl" },			//Sep. 8, 2000 genta
	{ OUTLINE_ASM,	"アセンブラ" },
	{ OUTLINE_VB,	"Visual Basic" },	// 2001/06/23 N.Nakatani
	{ OUTLINE_TEXT,	"テキスト" }		//Jul. 08, 2001 JEPRO 常に最後尾におく
};
const int	nOlmArrNum = sizeof( OlmArr ) / sizeof( OlmArr[0] );


TYPE_NAME SmartIndentArr[] = {
	{ SMARTINDENT_NONE,	"なし" },
	{ SMARTINDENT_CPP,	"C/C++" }
};
const int	nSmartIndentArrNum = sizeof( SmartIndentArr ) / sizeof( SmartIndentArr[0] );

//	Nov. 20, 2000 genta
TYPE_NAME ImeStateArr[] = {
	{ 0, "標準設定" },
	{ 1, "全角" },
	{ 2, "全角ひらがな" },
	{ 3, "全角カタカナ" },
	{ 4, "無変換" }
};
const int nImeStateArrNum = sizeof( ImeStateArr ) / sizeof( ImeStateArr[0] );

TYPE_NAME ImeSwitchArr[] = {
	{ 0, "そのまま" },
	{ 1, "常にON" },
	{ 2, "常にOFF" },
};
const int nImeSwitchArrNum = sizeof( ImeSwitchArr ) / sizeof( ImeSwitchArr[0] );


WNDPROC	m_wpColorListProc;

//Sept. 5, 2000 JEPRO 半角カタカナの全角化に伴い文字長を変更(21→32)
#define STR_COLORDATA_HEAD_LEN	32
#define STR_COLORDATA_HEAD		"テキストエディタ 色設定ファイル\x1a"

//#define STR_COLORDATA_HEAD2	" テキストエディタ色設定 Ver2"
//#define STR_COLORDATA_HEAD21	" テキストエディタ色設定 Ver2.1"	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
#define STR_COLORDATA_HEAD3		" テキストエディタ色設定 Ver3"		//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした
#define STR_COLORDATA_SECTION	"SakuraColor"

#define BOOL2STR( b )			TRUE==(b)?"TRUE":"FALSE"
char* MakeRGBStr( DWORD dwRGB, char* pszText )
{
	wsprintf( pszText, "RGB( %d, %d, %d )",
		GetRValue( dwRGB ),
		GetGValue( dwRGB ),
		GetBValue( dwRGB )
	);
	return pszText;

}


//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids1[] = {	//11300
	IDC_CHECK_WORDWRAP,				11310,	//英文ワードラップ
	IDC_COMBO_TABSPACE,				11330,	//TAB幅
	IDC_COMBO_IMESWITCH,			11331,	//IMEのON/OFF状態
	IDC_COMBO_IMESTATE,				11332,	//IMEの入力モード
	IDC_COMBO_SMARTINDENT,			11333,	//スマートインデント
	IDC_COMBO_OUTLINES,				11334,	//アウトライン解析方法
	IDC_EDIT_TYPENAME,				11340,	//設定の名前
	IDC_EDIT_TYPEEXTS,				11341,	//ファイル拡張子
	IDC_EDIT_MAXLINELEN,			11342,	//折り返し桁数
	IDC_EDIT_CHARSPACE,				11343,	//文字の間隔
	IDC_EDIT_LINESPACE,				11344,	//行の間隔
	IDC_EDIT_INDENTCHARS,			11345,	//その他のインデント対象文字
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	IDC_EDIT_TABVIEWSTRING,			11346,  //TAB表示文字列
//#endif
// From Here 2001.12.03 hor
	IDC_CHECK_INS_SPACE,			11347,	//スペースの挿入
// To Here 2001.12.03 hor
	IDC_SPIN_MAXLINELEN,			-1,
	IDC_SPIN_CHARSPACE,				-1,
	IDC_SPIN_LINESPACE,				-1,
//	IDC_STATIC,						-1,
	0, 0
};
const DWORD p_helpids2[] = {	//11400
	IDC_BUTTON_TEXTCOLOR,			11400,	//文字色
	IDC_BUTTON_BACKCOLOR,			11401,	//背景色
	IDC_BUTTON_SAMETEXTCOLOR,		11402,	//文字色統一
	IDC_BUTTON_SAMEBKCOLOR,			11403,	//背景色統一
	IDC_BUTTON_IMPORT,				11404,	//インポート
	IDC_BUTTON_EXPORT,				11405,	//エクスポート
	IDC_CHECK_DISP,					11410,	//色分け表示
	IDC_CHECK_FAT,					11411,	//太字
	IDC_CHECK_UNDERLINE,			11412,	//下線
	IDC_CHECK_LCPOS,				11413,	//桁指定１
	IDC_CHECK_LCPOS2,				11414,	//桁指定２
	IDC_COMBO_SET,					11430,	//強調キーワード１セット名
	IDC_COMBO_SET2,					11431,	//強調キーワード２セット名
	IDC_EDIT_BLOCKCOMMENT_FROM,		11440,	//ブロックコメント１開始
	IDC_EDIT_BLOCKCOMMENT_TO,		11441,	//ブロックコメント１終了
	IDC_EDIT_LINECOMMENT,			11442,	//行コメント１
	IDC_EDIT_LINECOMMENT2,			11443,	//行コメント２
	IDC_EDIT_LINECOMMENTPOS,		11444,	//桁数１
	IDC_EDIT_LINECOMMENTPOS2,		11445,	//桁数２
	IDC_EDIT_LINETERMCHAR,			11446,	//行番号区切り
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	IDC_EDIT_BLOCKCOMMENT_FROM2,	11447,	//ブロックコメント２開始
	IDC_EDIT_BLOCKCOMMENT_TO2,		11448,	//ブロックコメント２終了
//#endif
	IDC_EDIT_LINECOMMENT3,			11449,	//行コメント３
	IDC_LIST_COLORS,				11450,	//色指定
	IDC_CHECK_LCPOS3,				11451,	//桁指定３
	IDC_EDIT_LINECOMMENTPOS3,		11452,	//桁数３
	IDC_RADIO_ESCAPETYPE_1,			11460,	//文字列エスケープ（C言語風）
	IDC_RADIO_ESCAPETYPE_2,			11461,	//文字列エスケープ（PL/SQL風）
	IDC_RADIO_LINENUM_LAYOUT,		11462,	//行番号の表示（折り返し単位）
	IDC_RADIO_LINENUM_CRLF,			11463,	//行番号の表示（改行単位）
	IDC_RADIO_LINETERMTYPE0,		11464,	//行番号区切り（なし）
	IDC_RADIO_LINETERMTYPE1,		11465,	//行番号区切り（縦線）
	IDC_RADIO_LINETERMTYPE2,		11466,	//行番号区切り（任意）
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//From Here Jul. 05, 2001 JEPRO 追加
const DWORD p_helpids3[] = {	//11500
	IDC_BUTTON_HOKANFILE_REF,		11500,	//入力補完 単語ファイル参照
	IDC_BUTTON_KEYWORDHELPFILE_REF,	11501,	//キーワードヘルプファイル参照
	IDC_CHECK_HOKANLOHICASE,		11510,	//入力補完の英大文字小文字
	IDC_CHECK_USEKEYWORDHELP,		11511,	//キーワードヘルプ機能
	IDC_EDIT_HOKANFILE,				11540,	//単語ファイル名
	IDC_EDIT_KEYWORDHELPFILE,		11541,	//辞書ファイル名
//	IDC_STATIC,						-1,
	0, 0
};
//To Here Jul. 05, 2001



/* p1 ダイアログプロシージャ */
BOOL CALLBACK PropTypesP1Proc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p1( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		pCPropTypes = ( CPropTypes* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p1( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* p2 ダイアログプロシージャ */
BOOL CALLBACK PropTypesP2Proc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p2( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		pCPropTypes = ( CPropTypes* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p2( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}






//	/* p3 ダイアログプロシージャ */
//	BOOL CALLBACK PropTypesP3Proc(
//		HWND	hwndDlg,	// handle to dialog box
//		UINT	uMsg,		// message
//		WPARAM	wParam,		// first message parameter
//		LPARAM	lParam 		// second message parameter
//	)
//	{
//	PROPSHEETPAGE*	pPsp;
//	CPropTypes*		pCPropTypes;
//	switch( uMsg ){
//	case WM_INITDIALOG:
//		pPsp = (PROPSHEETPAGE*)lParam;
//		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
//		if( NULL != pCPropTypes ){
//			return pCPropTypes->DispatchEvent_p3( hwndDlg, uMsg, wParam, pPsp->lParam );
//		}else{
//			return FALSE;
//		}
//	default:
//		pCPropTypes = ( CPropTypes* )::GetWindowLong( hwndDlg, DWL_USER );
//		if( NULL != pCPropTypes ){
//			return pCPropTypes->DispatchEvent_p3( hwndDlg, uMsg, wParam, lParam );
//		}else{
//			return FALSE;
//		}
//	}
//	}



/* p3 ダイアログプロシージャ */
BOOL CALLBACK PropTypesP3_newProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes*		pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p3_new( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		pCPropTypes = ( CPropTypes* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p3_new( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}




CPropTypes::CPropTypes()
{
//	int		i;
//	long	lPathLen;

//	m_hbmpToolButtons = NULL;

//	/* 共有データ構造体のアドレスを返す */
//	m_cShareData.Init();
//	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	/* 共有データ構造体のアドレスを返す */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = 0;

	/* ヘルプファイルのフルパスを返す */
	::GetHelpFilePath( m_szHelpFile );

	return;
}





CPropTypes::~CPropTypes()
{
//	if( NULL != m_hbmpToolButtons ){
//		/* ビットマップ破棄 */
//		::DeleteObject( m_hbmpToolButtons );
//	}
	return;
}





/* 初期化 */
void CPropTypes::Create( HINSTANCE hInstApp, HWND hwndParent )
{
	m_hInstance = hInstApp;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	return;
}





/* 色選択ダイアログ */
BOOL CPropTypes::SelectColor( HWND hwndParent, COLORREF* pColor )
{
	int				i;
	CHOOSECOLOR		cc;
	DWORD			dwCustColors[16];
	for( i = 0; i < 16; i++ ){
		dwCustColors[i] = (DWORD)RGB( 255, 255, 255 );
	}
	cc.lStructSize = sizeof( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = (LPDWORD) dwCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( FALSE == ::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}





/* 色ボタンの描画 */
void CPropTypes::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE( "pDis->itemAction = " );

	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	COLORREF	cRim;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	RECT		rc;
	RECT		rcFocus;

	/* ボタンの表面の色で塗りつぶす */
	hBrush = ::CreateSolidBrush( cBtnFace );
	::FillRect( pDis->hDC, &(pDis->rcItem), hBrush );
	::DeleteObject( hBrush );

	/* 枠の描画 */
	rcFocus = rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if( pDis->itemState & ODS_SELECTED ){
		hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, 0, 0 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 1, pDis->rcItem.bottom - 3, NULL );
		::LineTo( pDis->hDC, 1, 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, 1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, -1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}else{
		hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, 0, 0 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 1, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, -1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );
	}
	if( pDis->itemState & ODS_DISABLED ){
	}else{
		/* 指定色で塗りつぶす */
		hBrush = ::CreateSolidBrush( cColor );
		hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
		cRim = cBtnShadow;
		hPen = ::CreatePen( PS_SOLID, 0, cRim );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::RoundRect( pDis->hDC, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
		::SelectObject( pDis->hDC, hPenOld );
		::SelectObject( pDis->hDC, hBrushOld );
		::DeleteObject( hPen );
		::DeleteObject( hBrush );
	}


//	/* 区切りたて棒 */
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 3, rc.top, NULL );
//	::LineTo( pDis->hDC, rc.right + 3, rc.bottom );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );
//
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 4, rc.top, NULL );
//	::LineTo( pDis->hDC, rc.right + 4, rc.bottom );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );
//
//	/* ▼記号 */
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 6		, rc.top + 6, NULL );
//	::LineTo(	pDis->hDC, rc.right + 6 + 5	, rc.top + 6 );
//	::MoveToEx( pDis->hDC, rc.right + 7		, rc.top + 7, NULL );
//	::LineTo(	pDis->hDC, rc.right + 7 + 3	, rc.top + 7 );
//	::MoveToEx( pDis->hDC, rc.right + 8		, rc.top + 8, NULL );
//	::LineTo(	pDis->hDC, rc.right + 8 + 1	, rc.top + 8 );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );

	/* フォーカスの長方形 */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( pDis->hDC, &rcFocus );
	}
	return;
}










//	キーワード：タイプ別設定タブ順序(プロパティシート)
/* プロパティシートの作成 */
int CPropTypes::DoPropertySheet( int nPageNum )
{
//	/* 共有データ構造体のアドレスを返す */
//	m_cShareData.Init();
//	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	int				nRet;
	PROPSHEETPAGE	psp[16];
	PROPSHEETHEADER	psh;
	int				nIdx;

	m_nMaxLineSize_org = m_Types.m_nMaxLineSize;


	nIdx = 0;
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP1 );
	psp[nIdx].pszIcon = NULL/*MAKEINTRESOURCE( IDI_FONT )*/;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP1Proc;
	psp[nIdx].pszTitle = "スクリーン";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

//	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
//	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
//	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
//	psp[nIdx].hInstance = m_hInstance;
//	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP1P3 );
//	psp[nIdx].pszIcon = NULL /*MAKEINTRESOURCE( IDI_BORDER )*/;
//	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP3Proc;
//	psp[nIdx].pszTitle = "カラー";
//	psp[nIdx].lParam = (LPARAM)this;
//	psp[nIdx].pfnCallback = NULL;
//	nIdx++;

	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_COLOR );
	psp[nIdx].pszIcon = NULL /*MAKEINTRESOURCE( IDI_BORDER) */;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP3_newProc;
	psp[nIdx].pszTitle = "カラー";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	// 2001/06/14 Start by asa-o: タイプ別設定に支援タブ追加
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP2 );
	psp[nIdx].pszIcon = NULL;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP2Proc;
	psp[nIdx].pszTitle = "支援";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001/06/14 End

	// 2001.11.17 add start MIK タイプ別設定に正規表現キーワードタブ追加
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_REGEX );
	psp[nIdx].pszIcon = NULL;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesRegex;
	psp[nIdx].pszTitle = "正規表現キーワード";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001.11.17 add end MIK

	memset( &psh, 0, sizeof( PROPSHEETHEADER ) );
	psh.dwSize = sizeof( PROPSHEETHEADER );
// JEPROtest Sept. 30, 2000 タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
//  psh.dwFlags = /*PSH_USEICONID |*/ /*PSH_NOAPPLYNOW |*/ PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.dwFlags = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance = m_hInstance;
	psh.pszIcon = NULL /*MAKEINTRESOURCE( IDI_CELL_PROPERTIES )*/;
	psh.pszCaption = (LPSTR)"タイプ別設定";	// Sept. 8, 2000 jepro 単なる「設定」から変更
	psh.nPages = nIdx;

	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}else{
		psh.nStartPage = nPageNum;
	}
	if( 0 > psh.nStartPage ){
		psh.nStartPage = 0;
	}
	if( psh.nPages - 1 < psh.nStartPage ){
		psh.nStartPage = psh.nPages - 1;
	}
	psh.ppsp = (LPCPROPSHEETPAGE)psp;
	psh.pfnCallback = NULL;

//	m_hbmpToolButtons = ::LoadBitmap( m_hInstance, MAKEINTRESOURCE( IDB_MYTOOL ) );

	nRet = ::PropertySheet( &psh );
	if( -1 == nRet ){
		char*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
			"CPropTypes::DoPropertySheet()内でエラーが出ました。\npsh.nStartPage=[%d]\n::PropertySheet()失敗。\n\n%s\n", psh.nStartPage, pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

//	::DeleteObject( m_hbmpToolButtons );
//	m_hbmpToolButtons = NULL;
	return nRet;
}





/* p1 メッセージ処理 */
BOOL CPropTypes::DispatchEvent_p1(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		m_hwndThis = hwndDlg;
		/* ダイアログデータの設定 p1 */
		SetData_p1( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTypeName ) - 1 ), 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTypeExts ) - 1 ), 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_INDENTCHARS ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szIndentChars ) - 1 ), 0 );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTabViewString ) - 1 ), 0 );
//#endif

		if( 0 == m_Types.m_nIdx ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), FALSE );
		}

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_INDENT:	/* オートインデント */
//				MYTRACE( "IDC_CHECK_INDENT\n" );
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT ) ){
					/* 日本語空白もインデント */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), TRUE );
				}else{
					/* 日本語空白もインデント */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), FALSE );
				}
				return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_SPIN_MAXLINELEN:
			/* 折り返し文字数 */
//			MYTRACE( "IDC_SPIN_MAXLINELEN\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 10 ){
				nVal = 10;
			}
			if( nVal > 10240 ){
				nVal = 10240;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			/* 文字の隙間 */
//			MYTRACE( "IDC_SPIN_CHARSPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > 16 ){
				nVal = 16;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LINESPACE:
			/* 行の隙間 */
//			MYTRACE( "IDC_SPIN_LINESPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO 行間も最小0まで設定できるように変更(昔に戻っただけ?)
//			if( nVal < 1 ){
//				nVal = 1;
//			}
			if( nVal < 0 ){
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if( nVal > 16 ){
				nVal = 16;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_REPEATEDSCROLLLINENUM:
			/* キーリピート時のスクロール行数 */
//			MYTRACE( "IDC_SPIN_REPEATEDSCROLLLINENUM\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 10 ){
				nVal = 10;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE );
			return TRUE;

		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROPTYPESP1 );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p1 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p1 */
				GetData_p1( hwndDlg );

//				if( m_nMaxLineSize_org != m_Types.m_nMaxLineSize ){
//					if( IDNO == ::MessageBox( hwndDlg, "折り返し桁数を変更すると、アンドゥ・リドゥバッファがクリアされます。\nよろしいですか？", GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){
//						m_Types.m_nMaxLineSize = m_nMaxLineSize_org;
//						SetData_p1( hwndDlg );
//					}
//				}
				return TRUE;
			}
			break;
		}

//		MYTRACE( "pNMHDR->hwndFrom	=%xh\n",	pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom	=%xh\n",	pNMHDR->idFrom );
//		MYTRACE( "pNMHDR->code		=%xh\n",	pNMHDR->code );
//		MYTRACE( "pMNUD->iPos		=%d\n",		pMNUD->iPos );
//		MYTRACE( "pMNUD->iDelta		=%d\n",		pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids1 );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids1 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* ダイアログデータの設定 p1 */
void CPropTypes::SetData_p1( HWND hwndDlg )
{
	BOOL	bRet;
	static	int	nTabArr[] = { 2, 3, 4, 6, 8 };
	static	int	nTabArrNum = sizeof( nTabArr ) / sizeof( nTabArr[0] );
	int			i, j;
	char		szWork[32];

	/* タイプ属性：名称 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName );

	/* タイプ属性：拡張子リスト */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts );

	/* 折り返し文字数 */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, m_Types.m_nMaxLineSize, FALSE );

	/* 文字の隙間 */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, m_Types.m_nColmSpace, FALSE );

	/* 行の隙間 */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, m_Types.m_nLineSpace, FALSE );

	/* TAB幅 */
	j = 0;
	for( i = 0; i < nTabArrNum; ++i ){
		wsprintf( szWork, "%d", nTabArr[i] );
		::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)szWork );
		if( m_Types.m_nTabSpace == nTabArr[i] ){
			j = i;
		}
	}
	::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_SETCURSEL, (WPARAM)j, 0 );

//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	/* TAB表示文字列 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TABVIEWSTRING, m_Types.m_szTabViewString );
//#endif

// From Here 2001.12.03 hor
	/* スペースの挿入 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INS_SPACE, m_Types.m_bInsSpace );
// To Here 2001.12.03 hor

	/* その他のインデント対象文字 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars );


	/* アウトライン解析方法 */
	HWND	hwndCombo;
	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	nSelPos = 0;
	for( i = 0; i < nOlmArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)OlmArr[i].pszName );
		if( OlmArr[i].nMethod == m_Types.m_nDefaultOutline ){	/* アウトライン解析方法 */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );


	/* スマートインデント種別 */
//	HWND	hwndCombo;
//	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	nSelPos = 0;
	for( i = 0; i < nSmartIndentArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)SmartIndentArr[i].pszName );
		if( SmartIndentArr[i].nMethod == m_Types.m_nSmartIndent ){	/* スマートインデント種別 */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

	//	From Here Nov. 20, 2000 genta
	//	IME入力モード
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	int ime = m_Types.m_nImeState >> 2;
	nSelPos = 0;
	for( i = 0; i < nImeStateArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeStateArr[i].pszName );
		if( ImeStateArr[i].nMethod == ime ){	/* IME状態 */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	//	IME ON/OFF制御
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	ime = m_Types.m_nImeState & 3;
	nSelPos = 0;
	for( i = 0; i < nImeSwitchArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeSwitchArr[i].pszName );
		if( ImeSwitchArr[i].nMethod == ime ){	/* IME状態 */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	//	To Here Nov. 20, 2000 genta

	/* 英文ワードラップをする */
	::CheckDlgButton( hwndDlg, IDC_CHECK_WORDWRAP, m_Types.m_bWordWrap );
	return;
}





/* ダイアログデータの取得 p1 */
int CPropTypes::GetData_p1( HWND hwndDlg )
{
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	char szTab[8+1+1]; /* +1. happy */
	int  i;
//#endif

	m_nPageNum = 0;
	/* タイプ属性：名称 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName, sizeof( m_Types.m_szTypeName ) );
	/* タイプ属性：拡張子リスト */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts, sizeof( m_Types.m_szTypeExts ) );


	/* 折り返し文字数 */
	m_Types.m_nMaxLineSize = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
	if( m_Types.m_nMaxLineSize < 10 ){
		m_Types.m_nMaxLineSize = 10;
	}
	if( m_Types.m_nMaxLineSize > 10240 ){
		m_Types.m_nMaxLineSize = 10240;
	}

	/* 文字の隙間 */
	m_Types.m_nColmSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
	if( m_Types.m_nColmSpace < 0 ){
		m_Types.m_nColmSpace = 0;
	}
	if( m_Types.m_nColmSpace > 16 ){
		m_Types.m_nColmSpace = 16;
	}

	/* 行の隙間 */
	m_Types.m_nLineSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
//	From Here Oct. 8, 2000 JEPRO 行間も最小0まで設定できるように変更(昔に戻っただけ?)
//	if( m_Types.m_nLineSpace < 1 ){
//		m_Types.m_nLineSpace = 1;
//	}
	if( m_Types.m_nLineSpace < 0 ){
		m_Types.m_nLineSpace = 0;
	}
//	To Here  Oct. 8, 2000
	if( m_Types.m_nLineSpace > 16 ){
		m_Types.m_nLineSpace = 16;
	}

	/* その他のインデント対象文字 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars, sizeof( m_Types.m_szIndentChars ) - 1 );

	/* TAB幅 */
	m_Types.m_nTabSpace = ::GetDlgItemInt( hwndDlg, IDC_COMBO_TABSPACE, NULL, FALSE );

//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	/* TAB表示文字列 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, sizeof( szTab ) - 1 );
	strcpy( m_Types.m_szTabViewString, "^       " );
	for( i = 0; i < 8; i++ ){
		if( (szTab[i] == '\0') || (szTab[i] < 0x20 || szTab[i] >= 0x7f) ) break;
		m_Types.m_szTabViewString[i] = szTab[i];
	}
//#endif

// 2001.12.03 hor
	/* スペースの挿入 */
	m_Types.m_bInsSpace = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INS_SPACE );
// From Here 2001.12.03 hor

	/* アウトライン解析方法 */
	HWND	hwndCombo;
	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nDefaultOutline = OlmArr[nSelPos].nMethod;	/* アウトライン解析方法 */

	/* スマートインデント種別 */
//	HWND	hwndCombo;
//	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nSmartIndent = SmartIndentArr[nSelPos].nMethod;	/* スマートインデント種別 */

	//	From Here Nov. 20, 2000 genta	IME状態
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME入力モード

	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	//	To Here Nov. 20, 2000 genta

	/* 英文ワードラップをする */
	m_Types.m_bWordWrap = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WORDWRAP );

	return TRUE;
}



// 2001/06/13 Start By asa-o: タイプ別設定の支援タブに関する処理

/* p2 メッセージ処理 */
BOOL CPropTypes::DispatchEvent_p2(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p2 */
		SetData_p2( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 入力補完 単語ファイル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* キーワードヘルプ 辞書ファイル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p2 */
			GetData_p2( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完 単語ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Types.m_szHokanFile );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.*",
						m_Types.m_szHokanFile,
						(const char **)&pszMRU,
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szHokanFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;

			//	From Here Sept. 12, 2000 JEPRO
			case IDC_CHECK_USEKEYWORDHELP:	/* キーワードヘルプ機能を使う時だけ辞書ファイル指定と参照ボタンをEnableにする */
				::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Types.m_bUseKeyWordHelp );
				if( BST_CHECKED == m_Types.m_bUseKeyWordHelp ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
				}else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
				}
				return TRUE;
			//	To Here Sept. 12, 2000

			case IDC_BUTTON_KEYWORDHELPFILE_REF:	/* キーワードヘルプ 辞書ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					char*			pszMRU = NULL;;
					char*			pszOPENFOLDER = NULL;;
					char			szPath[_MAX_PATH + 1];
					strcpy( szPath, m_Types.m_szKeyWordHelpFile );
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.*",
						m_Types.m_szKeyWordHelpFile,
						(const char **)&pszMRU,
						(const char **)&pszOPENFOLDER
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szKeyWordHelpFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Types.m_szKeyWordHelpFile );
					}
				}
				return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		case ???????:
//			return 0L;
//		default:
			switch( pNMHDR->code ){
			case PSN_HELP:	//Jul. 03, 2001 JEPRO 支援タブのヘルプを有効化
				OnHelp( hwndDlg, IDD_PROPTYPESP2 );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p10 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p2 */
				GetData_p2( hwndDlg );
				return TRUE;
			}
			break;
//		}

		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids3 );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids3 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* ダイアログデータの設定 p2 */
void CPropTypes::SetData_p2( HWND hwndDlg )
{
	/* 入力補完 単語ファイル */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

//	2001/06/19 asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase );

	/* キーワードヘルプを使用する  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USEKEYWORDHELP, m_Types.m_bUseKeyWordHelp );
//	From Here Sept. 12, 2000 JEPRO キーワードヘルプ機能を使う時だけ辞書ファイル指定と参照ボタンをEnableにする
	if( BST_CHECKED == m_Types.m_bUseKeyWordHelp ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_KEYWORDHELPFILE_REF ), FALSE );
	}
//	To Here Sept. 12, 2000

	/* キーワードヘルプ 辞書ファイル */
	::SetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Types.m_szKeyWordHelpFile );

	return;
}

/* ダイアログデータの取得 p2 */
int CPropTypes::GetData_p2( HWND hwndDlg )
{
	m_nPageNum = 2;

//	2001/06/19	asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	/* 入力補完 単語ファイル */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, MAX_PATH - 1 );

	/* キーワードヘルプを使用する */
	m_Types.m_bUseKeyWordHelp = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USEKEYWORDHELP );

	/* キーワードヘルプ 辞書ファイル */
	::GetDlgItemText( hwndDlg, IDC_EDIT_KEYWORDHELPFILE, m_Types.m_szKeyWordHelpFile, MAX_PATH - 1 );

	return TRUE;
}

// 2001/06/13 End



//	/* p3 メッセージ処理 */
//	BOOL CPropTypes::DispatchEvent_p3(
//		HWND	hwndDlg,	// handle to dialog box
//		UINT	uMsg,		// message
//		WPARAM	wParam,		// first message parameter
//		LPARAM	lParam 		// second message parameter
//	)
//	{
//		WORD				wNotifyCode;
//		WORD				wID;
//		HWND				hwndCtl;
//		NMHDR*				pNMHDR;
//		NM_UPDOWN*			pMNUD;
//		int					idCtrl;
//	//	int					nVal;
//		LPDRAWITEMSTRUCT	pDis;
//
//		switch( uMsg ){
//		case WM_INITDIALOG:
//			/* ダイアログデータの設定 p3 */
//			SetData_p3( hwndDlg );
//			::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );
//			return TRUE;
//
//		case WM_COMMAND:
//			wNotifyCode	= HIWORD(wParam);	/* 通知コード */
//			wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//			hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
//			switch( wNotifyCode ){
//			/* ボタン／チェックボックスがクリックされた */
//			case BN_CLICKED:
//				switch( wID ){
//				case IDC_BUTTON_SAMEBKCOLOR:	/* すべて同じ背景色にする */	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
//					m_Types.m_colorCRLFBACK =
//					m_Types.m_colorGYOUBACK =
//					m_Types.m_colorTABBACK =
//					m_Types.m_colorZENSPACEBACK =
//					m_Types.m_colorEOFBACK =
//					m_Types.m_colorCCPPKEYWORDBACK = m_Types.m_colorBACK;
//					m_Types.m_colorCOMMENTBACK = m_Types.m_colorBACK;
//					m_Types.m_colorSSTRINGBACK = m_Types.m_colorBACK;
//					m_Types.m_colorWSTRINGBACK = m_Types.m_colorBACK;
//
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CRLFBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_GYOUBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TABBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_ZENBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_EOFBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CCPPKEYWORDBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_COMMENTBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_SSTRINGBACKCOLOR ), NULL, TRUE );
//					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_WSTRINGBACKCOLOR ), NULL, TRUE );
//					return TRUE;
//
//				case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorTEXT ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//
//				case IDC_BUTTON_CRLFCOLOR:	/* 改行の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCRLF ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CRLFCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_CRLFBACKCOLOR:	/* 改行背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCRLFBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CRLFBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//
//				case IDC_BUTTON_GYOUCOLOR:	/* 行番号の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorGYOU ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_GYOUCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_GYOUBACKCOLOR:	/* 行番号背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorGYOUBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_GYOUBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//
//				case IDC_BUTTON_TABCOLOR:	/* TAB文字の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorTAB ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TABCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_TABBACKCOLOR:	/* TAB文字背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorTABBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TABBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//
//				case IDC_BUTTON_ZENCOLOR:	/* 全角スペース文字の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorZENSPACE ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_ZENCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_ZENBACKCOLOR:	/* 全角スペース文字背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorZENSPACEBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_ZENBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//				case IDC_BUTTON_EOFCOLOR:	/* EOFの色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorEOF ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_EOFCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_EOFBACKCOLOR:	/* EOF背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorEOFBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_EOFBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//
//				case IDC_BUTTON_CCPPKEYWORDCOLOR:	/* 強調キーワードの色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCCPPKEYWORD ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CCPPKEYWORDCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_CCPPKEYWORDBACKCOLOR:	/* 強調キーワード背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCCPPKEYWORDBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_CCPPKEYWORDBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//				case IDC_BUTTON_COMMENTCOLOR:	/* コメントの色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCOMMENT ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_COMMENTCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_COMMENTBACKCOLOR:	/* コメント背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorCOMMENTBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_COMMENTBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_SSTRINGCOLOR:	/* シングルクォーテーション文字列の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorSSTRING ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_SSTRINGCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_SSTRINGBACKCOLOR:	/* シングルクォーテーション文字列背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorSSTRINGBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_SSTRINGBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_WSTRINGCOLOR:	/* ダブルクォーテーション文字列の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorWSTRING ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_WSTRINGCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_WSTRINGBACKCOLOR:	/* ダブルクォーテーション文字列背景の色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorWSTRINGBACK ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_WSTRINGBACKCOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//				case IDC_BUTTON_UNDERLINECOLOR:	/* カーソル行アンダーラインの色 */
//					/* 色選択ダイアログ */
//					if( SelectColor( hwndDlg, &m_Types.m_colorUNDERLINE ) ){
//						::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_UNDERLINECOLOR ), NULL, TRUE );
//					}
//					return TRUE;
//
//	//			case IDC_BUTTON_IMPORT:	/* 色の設定をインポート */
//	//				p3_Import_Colors( hwndDlg );
//	//				return TRUE;
//
//	//			case IDC_BUTTON_EXPORT:	/* 色の設定をエクスポート */
//	//				p3_Export_Colors( hwndDlg );
//	//				return TRUE;
//				}
//			}
//			break;
//		case WM_NOTIFY:
//			idCtrl = (int)wParam;
//			pNMHDR = (NMHDR*)lParam;
//			pMNUD  = (NM_UPDOWN*)lParam;
//	//		switch( idCtrl ){
//	//		default:
//				switch( pNMHDR->code ){
//				case PSN_HELP:
//					OnHelp( hwndDlg, IDD_PROP1P3 );
//					return TRUE;
//				case PSN_KILLACTIVE:
//					MYTRACE( "p3 PSN_KILLACTIVE\n" );
//					/* ダイアログデータの取得 p3 */
//					GetData_p3( hwndDlg );
//					return TRUE;
//				}
//	//		}
//			break;
//		case WM_DRAWITEM:
//			idCtrl = (UINT) wParam;	/* コントロールのID */
//			pDis = (LPDRAWITEMSTRUCT) lParam;	/* 項目描画情報 */
//			switch( idCtrl ){
//
//			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
//				DrawColorButton( pDis, m_Types.m_colorTEXT );
//				return TRUE;
//			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
//				DrawColorButton( pDis, m_Types.m_colorBACK );
//				return TRUE;
//
//			case IDC_BUTTON_CRLFCOLOR:	/* 改行の色 */
//				DrawColorButton( pDis, m_Types.m_colorCRLF );
//				return TRUE;
//			case IDC_BUTTON_CRLFBACKCOLOR:	/* 改行背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorCRLFBACK );
//				return TRUE;
//
//			case IDC_BUTTON_GYOUCOLOR:	/* 行番号の色 */
//				DrawColorButton( pDis, m_Types.m_colorGYOU );
//				return TRUE;
//			case IDC_BUTTON_GYOUBACKCOLOR:	/* 行番号背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorGYOUBACK );
//				return TRUE;
//
//			case IDC_BUTTON_TABCOLOR:	/* TAB文字の色 */
//				DrawColorButton( pDis, m_Types.m_colorTAB );
//				return TRUE;
//			case IDC_BUTTON_TABBACKCOLOR:	/* TAB文字背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorTABBACK );
//				return TRUE;
//
//			case IDC_BUTTON_ZENCOLOR:	/* 全角スペース文字の色 */
//				DrawColorButton( pDis, m_Types.m_colorZENSPACE );
//				return TRUE;
//			case IDC_BUTTON_ZENBACKCOLOR:	/* 全角スペース文字背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorZENSPACEBACK );
//				return TRUE;
//
//			case IDC_BUTTON_EOFCOLOR:	/* EOFの色 */
//				DrawColorButton( pDis, m_Types.m_colorEOF );
//				return TRUE;
//			case IDC_BUTTON_EOFBACKCOLOR:	/* EOF背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorEOFBACK );
//				return TRUE;
//
//			case IDC_BUTTON_CCPPKEYWORDCOLOR:	/*  強調キーワードの色 */
//				DrawColorButton( pDis, m_Types.m_colorCCPPKEYWORD );
//				return TRUE;
//			case IDC_BUTTON_CCPPKEYWORDBACKCOLOR:	/*  強調キーワード背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorCCPPKEYWORDBACK );
//				return TRUE;
//
//			case IDC_BUTTON_COMMENTCOLOR:	/* コメントの色 */
//				DrawColorButton( pDis, m_Types.m_colorCOMMENT );
//				return TRUE;
//			case IDC_BUTTON_COMMENTBACKCOLOR:	/* コメント背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorCOMMENTBACK );
//				return TRUE;
//			case IDC_BUTTON_SSTRINGCOLOR:	/* シングルクォーテーション文字列の色 */
//				DrawColorButton( pDis, m_Types.m_colorSSTRING );
//				return TRUE;
//			case IDC_BUTTON_SSTRINGBACKCOLOR:	/* シングルクォーテーション文字列背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorSSTRINGBACK );
//				return TRUE;
//			case IDC_BUTTON_WSTRINGCOLOR:	/* ダブルクォーテーション文字列の色  */
//				DrawColorButton( pDis, m_Types.m_colorWSTRING );
//				return TRUE;
//			case IDC_BUTTON_WSTRINGBACKCOLOR:	/* ダブルクォーテーション文字列背景の色 */
//				DrawColorButton( pDis, m_Types.m_colorWSTRINGBACK );
//				return TRUE;
//			case IDC_BUTTON_UNDERLINECOLOR:	/* カーソル行アンダーラインの色 */
//				DrawColorButton( pDis, m_Types.m_colorUNDERLINE );
//				return TRUE;
//			}
//			break;
//		}
//		return FALSE;
//	}
//
//
//
//
//
//	/* ダイアログデータの設定 p3 */
//	void CPropTypes::SetData_p3( HWND hwndDlg )
//	{
//
//		HWND	hwndWork;
//		int		i;
//
//
//		/* 行番号を表示 */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DISPLINE, m_Types.m_bDispLINE );
//
//		/* タブ記号を表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DISPTAB, m_Types.m_bDispTAB );
//
//		/* 日本語空白を表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DISPZENSPACE, m_Types.m_bDispZENSPACE );
//
//		/* 改行記号を表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DISPCRLF, m_Types.m_bDispCRLF );
//
//		/* EOFを表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_DISPEOF, m_Types.m_bDispEOF );
//
//		/* 強調キーワードを表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCCPP, m_Types.m_bDispCCPPKEYWORD );
//
//		/* コメントを表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_COMMENT, m_Types.m_bDispCOMMENT );
//
//		/* シングルクォーテーション文字列を表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_SSTRING, m_Types.m_bDispSSTRING );
//
//		/* ダブルクォーテーション文字列を表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_WSTRING, m_Types.m_bDispWSTRING );
//
//		/* カーソル行アンダーラインを表示する */
//		::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, m_Types.m_bDispUNDERLINE );
//
//	//	/* キーワードの英大文字小文字区別 */
//	//	::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, m_Types.m_nKEYWORDCASE );
//
//
//
//
//		/* Grepモード: エンターキーでタグジャンプ */
//	//	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_bGTJW_RETURN );
//
//		/* Grepモード: ダブルクリックでタグジャンプ */
//	//	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_bGTJW_LDBLCLK );
//
//
//		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT ), EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szLineComment ) - 1 ), 0 );
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM ), EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentFrom ) - 1 ), 0 );
//		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO ), EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentTo ) - 1 ), 0 );
//
//		::SetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT, m_Types.m_szLineComment );				/* 行コメントデリミタ */
//		::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM, m_Types.m_szBlockCommentFrom );	/* ブロックコメントデリミタ(From) */
//		::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO, m_Types.m_szBlockCommentTo );		/* ブロックコメントデリミタ(To) */
//
//		if( 0 == m_Types.m_nStringType ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
//			::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, TRUE );
//			::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, FALSE );
//		}else{
//			::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, FALSE );
//			::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, TRUE );
//		}
//
//		/* セット名コンボボックスの値セット */
//		hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
//		::SendMessage( hwndWork, CB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
//		/* 一行目は空白 */
//		::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)" " );
//		if( 0 < m_CKeyWordSetMgr.m_nKeyWordSetNum ){
//			for( i = 0; i < m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
//				::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_CKeyWordSetMgr.GetTypeName( i ) );
//			}
//			if( -1 == m_Types.m_nKeyWordSetIdx ){
//				/* セット名コンボボックスのデフォルト選択 */
//				::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );
//			}else{
//				/* セット名コンボボックスのデフォルト選択 */
//				::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx + 1, 0 );
//			}
//		}
//
//
//		return;
//	}
//
//
//
//	/* ダイアログデータの取得 p3 */
//	int CPropTypes::GetData_p3( HWND hwndDlg )
//	{
//		int		nIdx;
//		HWND	hwndWork;
//		m_nPageNum = 1;
//
//		/* 行番号を表示 */
//		m_Types.m_bDispLINE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISPLINE );
//
//		/* タブ記号を表示する */
//		m_Types.m_bDispTAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISPTAB );
//
//		/* 日本語空白を表示する */
//		m_Types.m_bDispZENSPACE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISPZENSPACE );
//
//		/* 改行記号を表示する */
//		m_Types.m_bDispCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISPCRLF );
//
//		/* EOFを表示する */
//		m_Types.m_bDispEOF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISPEOF );
//
//		/* 強調キーワードを表示する */
//		m_Types.m_bDispCCPPKEYWORD = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCCPP );
//
//
//		/* コメントを表示する */
//		m_Types.m_bDispCOMMENT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COMMENT );
//
//		/* シングルクォーテーション文字列を表示する */
//		m_Types.m_bDispSSTRING = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SSTRING );
//
//		/* ダブルクォーテーション文字列を表示する */
//		m_Types.m_bDispWSTRING = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WSTRING );
//
//		/* カーソル行アンダーラインを表示する */
//		m_Types.m_bDispUNDERLINE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE );
//
//	//	/* キーワードの英大文字小文字区別 */
//	//	m_Types.m_nKEYWORDCASE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );
//
//
//
//
//		/* Grepモード: エンターキーでタグジャンプ */
//	//	m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );
//
//		/* Grepモード: ダブルクリックでタグジャンプ */
//	//	m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );
//
//		::GetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT, m_Types.m_szLineComment, sizeof( m_Types.m_szLineComment ) );					/* 行コメントデリミタ */
//		::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM, m_Types.m_szBlockCommentFrom, sizeof( m_Types.m_szBlockCommentFrom ) );	/* ブロックコメントデリミタ(From) */
//		::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO, m_Types.m_szBlockCommentTo, sizeof( m_Types.m_szBlockCommentTo ) );			/* ブロックコメントデリミタ(To) */
//
//		/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
//		if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_ESCAPETYPE_1 ) ){
//			m_Types.m_nStringType = 0;
//		}else{
//			m_Types.m_nStringType = 1;
//		}
//
//
//		/* セット名コンボボックスの値セット */
//		hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
//		nIdx = ::SendMessage( hwndWork, CB_GETCURSEL, 0, 0 );
//		if( CB_ERR == nIdx ||
//			0 == nIdx ){
//			m_Types.m_nKeyWordSetIdx = -1;
//		}else{
//			m_Types.m_nKeyWordSetIdx = nIdx - 1;
//
//		}
//
//		return TRUE;
//	}




/* 色の設定をインポート */
void CPropTypes::p3_Import_Colors( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
//	char			szLine[1024];
//	int				i;
	int				i;

	char			pHeader[1024];
	int				nColorInfoArrNum;						/* キー割り当て表の有効データ数 */
	ColorInfo		ColorInfoArr[64];
	char			szInitDir[_MAX_PATH + 1];
//	HWND			hwndCtrl;
	CProfile		cProfile;

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */

	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.col",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );


	/* 色設定Ver1か */
	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "ファイルを開けませんでした。\n\n%s", szPath );
		return;
	}
//	if( STR_COLORDATA_HEAD_LEN		== _lread( hFile, pHeader, STR_COLORDATA_HEAD_LEN )
//	 && sizeof( nColorInfoArrNum )	== _lread( hFile, &nColorInfoArrNum, sizeof( nColorInfoArrNum ) )
//	 && 0 == memcmp( pHeader, STR_COLORDATA_HEAD, STR_COLORDATA_HEAD_LEN )
//	){
//		int nWorkWork = sizeof( ColorInfoArr[0] );
//
//		for( i = 0; i < nColorInfoArrNum && i < m_Types.m_nColorInfoArrNum; ++i ){
//			if( sizeof( ColorInfoArr[i] ) != _lread( hFile,  &ColorInfoArr[i], sizeof( ColorInfoArr[i] ) ) ){
//				::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//					"色設定ファイルの形式が違います。\n\n%s", szPath
//				);
//				_lclose( hFile );
//				return;
//			}
//		}
//		_lclose( hFile );
//		goto complete;
//	}else{
		/* ファイル先頭 */
		_llseek( hFile, 0, FILE_BEGIN );
		char	szWork[256];
		int		nWorkLen;
//		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD2 );
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD21 );	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
#else
		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD3 );	//Jan. 15, 2001 Stonee
#endif
		nWorkLen = strlen( szWork );
		if( nWorkLen == (int)_lread( hFile, pHeader, nWorkLen ) &&
			0 == memcmp( pHeader, szWork, nWorkLen )
		){
		}else{
			::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//				"色設定ファイルの形式が違います。\n古い形式はサポートされなくなりました。\n%s", szPath
//				Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
//				Dec. 26, 2000 JEPRO UR1.2.24.0で強調キーワード2が入ってきたためCI[13]が追加された. それに伴い13番以降を1つづらした
//				"色設定ファイルの形式が違います。\n古い形式はサポートされなくなりました。\n%s\n\n"
				"色設定ファイルの形式が違います。古い形式はサポートされなくなりました。\n%s\n\n"	//Jan. 20, 2001 JEPRO 改行を1つ取った
#ifdef STR_COLORDATA_HEAD3
//				"色設定ファイルの変更内容はヘルプをご覧ください。"	//Jan. 15, 2001 Stonee added	//Jan. 20, 2001 JEPRO killed
// From Here Jan. 20, 2001 JEPRO 文字数オーバーのためコメントアウト！
//				"現在の色設定Ver3では CI[インデックス番号] から C[インデックス名] に仕様が変更されました (CI→C に注意)。\n"
//				"上記の色設定ファイルの設定内容を利用したい場合は、そのファイルをコピーしエディタで\n"
//				"以下の修正を行ってからインポートしてください。\n\n"
//				"・UR1.2.24.0 (2000/12/04) 以降で使っていた場合は\n"
//				"  (1) 一行目に書いてある Ver2 (or 2.1) を Ver3 と書き換え、CI をすべて C に縮める\n"
//				"  (2) (1)に加えて、インデックス番号を( )内の文字列に変更:\n"
//				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
//				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW2), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
//				"・ur3β10 (2000/09/28)～UR1.2.23.0 (2000/11/29) で使っていた場合は\n"
//				"  (3) (1)に加えて、インデックス番号を( )内の文字列に変更:\n"
//				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
//				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
//				"  (4) (1)に加えて、番号を( )内の文字列に変更:\n"
//				"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
//				"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
// To Here Jan. 20, 2001
// From Here Jan. 21, 2001 JEPRO
				"色設定Ver3では CI[番号] から C[名前] に変更されました。\n"
				"上記ファイルの設定内容を利用したい場合は、以下の修正を行ってからインポートしてください。\n\n"
				"・UR1.2.24.0 (00/12/04) 以降で使っていた場合は\n"
				"  (1) 一行目を Ver3 と書き換え、CI をすべて C に縮める\n"
				"  (2) (1)の後、番号を( )内の文字列に変更:\n"
				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW2), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
				"・ur3β10 (00/09/28)～UR1.2.23.0 (00/11/29) で使っていた場合は\n"
				"  (3) (1)の後、00-12 までは(2)と同じ  13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
				"・ur3β9 (00/09/26) 以前で使っていた場合は\n"
				"  (4) (1)の後、(2)と同様:\n"
				"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
				"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
// To Here Jan. 21, 2001
#else
// From Here Nov. 2, Dec. 26, 2000, Dec. 26, 2000 追加, Jan. 21, 2001 修正 JEPRO
				"現在の色設定Ver2.1ではVer2での仕様が一部変更されました。\n"
				"上記のファイルの設定内容を利用したい場合は、そのファイルをコピーしエディタで\n"
				"以下の修正を行ってからインポートしてください。\n\n"
				"・0.3.9.0 UR1.2.24.0 (2000/12/04) 以降で使っていた場合は\n"
				"  (1) 一行目に書かれている Ver2 を Ver2.1 と書き換える\n\n"
				"・0.3.9.0:ur3β10 (2000/09/28)～UR1.2.23.0 (2000/11/29) で使っていた場合は\n"
				"  (2) (1)に加えて CI[12] の次行に CI[12] の設定をコピーして CI[13] とし\n"
				"      元の CI[13] 以降の番号を1つづつ17までずらす\n\n"
				"・0.3.9.0:ur3β9 (2000/09/26) 以前で使っていた場合は\n"
				"  (3) (1)に加えて CI の[ ]内を順に\n"
				"      00, 17, 10, 01, 16, 03, 04, 12, 02, 11, 05, 15, 06, 07, 08, 09, 14\n"
				"      と書き換えた後、CI[12] の設定をコピーして CI[13] とする\n\n"
// To Here Nov. 2, Dec. 26, 2000, Jan. 21, 2001 JEPRO
#endif
				, szPath
			);
			_lclose( hFile );
			return;
		}
//	}
	_lclose( hFile );


	/* 色設定Ver2 */
	nColorInfoArrNum = COLORIDX_LAST;
	if( FALSE == cProfile.ReadProfile( szPath ) ){
		/* 設定ファイルが存在しない */
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", szPath
		);
		return;
	}
	/* 色設定 I/O */
	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
		strcpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
	}
	CShareData::IO_ColorSet( &cProfile, TRUE, STR_COLORDATA_SECTION, ColorInfoArr );


//complete:;
	/* データのコピー */
	m_Types.m_nColorInfoArrNum = nColorInfoArrNum;
	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		strcpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* ダイアログデータの設定 p5 */
	SetData_p3_new( hwndDlg );
	return;
}


/* 色の設定をエクスポート */
void CPropTypes::p3_Export_Colors( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
//	HFILE			hFile;
//	int				i;
	char			szInitDir[_MAX_PATH + 1];
//	char			szLine[1024];
//	int				i;
//	char			pHeader[STR_KEYDATA_HEAD_LEN + 1];
//	short			m_nColorInfoArrNum;				/* キー割り当て表の有効データ数 */
//	KEYDATA			pKeyNameArr[100];				/* キー割り当て表 */
	CProfile		cProfile;

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */

	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.col",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスをフォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

//	hFile = _lcreat( szPath, 0 );
//	if( HFILE_ERROR == hFile ){
//		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//			"ファイルを開けませんでした。\n\n%s", szPath
//		);
//		return;
//	}
//	if( STR_COLORDATA_HEAD_LEN					!= _lwrite( hFile, (LPCSTR)STR_COLORDATA_HEAD, STR_COLORDATA_HEAD_LEN )
//	 || sizeof( m_Types.m_nColorInfoArrNum )	!= _lwrite( hFile, (LPCSTR)&m_Types.m_nColorInfoArrNum, sizeof( m_Types.m_nColorInfoArrNum ) )
//	){
//		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//			"ファイルの書き込みに失敗しました。\n\n%s", szPath
//		);
//		return;
//	}
//	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
//		if( sizeof( m_Types.m_ColorInfoArr[i] ) != _lwrite( hFile, (LPCSTR)&m_Types.m_ColorInfoArr[i], sizeof( m_Types.m_ColorInfoArr[i] ) ) ){
//			::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//				"ファイルの書き込みに失敗しました。\n\n%s", szPath
//			);
//			return;
//		}
//	}
//	_lclose( hFile );

	/* 色設定 I/O */
	CShareData::IO_ColorSet( &cProfile, FALSE, STR_COLORDATA_SECTION, m_Types.m_ColorInfoArr );
//	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD2 );
#ifndef STR_COLORDATA_HEAD3
	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD21 );	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
#else
	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD3 );	//Jan. 15, 2001 Stonee
#endif
	return;
}


LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WPARAM		fwKeys;	// key flags
//	int			xPos;	// horizontal position of cursor
//	int			yPos;	// vertical position of cursor
//	HWND		hwndLoseFocus;
//	HWND		hwndGetFocus;
	int			fwKeys;
	int			xPos;
	int			yPos;
	int			nIndex;
	int			nItemNum;
	RECT		rcItem;
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch( uMsg ){
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		fwKeys = wParam;		// key flags
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor
//		nIndex = ::SendMessage( hwnd, LB_GETCURSEL, 0, 0 );
//		MYTRACE( "fwKeys=%d\n", fwKeys );
//		MYTRACE( "xPos  =%d\n", xPos );
//		MYTRACE( "yPos  =%d\n", yPos );
//		MYTRACE( "nIndex=%d\n", nIndex );
//		MYTRACE( "\n" );
		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = ::SendMessage( hwnd, LB_GETCOUNT, 0, 0 );
		nIndex = -1;
		for( i = 0; i < nItemNum; ++i ){
			::SendMessage( hwnd, LB_GETITEMRECT, i, (LPARAM)(RECT*)&rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE( "hit at i==%d\n", i );
//				MYTRACE( "\n" );
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
		::SendMessage( hwnd, LB_SETCURSEL, nIndex, 0 );
		::SendMessage( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* 下線 */
		if( nIndex != COLORIDX_UNDERLINE	/* カーソル行アンダーライン */
		 && nIndex != COLORIDX_TEXT			/* テキスト */
		 && nIndex != COLORIDX_RULER		/* ルーラー */
		){
			if( pColorInfo->m_bUnderLine ){	/* 下線で表示 */
				pColorInfo->m_bUnderLine = FALSE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, FALSE );
			}else{
				pColorInfo->m_bUnderLine = TRUE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, TRUE );
			}
			::InvalidateRect( hwnd, &rcItem, TRUE );
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* 太字で表示 */
		if( nIndex != COLORIDX_UNDERLINE	/* カーソル行アンダーライン */
		 && nIndex != COLORIDX_TEXT			/* テキスト */
		 && nIndex != COLORIDX_RULER		/* ルーラー */
		){
			if( pColorInfo->m_bFatFont ){	/* 太字で表示 */
				pColorInfo->m_bFatFont = FALSE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, FALSE );
			}else{
				pColorInfo->m_bFatFont = TRUE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, TRUE );
			}
			::InvalidateRect( hwnd, &rcItem, TRUE );
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* 色分け/表示 する */
		if( 2 <= xPos && xPos <= 16 && nIndex != COLORIDX_TEXT ){	/* テキスト */
			if( pColorInfo->m_bDisp ){	/* 色分け/表示する */
				pColorInfo->m_bDisp = FALSE;
			}else{
				pColorInfo->m_bDisp = TRUE;
			}
			if( COLORIDX_GYOU == nIndex ){
				pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12 ){
			/* 色選択ダイアログ */
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colTEXT ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
			}
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12 && nIndex != COLORIDX_UNDERLINE ){	/* カーソル行アンダーライン */
			/* 色選択ダイアログ */
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colBACK ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
			}
		}
		break;
	}
//	return CallWindowProc( (int (__stdcall *)(void))(WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
	return CallWindowProc( (WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* p3 メッセージ処理 */
BOOL CPropTypes::DispatchEvent_p3_new(
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
	int					i;

	switch( uMsg ){
	case WM_INITDIALOG:
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		hwndListColor = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );

		/* ダイアログデータの設定 p3 */
		SetData_p3_new( hwndDlg );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), EM_LIMITTEXT, (WPARAM)1, 0 );


		/* 色リストをフック */
		m_wpColorListProc = (WNDPROC) ::SetWindowLong( hwndListColor, GWL_WNDPROC, (LONG)ColorList_SubclassProc );

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		if( hwndListColor == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndListColor, LB_GETCURSEL, 0, 0 );
				m_nCurrentColorType = nIndex;		/* 現在選択されている色タイプ */
				switch( m_nCurrentColorType ){
				case COLORIDX_UNDERLINE:	/* カーソル行アンダーライン */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), FALSE );

					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), FALSE );


//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_HIDE );
					break;
				case COLORIDX_TEXT:	/* テキスト */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), FALSE );

					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), TRUE );

//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
					break;
//				case COLORIDX_CRLF:	/* CRLF */
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
//					break;
				default:
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), TRUE );

//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
					break;
				}


				/* 色分け/表示 をする */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, FALSE );
				}
				/* 太字で表示 */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_FAT, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_FAT, FALSE );
				}
				/* 下線を表示 */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, FALSE );
				}


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
				for( i = 0; i < COLORIDX_LAST; ++i ){
					if( i != m_nCurrentColorType ){
						m_Types.m_ColorInfoArr[i].m_colTEXT = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT;
					}
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	/* 背景色統一 */
				for( i = 0; i < COLORIDX_LAST; ++i ){
					if( i != m_nCurrentColorType ){
						m_Types.m_ColorInfoArr[i].m_colBACK = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK;
					}
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_DISP:	/* 色分け/表示 をする */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = FALSE;
				}
				/* 現在選択されている色タイプ */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;
			case IDC_CHECK_FAT:	/* 太字か */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FAT ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = FALSE;
				}
				/* 現在選択されている色タイプ */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* 下線を表示 */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = FALSE;
				}
				/* 現在選択されている色タイプ */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* 色の設定をインポート */
				p3_Import_Colors( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* 色の設定をエクスポート */
				p3_Export_Colors( hwndDlg );
				return TRUE;

			//	From Here Sept. 10, 2000 JEPRO
			//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
			case IDC_RADIO_LINETERMTYPE0: /* 行番号区切り 0=なし 1=縦線 2=任意 */
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
			//	From Here Jun. 6, 2001 genta
			//	行コメント開始桁指定のON/OFF
			case IDC_CHECK_LCPOS:
			case IDC_CHECK_LCPOS2:
			case IDC_CHECK_LCPOS3:
			//	To Here Jun. 6, 2001 genta
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			}
		}
		break;
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
//				MYTRACE( "p3 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p3 */
				GetData_p3_new( hwndDlg );
				return TRUE;
			}
		}
		break;
	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;				/* コントロールのID */
		pDis = (LPDRAWITEMSTRUCT) lParam;	/* 項目描画情報 */
		switch( idCtrl ){

		case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT );
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK );
			return TRUE;
		case IDC_LIST_COLORS:		/* 色種別リスト */
			DrawColorListItem( pDis );
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids2 );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids2 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* ダイアログデータの設定 p3 */
void CPropTypes::SetData_p3_new( HWND hwndDlg )
{

	HWND	hwndWork;
	int		i;
	int		nItem;

	m_nCurrentColorType = 0;	/* 現在選択されている色タイプ */

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT )		, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szLineComment ) - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT2 )		, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szLineComment2 ) - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT3 )		, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szLineComment3 ) - 1 ), 0 );	//Jun. 01, 2001 JEPRO 追加
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM )	, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentFrom ) - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO )	, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentTo ) - 1 ), 0 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 )	, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentFrom2 ) - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_szBlockCommentTo2 ) - 1 ), 0 );
//#endif

	::SetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT			, m_Types.m_szLineComment );		/* 行コメントデリミタ */
	::SetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT2		, m_Types.m_szLineComment2 );		/* 行コメントデリミタ2 */
	::SetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT3		, m_Types.m_szLineComment3 );		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_szBlockCommentFrom );	/* ブロックコメントデリミタ(From) */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, m_Types.m_szBlockCommentTo );		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_szBlockCommentFrom2 );	/* ブロックコメントデリミタ2(From) */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_szBlockCommentTo2 );	/* ブロックコメントデリミタ2(To) */
//#endif
	//	From Here May 12, 2001 genta
	//	行コメントの開始桁位置設定
	//	May 21, 2001 genta 桁位置を1から数えるように
	if( m_Types.m_nLineCommentPos >= 0 ){
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS, TRUE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, m_Types.m_nLineCommentPos + 1, FALSE );
	}
	else {
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS, FALSE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, (~m_Types.m_nLineCommentPos) + 1, FALSE );
	}

	if( m_Types.m_nLineCommentPos2 >= 0 ){
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS2, TRUE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, m_Types.m_nLineCommentPos2 + 1, FALSE );
	}
	else {
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS2, FALSE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, (~m_Types.m_nLineCommentPos2) + 1, FALSE );
	}

	//	To Here May 12, 2001 genta

	//	From Here Jun. 01, 2001 JEPRO 3つ目を追加
	if( m_Types.m_nLineCommentPos3 >= 0 ){
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS3, TRUE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, m_Types.m_nLineCommentPos3 + 1, FALSE );
	}
	else {
		::CheckDlgButton( hwndDlg, IDC_CHECK_LCPOS3, FALSE );
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, (~m_Types.m_nLineCommentPos3) + 1, FALSE );
	}
	//	To Here Jun. 01, 2001


	if( 0 == m_Types.m_nStringType ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, TRUE );
	}

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( FALSE == m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}


	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessage( hwndWork, CB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
	/* 一行目は空白 */
	::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)" " );
	if( 0 < m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		if( -1 == m_Types.m_nKeyWordSetIdx ){
			/* セット名コンボボックスのデフォルト選択 */
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );
		}else{
			/* セット名コンボボックスのデフォルト選択 */
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx + 1, 0 );
		}
	}

	//MIK START 2000.12.01 second keyword
	/* セット名コンボボックスの値セット */																		//MIK
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET2 );															//MIK
	::SendMessage( hwndWork, CB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */							//MIK
	/* 一行目は空白 */																							//MIK
	::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)" " );											//MIK
	if( 0 < m_CKeyWordSetMgr.m_nKeyWordSetNum ){																//MIK
		for( i = 0; i < m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){												//MIK
			::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_CKeyWordSetMgr.GetTypeName( i ) );	//MIK
		}																										//MIK
		if( -1 == m_Types.m_nKeyWordSetIdx2 ){																	//MIK
			/* セット名コンボボックスのデフォルト選択 */														//MIK
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );												//MIK
		}else{																									//MIK
			/* セット名コンボボックスのデフォルト選択 */														//MIK
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx2 + 1, 0 );					//MIK
		}																										//MIK
	}																											//MIK
	//MIK END

	/* 色をつける文字種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	::SendMessage( hwndWork, LB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
	for( i = 0; i < COLORIDX_LAST; ++i ){
		nItem = ::SendMessage( hwndWork, LB_ADDSTRING, 0, (LPARAM)(char*)m_Types.m_ColorInfoArr[i].m_szName );
		::SendMessage( hwndWork, LB_SETITEMDATA, nItem, (LPARAM)(void*)&m_Types.m_ColorInfoArr[i] );
	}
	/* 現在選択されている色タイプ */
	::SendMessage( hwndWork, LB_SETCURSEL, m_nCurrentColorType, 0 );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( 0 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 1 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 2 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE );
	}

	/* 行番号区切り文字 */
	char	szLineTermChar[2];
	wsprintf( szLineTermChar, "%c", m_Types.m_cLineTermChar );
	::SetDlgItemText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000

	return;
}





/* ダイアログデータの取得 p3 */
int CPropTypes::GetData_p3_new( HWND hwndDlg )
{
	int		nIdx;
	HWND	hwndWork;

	m_nPageNum = 1;


	::GetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT		, m_Types.m_szLineComment	, sizeof( m_Types.m_szLineComment ) );		/* 行コメントデリミタ */
	::GetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT2	, m_Types.m_szLineComment2	, sizeof( m_Types.m_szLineComment2 ) );		/* 行コメントデリミタ2 */
	::GetDlgItemText( hwndDlg, IDC_EDIT_LINECOMMENT3	, m_Types.m_szLineComment3	, sizeof( m_Types.m_szLineComment3 ) );		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	//	From Here May 12, 2001 genta
	//	コメントの開始桁位置の取得
	//	May 21, 2001 genta 桁位置を1から数えるように
	int pos;
	UINT en;
	BOOL bTranslated;

	en = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS );
	pos = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, &bTranslated, FALSE );
	if( bTranslated != TRUE ){
		en = 0;
		pos = 0;
	}
	//	pos == 0のときは無効扱い
	if( pos == 0 )	en = 0;
	else			--pos;
	//	無効のときは1の補数で格納
	m_Types.m_nLineCommentPos = en ? pos : ~pos;

	en = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS2 );
	pos = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, &bTranslated, FALSE );
	if( bTranslated != TRUE ){
		en = 0;
		pos = 0;
	}
	//	pos == 0のときは無効扱い
	if( pos == 0 )	en = 0;
	else			--pos;
	m_Types.m_nLineCommentPos2 = en ? pos : ~pos;

	//	To Here May 12, 2001 genta

	//	From Here Jun. 01, 2001 JEPRO 3つ目を追加
	en = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS3 );
	pos = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, &bTranslated, FALSE );
	if( bTranslated != TRUE ){
		en = 0;
		pos = 0;
	}
	//	pos == 0のときは無効扱い
	if( pos == 0 )	en = 0;
	else			--pos;
	m_Types.m_nLineCommentPos3 = en ? pos : ~pos;
	//	To Here Jun. 01, 2001

	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_szBlockCommentFrom	, sizeof( m_Types.m_szBlockCommentFrom ) );	/* ブロックコメントデリミタ(From) */
	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, m_Types.m_szBlockCommentTo	, sizeof( m_Types.m_szBlockCommentTo ) );	/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_szBlockCommentFrom2	, sizeof( m_Types.m_szBlockCommentFrom2 ) );/* ブロックコメントデリミタ2(From) */
	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_szBlockCommentTo2	, sizeof( m_Types.m_szBlockCommentTo2 ) );	/* ブロックコメントデリミタ2(To) */
//#endif

	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_ESCAPETYPE_1 ) ){
		m_Types.m_nStringType = 0;
	}else{
		m_Types.m_nStringType = 1;
	}
	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = FALSE;
	}else{
		m_Types.m_bLineNumIsCRLF = TRUE;
	}

	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	nIdx = ::SendMessage( hwndWork, CB_GETCURSEL, 0, 0 );
	if( CB_ERR == nIdx ||
		0 == nIdx ){
		m_Types.m_nKeyWordSetIdx = -1;
	}else{
		m_Types.m_nKeyWordSetIdx = nIdx - 1;

	}

	//MIK START 2000.12.01 second keyword
	/* セット名コンボボックスの値セット */					//MIK
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET2 );		//MIK
	nIdx = ::SendMessage( hwndWork, CB_GETCURSEL, 0, 0 );	//MIK
	if( CB_ERR == nIdx ||									//MIK
		0 == nIdx ){										//MIK
		m_Types.m_nKeyWordSetIdx2 = -1;						//MIK
	}else{													//MIK
		m_Types.m_nKeyWordSetIdx2 = nIdx - 1;				//MIK
	}														//MIK
	//MIK END

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE0 ) ){
		m_Types.m_nLineTermType = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE1 ) ){
		m_Types.m_nLineTermType = 1;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		m_Types.m_nLineTermType = 2;
	}


	/* 行番号区切り文字 */
	char	szLineTermChar[2];
//	sprintf( szLineTermChar, "%c", m_Types.m_cLineTermChar );
	::GetDlgItemText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];

	return TRUE;
}





/* 色種別リスト オーナー描画 */
void CPropTypes::DrawColorListItem( DRAWITEMSTRUCT* pDis )
{
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	ColorInfo*	pColorInfo;
	RECT		rc0,rc1,rc2;
	COLORREF	cRim = (COLORREF)::GetSysColor( COLOR_3DSHADOW );

	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
	rc2 = pDis->rcItem;

	/* アイテムデータの取得 */
	pColorInfo = (ColorInfo*)pDis->itemData;

	/* アイテム矩形塗りつぶし */
	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
	::DeleteObject( hBrush );


	/* アイテムが選択されている */
	if( pDis->itemState & ODS_SELECTED ){
		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
		::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
	}else{
		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
		::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
	}

//	/* 選択ハイライト矩形 */
//	::FillRect( pDis->hDC, &rc1, hBrush );
//	::DeleteObject( hBrush );
//	/* テキスト */
//	::SetBkMode( pDis->hDC, TRANSPARENT );
//	::TextOut( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
//	if( pColorInfo->m_bFatFont ){	/* 太字か */
//		::TextOut( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
//	}
//	return;


	rc1.left+= (2 + 16);
	rc1.top += 2;
	rc1.right -= ( 2 + 27 );
	rc1.bottom -= 2;
	/* 選択ハイライト矩形 */
	::FillRect( pDis->hDC, &rc1, hBrush );
	::DeleteObject( hBrush );
	/* テキスト */
	::SetBkMode( pDis->hDC, TRANSPARENT );
	::TextOut( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	if( pColorInfo->m_bFatFont ){	/* 太字か */
		::TextOut( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* 下線か */
		SIZE	sz;
		::GetTextExtentPoint32( pDis->hDC, pColorInfo->m_szName, strlen( pColorInfo->m_szName ), &sz );
		::MoveToEx( pDis->hDC, rc1.left,		rc1.bottom - 2, NULL );
		::LineTo( pDis->hDC, rc1.left + sz.cx,	rc1.bottom - 2 );
		::MoveToEx( pDis->hDC, rc1.left,		rc1.bottom - 1, NULL );
		::LineTo( pDis->hDC, rc1.left + sz.cx,	rc1.bottom - 1 );
	}

	/* アイテムのフォーカスが変化した */
	if( pDis->itemAction & ODA_FOCUS ){
		/* アイテムにフォーカスがある */
		if( pDis->itemState & ODS_FOCUS ){
		}else{
			::DrawFocusRect( pDis->hDC, &pDis->rcItem );
		}
	}

	/* 「色分け/表示する」のチェック */
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if( pColorInfo->m_bDisp ){	/* 色分け/表示する */
		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );
	}
//	return;



	if( 0 != strcmp( "カーソル行アンダーライン", pColorInfo->m_szName ) ){
		/* 背景色 見本矩形 */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		hBrush = ::CreateSolidBrush( pColorInfo->m_colBACK );
		hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
		hPen = ::CreatePen( PS_SOLID, 1, cRim );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
		::SelectObject( pDis->hDC, hPenOld );
		::SelectObject( pDis->hDC, hBrushOld );
		::DeleteObject( hPen );
		::DeleteObject( hBrush );
	}


	/* 前景色 見本矩形 */
	rc1 = pDis->rcItem;
	rc1.left = rc1.right - 27;
	rc1.top += 2;
	rc1.right = rc1.left + 12;
	rc1.bottom -= 2;
	hBrush = ::CreateSolidBrush( pColorInfo->m_colTEXT );
	hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
	hPen = ::CreatePen( PS_SOLID, 1, cRim );
	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
	::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	::SelectObject( pDis->hDC, hPenOld );
	::SelectObject( pDis->hDC, hBrushOld );
	::DeleteObject( hPen );
	::DeleteObject( hBrush );





//	::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
//	::SetBkColor( pDis->hDC, ::GetSysColor( COLOR_WINDOW ) );
//	::TextOut( pDis->hDC, pDis->rcItem.left + 3, pDis->rcItem.top + 3, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );

//	::SetTextColor( pDis->hDC, pColorInfo->m_colTEXT );
//	::SetBkColor( pDis->hDC, pColorInfo->m_colBACK );
//	::TextOut( pDis->hDC, pDis->rcItem.left + 3 + 128, pDis->rcItem.top + 3, gpColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	return;
}


/* ヘルプ */
//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
void CPropTypes::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROPTYPESP1:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);
		break;
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//	case IDD_PROP1P3:
	case IDD_PROP_COLOR:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);
		break;
//	Jul. 03, 2001 JEPRO 支援タブのヘルプを有効化
		case IDD_PROPTYPESP2:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);
		break;
//@@@ 2001.11.17 add start MIK
	case IDD_PROP_REGEX:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);
		break;
//@@@ 2001.11.17 add end MIK
	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		::WinHelp( hwndParent, m_szHelpFile, HELP_CONTEXT, nContextID );
	}
	return;
}





//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropTypes::EnableTypesPropInput( HWND hwndDlg )
{
	//	行番号区切りを任意の半角文字にするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), FALSE );
	}

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



/*[EOF]*/
