/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "input.h"

// novice 2004/10/10 マウスサイドボタン対応
/*!
	Shift,Ctrl,Altキー状態の取得

	@retval nIdx Shift,Ctrl,Altキー状態
	@date 2004.10.10 関数化
*/
int getCtrlKeyState()
{
	int nIdx = 0;

	/* Shiftキーが押されているなら */
	if(GetKeyState_Shift()){
		nIdx |= _SHIFT;
	}
	/* Ctrlキーが押されているなら */
	if( GetKeyState_Control() ){
		nIdx |= _CTRL;
	}
	/* Altキーが押されているなら */
	if( GetKeyState_Alt() ){
		nIdx |= _ALT;
	}

	return nIdx;
}
