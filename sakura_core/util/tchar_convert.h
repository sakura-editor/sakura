﻿/*! @file */
/*
	Copyright (C) 2008, kobake
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
#ifndef SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#define SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#pragma once

#include "basis/primitive.h"

//WCHARに変換
const WCHAR* to_wchar(const ACHAR* src);
const WCHAR* to_wchar(const ACHAR* pSrcData, int nSrcLength);

//ACHARに変換
inline
const ACHAR* to_achar(const ACHAR* src){ return src; }
const ACHAR* to_achar(const WCHAR* src);
const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength);

#endif /* SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_ */
