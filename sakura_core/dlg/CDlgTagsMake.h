/*!	@file
	@brief タグファイル作成ダイアログボックス

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK

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

class CDlgTagsMake;

#ifndef _CDLGTAGSMAKE_H_
#define _CDLGTAGSMAKE_H_

#include "dlg/CDialog.h"
/*!
	@brief タグファイル作成ダイアログボックス
*/
class CDlgTagsMake : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagsMake();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const TCHAR* );	/* モーダルダイアログの表示 */

	TCHAR	m_szPath[_MAX_PATH+1];	/* フォルダ */
	TCHAR	m_szTagsCmdLine[_MAX_PATH];	/* コマンドラインオプション(個別) */
	int		m_nTagsOpt;				/* CTAGSオプション(チェック) */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked( int );
	LPVOID	GetHelpIdTable(void);

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */

private:
	void SelectFolder( HWND hwndDlg );

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTAGSMAKE_H_ */


