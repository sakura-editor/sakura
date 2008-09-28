/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, ryoji
	Copyright (C) 2004, genta, MIK
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "env/CShareData.h"
#include "CEol.h"
#include "COsVersionInfo.h"	// 2005.11.02 ryoji

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
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif // (_WIN32_WINNT >= 0x0500)


/*!	ファイルオープンダイアログボックス

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CDlgOpenFile
{
public:
	//コンストラクタ・デストラクタ
	CDlgOpenFile();
	~CDlgOpenFile();
	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const TCHAR*				pszUserWildCard,
		const TCHAR*				pszDefaultPath,
		const std::vector<LPCTSTR>& vMRU			= std::vector<LPCTSTR>(),
		const std::vector<LPCTSTR>& vOPENFOLDER		= std::vector<LPCTSTR>()
	);

	//操作
	bool DoModal_GetOpenFileName( TCHAR*, bool bSetCurDir = false );	/* 開くダイアログ モーダルダイアログの表示 */	//2002/08/21 moca	引数追加
	bool DoModal_GetSaveFileName( TCHAR*, bool bSetCurDir = false );	/* 保存ダイアログ モーダルダイアログの表示 */	//2002/08/21 30,2002 moca	引数追加
	bool DoModalOpenDlg( SLoadInfo* pLoadInfo );	/* 開くダイアグ モーダルダイアログの表示 */
	bool DoModalSaveDlg( SSaveInfo*	pSaveInfo, bool bSimpleMode );	/* 保存ダイアログ モーダルダイアログの表示 */

public:
	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hWnd;			/* このダイアログのハンドル */

	DLLSHAREDATA*	m_pShareData;

	SFilePath		m_szDefaultWildCard;	/* 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */
	SFilePath		m_szInitialDir;			/* 「開く」での初期ディレクトリ */
	OPENFILENAMEZ	m_ofn;							/* 2005.10.29 ryoji OPENFILENAMEZ「ファイルを開く」ダイアログ用構造体 */
	ECodeType		m_nCharCode;					/* 文字コード */

	CEol			m_cEol;		//	Feb. 9, 2001 genta
	bool			m_bUseEol;	//	Feb. 9, 2001 genta
	
	bool			m_bBom;		//!< BOMを付けるかどうか	//	Jul. 26, 2003 ryoji BOM
	bool			m_bUseBom;	//!< BOMの有無を選択する機能を利用するかどうか

	SFilePath		m_szPath;	// 拡張子の補完を自前で行ったときのファイルパス	// 2006.11.10 ryoji

protected:
	/*
	||  実装ヘルパ関数
	*/

	//	May 29, 2004 genta エラー処理をまとめる (advised by MIK)
	void	DlgOpenFail(void);

	// 2005.11.02 ryoji OS バージョン対応の OPENFILENAME 初期化用関数
	static COsVersionInfo m_cOsVer;
	BOOL IsOfnV5( void ) { return ( m_cOsVer.GetVersion() && (m_cOsVer.IsWin2000_or_later() || m_cOsVer.IsWinMe()) ); }
	void InitOfn( OPENFILENAMEZ* );

	// 2005.11.02 ryoji 初期レイアウト設定処理
	static void InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl );

	// 2005.10.29 ryoji コンボボックスのドロップダウン時処理
	static void OnCmbDropdown( HWND hwnd );

	// 2006.09.03 Moca ファイルダイアログのエラー回避
	//! リトライ機能付き GetOpenFileName
	bool _GetOpenFileNameRecover( OPENFILENAMEZ* ofn );
	//! リトライ機能付き GetOpenFileName
	bool GetSaveFileNameRecover( OPENFILENAMEZ* ofn );

	friend UINT_PTR CALLBACK OFNHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */



