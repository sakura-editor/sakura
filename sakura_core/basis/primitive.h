﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_
#define SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_
#pragma once

#include <Windows.h>
#include "config/build_config.h"

// -- -- -- -- 文字 -- -- -- -- //

//char,wchar_t の変わりに、別名の ACHAR,WCHAR を使うと、ソース整形がしやすいケースがある。
typedef char ACHAR;

//WIN_CHAR (WinAPIに渡すので、必ずTCHARでなければならないもの)
typedef WCHAR WIN_CHAR;

//EDIT_CHAR
typedef wchar_t WChar;      //エディタで用いるテキスト管理データ型
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)

//文字コード別、文字型
typedef unsigned char	uchar_t;		//  unsigned char の別名．
typedef unsigned short	uchar16_t;		//  UTF-16 用．
typedef unsigned long	uchar32_t;		//  UTF-32 用．
typedef long			wchar32_t;

// -- -- -- -- その他 -- -- -- -- //

typedef char KEYCODE;

//int互換
#ifdef USE_STRICT_INT
	#include "CLaxInteger.h"
	typedef CLaxInteger Int;
#else
	typedef int Int;
#endif
#endif /* SAKURA_PRIMITIVE_C8059DE4_C986_492E_9C09_7F044049C481_H_ */
