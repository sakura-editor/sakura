/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()に微妙～な修正
	@date 2002/01/17 aroka 型の修正
	@date 2013/03/03 Uchi MessageBox用関数を分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "debug/Debug1.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(_DEBUG) || defined(USE_RELPRINT)

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief 書式付きデバッガ出力

	@param[in] lpFmt printfの書式付き文字列

	引数で与えられた情報をDebugStringとして出力する．
*/
void DebugOutW(LPCWSTR lpFmt, ...)
{
    static WCHAR szText[16000];

    va_list argList;
    va_start(argList, lpFmt);

    //整形
    int ret = _vsnwprintf_s(szText, _TRUNCATE, lpFmt, argList);

    //出力
    if (errno != EINVAL)
    {
        ::OutputDebugStringW(szText);
    }

    //切り捨て対策
    if (-1 == ret && errno != ERANGE)
    {
        ::OutputDebugStringW(L"(切り捨てました...)\n");

        ::DebugBreak();

        int count      = _vscwprintf(lpFmt, argList);
        auto pLargeBuf = std::make_unique<WCHAR[]>(count + 1);
        if (vswprintf(&pLargeBuf[0], count + 1, lpFmt, argList) > 0)
            ::OutputDebugStringW(&pLargeBuf[0]);
    }

    va_end(argList);

    //ウェイト
    ::Sleep(1); // Norio Nakatani, 2001/06/23 大量にトレースするときのために
}

#endif // _DEBUG || USE_RELPRINT
