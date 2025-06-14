/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
