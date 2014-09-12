/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca
	Copyright (C) 2008, nasukoji, ryoji, novice

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <cderr.h>
#include <dlgs.h>
#include "global.h"
#include "CDlgOpenFile.h"
#include "Funccode.h"	//Stonee, 2001/05/18
#include "Debug.h"
#include "CFileExt.h"
#include "CEditApp.h"
#include "shell.h"
#include "file.h"
#include "os.h"
#include "module.h"
#include "my_icmp.h"
#include "sakura_rc.h"
#include "sakura.hh"

// オープンファイル CDlgOpenFile.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDCANCEL,				HIDCANCEL_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		//ヘルプボタン
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		//文字コードセット
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			//最近のファイル
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	//最近のフォルダ
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			//改行コード
	IDC_CHECK_BOM,			HIDC_OPENDLG_CHECK_BOM,			//BOM	// 2006.08.06 ryoji
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

// 2005.10.29 ryoji
// Windows 2000 version of OPENFILENAME.
// The new version has three extra members.
// See commdlg.h
#if (_WIN32_WINNT >= 0x0500)
struct OPENFILENAMEZ : public OPENFILENAME {
};
#else
struct OPENFILENAMEZ : public OPENFILENAME {
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
};
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif
#endif // (_WIN32_WINNT >= 0x0500)

#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

// 2014.05.22 Moca FileDialogの再入サポート
class CDlgOpenFileMem{
public:
	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */

	DLLSHAREDATA*	m_pShareData;

	TCHAR			m_szDefaultWildCard[_MAX_PATH + 1];	/* 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */
	TCHAR			m_szInitialDir[_MAX_PATH + 1];		/* 「開く」での初期ディレクトリ */

	std::vector<LPCTSTR>	m_vMRU;
	std::vector<LPCTSTR>	m_vOPENFOLDER;
};

class CDlgOpenFileData{
public:
	CDlgOpenFile*	m_pcDlgOpenFile;

	WNDPROC			m_wpOpenDialogProc;
	int				m_nHelpTopicID;
	bool			m_bReadOnly;		// 読み取り専用か
	BOOL			m_bIsSaveDialog;	// 保存のダイアログか
	ECodeType		m_nCharCode;		// 文字コード

	CEol			m_cEol;
	bool			m_bUseEol;
	bool			m_bBom;		//!< BOMを付けるかどうか	//	Jul. 26, 2003 ryoji BOM
	bool			m_bUseBom;	//!< BOMの有無を選択する機能を利用するかどうか
	TCHAR			m_szPath[_MAX_PATH];	// 拡張子の補完を自前で行ったときのファイルパス	// 2006.11.10 ryoji

	OPENFILENAME*	m_pOf;
	OPENFILENAMEZ	m_ofn;		/* 2005.10.29 ryoji OPENFILENAMEZ「ファイルを開く」ダイアログ用構造体 */
	HWND			m_hwndOpenDlg;
	HWND			m_hwndComboMRU;
	HWND			m_hwndComboOPENFOLDER;
	HWND			m_hwndComboCODES;
	HWND			m_hwndComboEOL;	//	Feb. 9, 2001 genta
	HWND			m_hwndCheckBOM;	//	Jul. 26, 2003 ryoji BOMチェックボックス

	CDlgOpenFileData():
		m_pcDlgOpenFile(NULL)
		,m_nHelpTopicID(0)
	{}
};

static const TCHAR* s_pszOpenFileDataName = _T("FileOpenData");



/*
|| 	開くダイアログのサブクラスプロシージャ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
LRESULT APIENTRY OFNHookProcMain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	OFNOTIFY*				pofn;
	CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetProp( hwnd, s_pszOpenFileDataName );
	WORD					wNotifyCode;
	WORD					wID;
	static DLLSHAREDATA*	pShareData;
	switch( uMsg ){
	case WM_MOVE:
		/* 「開く」ダイアログのサイズと位置 */
		pShareData = CShareData::getInstance()->GetShareData();
		::GetWindowRect( hwnd, &pShareData->m_Common.m_sOthers.m_rcOpenDialog );
//		MYTRACE( _T("WM_MOVE 1\n") );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		switch( wNotifyCode ){
//			break;
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case pshHelp:
				/* ヘルプ */
				MyWinHelp( hwnd, HELP_CONTEXT, pData->m_nHelpTopicID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
				break;
			case chx1:	// The read-only check box
				pData->m_bReadOnly = ( 0 != ::IsDlgButtonChecked( hwnd , chx1 ) );
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
//		pofn = (OFNOTIFY*) lParam;
//		MYTRACE( _T("=========WM_NOTIFY=========\n") );
//		MYTRACE( _T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom );
//		MYTRACE( _T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( _T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code );
		break;
	}
//	return ::CallWindowProc( (int (__stdcall *)( void ))(WNDPROC)m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );

	return ::CallWindowProc( pData->m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );
}




/*!
	開くダイアログのフックプロシージャ
*/
// Modified by KEITA for WIN64 2003.9.6
// APIENTRY -> CALLBACK Moca 2003.09.09
//UINT APIENTRY OFNHookProc(
UINT_PTR CALLBACK OFNHookProc(
	HWND hdlg,		// handle to child dialog window
	UINT uiMsg,		// message identifier
	WPARAM wParam,	// message parameter
	LPARAM lParam 	// message parameter
)
{
	POINT					po;
	RECT					rc;
	int						i;
	OFNOTIFY*				pofn;
	LRESULT					lRes;
	WORD					wNotifyCode;
	WORD					wID;
	HWND					hwndCtl;
	HWND					hwndFilebox;	// 2005.11.02 ryoji
	int						nIdx;
	int						nIdxSel;
	int						nWidth;
	WPARAM					fCheck;	//	Jul. 26, 2003 ryoji BOM状態用

	//	From Here	Feb. 9, 2001 genta
	static const int		nEolValueArr[] = {
		EOL_NONE,
		EOL_CRLF,
		EOL_LF,
		EOL_CR,
	};
	//	文字列はResource内に入れる
	static const TCHAR*	const	pEolNameArr[] = {
		_T("変換なし"),
		_T("CR+LF"),
		_T("LF (UNIX)"),
		_T("CR (Mac)"),
	};
	int nEolNameArrNum = (int)_countof(pEolNameArr);

//	To Here	Feb. 9, 2001 genta
	int	nRightMargin = 24;
	HWND	hwndFrame;

	switch( uiMsg ){
	case WM_MOVE:
//		MYTRACE( _T("WM_MOVE 2\n") );
		break;
	case WM_SIZE:
		{
			nWidth = LOWORD(lParam);	// width of client area

			/* 「開く」ダイアログのサイズと位置 */
			CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			hwndFrame = ::GetParent( hdlg );
			::GetWindowRect( hwndFrame, &pData->m_pcDlgOpenFile->m_mem->m_pShareData->m_Common.m_sOthers.m_rcOpenDialog );

			// 2005.10.29 ryoji 最近のファイル／フォルダ コンボの右端を子ダイアログの右端に合わせる
			::GetWindowRect( pData->m_hwndComboMRU, &rc );
			po.x = rc.left;
			po.y = rc.top;
			::ScreenToClient( hdlg, &po );
			::SetWindowPos( pData->m_hwndComboMRU, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			::SetWindowPos( pData->m_hwndComboOPENFOLDER, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
			return 0;
		}
	case WM_INITDIALOG:
		{
			// Save off the long pointer to the OPENFILENAME structure.
			// Modified by KEITA for WIN64 2003.9.6
			OPENFILENAME* pOfn = (OPENFILENAME*)lParam;
			CDlgOpenFileData* pData = (CDlgOpenFileData*)(pOfn->lCustData);
			::SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pData);
			pData->m_pOf = pOfn;

			/* Explorerスタイルの「開く」ダイアログのハンドル */
			pData->m_hwndOpenDlg = ::GetParent( hdlg );
			/* コントロールのハンドル */
			pData->m_hwndComboCODES = ::GetDlgItem( hdlg, IDC_COMBO_CODE );
			pData->m_hwndComboMRU = ::GetDlgItem( hdlg, IDC_COMBO_MRU );
			pData->m_hwndComboOPENFOLDER = ::GetDlgItem( hdlg, IDC_COMBO_OPENFOLDER );
			pData->m_hwndComboEOL = ::GetDlgItem( hdlg, IDC_COMBO_EOL );
			pData->m_hwndCheckBOM = ::GetDlgItem( hdlg, IDC_CHECK_BOM );//	Jul. 26, 2003 ryoji BOMチェックボックス

			// 2005.11.02 ryoji 初期レイアウト設定
			CDlgOpenFile::InitLayout( pData->m_hwndOpenDlg, hdlg, pData->m_hwndComboCODES );

			/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
			::SendMessage( pData->m_hwndComboCODES, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
			::SendMessage( pData->m_hwndComboMRU, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
			::SendMessage( pData->m_hwndComboOPENFOLDER, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
			::SendMessage( pData->m_hwndComboEOL, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

			//	From Here Feb. 9, 2001 genta
			//	改行コードの選択コンボボックス初期化
			//	必要なときのみ利用する
			if( pData->m_bUseEol ){
				//	値の設定
				for( i = 0; i < nEolNameArrNum; ++i ){
					nIdx = ::SendMessage( pData->m_hwndComboEOL, CB_ADDSTRING, 0, (LPARAM)pEolNameArr[i] );
					::SendMessage( pData->m_hwndComboEOL, CB_SETITEMDATA, nIdx, nEolValueArr[i] );
				}
				//	使うときは先頭の要素を選択状態にする
				::SendMessage( pData->m_hwndComboEOL, CB_SETCURSEL, (WPARAM)0, 0 );
			}
			else {
				//	使わないときは隠す
				::ShowWindow( ::GetDlgItem( hdlg, IDC_STATIC_EOL ), SW_HIDE );
				::ShowWindow( pData->m_hwndComboEOL, SW_HIDE );
			}
			//	To Here Feb. 9, 2001 genta

			//	From Here Jul. 26, 2003 ryoji BOMチェックボックスの初期化
			if( pData->m_bUseBom ){
				//	使うときは有効／無効を切り替え、チェック状態を初期値に設定する
				switch( pData->m_nCharCode ){
				case CODE_UNICODE:
				case CODE_UTF8:
				case CODE_UNICODEBE:
					::EnableWindow( pData->m_hwndCheckBOM, TRUE );
					fCheck = pData->m_bBom? BST_CHECKED: BST_UNCHECKED;
					break;
				default:
					::EnableWindow( pData->m_hwndCheckBOM, FALSE );
					fCheck = BST_UNCHECKED;
					break;
				}
				::SendMessage( pData->m_hwndCheckBOM, BM_SETCHECK, fCheck, 0 );
			}
			else {
				//	使わないときは隠す
				::ShowWindow( pData->m_hwndCheckBOM, SW_HIDE );
			}
			//	To Here Jul. 26, 2003 ryoji BOMチェックボックスの初期化

			/* Explorerスタイルの「開く」ダイアログをフック */
			::SetProp( pData->m_hwndOpenDlg, s_pszOpenFileDataName, (HANDLE)pData );
			// Modified by KEITA for WIN64 2003.9.6
			pData->m_wpOpenDialogProc = (WNDPROC) ::SetWindowLongPtr( pData->m_hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR) OFNHookProcMain );

			/* 文字コード選択コンボボックス初期化 */
			nIdxSel = 0;
			if( pData->m_bIsSaveDialog ){	/* 保存のダイアログか */
				i = 1;
			}else{
				i = 0;
			}
			for( /*i = 0*/; i < gm_nCodeComboNameArrNum; ++i ){
				nIdx = ::SendMessage( pData->m_hwndComboCODES, CB_ADDSTRING, 0, (LPARAM)gm_pszCodeComboNameArr[i] );
				::SendMessage( pData->m_hwndComboCODES, CB_SETITEMDATA, nIdx, gm_nCodeComboValueArr[i] );
				if( gm_nCodeComboValueArr[i] == pData->m_nCharCode ){
					nIdxSel = nIdx;
				}
			}
			::SendMessage( pData->m_hwndComboCODES, CB_SETCURSEL, (WPARAM)nIdxSel, 0 );


			/* 読み取り専用の初期値セット */
			::CheckDlgButton( pData->m_hwndOpenDlg, chx1, pData->m_bReadOnly );
		}
		break;


	case WM_DESTROY:
		/* フック解除 */
		{
			CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr( pData->m_hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR)pData->m_wpOpenDialogProc );
			::RemoveProp( pData->m_hwndOpenDlg, s_pszOpenFileDataName );

		}
		return FALSE;

	case WM_NOTIFY:
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE( _T("=========WM_NOTIFY=========\n") );
//		MYTRACE( _T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom );
//		MYTRACE( _T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( _T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code );

		switch( pofn->hdr.code ){
		case CDN_FILEOK:
			// 拡張子の補完を自前で行う	// 2006.11.10 ryoji
			{
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				if( pData->m_bIsSaveDialog ){
					TCHAR szDefExt[_MAX_EXT];	// 補完する拡張子
					TCHAR szBuf[_MAX_PATH + _MAX_EXT];	// ワーク
					LPTSTR pszCur, pszNext;
					int i;

					CommDlg_OpenSave_GetSpec(pData->m_hwndOpenDlg, szBuf, _MAX_PATH);	// ファイル名入力ボックス内の文字列
					pszCur = szBuf;
					while( *pszCur == _T(' ') )	// 空白を読み飛ばす
						pszCur = ::CharNext(pszCur);
					if( *pszCur == _T('\"') ){	// 二重引用部で始まっている
						::lstrcpyn(pData->m_szPath, pData->m_pOf->lpstrFile, _MAX_PATH);
					}
					else{
						_tsplitpath( pData->m_pOf->lpstrFile, NULL, NULL, NULL, szDefExt );
						if( szDefExt[0] == _T('.') /* && szDefExt[1] != _T('\0') */ ){	// 既に拡張子がついている
							// .のみの場合にも拡張子付きとみなす。
							lstrcpyn(pData->m_szPath, pData->m_pOf->lpstrFile, _MAX_PATH);
						}
						else{
							switch( pData->m_pOf->nFilterIndex ){	// 選択されているファイルの種類
							case 1:		// ユーザー定義
								pszCur = pData->m_pcDlgOpenFile->m_mem->m_szDefaultWildCard;
								while( *pszCur != _T('.') && *pszCur != _T('\0') )	// '.'まで読み飛ばす
									pszCur = ::CharNext(pszCur);
								i = 0;
								while( *pszCur != _T(';') && *pszCur != _T('\0') ){	// ';'までコピーする
									pszNext = ::CharNext(pszCur);
									while( pszCur < pszNext )
										szDefExt[i++] = *pszCur++;
								}
								szDefExt[i] = _T('\0');
								if( ::_tcslen(szDefExt) < 2 || szDefExt[1] == _T('*') )	// 無効な拡張子?
									szDefExt[0] = _T('\0');
								break;
							case 2:		// *.txt
								::_tcscpy(szDefExt, _T(".txt"));
								break;
							case 3:		// *.*
							default:	// 不明
								szDefExt[0] = _T('\0');
								break;
							}
							lstrcpyn(szBuf, pData->m_pOf->lpstrFile, _MAX_PATH + 1);
							::_tcscat(szBuf, szDefExt);
							lstrcpyn(pData->m_szPath, szBuf, _MAX_PATH);
						}
					}

					// ファイルの上書き確認を自前で行う	// 2006.11.10 ryoji
					if( IsFileExists(pData->m_szPath, true) ){
						TCHAR szText[_MAX_PATH + 100];
						lstrcpyn(szText, pData->m_szPath, _MAX_PATH);
						::_tcscat(szText, _T(" は既に存在します。\r\n上書きしますか？"));
						if( IDYES != ::MessageBox( pData->m_hwndOpenDlg, szText, _T("名前を付けて保存"), MB_YESNO | MB_ICONEXCLAMATION) ){
							::SetWindowLongPtr( hdlg, DWLP_MSGRESULT, TRUE );
							return TRUE;
						}
					}
				}

				/* 文字コード選択コンボボックス 値を取得 */
				nIdx = ::SendMessage( pData->m_hwndComboCODES, CB_GETCURSEL, 0, 0 );
				lRes = ::SendMessage( pData->m_hwndComboCODES, CB_GETITEMDATA, nIdx, 0 );
				pData->m_nCharCode = (ECodeType)lRes;	/* 文字コード */
				//	Feb. 9, 2001 genta
				if( pData->m_bUseEol ){
					nIdx = ::SendMessage( pData->m_hwndComboEOL, CB_GETCURSEL, 0, 0 );
					lRes = ::SendMessage( pData->m_hwndComboEOL, CB_GETITEMDATA, nIdx, 0 );
					pData->m_cEol = (EEolType)lRes;	/* 文字コード */
				}
				//	From Here Jul. 26, 2003 ryoji
				//	BOMチェックボックスの状態を取得
				if( pData->m_bUseBom ){
					lRes = ::SendMessage( pData->m_hwndCheckBOM, BM_GETCHECK, 0, 0 );
					pData->m_bBom = (lRes == BST_CHECKED);	/* BOM */
				}
				//	To Here Jul. 26, 2003 ryoji

//				MYTRACE( _T("文字コード  lRes=%d\n"), lRes );
//				MYTRACE( _T("pofn->hdr.code=CDN_FILEOK        \n") );break;
			}
			break;	/* CDN_FILEOK */

		case CDN_FOLDERCHANGE  :
//			MYTRACE( _T("pofn->hdr.code=CDN_FOLDERCHANGE  \n") );
			{
				TCHAR szFolder[_MAX_PATH];
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				lRes = ::SendMessage( pData->m_hwndOpenDlg, CDM_GETFOLDERPATH, _countof( szFolder ), (LPARAM)szFolder );
			}
//			MYTRACE( _T("\tlRes=%d\tszFolder=[%s]\n"), lRes, szFolder );

			break;
//		case CDN_HELP			:	MYTRACE( _T("pofn->hdr.code=CDN_HELP          \n") );break;
//		case CDN_INITDONE		:	MYTRACE( _T("pofn->hdr.code=CDN_INITDONE      \n") );break;
//		case CDN_SELCHANGE		:	MYTRACE( _T("pofn->hdr.code=CDN_SELCHANGE     \n") );break;
//		case CDN_SHAREVIOLATION	:	MYTRACE( _T("pofn->hdr.code=CDN_SHAREVIOLATION\n") );break;
//		case CDN_TYPECHANGE		:	MYTRACE( _T("pofn->hdr.code=CDN_TYPECHANGE    \n") );break;
//		default:					MYTRACE( _T("pofn->hdr.code=???\n") );break;

		}

//		MYTRACE( _T("=======================\n") );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND) lParam;		// handle of control
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			switch( (int) LOWORD(wParam) ){
			//	From Here Jul. 26, 2003 ryoji
			//	文字コードの変更をBOMチェックボックスに反映
			case IDC_COMBO_CODE:
				{
					CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					nIdx = ::SendMessage( (HWND) lParam, CB_GETCURSEL, 0, 0 );
					lRes = ::SendMessage( (HWND) lParam, CB_GETITEMDATA, nIdx, 0 );
					switch( lRes ){
					case CODE_UNICODE:
					case CODE_UTF8:
					case CODE_UNICODEBE:
						::EnableWindow( pData->m_hwndCheckBOM, TRUE );
						if (lRes == pData->m_nCharCode){
							fCheck = pData->m_bBom? BST_CHECKED: BST_UNCHECKED;
						}else{
							fCheck = (lRes == CODE_UTF8)? BST_UNCHECKED: BST_CHECKED;
						}
						break;
					default:
						::EnableWindow( pData->m_hwndCheckBOM, FALSE );
						fCheck = BST_UNCHECKED;
						break;
					}
					::SendMessage( pData->m_hwndCheckBOM, BM_SETCHECK, fCheck, 0 );
				}
				break;
			//	To Here Jul. 26, 2003 ryoji
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				{
					CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					TCHAR	szWork[_MAX_PATH + 1];
					nIdx = ::SendMessage( (HWND) lParam, CB_GETCURSEL, 0, 0 );

					if( CB_ERR != ::SendMessage( (HWND) lParam, CB_GETLBTEXT, nIdx, (LPARAM) (LPCSTR)szWork ) ){
						// 2005.11.02 ryoji ファイル名指定のコントロールを確認する
						hwndFilebox = ::GetDlgItem( pData->m_hwndOpenDlg, cmb13 );		// ファイル名コンボ（Windows 2000タイプ）
						if( !::IsWindow( hwndFilebox ) )
							hwndFilebox = ::GetDlgItem( pData->m_hwndOpenDlg, edt1 );	// ファイル名エディット（レガシータイプ）
						if( ::IsWindow( hwndFilebox ) ){
							::SetWindowText( hwndFilebox, szWork );
							if( IDC_COMBO_OPENFOLDER == wID )
								::PostMessage( hwndFilebox, WM_KEYDOWN, VK_RETURN, (LPARAM)0 );
						}
					}
				}
				break;
			}
			break;	/* CBN_SELCHANGE */
		case CBN_DROPDOWN:
			{
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);

				switch( wID ){
				case IDC_COMBO_MRU:
					if ( ::SendMessage(pData->m_hwndComboMRU, CB_GETCOUNT, 0L, 0L) == 0) {
						/* 最近開いたファイル コンボボックス初期値設定 */
						//	2003.06.22 Moca m_vMRU がNULLの場合を考慮する
						int nSize = (int)pData->m_pcDlgOpenFile->m_mem->m_vMRU.size();
						for( i = 0; i < nSize; i++ ){
							::SendMessage( pData->m_hwndComboMRU, CB_ADDSTRING, 0, (LPARAM)pData->m_pcDlgOpenFile->m_mem->m_vMRU[i] );
						}
					}
					CDlgOpenFile::OnCbnDropDown( hwndCtl );
					break;

				case IDC_COMBO_OPENFOLDER:
					if ( ::SendMessage(pData->m_hwndComboOPENFOLDER, CB_GETCOUNT, 0L, 0L) == 0) {
						/* 最近開いたフォルダ コンボボックス初期値設定 */
						//	2003.06.22 Moca m_vOPENFOLDER がNULLの場合を考慮する
						int nSize = (int)pData->m_pcDlgOpenFile->m_mem->m_vOPENFOLDER.size();
						for( i = 0; i < nSize; i++ ){
							::SendMessage( pData->m_hwndComboOPENFOLDER, CB_ADDSTRING, 0, (LPARAM)pData->m_pcDlgOpenFile->m_mem->m_vOPENFOLDER[i] );
						}
					}
					CDlgOpenFile::OnCbnDropDown( hwndCtl );
					break;
				}
				break;	/* CBN_DROPDOWN */
			}
		}
		break;	/* WM_COMMAND */

	//@@@ 2002.01.08 add start
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hdlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	//@@@ 2002.01.08 add end

	default:
		return FALSE;
	}
	return TRUE;
}





/*! コンストラクタ
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
CDlgOpenFile::CDlgOpenFile()
{
	/* メンバの初期化 */
	m_mem = new CDlgOpenFileMem();

	m_mem->m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_mem->m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */

	/* 共有データ構造体のアドレスを返す */
	m_mem->m_pShareData = CShareData::getInstance()->GetShareData();

	TCHAR	szFile[_MAX_PATH + 1];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	::GetModuleFileName(
		NULL,
		szFile, _countof( szFile )
	);
	_tsplitpath( szFile, szDrive, szDir, NULL, NULL );
	_tcscpy( m_mem->m_szInitialDir, szDrive );
	_tcscat( m_mem->m_szInitialDir, szDir );



	_tcscpy( m_mem->m_szDefaultWildCard, _T("*.*") );	/*「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */

	return;
}





CDlgOpenFile::~CDlgOpenFile()
{
	delete m_mem;
	m_mem = NULL;
	return;
}


/* 初期化 */
void CDlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER
)
{
	m_mem->m_hInstance = hInstance;
	m_mem->m_hwndParent = hwndParent;

	/* ユーザー定義ワイルドカード（保存時の拡張子補完でも使用される） */
	if( NULL != pszUserWildCard ){
		_tcscpy( m_mem->m_szDefaultWildCard, pszUserWildCard );
	}

	/* 「開く」での初期フォルダ */
	if( pszDefaultPath && pszDefaultPath[0] != _T('\0') ){	//現在編集中のファイルのパス	//@@@ 2002.04.18
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		//	Jun. 23, 2002 genta
		my_splitpath( pszDefaultPath, szDrive, szDir, NULL, NULL );
		// 2010.08.28 相対パス解決
		TCHAR szRelPath[_MAX_PATH];
		sprintf( szRelPath, _T("%s%s"), szDrive, szDir );
		const TCHAR* p = szRelPath;
		if( ! ::GetLongFileName( p, m_mem->m_szInitialDir ) ){
			_tcscpy(m_mem->m_szInitialDir, p );
		}
	}
	m_mem->m_vMRU = vMRU;
	m_mem->m_vOPENFOLDER = vOPENFOLDER;
	return;
}




/*! 「開く」ダイアログ モーダルダイアログの表示

	@param[in,out] pszPath 初期ファイル名．選択されたファイル名の格納場所
	@param[in] bSetCurDir カレントディレクトリを変更するか デフォルト: false
	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool CDlgOpenFile::DoModal_GetOpenFileName( TCHAR* pszPath , bool bSetCurDir )
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("ユーザー指定"),     m_mem->m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("テキストファイル"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("すべてのファイル"), _T("*.*") );

	/* 構造体の初期化 */
	std::auto_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());

	pData->m_ofn.hwndOwner = m_mem->m_hwndParent;
	pData->m_ofn.hInstance = m_mem->m_hInstance;
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	// From Here Jun. 23, 2002 genta
	// 「開く」での初期フォルダチェック強化
// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		TCHAR szExt  [_MAX_EXT];

		//	Jun. 23, 2002 Thanks to sui
		my_splitpath( pszPath, szDrive, szDir, szName, szExt );

		//	指定されたファイルが存在しないとき szName == NULL
		//	ファイルの場所にディレクトリを指定するとエラーになるので
		//	ファイルが無い場合は全く指定しないことにする．
		if( szName[0] == _T('\0') ){
			pszPath[0] = _T('\0');
		}
		else {
			TCHAR szRelPath[_MAX_PATH];
			wsprintf( szRelPath, _T("%s%s%s%s"), szDrive, szDir, szName, szExt );
			const TCHAR* p = szRelPath;
			if( ! ::GetLongFileName( p, pszPath ) ){
				_tcscpy( pszPath, p );
			}
		}
	}
	pData->m_ofn.lpstrFile = pszPath;
	// To Here Jun. 23, 2002 genta
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_mem->m_szInitialDir;
	pData->m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	pData->m_ofn.lpstrDefExt = _T(""); // 2005/02/20 novice 拡張子を省略したら補完する

	// 2010.08.28 フォルダを開くとフックも含めて色々DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	if( _GetOpenFileNameRecover( &pData->m_ofn ) ){
		return true;
	}
	else{
		//	May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}


/*! 保存ダイアログ モーダルダイアログの表示
	@param pszPath [i/o] 初期ファイル名．選択されたファイル名の格納場所
	@param bSetCurDir [in] カレントディレクトリを変更するか デフォルト: false
	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool CDlgOpenFile::DoModal_GetSaveFileName( TCHAR* pszPath, bool bSetCurDir )
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("ユーザー指定"), m_mem->m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("テキストファイル"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("すべてのファイル"), _T("*.*") );
	
	// 2010.08.28 カレントディレクトリを移動するのでパス解決する
	if( pszPath[0] ){
		TCHAR szFullPath[_MAX_PATH];
		const TCHAR* pOrg = pszPath;
		if( ::GetLongFileName( pOrg, szFullPath ) ){
			// 成功。書き戻す
			_tcscpy( pszPath , szFullPath );
		}
	}

	/* 構造体の初期化 */
	std::auto_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_mem->m_hwndParent;
	pData->m_ofn.hInstance = m_mem->m_hInstance;
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pszPath; // 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_mem->m_szInitialDir;
	pData->m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	pData->m_ofn.lpstrDefExt = _T("");	// 2005/02/20 novice 拡張子を省略したら補完する

	// 2010.08.28 フォルダを開くとフックも含めて色々DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	if( GetSaveFileNameRecover( &pData->m_ofn ) ){
		return true;
	}
	else{
		//	May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}





/*! 「開く」ダイアログ モーダルダイアログの表示
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool CDlgOpenFile::DoModalOpenDlg( char* pszPath, ECodeType* pnCharCode, bool* pbReadOnly )
{
	std::auto_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	pData->m_bIsSaveDialog = FALSE;	/* 保存のダイアログか */

	int		i;

	// ファイルの種類	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("すべてのファイル"), _T("*.*") );
	cFileExt.AppendExtRaw( _T("テキストファイル"), _T("*.txt") );
	for( i = 0; i < MAX_TYPES; i++ ){
		const STypeConfig& types = m_mem->m_pShareData->m_Types[i];
		cFileExt.AppendExt( types.m_szTypeName, types.m_szTypeExts );
	}

	//メンバの初期化
	pData->m_bReadOnly = pbReadOnly ? *pbReadOnly : false;
	pData->m_nCharCode = pnCharCode ? *pnCharCode : CODE_AUTODETECT;	/* 文字コード自動判別 */
	pData->m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	pData->m_bUseEol = false;	//	Feb. 9, 2001 genta
	pData->m_bUseBom = false;	//	Jul. 26, 2003 ryoji

	//OPENFILENAME構造体の初期化
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_mem->m_hwndParent;
	pData->m_ofn.hInstance = m_mem->m_hInstance;
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pszPath;	// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_mem->m_szInitialDir;
	pData->m_ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( pData->m_bReadOnly ) pData->m_ofn.Flags |= OFN_READONLY;
	pData->m_ofn.lpstrDefExt = _T("");	// 2005/02/20 novice 拡張子を省略したら補完する

	//カレントディレクトリを保存。関数を抜けるときに自動でカレントディレクトリは復元されます。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 フォルダを開くとフックも含めて色々DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	//ダイアログ表示
	bool bDlgResult = _GetOpenFileNameRecover( &pData->m_ofn );
	if( bDlgResult ){
		if( pnCharCode ){
			*pnCharCode = pData->m_nCharCode;
		}
		if( pbReadOnly ){
			*pbReadOnly = pData->m_bReadOnly;
		}
	}
	else{
		DlgOpenFail();
	}
	return bDlgResult;
}

/*! 保存ダイアログ モーダルダイアログの表示

	@param pszPath [out]	取得したパス名
	@param pnCharCode [out]	文字コード
	@param pcEol [out]		改行コード
	@param pbBom [out]		BOM

	@date 2001.02.09 genta	引数追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2003.07.26 ryoji BOMパラメータ追加
	@date 2005.02.20 novice 拡張子を省略したら補完する
	@date 2006.11.10 ryoji フックを使う場合は拡張子の補完を自前で行う
		Windowsで関連付けが無いような拡張子を指定して保存すると、明示的に
		拡張子入力してあるのにデフォルト拡張子が補完されてしまうことがある。
			例）hoge.abc -> hoge.abc.txt
		自前で補完することでこれを回避する。（実際の処理はフックプロシージャの中）
*/
bool CDlgOpenFile::DoModalSaveDlg( char* pszPath, ECodeType* pnCharCode, CEol* pcEol, bool* pbBom )
{
	std::auto_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	pData->m_bIsSaveDialog = TRUE;	/* 保存のダイアログか */

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("ユーザー指定"), m_mem->m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("テキストファイル"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("すべてのファイル"), _T("*.*") );

	// ファイル名の初期設定	// 2006.11.10 ryoji
	if( pszPath[0] == _T('\0') )
		lstrcpyn(pszPath, _T("無題"), _MAX_PATH);

	//OPENFILENAME構造体の初期化
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_mem->m_hwndParent;
	pData->m_ofn.hInstance = m_mem->m_hInstance;
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pszPath; // 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_mem->m_szInitialDir;
	pData->m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( pnCharCode || pcEol ){
		pData->m_ofn.Flags = pData->m_ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
		pData->m_ofn.Flags &= ~OFN_OVERWRITEPROMPT;	// 2006.11.10 ryoji 上書き確認もフックの中で自前で処理する
	}


// 2005/02/20 novice 拡張子を省略したら補完する
//	pData->m_ofn.lpstrDefExt = _T("");
	pData->m_ofn.lpstrDefExt = (pData->m_ofn.Flags & OFN_ENABLEHOOK)? NULL: _T("");	// 2006.11.10 ryoji フックを使うときは自前で拡張子を補完する

	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 フォルダを開くとフックも含めて色々DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	if( pnCharCode ){
		pData->m_nCharCode = *pnCharCode;
	}
	//	From Here Feb. 9, 2001 genta
	if( pcEol ){
		//m_cEol = EOL_NONE;	//	初期値は「改行コードを保存」に固定
		pData->m_cEol = *pcEol;	// 	// 2008.03.20 ryoji 初期値の指定は上位に任せる
		pData->m_bUseEol = true;
	}
	else{
		pData->m_bUseEol = false;
	}

	//	To Here Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOM設定
	if( pbBom ){
		pData->m_bBom = *pbBom;
		pData->m_bUseBom = true;
	}
	else{
		pData->m_bUseBom = false;
	}

	pData->m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	if( GetSaveFileNameRecover( &pData->m_ofn ) ){
		if( pData->m_ofn.Flags & OFN_ENABLEHOOK )
			lstrcpyn(pszPath, pData->m_szPath, _MAX_PATH);	// 自前で拡張子の補完を行ったときのファイルパス	// 2006.11.10 ryoji

		if( pnCharCode ){
			*pnCharCode = pData->m_nCharCode;
		}
		//	Feb. 9, 2001 genta
		if( pData->m_bUseEol ){
			*pcEol = pData->m_cEol;
		}
		//	Jul. 26, 2003 ryoji BOM設定
		if( pData->m_bUseBom ){
			*pbBom = pData->m_bBom;
		}
		return true;
	}
	else{
		//	May 29, 2004 genta 関数にまとめた
		DlgOpenFail();
		return false;
	}
}

/*! @brief コモンダイアログボックス失敗処理

	コモンダイアログボックスからFALSEが返された場合に
	エラー原因を調べてエラーならメッセージを出す．
	
	@author genta
	@date 2004.05.29 genta 元々あった部分をまとめた
*/
void CDlgOpenFile::DlgOpenFail(void)
{
	const TCHAR*	pszError;
	DWORD dwError = ::CommDlgExtendedError();
	if( dwError == 0 ){
		//	ユーザキャンセルによる
		return;
	}
	
	switch( dwError ){
	case CDERR_DIALOGFAILURE  : pszError = _T("CDERR_DIALOGFAILURE  "); break;
	case CDERR_FINDRESFAILURE : pszError = _T("CDERR_FINDRESFAILURE "); break;
	case CDERR_NOHINSTANCE    : pszError = _T("CDERR_NOHINSTANCE    "); break;
	case CDERR_INITIALIZATION : pszError = _T("CDERR_INITIALIZATION "); break;
	case CDERR_NOHOOK         : pszError = _T("CDERR_NOHOOK         "); break;
	case CDERR_LOCKRESFAILURE : pszError = _T("CDERR_LOCKRESFAILURE "); break;
	case CDERR_NOTEMPLATE     : pszError = _T("CDERR_NOTEMPLATE     "); break;
	case CDERR_LOADRESFAILURE : pszError = _T("CDERR_LOADRESFAILURE "); break;
	case CDERR_STRUCTSIZE     : pszError = _T("CDERR_STRUCTSIZE     "); break;
	case CDERR_LOADSTRFAILURE : pszError = _T("CDERR_LOADSTRFAILURE "); break;
	case FNERR_BUFFERTOOSMALL : pszError = _T("FNERR_BUFFERTOOSMALL "); break;
	case CDERR_MEMALLOCFAILURE: pszError = _T("CDERR_MEMALLOCFAILURE"); break;
	case FNERR_INVALIDFILENAME: pszError = _T("FNERR_INVALIDFILENAME"); break;
	case CDERR_MEMLOCKFAILURE : pszError = _T("CDERR_MEMLOCKFAILURE "); break;
	case FNERR_SUBCLASSFAILURE: pszError = _T("FNERR_SUBCLASSFAILURE"); break;
	default: pszError = _T("UNKNOWN_ERRORCODE"); break;
	}

	ErrorBeep();
	TopErrorMessage( m_mem->m_hwndParent,
		_T("ダイアログが開けません。\n")
		_T("\n")
		_T("エラー:%s"),
		pszError
	);
}

/*! OPENFILENAME 初期化

	OPENFILENAME に CDlgOpenFile クラス用の初期規定値を設定する

	@author ryoji
	@date 2005.10.29
*/
void CDlgOpenFile::InitOfn( OPENFILENAMEZ* ofn )
{
	memset(ofn, 0, sizeof(*ofn));

	ofn->lStructSize = IsWinV5forOfn()? sizeof(OPENFILENAMEZ): OPENFILENAME_SIZE_VERSION_400;
	ofn->lpfnHook = OFNHookProc;
	ofn->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);	// <-_T("IDD_FILEOPEN"); 2008/7/26 Uchi
	ofn->nFilterIndex = 1;	//Jul. 09, 2001 JEPRO		/* 「開く」での最初のワイルドカード */
}

/*! 初期レイアウト設定処理

	追加コントロールのレイアウトを変更する

	@param hwndOpenDlg [in]		ファイルダイアログのウィンドウハンドル
	@param hwndDlg [in]			子ダイアログのウィンドウハンドル
	@param hwndBaseCtrl [in]	移動基準コントロール（ファイル名ボックスと左端を合わせるコントロール）のウィンドウハンドル

	@author ryoji
	@date 2005.11.02
*/
void CDlgOpenFile::InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl )
{
	HWND hwndFilelabel;
	HWND hwndFilebox;
	HWND hwndCtrl;
	RECT rcBase;
	RECT rc;
	POINT po;
	int nLeft;
	int nShift;
	int nWidth;

	// ファイル名ラベルとファイル名ボックスを取得する
	if( !::IsWindow( hwndFilelabel = ::GetDlgItem( hwndOpenDlg, stc3 ) ) )		// ファイル名ラベル
		return;
	if( !::IsWindow( hwndFilebox = ::GetDlgItem( hwndOpenDlg, cmb13 ) ) ){		// ファイル名コンボ（Windows 2000タイプ）
		if( !::IsWindow( hwndFilebox = ::GetDlgItem( hwndOpenDlg, edt1 ) ) )	// ファイル名エディット（レガシータイプ）
			return;
	}

	// コントロールの基準位置、移動量を決定する
	::GetWindowRect( hwndFilelabel, &rc );
	nLeft = rc.left;						// 左端に揃えるコントロールの位置
	::GetWindowRect( hwndFilebox, &rc );
	::GetWindowRect( hwndBaseCtrl, &rcBase );
	nShift = rc.left - rcBase.left;			// 左端以外のコントロールの右方向への相対移動量

	// 追加コントロールをすべて移動する
	// ・基準コントロールよりも左にあるものはファイル名ラベルに合わせて左端に移動
	// ・その他は移動基準コントロール（ファイル名ボックスと左端を合わせるコントロール）と同じだけ右方向へ相対移動
	hwndCtrl = ::GetWindow( hwndDlg, GW_CHILD );
	while( hwndCtrl ){
		if( ::GetDlgCtrlID(hwndCtrl) != stc32 ){
			::GetWindowRect( hwndCtrl, &rc );
			po.x = ( rc.right < rcBase.left )? nLeft: rc.left + nShift;
			po.y = rc.top;
			::ScreenToClient( hwndDlg, &po );
			::SetWindowPos( hwndCtrl, 0, po.x, po.y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
		hwndCtrl = ::GetWindow( hwndCtrl, GW_HWNDNEXT );
	}


	// 標準コントロールのプレースフォルダ（stc32）と子ダイアログの幅をオープンダイアログの幅にあわせる
	//     WM_INITDIALOG を抜けるとさらにオープンダイアログ側で現在の位置関係からレイアウト調整が行われる
	//     ここで以下の処理をやっておかないとコントロールが意図しない場所に動いてしまうことがある
	//     （例えば、BOM のチェックボックスが画面外に飛んでしまうなど）

	// オープンダイアログのクライアント領域の幅を取得する
	::GetClientRect( hwndOpenDlg, &rc );
	nWidth = rc.right - rc.left;

	// 標準コントロールプレースフォルダの幅を変更する
	hwndCtrl = ::GetDlgItem( hwndDlg, stc32 );
	::GetWindowRect( hwndCtrl, &rc );
	::SetWindowPos( hwndCtrl, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

	// 子ダイアログの幅を変更する
	// ※この SetWindowPos() の中で WM_SIZE が発生する
	::GetWindowRect( hwndDlg, &rc );
	::SetWindowPos( hwndDlg, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
}

/*! コンボボックスのドロップダウン時処理

	コンボボックスがドロップダウンされる時に
	ドロップダウンリストの幅をアイテム文字列の最大表示幅に合わせる

	@param hwndCtl [in]		コンボボックスのウィンドウハンドル

	@author ryoji
	@date 2005.10.29
*/
void CDlgOpenFile::OnCbnDropDown( HWND hwndCtl )
{
	HDC hDC;
	HFONT hFont;
	LONG nWidth;
	RECT rc;
	SIZE sizeText;
	int nTextLen;
	int iItem;
	int nItem;
	const int nMargin = 8;

	hDC = ::GetDC( hwndCtl );
	if( NULL == hDC )
		return;
	hFont = (HFONT)::SendMessage( hwndCtl, WM_GETFONT, 0, (LPARAM)NULL );
	hFont = (HFONT)::SelectObject( hDC, hFont );
	nItem = ::SendMessage( hwndCtl, CB_GETCOUNT, 0, (LPARAM)NULL );
	::GetWindowRect( hwndCtl, &rc );
	nWidth = rc.right - rc.left - nMargin;
	for( iItem = 0; iItem < nItem; iItem++ ){
		nTextLen = ::SendMessage( hwndCtl, CB_GETLBTEXTLEN, (WPARAM)iItem, (LPARAM)NULL );
		if( 0 < nTextLen ) {
			TCHAR* pszText = new TCHAR[nTextLen + 1];
			::SendMessage( hwndCtl, CB_GETLBTEXT, (WPARAM)iItem, (LPARAM)pszText );
			if( ::GetTextExtentPoint32( hDC, pszText, nTextLen, &sizeText ) ){
				if ( nWidth < sizeText.cx )
					nWidth = sizeText.cx;
			}
			delete []pszText;
		}
	}
	::SendMessage( hwndCtl, CB_SETDROPPEDWIDTH, (WPARAM)(nWidth + nMargin), (LPARAM)NULL );
	::SelectObject( hDC, hFont );
	::ReleaseDC( hwndCtl, hDC );
}

/*! リトライ機能付き GetOpenFileName
	@author Moca
	@date 2006.09.03 新規作成
	@date 2008.11.23 nasukoji	パスが長すぎる場合への対応
*/
bool CDlgOpenFile::_GetOpenFileNameRecover( OPENFILENAMEZ* ofn )
{
	// 2008.11.23 nasukoji	パスが長すぎる場合への対応
	// パスが長すぎた場合、ofn.lpstrFile[m_ofn.nMaxFile - 1] まで値を入れてしまう
	// ことがある。
	// しかし、[ofn.nMaxFile - 1] がSJISの1バイト目となる場合は [ofn.nMaxFile - 2]
	// まで値を入れて [ofn.nMaxFile - 1] には値をセットせずに返ってくる。
	// そのため、最初から [ofn.nMaxFile - 1] が0だと指定のパスが長すぎたのかチェック
	// できない。
	// 先に [ofn.nMaxFile - 1] を0以外に設定しておく事で、パスが長すぎたことをチェック
	// 可能とする。
	// ただし、元の文字列が(ofn.nMaxFile - 1)バイト以下である時のみこの書き換えを
	// 実施する（元の文字列がofn.nMaxFileバイト以上の時はバッファを破壊してしまう為）
	if( CheckPathLengthOverflow( ofn->lpstrFile, ofn->nMaxFile - 1, FALSE) )
		ofn->lpstrFile[ofn->nMaxFile - 1] = -1;

	BOOL bRet = ::GetOpenFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			_tcscpy( ofn->lpstrFile, _T("") );
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetOpenFileName( ofn );
		}
	}

	// ファイルパスが長すぎたらエラーを表示してFALSEを返す
	if( bRet && !CheckPathLengthOverflow( ofn->lpstrFile, ofn->nMaxFile )){
		bRet = FALSE;
	}

	return bRet!=FALSE;
}

/*! リトライ機能付き GetSaveFileName
	@author Moca
	@date 2006.09.03 新規作成
	@date 2008.11.23 nasukoji	パスが長すぎる場合への対応
*/
bool CDlgOpenFile::GetSaveFileNameRecover( OPENFILENAMEZ* ofn )
{
	// 2008.11.23 nasukoji	パスが長すぎる場合への対応
	// パスが長すぎた場合、ofn.lpstrFile[m_ofn.nMaxFile - 1] まで値を入れてしまう
	// ことがある。
	// しかし、[ofn.nMaxFile - 1] がSJISの1バイト目となる場合は [ofn.nMaxFile - 2]
	// まで値を入れて [ofn.nMaxFile - 1] には値をセットせずに返ってくる。
	// そのため、最初から [ofn.nMaxFile - 1] が0だと指定のパスが長すぎたのかチェック
	// できない。
	// 先に [ofn.nMaxFile - 1] を0以外に設定しておく事で、パスが長すぎたことをチェック
	// 可能とする。
	// ただし、元の文字列が(ofn.nMaxFile - 1)バイト以下である時のみこの書き換えを
	// 実施する（元の文字列がofn.nMaxFileバイト以上の時はバッファを破壊してしまう為）
	if( CheckPathLengthOverflow( ofn->lpstrFile, ofn->nMaxFile - 1, FALSE) )
		ofn->lpstrFile[ofn->nMaxFile - 1] = -1;

	BOOL bRet = ::GetSaveFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			_tcscpy( ofn->lpstrFile, _T("") );
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetSaveFileName( ofn );
		}
	}

	// ファイルパスが長すぎたらエラーを表示してFALSEを返す
	if( bRet && !CheckPathLengthOverflow( ofn->lpstrFile, ofn->nMaxFile )){
		bRet = FALSE;
	}

	return bRet!=FALSE;
}

/*!
	@brief 指定のファイルパスのバッファオーバーフローをチェックする
	
	指定のファイルパスがチェックサイズ以内に'\0'でターミネートされているか
	チェックする。
	チェックサイズ以内に'\0'が現れなかった場合バッファオーバーフローとし、
	エラー表示が指定されていればエラーを表示する。
	
	@param[in] pszPath		チェックする文字列
	@param[in] nLength		文字列のチェックサイズ
	@param[in] bErrDisp		オーバーフローの時にエラー表示する
	@return		文字列がオーバーフローでない時はTRUEを返す
				文字列がオーバーフロー時はFALSEを返す
	
	@note ファイルパスが _MAX_PATH を超える場合、バッファい内に'\0'が存在
	      しない状態となり異常終了することへの対策。
	      バッファが小さ過ぎてフルパスを格納できないのにANSI版のAPIがエラーを
	      返さないため自力でチェックする。
	      unicode版は長いファイルパスを正しく開けるので、ANSI版ではエラー表示
	      に留めて無理に対応を考えない。
	
	@date 2008.06.25 nasukoji	新規作成
*/
BOOL CDlgOpenFile::CheckPathLengthOverflow( const char *pszPath, int nLength, BOOL bErrDisp )
{
	int i;
	
	// nLength文字内に'\0'があるかチェック
	for( i = 0; i < nLength && pszPath[i]; i++ )
		;

	if( bErrDisp && i >= nLength ){
		ErrorBeep();
		TopErrorMessage( NULL,
					_T("ファイルパスが長すぎます。 ANSI 版では %d バイト以上の絶対パスを扱えません。"),
					nLength );
	}

	return ( i >= nLength ) ? FALSE : TRUE;
}

/*[EOF]*/
