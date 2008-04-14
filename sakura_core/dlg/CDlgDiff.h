/*!	@file
	@brief DIFF差分表示ダイアログボックス

	@author MIK
	@date 2002.5.27
*/
/*
	Copyright (C) 2002, MIK
	Copyright (C) 2004, genta

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

class CDlgDiff;

#ifndef _CDLGDIFF_H_
#define _CDLGDIFF_H_

#include "dlg/CDialog.h"
/*!
	@brief DIFF差分表示ダイアログボックス
*/
//	Feb. 28, 2004 genta 最後に選択されていた番号を保存する
class CDlgDiff : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgDiff();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const TCHAR*, bool );	/* モーダルダイアログの表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked( int );
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnEditChange( HWND hwndCtl, int wID );
	LPVOID	GetHelpIdTable(void);

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */

private:
	int			m_nIndexSave;		// 最後に選択されていた番号

public:
	SFilePath	m_szFile1;			// 自ファイル
	SFilePath	m_szFile2;			// 相手ファイル
	bool		m_bIsModified;		// 自ファイル更新中
	bool		m_bIsModifiedDst;	// 相手ファイル更新中
	int			m_nDiffFlgOpt;		// DIFFオプション
	HWND		m_hWnd_Dst;			// 相手ウインドウハンドル

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGDIFF_H_ */

/*[EOF]*/
