/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
#pragma once

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr(VARIANT v, std::wstring &wstr); // VARIANT変数をBSTRとみなし、wstringに変換する
bool variant_to_int(VARIANT v, int &n); // VARIANT変数を整数とみなし、intに変換する
