//	$Id$
/*!	@file
	@brief ウィンドウの位置と大きさダイアログ

	@author Moca
	@date 2004/05/13 作成
	$Revision$
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

#include "CDialog.h"

class CDlgWinSize : public CDialog
{
public:
	CDlgWinSize();
	~CDlgWinSize();
	int DoModal( HINSTANCE, HWND, int&, int&, int&, RECT& );	//!< モーダルダイアログの表示

protected:

	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	int  GetData( void );
	void SetData( void );
	LPVOID GetHelpIdTable( void );

	void RenewItemState( void );

	int m_nSaveWinSize;
	int m_nSaveWinPos;
	int m_nWinSizeType;
	RECT m_rc;
};

#endif

/*[EOF]*/
