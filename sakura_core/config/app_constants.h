﻿/*! @file */
/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "build_config.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

extern const WCHAR g_szGStrAppName[];
extern const CHAR  g_szGStrAppNameA[];
extern const WCHAR g_szGStrAppNameW[];

#define GSTR_APPNAME   g_szGStrAppName	//!< アプリ名の文字列 (TCHAR版)
#define GSTR_APPNAME_A g_szGStrAppNameA //!< アプリ名の文字列 (CHAR版)
#define GSTR_APPNAME_W g_szGStrAppNameW //!< アプリ名の文字列 (UNICODE版)

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキストエリア                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta 最大値の定数化と値変更
const int LINESPACE_MAX	  = 128;
const int COLUMNSPACE_MAX = 64;

//	Aug. 14, 2005 genta 定数定義追加
// 2007.09.07 kobake 定数名変更: MAXLINESIZE→MAXLINEKETAS
// 2007.09.07 kobake 定数名変更: MINLINESIZE→MINLINEKETAS
const int MAXLINEKETAS = 10240; //!< 1行の桁数の最大値
const int MINLINEKETAS = 10;	//!< 1行の桁数の最小値

// 2014.08.02 定数定義追加 katze
const int LINENUMWIDTH_MIN = 2;
const int LINENUMWIDTH_MAX = 11;
