#pragma once

#include <commctrl.h>

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �X�e�[�^�X�o�[                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT StatusBar_SetText(HWND hwndStatus, WPARAM opt, const TCHAR* str)
	{
		return ::SendMessage( hwndStatus, SB_SETTEXT, opt, (LPARAM)str );
	}

}

using namespace ApiWrap;
