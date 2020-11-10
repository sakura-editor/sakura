﻿/*!	@file
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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <CdErr.h>
#include <Dlgs.h>
#include <CommDlg.h>
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDialog.h"
#include "func/Funccode.h"	//Stonee, 2001/05/18
#include "CFileExt.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "CEditApp.h"
#include "CEol.h"
#include "charset/CCodePage.h"
#include "doc/CDocListener.h"
#include "recent/CRecent.h"
#include "util/window.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/os.h"
#include "util/module.h"
#include "util/design_template.h"
#include "basis/CMyString.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDCANCEL,				HIDCANCEL_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		//ヘルプボタン
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		//文字コードセット
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			//最近のファイル
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	//最近のフォルダ
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			//改行コード
	IDC_CHECK_BOM,			HIDC_OPENDLG_CHECK_BOM,			//BOM	// 2006.08.06 ryoji
	IDC_CHECK_CP,			HIDC_OPENDLG_CHECK_CP,			//CP
//	IDC_STATIC,				-1,
	0, 0
};

static int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit);

struct CDlgOpenFile_CommonFileDialog final : public IDlgOpenFile
{
	CDlgOpenFile_CommonFileDialog();

	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const WCHAR*				pszUserWildCard,
		const WCHAR*				pszDefaultPath,
		const std::vector<LPCWSTR>& vMRU,
		const std::vector<LPCWSTR>& vOPENFOLDER
	) override;

	bool DoModal_GetOpenFileName( WCHAR* pszPath, EFilter eAddFileter ) override;
	bool DoModal_GetSaveFileName( WCHAR* pszPath ) override;
	bool DoModalOpenDlg( SLoadInfo* pLoadInfo, std::vector<std::wstring>*, bool bOptions ) override;
	bool DoModalSaveDlg( SSaveInfo*	pSaveInfo, bool bSimpleMode ) override;

	void DlgOpenFail(void);

	void InitOfn( OPENFILENAME* ofn );

	static void InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl );

	//! リトライ機能付き GetOpenFileName
	bool _GetOpenFileNameRecover( OPENFILENAME* ofn );
	//! リトライ機能付き GetOpenFileName
	bool GetSaveFileNameRecover( OPENFILENAME* ofn );

	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */

	DLLSHAREDATA*	m_pShareData;

	SFilePath		m_szDefaultWildCard;	/* 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */
	SFilePath		m_szInitialDir;			/* 「開く」での初期ディレクトリ */

	std::vector<LPCWSTR>	m_vMRU;
	std::vector<LPCWSTR>	m_vOPENFOLDER;
};

class CDlgOpenFileData{
public:
	CDlgOpenFile_CommonFileDialog*	m_pcDlgOpenFile;

	WNDPROC			m_wpOpenDialogProc;
	int				m_nHelpTopicID;
	bool			m_bViewMode;		// ビューモードか
	BOOL			m_bIsSaveDialog;	// 保存のダイアログか
	ECodeType		m_nCharCode;		// 文字コード

	CEol			m_cEol;
	bool			m_bUseCharCode;
	bool			m_bUseEol;
	bool			m_bBom;		//!< BOMを付けるかどうか	//	Jul. 26, 2003 ryoji BOM
	bool			m_bUseBom;	//!< BOMの有無を選択する機能を利用するかどうか
	SFilePath		m_szPath;	// 拡張子の補完を自前で行ったときのファイルパス	// 2006.11.10 ryoji

	bool			m_bInitCodePage;

	CRecentFile				m_cRecentFile;
	CRecentFolder			m_cRecentFolder;

	OPENFILENAME*	m_pOf;
	OPENFILENAME	m_ofn;		/* 2005.10.29 ryoji OPENFILENAME「ファイルを開く」ダイアログ用構造体 */
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

static const WCHAR* s_pszOpenFileDataName = L"FileOpenData";

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
		pShareData = &GetDllShareData();
		::GetWindowRect( hwnd, &pShareData->m_Common.m_sOthers.m_rcOpenDialog );
//		MYTRACE( L"WM_MOVE 1\n" );
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
				pData->m_bViewMode = ( 0 != ::IsDlgButtonChecked( hwnd , chx1 ) );
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
//		pofn = (OFNOTIFY*) lParam;
//		MYTRACE( L"=========WM_NOTIFY=========\n" );
//		MYTRACE( L"pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE( L"pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( L"pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );
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
	static const WCHAR*	const	pEolNameArr[] = {
		L"変換なし", // ダミー
		L"CR+LF",
		L"LF (UNIX)",
		L"CR (Mac)",
	};
	int nEolNameArrNum = (int)_countof(pEolNameArr);

//	To Here	Feb. 9, 2001 genta
	int	nRightMargin = 24;
	HWND	hwndFrame;

	switch( uiMsg ){
	case WM_MOVE:
//		MYTRACE( L"WM_MOVE 2\n" );
		break;
	case WM_SIZE:
		{
			nWidth = LOWORD(lParam);	// width of client area

			/* 「開く」ダイアログのサイズと位置 */
			CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			hwndFrame = ::GetParent( hdlg );
			::GetWindowRect( hwndFrame, &pData->m_pcDlgOpenFile->m_pShareData->m_Common.m_sOthers.m_rcOpenDialog );

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
			UpdateDialogFont( hdlg );

			// Save off the long pointer to the OPENFILENAME structure.
			// Modified by KEITA for WIN64 2003.9.6
			OPENFILENAME* pOfn = (OPENFILENAME*)lParam;
			CDlgOpenFileData* pData = reinterpret_cast<CDlgOpenFileData*>(pOfn->lCustData);
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
			pData->m_bInitCodePage = false;

			// 2005.11.02 ryoji 初期レイアウト設定
			CDlgOpenFile_CommonFileDialog::InitLayout( pData->m_hwndOpenDlg, hdlg, pData->m_hwndComboCODES );

			/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
			Combo_SetExtendedUI( pData->m_hwndComboCODES, TRUE );
			Combo_SetExtendedUI( pData->m_hwndComboMRU, TRUE );
			Combo_SetExtendedUI( pData->m_hwndComboOPENFOLDER, TRUE );
			Combo_SetExtendedUI( pData->m_hwndComboEOL, TRUE );

			//	From Here Feb. 9, 2001 genta
			//	改行コードの選択コンボボックス初期化
			//	必要なときのみ利用する
			if( pData->m_bUseEol ){
				//	値の設定
				// 2013.05.27 初期値をSaveInfoから設定する
				nIdxSel = 0;
				for( i = 0; i < nEolNameArrNum; ++i ){
					if( i == 0 ){
						nIdx = Combo_AddString( pData->m_hwndComboEOL, LS(STR_DLGOPNFL1) );
					}else{
						nIdx = Combo_AddString( pData->m_hwndComboEOL, pEolNameArr[i] );
					}
					Combo_SetItemData( pData->m_hwndComboEOL, nIdx, nEolValueArr[i] );
					if( nEolValueArr[i] == pData->m_cEol ){
						nIdxSel = nIdx;
					}
				}
				Combo_SetCurSel( pData->m_hwndComboEOL, nIdxSel );
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
				if (CCodeTypeName(pData->m_nCharCode).UseBom()) {
					::EnableWindow( pData->m_hwndCheckBOM, TRUE );
					fCheck = pData->m_bBom? BST_CHECKED: BST_UNCHECKED;
				}
				else {
					::EnableWindow( pData->m_hwndCheckBOM, FALSE );
					fCheck = BST_UNCHECKED;
				}
				BtnCtl_SetCheck( pData->m_hwndCheckBOM, fCheck );
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
			nIdxSel = -1;
			if( pData->m_bIsSaveDialog ){	/* 保存のダイアログか */
				i = 1; // 「自動選択」飛ばし
			}else{
				i = 0;
			}
			CCodeTypesForCombobox cCodeTypes;
			for( /*i = 0*/; i < cCodeTypes.GetCount(); ++i ){
				nIdx = Combo_AddString( pData->m_hwndComboCODES, cCodeTypes.GetName(i) );
				Combo_SetItemData( pData->m_hwndComboCODES, nIdx, cCodeTypes.GetCode(i) );
				if( cCodeTypes.GetCode(i) == pData->m_nCharCode ){
					nIdxSel = nIdx;
				}
			}
			if( nIdxSel != -1 ){
				Combo_SetCurSel( pData->m_hwndComboCODES, nIdxSel );
			}else{
				CheckDlgButtonBool( hdlg, IDC_CHECK_CP, true );
				if( -1 == AddComboCodePages( hdlg, pData->m_hwndComboCODES, pData->m_nCharCode, pData->m_bInitCodePage ) ){
					Combo_SetCurSel( pData->m_hwndComboCODES, 0 );
				}
			}
			if( !pData->m_bUseCharCode ){
				::ShowWindow( GetDlgItem( hdlg, IDC_STATIC_CHARCODE ), SW_HIDE );
				::ShowWindow( pData->m_hwndComboCODES, SW_HIDE );
			}


			/* ビューモードの初期値セット */
			::CheckDlgButton( pData->m_hwndOpenDlg, chx1, pData->m_bViewMode );

			CDialog::SetComboBoxDeleter(pData->m_hwndComboMRU, &pData->m_cRecentFile);
			CDialog::SetComboBoxDeleter(pData->m_hwndComboOPENFOLDER, &pData->m_cRecentFolder);
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
//		MYTRACE( L"=========WM_NOTIFY=========\n" );
//		MYTRACE( L"pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE( L"pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( L"pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );

		switch( pofn->hdr.code ){
		case CDN_FILEOK:
			// 拡張子の補完を自前で行う	// 2006.11.10 ryoji
			{
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				if( pData->m_bIsSaveDialog ){
					WCHAR szDefExt[_MAX_EXT];	// 補完する拡張子
					WCHAR szBuf[_MAX_PATH + _MAX_EXT];	// ワーク
					LPWSTR pszCur, pszNext;
					int i;

					CommDlg_OpenSave_GetSpec(pData->m_hwndOpenDlg, szBuf, _MAX_PATH);	// ファイル名入力ボックス内の文字列
					pszCur = szBuf;
					while( *pszCur == L' ' )	// 空白を読み飛ばす
						pszCur = ::CharNext(pszCur);
					if( *pszCur == L'\"' ){	// 二重引用部で始まっている
						::lstrcpyn(pData->m_szPath, pData->m_pOf->lpstrFile, _MAX_PATH);
					}
					else{
						_wsplitpath( pData->m_pOf->lpstrFile, NULL, NULL, NULL, szDefExt );
						if( szDefExt[0] == L'.' /* && szDefExt[1] != L'\0' */ ){	// 既に拡張子がついている	2文字目のチェックの削除	2008/6/14 Uchi
							// .のみの場合にも拡張子付きとみなす。
							lstrcpyn(pData->m_szPath, pData->m_pOf->lpstrFile, _MAX_PATH);
						}
						else{
							switch( pData->m_pOf->nFilterIndex ){	// 選択されているファイルの種類
							case 1:		// ユーザー定義
								pszCur = pData->m_pcDlgOpenFile->m_szDefaultWildCard;
								while( *pszCur != L'.' && *pszCur != L'\0' )	// '.'まで読み飛ばす
									pszCur = ::CharNext(pszCur);
								i = 0;
								while( *pszCur != L';' && *pszCur != L'\0' ){	// ';'までコピーする
									pszNext = ::CharNext(pszCur);
									while( pszCur < pszNext )
										szDefExt[i++] = *pszCur++;
								}
								szDefExt[i] = L'\0';
								if( ::wcslen(szDefExt) < 2 || szDefExt[1] == L'*' )	// 無効な拡張子?
									szDefExt[0] = L'\0';
								break;
							case 2:		// *.txt
								::wcscpy(szDefExt, L".txt");
								break;
							case 3:		// *.*
							default:	// 不明
								szDefExt[0] = L'\0';
								break;
							}
							lstrcpyn(szBuf, pData->m_pOf->lpstrFile, _MAX_PATH + 1);
							::wcscat(szBuf, szDefExt);
							lstrcpyn(pData->m_szPath, szBuf, _MAX_PATH);
						}
					}

					// ファイルの上書き確認を自前で行う	// 2006.11.10 ryoji
					if( IsFileExists(pData->m_szPath, true) ){
						WCHAR szText[_MAX_PATH + 100];
						lstrcpyn(szText, pData->m_szPath, _MAX_PATH);
						::wcscat(szText, LS(STR_DLGOPNFL2));
						if( IDYES != ::MessageBox( pData->m_hwndOpenDlg, szText, LS(STR_DLGOPNFL3), MB_YESNO | MB_ICONEXCLAMATION) ){
							::SetWindowLongPtr( hdlg, DWLP_MSGRESULT, TRUE );
							return TRUE;
						}
					}
				}

				/* 文字コード選択コンボボックス 値を取得 */
				nIdx = Combo_GetCurSel( pData->m_hwndComboCODES );
				lRes = Combo_GetItemData( pData->m_hwndComboCODES, nIdx );
				pData->m_nCharCode = (ECodeType)lRes;	/* 文字コード */
				//	Feb. 9, 2001 genta
				if( pData->m_bUseEol ){
					nIdx = Combo_GetCurSel( pData->m_hwndComboEOL );
					lRes = Combo_GetItemData( pData->m_hwndComboEOL, nIdx );
					pData->m_cEol = (EEolType)lRes;	/* 文字コード */
				}
				//	From Here Jul. 26, 2003 ryoji
				//	BOMチェックボックスの状態を取得
				if( pData->m_bUseBom ){
					lRes = BtnCtl_GetCheck( pData->m_hwndCheckBOM );
					pData->m_bBom = (lRes == BST_CHECKED);	/* BOM */
				}
				//	To Here Jul. 26, 2003 ryoji

//				MYTRACE( L"文字コード  lRes=%d\n", lRes );
//				MYTRACE( L"pofn->hdr.code=CDN_FILEOK        \n" );break;
			}
			break;	/* CDN_FILEOK */

		case CDN_FOLDERCHANGE  :
//			MYTRACE( L"pofn->hdr.code=CDN_FOLDERCHANGE  \n" );
			{
				wchar_t szFolder[_MAX_PATH];
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				lRes = CommDlg_OpenSave_GetFolderPath( pData->m_hwndOpenDlg, szFolder, _countof( szFolder ) );
			}
//			MYTRACE( L"\tlRes=%d\pszFolder=[%ls]\n", lRes, szFolder );

			break;
		case CDN_SELCHANGE :
			{
				CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				if( pData->m_ofn.Flags & OFN_ALLOWMULTISELECT ){
					DWORD nLength = CommDlg_OpenSave_GetSpec( pData->m_hwndOpenDlg, NULL, 0 );
					nLength += _MAX_PATH + 2;
					if( pData->m_ofn.nMaxFile < nLength ){
						delete [] pData->m_ofn.lpstrFile;
						pData->m_ofn.lpstrFile = new WCHAR[nLength];
						pData->m_ofn.nMaxFile = nLength;
					}
				}
			}
			// MYTRACE( L"pofn->hdr.code=CDN_SELCHANGE     \n" );
			break;
//		case CDN_HELP			:	MYTRACE( L"pofn->hdr.code=CDN_HELP          \n" );break;
//		case CDN_INITDONE		:	MYTRACE( L"pofn->hdr.code=CDN_INITDONE      \n" );break;
//		case CDN_SHAREVIOLATION	:	MYTRACE( L"pofn->hdr.code=CDN_SHAREVIOLATION\n" );break;
//		case CDN_TYPECHANGE		:	MYTRACE( L"pofn->hdr.code=CDN_TYPECHANGE    \n" );break;
//		default:					MYTRACE( L"pofn->hdr.code=???\n" );break;
		}

//		MYTRACE( L"=======================\n" );
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
					nIdx = Combo_GetCurSel( (HWND) lParam );
					lRes = Combo_GetItemData( (HWND) lParam, nIdx );
					CCodeTypeName	cCodeTypeName( lRes );
					if (cCodeTypeName.UseBom()) {
						::EnableWindow( pData->m_hwndCheckBOM, TRUE );
						if (lRes == pData->m_nCharCode){
							fCheck = pData->m_bBom? BST_CHECKED: BST_UNCHECKED;
						}else{
							fCheck = cCodeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
						}
					}
					else {
						::EnableWindow( pData->m_hwndCheckBOM, FALSE );
						fCheck = BST_UNCHECKED;
					}
					BtnCtl_SetCheck( pData->m_hwndCheckBOM, fCheck );
				}
				break;
			//	To Here Jul. 26, 2003 ryoji
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				{
					CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					WCHAR	szWork[_MAX_PATH + 1];
					nIdx = Combo_GetCurSel( (HWND) lParam );

					if( CB_ERR != Combo_GetLBText( (HWND) lParam, nIdx, szWork ) ){
						// 2005.11.02 ryoji ファイル名指定のコントロールを確認する
						hwndFilebox = ::GetDlgItem( pData->m_hwndOpenDlg, cmb13 );		// ファイル名コンボ（Windows 2000タイプ）
						if( !::IsWindow( hwndFilebox ) )
							hwndFilebox = ::GetDlgItem( pData->m_hwndOpenDlg, edt1 );	// ファイル名エディット（レガシータイプ）
						if( ::IsWindow( hwndFilebox ) ){
							::SetWindowText( hwndFilebox, szWork );
							if( IDC_COMBO_OPENFOLDER == wID )
								::PostMessageAny( hwndFilebox, WM_KEYDOWN, VK_RETURN, (LPARAM)0 );
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
					if ( Combo_GetCount( pData->m_hwndComboMRU ) == 0) {
						/* 最近開いたファイル コンボボックス初期値設定 */
						//	2003.06.22 Moca m_vMRU がNULLの場合を考慮する
						int nSize = (int)pData->m_pcDlgOpenFile->m_vMRU.size();
						for( i = 0; i < nSize; i++ ){
							Combo_AddString( pData->m_hwndComboMRU, pData->m_pcDlgOpenFile->m_vMRU[i] );
						}
					}
					CDialog::OnCbnDropDown( hwndCtl, true );
					break;

				case IDC_COMBO_OPENFOLDER:
					if ( Combo_GetCount( pData->m_hwndComboOPENFOLDER ) == 0) {
						/* 最近開いたフォルダ コンボボックス初期値設定 */
						//	2003.06.22 Moca m_vOPENFOLDER がNULLの場合を考慮する
						int nSize = (int)pData->m_pcDlgOpenFile->m_vOPENFOLDER.size();
						for( i = 0; i < nSize; i++ ){
							Combo_AddString( pData->m_hwndComboOPENFOLDER, pData->m_pcDlgOpenFile->m_vOPENFOLDER[i] );
						}
					}
					CDialog::OnCbnDropDown( hwndCtl, true );
					break;
				}
				break;	/* CBN_DROPDOWN */
			}
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_CP:
				{
					CDlgOpenFileData* pData = (CDlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					if( IsDlgButtonCheckedBool( hdlg, IDC_CHECK_CP ) ){
						AddComboCodePages( hdlg, pData->m_hwndComboCODES, -1, pData->m_bInitCodePage );
					}
				}
				break;
			}
			break;	// BN_CLICKED
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

int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit)
{
	int nSel = -1;
	if( !bInit ){
		::EnableWindow( GetDlgItem( hdlg, IDC_CHECK_CP ), FALSE );
		// コードページ追加
		bInit = true;
		nSel = CCodePage::AddComboCodePages(hdlg, combo, nSelCode);
	}
	return nSel;
}

/*! コンストラクタ
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
CDlgOpenFile_CommonFileDialog::CDlgOpenFile_CommonFileDialog()
{
	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	WCHAR	szFile[_MAX_PATH + 1];
	WCHAR	szDrive[_MAX_DRIVE];
	WCHAR	szDir[_MAX_DIR];
	::GetModuleFileName(
		NULL,
		szFile, _countof( szFile )
	);
	_wsplitpath( szFile, szDrive, szDir, NULL, NULL );
	wcscpy( m_szInitialDir, szDrive );
	wcscat( m_szInitialDir, szDir );

	wcscpy( m_szDefaultWildCard, L"*.*" );	/*「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */

	return;
}

/* 初期化 */
void CDlgOpenFile_CommonFileDialog::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const WCHAR*				pszUserWildCard,
	const WCHAR*				pszDefaultPath,
	const std::vector<LPCWSTR>& vMRU,
	const std::vector<LPCWSTR>& vOPENFOLDER
)
{
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* ユーザー定義ワイルドカード（保存時の拡張子補完でも使用される） */
	if( NULL != pszUserWildCard ){
		wcscpy( m_szDefaultWildCard, pszUserWildCard );
	}

	/* 「開く」での初期フォルダ */
	if( pszDefaultPath && pszDefaultPath[0] != L'\0' ){	//現在編集中のファイルのパス	//@@@ 2002.04.18
		WCHAR szDrive[_MAX_DRIVE];
		WCHAR szDir[_MAX_DIR];
		//	Jun. 23, 2002 genta
		my_splitpath_t( pszDefaultPath, szDrive, szDir, NULL, NULL );
		// 2010.08.28 相対パス解決
		WCHAR szRelPath[_MAX_PATH];
		auto_sprintf( szRelPath, L"%s%s", szDrive, szDir );
		const WCHAR* p = szRelPath;
		if( ! ::GetLongFileName( p, m_szInitialDir ) ){
			wcscpy(m_szInitialDir, p );
		}
	}
	m_vMRU = vMRU;
	m_vOPENFOLDER = vOPENFOLDER;
	return;
}

/*!
	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool CDlgOpenFile_CommonFileDialog::DoModal_GetOpenFileName( WCHAR* pszPath, EFilter eAddFilter )
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME1), m_szDefaultWildCard );

	switch( eAddFilter ){
	case EFITER_TEXT:
		cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME2), L"*.txt" );
		break;
	case EFITER_MACRO:
		cFileExt.AppendExtRaw( L"Macros", L"*.js;*.vbs;*.ppa;*.mac" );
		cFileExt.AppendExtRaw( L"JScript", L"*.js" );
		cFileExt.AppendExtRaw( L"VBScript", L"*.vbs" );
		cFileExt.AppendExtRaw( L"Pascal", L"*.ppa" );
		cFileExt.AppendExtRaw( L"Key Macro", L"*.mac" );
		break;
	case EFITER_NONE:
	default:
		break;
	}

	if( 0 != wcscmp(m_szDefaultWildCard, L"*.*") ){
		cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME3), L"*.*" );
	}

	/* 構造体の初期化 */
	std::unique_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());

	pData->m_ofn.hwndOwner = m_hwndParent;
	pData->m_ofn.hInstance = CSelectLang::getLangRsrcInstance();
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	// From Here Jun. 23, 2002 genta
	// 「開く」での初期フォルダチェック強化
// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	{
		WCHAR szDrive[_MAX_DRIVE];
		WCHAR szDir[_MAX_DIR];
		WCHAR szName[_MAX_FNAME];
		WCHAR szExt  [_MAX_EXT];

		//	Jun. 23, 2002 Thanks to sui
		my_splitpath_t( pszPath, szDrive, szDir, szName, szExt );

		//	指定されたファイルが存在しないとき szName == NULL
		//	ファイルの場所にディレクトリを指定するとエラーになるので
		//	ファイルが無い場合は全く指定しないことにする．
		if( szName[0] == L'\0' ){
			pszPath[0] = L'\0';
		}
		else {
			WCHAR szRelPath[_MAX_PATH];
			auto_sprintf( szRelPath, L"%s%s%s%s", szDrive, szDir, szName, szExt );
			const WCHAR* p = szRelPath;
			if( ! ::GetLongFileName( p, pszPath ) ){
				wcscpy( pszPath, p );
			}
		}
	}
	pData->m_ofn.lpstrFile = pszPath;
	// To Here Jun. 23, 2002 genta
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_szInitialDir;
	pData->m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	pData->m_ofn.lpstrDefExt = L""; // 2005/02/20 novice 拡張子を省略したら補完する

	// 2010.08.28 Moca DLLが読み込まれるので移動
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

	@date 2002/08/21 カレントディレクトリを変更するかどうかのオプションを追加
	@date 2003.05.12 MIK 拡張子フィルタでタイプ別設定の拡張子を使うように。
		拡張子フィルタの管理をCFileExtクラスで行う。
	@date 2005.02.20 novice 拡張子を省略したら補完する
*/
bool CDlgOpenFile_CommonFileDialog::DoModal_GetSaveFileName( WCHAR* pszPath )
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME1), m_szDefaultWildCard );
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME2), L"*.txt" );
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME3), L"*.*" );
	
	// 2010.08.28 カレントディレクトリを移動するのでパス解決する
	if( pszPath[0] ){
		WCHAR szFullPath[_MAX_PATH];
		const WCHAR* pOrg = pszPath;
		if( ::GetLongFileName( pOrg, szFullPath ) ){
			// 成功。書き戻す
			wcscpy( pszPath , szFullPath );
		}
	}

	/* 構造体の初期化 */
	std::unique_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_hwndParent;
	pData->m_ofn.hInstance = CSelectLang::getLangRsrcInstance();
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pszPath; // 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_szInitialDir;
	pData->m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	pData->m_ofn.lpstrDefExt = L"";	// 2005/02/20 novice 拡張子を省略したら補完する

	// 2010.08.28 Moca DLLが読み込まれるので移動
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
bool CDlgOpenFile_CommonFileDialog::DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFileNames,
	bool bOptions
)
{
	std::unique_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	pData->m_bIsSaveDialog = FALSE;	/* 保存のダイアログか */

	bool bMultiSelect = pFileNames != NULL;

	// ファイルの種類	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME3), L"*.*" );
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME2), L"*.txt" );
	for( int i = 0; i < GetDllShareData().m_nTypesCount; i++ ){
		const STypeConfigMini* type = NULL;
		if( !CDocTypeManager().GetTypeConfigMini( CTypeConfig( i ), &type ) ){
			continue;
		}
		cFileExt.AppendExt( type->m_szTypeName, type->m_szTypeExts );
	}

	//メンバの初期化
	pData->m_bViewMode = pLoadInfo->bViewMode;
	pData->m_nCharCode = pLoadInfo->eCharCode;	/* 文字コード自動判別 */
	pData->m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	pData->m_bUseCharCode = true;
	pData->m_bUseEol = false;	//	Feb. 9, 2001 genta
	pData->m_bUseBom = false;	//	Jul. 26, 2003 ryoji

	//ファイルパス受け取りバッファ
	WCHAR* pszPathBuf = new WCHAR[2000];
	wcscpy(pszPathBuf, pLoadInfo->cFilePath); // 2013.05.27 デフォルトファイル名を設定する

	//OPENFILENAME構造体の初期化
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_hwndParent;
	pData->m_ofn.hInstance = CSelectLang::getLangRsrcInstance();
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pszPathBuf;
	pData->m_ofn.nMaxFile = 2000;
	pData->m_ofn.lpstrInitialDir = m_szInitialDir;
	pData->m_ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( pData->m_bViewMode ) pData->m_ofn.Flags |= OFN_READONLY;
	if( bMultiSelect ){
		pData->m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	}
	pData->m_ofn.lpstrDefExt = L"";	// 2005/02/20 novice 拡張子を省略したら補完する
	if( bOptions == false ){
		pData->m_ofn.Flags |= OFN_HIDEREADONLY;
		pData->m_bUseCharCode = false;
	}

	//カレントディレクトリを保存。関数を抜けるときに自動でカレントディレクトリは復元されます。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	//ダイアログ表示
	bool bDlgResult = _GetOpenFileNameRecover( &pData->m_ofn );
	if( bDlgResult ){
		if( bMultiSelect ){
			pLoadInfo->cFilePath = L"";
			if( pData->m_ofn.nFileOffset < wcslen( pData->m_ofn.lpstrFile ) ){
				pFileNames->push_back( pData->m_ofn.lpstrFile );
			}else{
				std::wstring path;
				WCHAR* pos = pData->m_ofn.lpstrFile;
				pos += wcslen(pos) + 1;
				while( *pos != L'\0' ){
					path = pData->m_ofn.lpstrFile;
					path.append( L"\\" );
					path.append( pos );
					pFileNames->push_back( path );
					pos += wcslen(pos) + 1;
				}
			}
		}else{
			pLoadInfo->cFilePath = pData->m_ofn.lpstrFile;
		}
		pLoadInfo->eCharCode = pData->m_nCharCode;
		pLoadInfo->bViewMode = pData->m_bViewMode;
	}
	else{
		DlgOpenFail();
	}
	delete [] pData->m_ofn.lpstrFile;
	return bDlgResult;
}

/*! 保存ダイアログ モーダルダイアログの表示

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
bool CDlgOpenFile_CommonFileDialog::DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode
)
{
	std::unique_ptr<CDlgOpenFileData> pData( new CDlgOpenFileData() );
	pData->m_bIsSaveDialog = TRUE;	/* 保存のダイアログか */

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME1), m_szDefaultWildCard );
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME2), L"*.txt" );
	cFileExt.AppendExtRaw( LS(STR_DLGOPNFL_EXTNAME3), L"*.*" );

	// ファイル名の初期設定	// 2006.11.10 ryoji
	if( pSaveInfo->cFilePath[0] == L'\0' )
		lstrcpyn(pSaveInfo->cFilePath, LS(STR_NO_TITLE2), _MAX_PATH);	// 無題

	//OPENFILENAME構造体の初期化
	InitOfn( &pData->m_ofn );		// 2005.10.29 ryoji
	pData->m_pcDlgOpenFile = this;
	pData->m_ofn.lCustData = (LPARAM)(pData.get());
	pData->m_ofn.hwndOwner = m_hwndParent;
	pData->m_ofn.hInstance = CSelectLang::getLangRsrcInstance();
	pData->m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	pData->m_ofn.lpstrFile = pSaveInfo->cFilePath;	// 2005/02/20 novice デフォルトのファイル名は何も設定しない
	pData->m_ofn.nMaxFile = _MAX_PATH;
	pData->m_ofn.lpstrInitialDir = m_szInitialDir;
	pData->m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( !bSimpleMode ){
		pData->m_ofn.Flags = pData->m_ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
		pData->m_ofn.Flags &= ~OFN_OVERWRITEPROMPT;	// 2006.11.10 ryoji 上書き確認もフックの中で自前で処理する
	}

// 2005/02/20 novice 拡張子を省略したら補完する
//	pData->m_ofn.lpstrDefExt = L"";
	pData->m_ofn.lpstrDefExt = (pData->m_ofn.Flags & OFN_ENABLEHOOK)? NULL: L"";	// 2006.11.10 ryoji フックを使うときは自前で拡張子を補完する

	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	pData->m_nCharCode = pSaveInfo->eCharCode;

	//	From Here Feb. 9, 2001 genta
	if(!bSimpleMode){
		pData->m_cEol = pSaveInfo->cEol;	//	初期値は「改行コードを保存」に固定 // 2013.05.27 初期値を指定
		pData->m_bUseEol = true;
	}
	else{
		pData->m_bUseEol = false;
	}

	//	To Here Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOM設定
	if(!bSimpleMode){
		pData->m_bBom = pSaveInfo->bBomExist;
		pData->m_bUseBom = true;
		pData->m_bUseCharCode = true;
	}
	else{
		pData->m_bUseBom = false;
		pData->m_bUseCharCode = false;
	}

	pData->m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	if( GetSaveFileNameRecover( &pData->m_ofn ) ){
		pSaveInfo->cFilePath = pData->m_ofn.lpstrFile;
		if( pData->m_ofn.Flags & OFN_ENABLEHOOK )
			lstrcpyn(pSaveInfo->cFilePath, pData->m_szPath, _MAX_PATH);	// 自前で拡張子の補完を行ったときのファイルパス	// 2006.11.10 ryoji

		pSaveInfo->eCharCode = pData->m_nCharCode;

		//	Feb. 9, 2001 genta
		if( pData->m_bUseEol ){
			pSaveInfo->cEol = pData->m_cEol;
		}
		//	Jul. 26, 2003 ryoji BOM設定
		if( pData->m_bUseBom ){
			pSaveInfo->bBomExist = pData->m_bBom;
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
void CDlgOpenFile_CommonFileDialog::DlgOpenFail(void)
{
	const WCHAR*	pszError;
	DWORD dwError = ::CommDlgExtendedError();
	if( dwError == 0 ){
		//	ユーザキャンセルによる
		return;
	}
	
	switch( dwError ){
	case CDERR_DIALOGFAILURE  : pszError = L"CDERR_DIALOGFAILURE  "; break;
	case CDERR_FINDRESFAILURE : pszError = L"CDERR_FINDRESFAILURE "; break;
	case CDERR_NOHINSTANCE    : pszError = L"CDERR_NOHINSTANCE    "; break;
	case CDERR_INITIALIZATION : pszError = L"CDERR_INITIALIZATION "; break;
	case CDERR_NOHOOK         : pszError = L"CDERR_NOHOOK         "; break;
	case CDERR_LOCKRESFAILURE : pszError = L"CDERR_LOCKRESFAILURE "; break;
	case CDERR_NOTEMPLATE     : pszError = L"CDERR_NOTEMPLATE     "; break;
	case CDERR_LOADRESFAILURE : pszError = L"CDERR_LOADRESFAILURE "; break;
	case CDERR_STRUCTSIZE     : pszError = L"CDERR_STRUCTSIZE     "; break;
	case CDERR_LOADSTRFAILURE : pszError = L"CDERR_LOADSTRFAILURE "; break;
	case FNERR_BUFFERTOOSMALL : pszError = L"FNERR_BUFFERTOOSMALL "; break;
	case CDERR_MEMALLOCFAILURE: pszError = L"CDERR_MEMALLOCFAILURE"; break;
	case FNERR_INVALIDFILENAME: pszError = L"FNERR_INVALIDFILENAME"; break;
	case CDERR_MEMLOCKFAILURE : pszError = L"CDERR_MEMLOCKFAILURE "; break;
	case FNERR_SUBCLASSFAILURE: pszError = L"FNERR_SUBCLASSFAILURE"; break;
	default: pszError = L"UNKNOWN_ERRORCODE"; break;
	}

	ErrorBeep();
	TopErrorMessage( m_hwndParent,
		LS(STR_DLGOPNFL_ERR1),
		pszError
	);
}

/*! OPENFILENAME 初期化

	OPENFILENAME に CDlgOpenFile クラス用の初期規定値を設定する

	@author ryoji
	@date 2005.10.29
*/
void CDlgOpenFile_CommonFileDialog::InitOfn( OPENFILENAME* ofn )
{
	memset_raw(ofn, 0, sizeof(*ofn));

	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->lpfnHook = OFNHookProc;
	ofn->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);	// <-L"IDD_FILEOPEN"; 2008/7/26 Uchi
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
void CDlgOpenFile_CommonFileDialog::InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl )
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

/*! リトライ機能付き GetOpenFileName
	@author Moca
	@date 2006.09.03 新規作成
*/
bool CDlgOpenFile_CommonFileDialog::_GetOpenFileNameRecover( OPENFILENAME* ofn )
{
	BOOL bRet = ::GetOpenFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			ofn->lpstrFile[0] = L'\0';
			ofn->lpstrInitialDir = L"";
			bRet = ::GetOpenFileName( ofn );
		}
	}
	return bRet!=FALSE;
}

/*! リトライ機能付き GetSaveFileName
	@author Moca
	@date 2006.09.03 新規作成
*/
bool CDlgOpenFile_CommonFileDialog::GetSaveFileNameRecover( OPENFILENAME* ofn )
{
	BOOL bRet = ::GetSaveFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			ofn->lpstrFile[0] = L'\0';
			ofn->lpstrInitialDir = L"";
			bRet = ::GetSaveFileName( ofn );
		}
	}
	return bRet!=FALSE;
}

std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonFileDialog()
{
	std::shared_ptr<IDlgOpenFile> ret(new CDlgOpenFile_CommonFileDialog());
	return ret;
}
