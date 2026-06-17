/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "charset/charset.h"

/*!
 * googletestの出力にEEolTypeを出力させる
 * 
 * パラメータテストのパラメータにEEolTypeを渡した場合に文字列を併記して分かりやすくする。
 */
void PrintTo(EEolType eEolType, std::ostream* os)
{
	switch (eEolType) {
	case EEolType::none:                *os << "NONE"; break;
	case EEolType::cr_and_lf:          	*os << "CRLF"; break;
	case EEolType::line_feed:          	*os << "LF"; break;
	case EEolType::carriage_return:    	*os << "CR"; break;
	case EEolType::next_line:          	*os << "NEL"; break;
	case EEolType::line_separator:     	*os << "LS"; break;
	case EEolType::paragraph_separator:	*os << "PS"; break;

	default:
		// 未知の値は数値で出す
		*os << std::format("EEolType({})", static_cast<uint16_t>(eEolType));
		break;
	}
}
