//	$Id$
/************************************************************************

	CDlgOpenFile.h

    ファイルオープンダイアログ
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/08/10  新規作成


************************************************************************/

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "CShareData.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
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
	void Create( HINSTANCE, HWND, const char*, const char*, const char**,const char** );
	BOOL DoModal_GetOpenFileName( char* );	/* 開くダイアログ　モーダルダイアログの表示 */
	BOOL DoModal_GetSaveFileName( char* );	/* 保存ダイアログ　モーダルダイアログの表示 */
	BOOL DoModalOpenDlg( char* , int*, BOOL* );	/* 開くダイアログ　モーダルダイアログの表示 */
	BOOL DoModalSaveDlg( char* , int* );	/* 保存ダイアログ　モーダルダイアログの表示 */

	BOOL DispatchEvent(	HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hWnd;			/* このダイアログのハンドル */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* 「開く」での最初のワイルドカード */
	char			m_szInitialDir[_MAX_PATH + 1];		/* 「開く」での初期ディレクトリ	*/
	OPENFILENAME	m_ofn;							/* 「ファイルを開く」ダイアログ用構造体 */
	int				m_nCharCode;					/* 文字コード */
//	char			m_szHelpFile[_MAX_PATH + 1];
//	int				m_nHelpTopicID;

protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */

/*[EOF]*/
