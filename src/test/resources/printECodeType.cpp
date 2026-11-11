/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <ostream>

#include "charset/charset.h"

/*!
 * googletestにECodeTypeを出力させる
 * 
 * パラメータテストのパラメータにECodeTypeを渡した場合に文字列を併記して分かりやすくする。
 */
void PrintTo(ECodeType eCodeType, std::ostream* os)
{
	switch (eCodeType) {
	case CODE_SJIS:			*os << "SJIS"; break;
	case CODE_JIS:			*os << "JIS"; break;
	case CODE_EUC:			*os << "EUC"; break;
	case CODE_UTF8:			*os << "UTF8"; break;
	case CODE_UTF16LE:		*os << "UTF16LE"; break;
	case CODE_UTF16BE:		*os << "UTF16BE"; break;
	case CODE_UTF32LE:		*os << "UTF32LE"; break;
	case CODE_UTF32BE:		*os << "UTF32BE"; break;
	case CODE_UTF7:			*os << "UTF7"; break;
	case CODE_CESU8:		*os << "CESU8"; break;
	case CODE_LATIN1:		*os << "LATIN1"; break;

	default:
		// 未知の値は数値で出す
		*os << std::format("ECodeType({})", static_cast<uint16_t>(eCodeType));
		break;
	}
}
