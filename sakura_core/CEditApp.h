//	$Id$
/*!	@file
	MRU、キー割り当て、共通設定、編集ウィンドウの管理

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_





#include <windows.h>
#include "CEditWnd.h"
#include "CKeyBind.h"
#include "CShareData.h"
#include "CMenuDrawer.h"







/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CEditApp
{
public:
	/*
	||  Constructors
	*/
	CEditApp();
	~CEditApp();

	/*
	|| メンバ関数
	*/
	HWND Create( HINSTANCE );	/* 作成 */
	bool CreateTrayIcon( HWND );	// 20010412 by aroka
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */
	void MessageLoop( void );	/* メッセージループ */
	int	CreatePopUpMenu_L( void );	/* ポップアップメニュー(トレイ左ボタン) */
	int	CreatePopUpMenu_R( void );	/* ポップアップメニュー(トレイ右ボタン) */

	static bool OpenNewEditor( HINSTANCE, HWND, char*, int, BOOL, bool sync = false );		/* 新規編集ウィンドウの追加 ver 0 */
	static bool OpenNewEditor2( HINSTANCE, HWND , FileInfo*, BOOL, bool sync = false );	/* 新規編集ウィンドウの追加 ver 1 */
//シングルプロセス版用
//	static HWND OpenNewEditor3( HINSTANCE, HWND , const char*, BOOL );	/* 新規編集ウィンドウの追加 ver 2 */

	static BOOL CloseAllEditor( void );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	static void TerminateApplication( void );	/* サクラエディタの全終了 */
	/* コマンドラインの解析 */
	static void CEditApp::ParseCommandLine(
		const char*	pszCmdLineSrc,
		BOOL*		pbGrepMode,
		CMemory*	pcmGrepKey,
		CMemory*	pcmGrepFile,
		CMemory*	pcmGrepFolder,
		BOOL*		pbGrepSubFolder,
		BOOL*		pbGrepLoHiCase,
		BOOL*		pbGrepRegularExp,
		BOOL*		pbGrepKanjiCode_AutoDetect,
		BOOL*		pbGrepOutputLine,
		BOOL*		pbGrepWordOnly,
		int	*		pnGrepOutputStyle,
		BOOL*		pbDebugMode,
		BOOL*		pbNoWindow,
		FileInfo*	pfi,
		BOOL*		pbReadOnly
	);

	/*
	|| メンバ変数
	*/
//	CKeyBind		m_CKeyBind;
//	HACCEL			m_hAccel;

private:
	CMenuDrawer		m_CMenuDrawer;
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	BOOL			m_bCreatedTrayIcon;	/* トレイにアイコンを作った */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;
	CDlgGrep		m_cDlgGrep; // Jul. 2, 2001 genta

	CImageListMgr	m_hIcons;

	void	DoGrep();	//Stonee, 2001/03/21
	//	Apr. 6, 2001 genta コマンドラインオプションの解析
	static int CheckCommandLine( char *str, char** arg );
	//	Apr. 24, 2001 genta
	//	RegisterMessageで得られるMessage IDの保管場所
	UINT	m_uCreateTaskBarMsg;

	/*
	|| 実装ヘルパ系
	*/
protected:
	BOOL TrayMessage(HWND , DWORD , UINT , HICON , const char* );	/* タスクトレイのアイコンに関する処理 */
	void OnCommand( WORD , WORD  , HWND );	/* WM_COMMANDメッセージ処理 */


};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */


/*[EOF]*/
