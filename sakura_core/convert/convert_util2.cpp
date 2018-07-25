
/*!	@file
	@brief 変換ユーティリティ2 - BASE64 Ecode/Decode, UUDecode, Q-printable decode

	@author 
*/

/*
	Copyright (C)

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
#include "convert/convert_util2.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     デコーダーの実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //





/*
	変換テーブル   BASE64 文字 → 数値
	添え字の定義域：0x00 ～ 0x7F
	@author D. S. Koba
*/
const uchar_t cNA = 0xff;
const uchar_t TABLE_BASE64CharToValue[] = {
	cNA, cNA, cNA, cNA, cNA, cNA, cNA, cNA, //00-07:
	cNA, cNA, cNA, cNA, cNA, cNA, cNA, cNA, //08-0f:
	cNA, cNA, cNA, cNA, cNA, cNA, cNA, cNA, //10-17:
	cNA, cNA, cNA, cNA, cNA, cNA, cNA, cNA, //18-1f:
	cNA, cNA, cNA, cNA, cNA, cNA, cNA, cNA, //20-27:
	cNA, cNA, cNA,  62, cNA, cNA, cNA,  63, //28-2f:    +   /
	 52,  53,  54,  55,  56,  57,  58,  59, //30-37: 01234567
	 60,  61, cNA, cNA, cNA, cNA, cNA, cNA, //38-3f: 89
	cNA,   0,   1,   2,   3,   4,   5,   6, //40-47:  ABCDEFG
	  7,   8,   9,  10,  11,  12,  13,  14, //48-4f: HIJKLMNO
	 15,  16,  17,  18,  19,  20,  21,  22, //50-57: PQRSTUVW
	 23,  24,  25, cNA, cNA, cNA, cNA, cNA, //58-5f: XYZ
	cNA,  26,  27,  28,  29,  30,  31,  32, //60-67:  abcdefg
	 33,  34,  35,  36,  37,  38,  39,  40, //68-6f: hijklmno
	 41,  42,  43,  44,  45,  46,  47,  48, //70-77: pqrstuvw
	 49,  50,  51, cNA, cNA, cNA, cNA, cNA, //78-7f: xyz
};



/*!
	変換テーブル 数値 → Modified BASE64 文字
	添え字の定義域：0 ～ 63
	@author D. S. Koba
*/
const char TABLE_ValueToBASE64Char[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};



