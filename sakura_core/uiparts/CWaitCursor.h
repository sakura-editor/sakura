/*!	@file
	@brief 砂時計カーソル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CWAITCURSOR_6EAC4CB5_4D04_4501_B076_85C8A0395785_H_
#define SAKURA_CWAITCURSOR_6EAC4CB5_4D04_4501_B076_85C8A0395785_H_
#pragma once

#include <Windows.h>

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
	using Me = CWaitCursor;

public:
	/*
	||  Constructors
	*/
	CWaitCursor( HWND, bool bEnable = true );
	CWaitCursor(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWaitCursor(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CWaitCursor();

	bool IsEnable(){ return m_bEnable; }
private: // 2002/2/10 aroka
	HCURSOR	m_hCursor;
	HCURSOR	m_hCursorOld;
	bool	m_bEnable;
};
#endif /* SAKURA_CWAITCURSOR_6EAC4CB5_4D04_4501_B076_85C8A0395785_H_ */
