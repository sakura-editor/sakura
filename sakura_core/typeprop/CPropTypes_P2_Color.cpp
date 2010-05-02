/*
	タイプ別設定 - カラー

	2008.04.12 kobake CPropTypes.cppから分離
	2009.02.22 ryoji
*/
#include "StdAfx.h"
#include "CPropTypes.h"
#include "sakura.hh"
//#include "io/CTextStream.h"
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "util/shell.h"
#include "view/colors/CColorStrategy.h"
//#include "env/CShareData_IO.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi

using namespace std;

WNDPROC	m_wpColorListProc;

static const DWORD p_helpids2[] = {	//11400
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			//文字色
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			//背景色
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		//文字色統一
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		//背景色統一
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		//エクスポート
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				//色分け表示
	IDC_CHECK_FAT,					HIDC_CHECK_FAT,					//太字
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			//下線
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				//桁指定１
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				//桁指定２
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			//強調キーワード１セット名
//	IDC_COMBO_SET2,					HIDC_COMBO_SET2_COLOR,			//強調キーワード２セット名	del 2008/6/3 Uchi
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	//ブロックコメント１開始
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		//ブロックコメント１終了
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			//行コメント１
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			//行コメント２
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		//桁数１
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		//桁数２
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			//行番号区切り
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	//ブロックコメント２開始
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		//ブロックコメント２終了
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			//行コメント３
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				//色指定
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				//桁指定３
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		//桁数３
	IDC_RADIO_ESCAPETYPE_1,			HIDC_RADIO_ESCAPETYPE_1,		//文字列エスケープ（C言語風）
	IDC_RADIO_ESCAPETYPE_2,			HIDC_RADIO_ESCAPETYPE_2,		//文字列エスケープ（PL/SQL風）
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		//行番号の表示（折り返し単位）
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		//行番号の表示（改行単位）
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		//行番号区切り（なし）
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		//行番号区切り（縦線）
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		//行番号区切り（任意）
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		//強調キーワード2～10	// 2006.08.06 ryoji
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				//縦線の桁指定	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
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
void CPropTypes::Import_Colors( HWND hwndDlg )
{
//	/* ファイルオープンダイアログの初期化 */
//	CDlgOpenFile	cDlgOpenFile;
//	cDlgOpenFile.Create(
//		m_hInstance,
//		hwndDlg,
//		_T("*.col"),
//		GetDllShareData().m_sHistory.m_szIMPORTFOLDER, // インポート用フォルダ
//		std::vector<LPCTSTR>(),
//		std::vector<LPCTSTR>()
//	);
//	TCHAR	szPath[_MAX_PATH + 1] = _T("");
//	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//		return;
//	}
//
//	/* ファイルのフルパスを、フォルダとファイル名に分割 */
//	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
//	_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
//
//
//	/* 色設定Ver1か */
//	CTextInputStream in(szPath);
//	if(!in){
//		ErrorMessage(hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
//		return;
//	}
//
//	/* ファイル先頭 */
//	//ヘッダ読取
//	wstring szHeader = in.ReadLineW().c_str();
//	//if(szHeader.length()>=2)szHeader=&szHeader.c_str()[2]; //コメントを抜く
//	if(szHeader.length()>=2) {
//		//コメントを抜く	コメント文字 変更対応 2008/6/28 Uchi
//		szHeader = &szHeader.c_str()[ szHeader.c_str()[0] == _T(';') ? 1 : 2];
//	}
//	//比較
//	if(wcscmp(szHeader.c_str(),LTEXT(STR_COLORDATA_HEAD3))==0){
//		//OK
//	}
//	else{
//		ErrorMessage_A( hwndDlg,
//			"色設定ファイルの形式が違います。古い形式はサポートされなくなりました。\n%ts\n\n"	//Jan. 20, 2001 JEPRO 改行を1つ取った
//			"色設定Ver3では CI[番号] から C[名前] に変更されました。\n"
//			"上記ファイルの設定内容を利用したい場合は、以下の修正を行ってからインポートしてください。\n\n"
//			"・UR1.2.24.0 (00/12/04) 以降で使っていた場合は\n"
//			"  (1) 一行目を Ver3 と書き換え、CI をすべて C に縮める\n"
//			"  (2) (1)の後、番号を( )内の文字列に変更:\n"
//			"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
//			"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
//			"・ur3β10 (00/09/28)～UR1.2.23.0 (00/11/29) で使っていた場合は\n"
//			"  (3) (1)の後、00-12 までは(2)と同じ  13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
//			"・ur3β9 (00/09/26) 以前で使っていた場合は\n"
//			"  (4) (1)の後、(2)と同様:\n"
//			"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
//			"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
//			, szPath
//		);
//		in.Close();
//		return;
//	}
//	in.Close();
//
//
//	CDataProfile	cProfile;
//	cProfile.SetReadingMode();
//
//	int				nColorInfoArrNum;						/* キー割り当て表の有効データ数 */
//	ColorInfo		ColorInfoArr[64];
//
//	/* 色設定Ver2 */
//	nColorInfoArrNum = COLORIDX_LAST;
//	if( !cProfile.ReadProfile( szPath ) ){
//		/* 設定ファイルが存在しない */
//		ErrorMessage( hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
//		return;
//	}
//	/* 色設定Ver2 */
//	nColorInfoArrNum = COLORIDX_LAST;
//	if( !cProfile.ReadProfile( szPath ) ){
//		/* 設定ファイルが存在しない */
//		ErrorMessage( hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
//		return;
//	}
//	/* 色設定 I/O */
//	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
//		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
//		_tcscpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
//	}
//	CShareData_IO::IO_ColorSet( &cProfile, LTEXT(STR_COLORDATA_SECTION), ColorInfoArr );
//
//
////complete:;
//	/* データのコピー */
//	m_Types.m_nColorInfoArrNum = nColorInfoArrNum;
//	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
//		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
//		_tcscpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
//	}
//	/* ダイアログデータの設定 p5 */
//	SetData_Color( hwndDlg );

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
		return;
	}

	/* データのコピー */
	m_Types.m_nColorInfoArrNum = COLORIDX_LAST;
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		_tcscpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* ダイアログデータの設定 color */
	SetData_Color( hwndDlg );
}


/* 色の設定をエクスポート */
// 2010/4/23 Uchi Exportの外出し
void CPropTypes::Export_Colors( HWND hwndDlg )
{
//	/* ファイルオープンダイアログの初期化 */
//	CDlgOpenFile	cDlgOpenFile;
//	TCHAR			szPath[_MAX_PATH + 1] = _T("");
//	cDlgOpenFile.Create(
//		m_hInstance,
//		hwndDlg,
//		_T("*.col"),
//		GetDllShareData().m_sHistory.m_szIMPORTFOLDER, // インポート用フォルダ
//		std::vector<LPCTSTR>(),
//		std::vector<LPCTSTR>()
//	);
//	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
//		return;
//	}
//
//	/* ファイルのフルパスをフォルダとファイル名に分割 */
//	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
//	_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
//
//	/* 色設定 I/O */
//	CDataProfile	cProfile;
//	cProfile.SetWritingMode();
//	CShareData_IO::IO_ColorSet( &cProfile, LTEXT(STR_COLORDATA_SECTION), m_Types.m_ColorInfoArr );
//	cProfile.WriteProfile( szPath, LTEXT(STR_COLORDATA_HEAD3) );	//Jan. 15, 2001 Stonee

	CImpExpColors	cImpExpColors( m_Types.m_ColorInfoArr);

	// エクスポート
	cImpExpColors.ExportUI(m_hInstance, hwndDlg);
}



LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
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
//		fwKeys = wParam;		// key flags
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = ::SendMessageAny( hwnd, LB_GETCOUNT, 0, 0 );
		nIndex = -1;
		for( i = 0; i < nItemNum; ++i ){
			::SendMessageAny( hwnd, LB_GETITEMRECT, i, (LPARAM)&rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE_A( "hit at i==%d\n", i );
//				MYTRACE_A( "\n" );
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
			::SendMessageAny( hwnd, LB_SETCURSEL, nIndex, 0 );
			::SendMessageCmd( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
			pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
			/* 下線 */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )	// 2006.12.18 ryoji フラグ利用で簡素化
			{
				if( pColorInfo->m_bUnderLine ){	/* 下線で表示 */
					pColorInfo->m_bUnderLine = false;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, FALSE );
				}else{
					pColorInfo->m_bUnderLine = true;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, TRUE );
				}
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji 有効範囲の制限追加
			pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
			/* 太字で表示 */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) )	// 2006.12.18 ryoji フラグ利用で簡素化
			{
				if( pColorInfo->m_bFatFont ){	/* 太字で表示 */
					pColorInfo->m_bFatFont = false;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, FALSE );
				}else{
					pColorInfo->m_bFatFont = true;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, TRUE );
				}
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
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
				pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12 ){
			/* 色選択ダイアログ */
			// 2005.11.30 Moca カスタム色保持
			DWORD* pColors = (DWORD*)::GetProp( hwnd, _T("ptrCustomColors") );
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colTEXT, pColors ) ){
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
			DWORD* pColors = (DWORD*)::GetProp( hwnd, _T("ptrCustomColors") );
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colBACK, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
			}
		}
		break;
	// 2005.11.30 Moca カスタム色保持
	case WM_DESTROY:
		if( ::GetProp( hwnd, _T("ptrCustomColors") ) ){
			::RemoveProp( hwnd, _T("ptrCustomColors") );
		}
		break;
	}
	return CallWindowProc( (WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* color メッセージ処理 */
INT_PTR CPropTypes::DispatchEvent_Color(
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
		SetData_Color( hwndDlg );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), EM_LIMITTEXT, (WPARAM)1, 0 );


		/* 色リストをフック */
		// Modified by KEITA for WIN64 2003.9.6
		m_wpColorListProc = (WNDPROC) ::SetWindowLongPtr( hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );
		// 2005.11.30 Moca カスタム色を保持
		::SetProp( hwndListColor, _T("ptrCustomColors"), m_dwCustColors );
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		if( hwndListColor == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessageAny( hwndListColor, LB_GETCURSEL, 0, 0 );
				m_nCurrentColorType = nIndex;		/* 現在選択されている色タイプ */

				{
					// 各種コントロールの有効／無効を切り替える	// 2006.12.18 ryoji フラグ利用で簡素化
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),			(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ),				(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),		(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );

					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),				(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ),				(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),			(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	TRUE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
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
				{
					// 2006.04.26 ryoji 文字色／背景色統一ダイアログを使う
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	/* 背景色統一 */
				{
					// 2006.04.26 ryoji 文字色／背景色統一ダイアログを使う
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
				/* 色選択ダイアログ */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* 現在選択されている色タイプ */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_DISP:	/* 色分け/表示 をする */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = false;
				}
				/* 現在選択されている色タイプ */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;
			case IDC_CHECK_FAT:	/* 太字か */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FAT ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = false;
				}
				/* 現在選択されている色タイプ */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* 下線を表示 */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = false;
				}
				/* 現在選択されている色タイプ */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* 色の設定をインポート */
				Import_Colors( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* 色の設定をエクスポート */
				Export_Colors( hwndDlg );
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

			//強調キーワードの選択
			case IDC_BUTTON_KEYWORD_SELECT:
				{
					CDlgKeywordSelect cDlgKeywordSelect;
					//強調キーワード1を取得する。
					HWND hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
					int nIdx = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx || 0 == nIdx ){
						m_nSet[ 0 ] = -1;
					}else{
						m_nSet[ 0 ] = nIdx - 1;
					}
					cDlgKeywordSelect.DoModal( ::GetModuleHandle(NULL), hwndDlg, m_nSet );
					RearrangeKeywordSet( hwndDlg );	//	Jan. 23, 2005 genta キーワードセット再配置
					//強調キーワード1を反映する。
					if( -1 == m_nSet[ 0 ] ){
						::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)0, 0 );
					}else{
						::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)m_nSet[ 0 ] + 1, 0 );
					}
				}
				break;
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
//				MYTRACE_A( "color PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 color */
				GetData_Color( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = 1;
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
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT );
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK );
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* ダイアログデータの設定 color */
void CPropTypes::SetData_Color( HWND hwndDlg )
{

	HWND	hwndWork;
	int		i;
	int		nItem;

	m_nCurrentColorType = 0;	/* 現在選択されている色タイプ */

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */	//@@@ 2002.09.22 YAZAKI
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT2 )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT3 )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );	//Jun. 01, 2001 JEPRO 追加
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 ), EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
//#endif

	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_cBlockComments[0].getBlockCommentFrom() );	/* ブロックコメントデリミタ(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, m_Types.m_cBlockComments[0].getBlockCommentTo() );	/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_cBlockComments[1].getBlockCommentFrom() );	/* ブロックコメントデリミタ2(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_cBlockComments[1].getBlockCommentTo() );	/* ブロックコメントデリミタ2(To) */
//#endif

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

	if( 0 == m_Types.m_nStringType ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, TRUE );
	}

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( !m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}


	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessageAny( hwndWork, CB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
	/* 一行目は空白 */
	Combo_AddString( hwndWork, L" " );
	//	Mar. 31, 2003 genta KeyWordSetMgrをポインタに
	if( 0 < m_pCKeyWordSetMgr->m_nKeyWordSetNum ){
		for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_pCKeyWordSetMgr->GetTypeName( i ) );
		}
		if( -1 == m_Types.m_nKeyWordSetIdx[0] ){
			/* セット名コンボボックスのデフォルト選択 */
			::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );
		}else{
			/* セット名コンボボックスのデフォルト選択 */
			::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx[0] + 1, 0 );
		}
	}

	//強調キーワード1～10の設定
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		m_nSet[ i ] = m_Types.m_nKeyWordSetIdx[i];
	}

	/* 色をつける文字種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	::SendMessageAny( hwndWork, LB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
	for( i = 0; i < COLORIDX_LAST; ++i ){
		nItem = ::List_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
		::SendMessageAny( hwndWork, LB_SETITEMDATA, nItem, (LPARAM)&m_Types.m_ColorInfoArr[i] );
	}
	/* 現在選択されている色タイプ */
	::SendMessageAny( hwndWork, LB_SETCURSEL, m_nCurrentColorType, 0 );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

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
	wchar_t	szLineTermChar[2];
	auto_sprintf( szLineTermChar, L"%lc", m_Types.m_cLineTermChar );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000


	// from here 2005.11.30 Moca 指定位置縦線の設定
	WCHAR szVertLine[MAX_VERTLINES * 15] = L"";
	int offset = 0;
	for( i = 0; i < MAX_VERTLINES && m_Types.m_nVertLineIdx[i] != 0; i++ ){
		CLayoutInt nXCol = m_Types.m_nVertLineIdx[i];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
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
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_VERTLINE ), EM_LIMITTEXT, (WPARAM)(MAX_VERTLINES * 15), 0 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine );
	// to here 2005.11.30 Moca 指定位置縦線の設定
	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* ダイアログデータの取得 color */
int CPropTypes::GetData_Color( HWND hwndDlg )
{
	int		nIdx;
	HWND	hwndWork;

//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 1;

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

	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_ESCAPETYPE_1 ) ){
		m_Types.m_nStringType = 0;
	}else{
		m_Types.m_nStringType = 1;
	}
	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = false;
	}else{
		m_Types.m_bLineNumIsCRLF = true;
	}

	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	nIdx = ::SendMessageAny( hwndWork, CB_GETCURSEL, 0, 0 );
	if( CB_ERR == nIdx ||
		0 == nIdx ){
		m_Types.m_nKeyWordSetIdx[0] = -1;
	}else{
		m_Types.m_nKeyWordSetIdx[0] = nIdx - 1;

	}

	//強調キーワード2～10の取得(1は別)
	for( nIdx = 1; nIdx < MAX_KEYWORDSET_PER_TYPE; nIdx++ ){
		m_Types.m_nKeyWordSetIdx[nIdx] = m_nSet[nIdx];
	}

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
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];


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
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(-valueBegin);
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(valueEnd);
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(value);
			}
			else{
				break;
			}
		}
		else{
			m_Types.m_nVertLineIdx[i++] = CLayoutInt(value);
		}
		if( szVertLine[offset] != ',' ){
			break;
		}
		offset++;
	}
	if( i < MAX_VERTLINES ){
		m_Types.m_nVertLineIdx[i] = CLayoutInt(0);
	}
	// to here 2005.11.30 Moca 指定位置縦線の設定
	return TRUE;
}



/* 色ボタンの描画 */
void CPropTypes::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE_A( "pDis->itemAction = " );

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
	rcFocus = rc = pDis->rcItem;
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
void CPropTypes::RearrangeKeywordSet( HWND hwndDlg )
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
				ColorInfo colT;
				ColorInfo &col1 = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ];
				ColorInfo &col2   = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + j ];

				colT.m_bDisp		= col1.m_bDisp;
				colT.m_bFatFont		= col1.m_bFatFont;
				colT.m_bUnderLine	= col1.m_bUnderLine;
				colT.m_colTEXT		= col1.m_colTEXT;
				colT.m_colBACK		= col1.m_colBACK;

				col1.m_bDisp		= col2.m_bDisp;
				col1.m_bFatFont		= col2.m_bFatFont;
				col1.m_bUnderLine	= col2.m_bUnderLine;
				col1.m_colTEXT		= col2.m_colTEXT;
				col1.m_colBACK		= col2.m_colBACK;

				col2.m_bDisp		= colT.m_bDisp;
				col2.m_bFatFont		= colT.m_bFatFont;
				col2.m_bUnderLine	= colT.m_bUnderLine;
				col2.m_colTEXT		= colT.m_colTEXT;
				col2.m_colBACK		= colT.m_colBACK;
				
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
void CPropTypes::DrawColorListItem( DRAWITEMSTRUCT* pDis )
{
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor( COLOR_3DSHADOW );

	if( pDis == NULL || pDis->itemData == NULL ) return;

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
	if( pColorInfo->m_bFatFont ){	/* 太字か */
		::TextOut( gr, rc1.left + 1, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* 下線か */
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

		gr.SetBrushColor( pColorInfo->m_colBACK );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}


	/* 前景色 見本矩形 */
	rc1 = pDis->rcItem;
	rc1.left = rc1.right - 27;
	rc1.top += 2;
	rc1.right = rc1.left + 12;
	rc1.bottom -= 2;
	gr.SetBrushColor( pColorInfo->m_colTEXT );
	gr.SetPen( cRim );
	::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );

}



/* 色選択ダイアログ */
BOOL CPropTypes::SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors )
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof_raw( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( !::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}

