//	$Id$
/*!	@file
	砂時計カーソル

	@author Norio Nakatani
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

class CWaitCursor;

#ifndef _CWAITCURSOR_H_
#define _CWAITCURSOR_H_


#include <windows.h>


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//!	砂時計カーソルクラス
/*!
	オブジェクトの存続している間カーソル形状を砂時計にする．
	オブジェクトが破棄されるとカーソル形状は元に戻る
*/
class CWaitCursor
{
public:
	/*
	||  Constructors
	*/
	CWaitCursor( HWND );
	~CWaitCursor();
public:
	HCURSOR	m_hCursor;
	HCURSOR	m_hCursorOld;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CWAITCURSOR_H_ */


/*[EOF]*/
