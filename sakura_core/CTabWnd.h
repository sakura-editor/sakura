//	$Id$
/*!	@file
	@brief タブウィンドウ

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

class CTabWnd;

#ifndef _CTABWND_H_
#define _CTABWND_H_

#include "CWnd.h"
#include "CEditDoc.h"
#include "CShareData.h"

//! ファンクションキーウィンドウ
class SAKURA_CORE_API CTabWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTabWnd();
	virtual ~CTabWnd();

	/*
	|| メンバ関数
	*/
	HWND Open( HINSTANCE, HWND );		/*!< ウィンドウ オープン */
	void Close( void );					/*!< ウィンドウ クローズ */
	void TabWindowNotify( WPARAM wParam, LPARAM lParam );
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd );

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	/*
	|| メンバ変数
	*/
	LPCTSTR			m_pszClassName;	/*!< クラス名 */
	DLLSHAREDATA*	m_pShareData;	/*!< 共有データ */
	HFONT			m_hFont;		/*!< 表示用フォント */
	HWND			m_hwndTab;		/*!< タブコントロール */
	HWND			m_hwndToolTip;	/*!< ツールチップ */
	TCHAR			m_szTextTip1[1024];
#ifdef UNICODE
	//※現在扱っている文字コードとは逆の文字コードを指定する。
	char			m_szTextTip2[1024];	//!< SJIS文字列でのツールチップ
#else
	//※現在扱っている文字コードとは逆の文字コードを指定する。
	wchar_t			m_szTextTip2[1024];	//!< UNICODE文字列でのツールチップ
#endif	//UNICODE

protected:
	/*
	|| 実装ヘルパ系
	*/
	void Refresh( void );
	int FindTabIndexByHWND( HWND hWnd );
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	int GetFirstOpenedWindow( void );

	/* 仮想関数 メッセージ処理 */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE処理 */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY処理 */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY処理 */
};

#endif /* _CTABWND_H_ */

/*[EOF]*/
