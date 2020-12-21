/*! @file */
// 2007.10.20 kobake 書式関連
/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#define SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#pragma once

#include <string>
#include <string_view>

// 20051121 aroka
std::wstring GetDateTimeFormat( std::wstring_view format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const WCHAR* ver );	//バージョン番号の解析
int CompareVersion( const WCHAR* verA, const WCHAR* verB );	//バージョン番号の比較
#endif /* SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_ */
