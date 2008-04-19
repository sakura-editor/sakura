/*!	@file
	@brief 砂時計カーソル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
private: // 2002/2/10 aroka
	HCURSOR	m_hCursor;
	HCURSOR	m_hCursorOld;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CWAITCURSOR_H_ */



