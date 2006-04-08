//	$Id$
/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, ryoji
	Copyright (C) 2004, genta, MIK
	Copyright (C) 2005, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "CShareData.h"
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
	/*
	||  Constructors
	*/
	CDlgOpenFile();
	~CDlgOpenFile();
	/*
	||  Attributes & Operations
	*/
	void Create( HINSTANCE, HWND, const char*, const char*, const char** = NULL,const char** = NULL);
	//void Create( HINSTANCE, HWND, const char*, const char* );
	//	2002/08/21 moca	引数追加
	BOOL DoModal_GetOpenFileName( char*, bool bSetCurDir = false );	/* 開くダイアログ モーダルダイアログの表示 */
	//	2002/08/21 30,2002 moca	引数追加
	BOOL DoModal_GetSaveFileName( char*, bool bSetCurDir = false );	/* 保存ダイアログ モーダルダイアログの表示 */
	BOOL DoModalOpenDlg( char* , int*, BOOL* );	/* 開くダイアグ モーダルダイアログの表示 */
	//	Feb. 9, 2001 genta	引数追加
	//	Jul. 26, 2003 ryoji BOM用引数追加
	BOOL DoModalSaveDlg( char* , int*, CEOL*, BOOL* );	/* 保存ダイアログ モーダルダイアログの表示 */

//	INT_PTR DispatchEvent(	HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hWnd;			/* このダイアログのハンドル */

	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* 「開く」での最初のワイルドカード */
	char			m_szInitialDir[_MAX_PATH + 1];		/* 「開く」での初期ディレクトリ */
	OPENFILENAMEZ	m_ofn;							/* 2005.10.29 ryoji OPENFILENAMEZ「ファイルを開く」ダイアログ用構造体 */
	int				m_nCharCode;					/* 文字コード */
//	char			m_szHelpFile[_MAX_PATH + 1];
//	int				m_nHelpTopicID;
	CEOL			m_cEol;	//	Feb. 9, 2001 genta
	bool			m_bUseEol;	//	Feb. 9, 2001 genta

	//	Jul. 26, 2003 ryoji BOM
	BOOL			m_bBom;	//!< BOMを付けるかどうか
	bool			m_bUseBom;	//!< BOMの有無を選択する機能を利用するかどうか

protected:
	/*
	||  実装ヘルパ関数
	*/

	//	May 29, 2004 genta エラー処理をまとめる (advised by MIK)
	void	DlgOpenFail(void);

	// 2005.11.02 ryoji OS バージョン対応の OPENFILENAME 初期化用関数
	static COsVersionInfo m_cOsVer;
	BOOL IsOfnV5( void ) { return ( m_cOsVer.GetVersion() && (m_cOsVer.IsWin2000orLater() || m_cOsVer.IsWinMe()) ); }
	void InitOfn( OPENFILENAMEZ& );

	// 2005.11.02 ryoji 初期レイアウト設定処理
	static void InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl );

	// 2005.10.29 ryoji コンボボックスのドロップダウン時処理
	static void OnCmbDropdown( HWND hwnd );

	friend UINT_PTR CALLBACK OFNHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */


/*[EOF]*/
