﻿/*! @file */
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
#include "CMyRect.h"
#include "util/std_macro.h"

CMyRect MergeRect(const CMyRect& rc1, const CMyRect& rc2)
{
	return CMyRect(
		t_min(rc1.left  , rc2.left),
		t_min(rc1.top   , rc2.top),
		t_max(rc1.right , rc2.right),
		t_max(rc1.bottom, rc2.bottom)
	);
}
