/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
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
#include "ole_convert.h"

// VARIANT変数をBSTRとみなし、wstringに変換する
// CMacro::HandleFunctionを参考とした。
bool variant_to_wstr( VARIANT v, std::wstring& wstr )
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用
	if(VariantChangeType(&varCopy.Data, &v, 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈

	wchar_t *Source;
	int SourceLength;
	Wrap(&varCopy.Data.bstrVal)->GetW(&Source, &SourceLength);

	wstr.assign( Source, SourceLength );
	delete[] Source;

	return true;
}

// VARIANT変数を整数とみなし、intに変換する
// CMacro::HandleFunctionを参考とした。
bool variant_to_int( VARIANT v, int& n )
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用
	if(VariantChangeType(&varCopy.Data, &v, 0, VT_I4) != S_OK) return false;	// VT_I4として解釈

	n = varCopy.Data.lVal;
	return true;
}
