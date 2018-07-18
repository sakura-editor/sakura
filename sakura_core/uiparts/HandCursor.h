/*!	@file
	@brief 指差しカーソル

	@author Uchi
	@date 2013年1月29日
*/
/*
	Copyright (C) 2013, Uchi

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

#ifndef SAKURA_HANDCURSOR_A545F10D_9F24_4AB5_889F_13732FC0150B_H_
#define SAKURA_HANDCURSOR_A545F10D_9F24_4AB5_889F_13732FC0150B_H_

#include "_os/COsVersionInfo.h"
#include "sakura_rc.h"

#ifndef IDC_HAND
#define IDC_HAND	MAKEINTRESOURCE(32649)
#endif

inline void SetHandCursor ()
{
#ifdef IDC_CURSOR_HAND
	if (!IsWin2000_or_later()) {
		SetCursor( LoadCursor( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
	}
	else
#endif
	{
		SetCursor( LoadCursor( NULL, IDC_HAND ) );
	}
}

#endif	// SAKURA_HANDCURSOR_A545F10D_9F24_4AB5_889F_13732FC0150B_H_
