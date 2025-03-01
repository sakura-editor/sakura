/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include "pch.h"
#include <ostream>

#include "Funccode_enum.h"

/*!
	googletestの出力にEFunctionCodeを出力させる

	パラメータテストのパラメータにEFunctionCodeを渡した場合に文字列を併記して分かりやすくする。

	テストで使用するコード値のみを定義してあるので、必要があれば追加定義すること。
 */
std::ostream& operator << (std::ostream& os, const EFunctionCode& eFuncCode)
{
	switch( eFuncCode ){
	case F_TOLOWER:					return os << u8"TOLOWER";
	case F_TOUPPER:					return os << u8"TOUPPER";
	case F_TOHANKAKU:				return os << u8"TOHANKAKU";
	case F_TOHANKATA:				return os << u8"TOHANKATA";
	case F_TOZENEI:					return os << u8"TOZENEI";
	case F_TOHANEI:					return os << u8"TOHANEI";
	case F_TOZENKAKUKATA:			return os << u8"TOZENKAKUKATA";
	case F_TOZENKAKUHIRA:			return os << u8"TOZENKAKUHIRA";
	case F_HANKATATOZENKATA:		return os << u8"HANKATATOZENKATA";
	case F_HANKATATOZENHIRA:		return os << u8"HANKATATOZENHIRA";
	case F_TABTOSPACE:				return os << u8"TABTOSPACE";
	case F_SPACETOTAB:				return os << u8"SPACETOTAB";
	case F_LTRIM:					return os << u8"LTRIM";
	case F_RTRIM:					return os << u8"RTRIM";
	case F_CODECNV_AUTO2SJIS:		return os << u8"CODECNV_AUTO2SJIS";
	case F_CODECNV_EMAIL:			return os << u8"CODECNV_JIS2SJIS";
	case F_CODECNV_EUC2SJIS:		return os << u8"CODECNV_EUC2SJIS";
	case F_CODECNV_UNICODE2SJIS:	return os << u8"CODECNV_UTF16LE2SJIS";
	case F_CODECNV_UNICODEBE2SJIS:	return os << u8"CODECNV_UTF16BE2SJIS";
	case F_CODECNV_UTF82SJIS:		return os << u8"CODECNV_UTF82SJIS";
	case F_CODECNV_UTF72SJIS:		return os << u8"CODECNV_UTF72SJIS";
	case F_CODECNV_SJIS2JIS:		return os << u8"CODECNV_SJIS2JIS";
	case F_CODECNV_SJIS2EUC:		return os << u8"CODECNV_SJIS2EUC";
	case F_CODECNV_SJIS2UTF8:		return os << u8"CODECNV_SJIS2UTF8";
	case F_CODECNV_SJIS2UTF7:		return os << u8"CODECNV_SJIS2UTF7";
	default:
		throw std::invalid_argument("unknown functionCode");
	}
}
