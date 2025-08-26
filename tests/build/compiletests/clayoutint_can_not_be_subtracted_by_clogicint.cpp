﻿/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#ifndef USE_STRICT_INT
#define USE_STRICT_INT
#endif /* #ifndef USE_STRICT_INT */

#include "basis/SakuraBasis.h"

/*!
 * @brief テンプレートのテスト
 * このファイルはビルドエラーになる
 */
void main()
{
	CLayoutInt a( 1 );
	CLogicInt b( 2 );

	//CLayoutInt から CLogicIntを減算することはできない
	a = a - b;
}
