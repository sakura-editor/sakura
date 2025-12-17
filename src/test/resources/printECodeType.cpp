/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <ostream>

#include "charset/charset.h"

/*!
	googletestの出力にECodeTypeを出力させる

	パラメータテストのパラメータにECodeTypeを渡した場合に文字列を併記して分かりやすくする。

	テストで使用するコード値のみを定義してあるので、必要があれば追加定義すること。
 */
std::ostream& operator << (std::ostream& os, const ECodeType& eCodeType)
{
	switch(eCodeType){
	case CODE_SJIS:			return os << u8"SJIS";
	case CODE_JIS:			return os << u8"JIS";
	case CODE_EUC:			return os << u8"EUC";
	case CODE_UTF8:			return os << u8"UTF8";
	case CODE_UNICODE:		return os << u8"UTF16LE";
	case CODE_UNICODEBE:	return os << u8"UTF16BE";
	case (ECodeType)12000:	return os << u8"UTF32LE";
	case (ECodeType)12001:	return os << u8"UTF32BE";
	case CODE_UTF7:			return os << u8"UTF7";
	case CODE_CESU8:		return os << u8"CESU8";
	case CODE_LATIN1:		return os << u8"LATIN1";
	default:
		throw std::invalid_argument("unknown ECodeType");
	}
}
