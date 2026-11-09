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
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "debug/Debug1.h"

#include "basis/primitive.h"

namespace cxx {

/*!
 * @brief デバッガーに文字列を送信します。
 *
 * MSVCのデバッグウィンドウにログ出力するためのWindows API。
 *
 * 将来的にはロガーを使うべきと考えられる
 * 
 * @param[in] outputString 出力する文字列
 */
void OutputDebugStringW(
	std::wstring_view outputString
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString str{ outputString };

	// デバッガーに文字列を送信する
	::OutputDebugStringW(str.c_str());
}

} // namespace cxx
