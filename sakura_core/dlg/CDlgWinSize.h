/*!	@file
	@brief ウィンドウの位置と大きさダイアログ

	@author Moca
	@date 2004/05/13 作成
*/
/*
	Copyright (C) 2004, Moca

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

#ifndef SC_CDLGWINPOSSIZE_H__
#define SC_CDLGWINPOSSIZE_H__

#include "dlg/CDialog.h"
#include "env/CommonSetting.h"

/*!	@brief 位置と大きさの設定ダイアログ

	共通設定のウィンドウ設定で，ウィンドウ位置を指定するために補助的に
	使用されるダイアログボックス
*/
class CDlgWinSize : public CDialog
{
public:
	CDlgWinSize();
	~CDlgWinSize();
	int DoModal( HINSTANCE, HWND, EWinSizeMode&, EWinSizeMode&, int&, RECT& );	//!< モーダルダイアログの表示

protected:

	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	int  GetData( void );
	void SetData( void );
	LPVOID GetHelpIdTable( void );

	void RenewItemState( void );

private:
	EWinSizeMode	m_eSaveWinSize;	//!< ウィンドウサイズの保存: 0/デフォルト，1/継承，2/指定
	EWinSizeMode	m_eSaveWinPos;	//!< ウィンドウ位置の保存: 0/デフォルト，1/継承，2/指定
	int				m_nWinSizeType;	//!< ウィンドウ表示方法: 0/標準，1/最大化，2/最小化
	RECT			m_rc;
};

#endif


