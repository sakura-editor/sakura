/*! @file */
/*
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
#ifndef SAKURA_VERSION_F8EBA970_BB7A_43D9_B89A_04FB0B586A8A_H_
#define SAKURA_VERSION_F8EBA970_BB7A_43D9_B89A_04FB0B586A8A_H_
#pragma once

#include "config/build_config.h"
#include "githash.h"

// バージョン定義 //
#define VER_A	2				// メジャーバージョン(2固定)
#define VER_B	4				// マイナーバージョン(4以降はGitHub版)
#define VER_C	2				// 連番(マージの通し番号)
#define VER_D	BUILD_VERSION	// Gitの累積コミット数

#define TO_STR(arg)                            #arg
#define _MAKE_VERSION(a, b, c, d)              a, b, c, d
#define _MAKE_VERSION_STR(a, b, c, d, sep)     TO_STR(a) sep TO_STR(b) sep TO_STR(c) sep TO_STR(d)

#define PRODUCT_VERSION _MAKE_VERSION(VER_A, VER_B, VER_C, VER_D)
#define FILE_VERSION    _MAKE_VERSION(VER_A, VER_B, VER_C, VER_D)
#define VERSION_STR     _MAKE_VERSION_STR(VER_A, VER_B, VER_C, VER_D, ".")

#ifdef GIT_SHORT_COMMIT_HASH
#define VERSION_HASH " (" GIT_SHORT_COMMIT_HASH ")"
#else
#define VERSION_HASH " (Undefined)"
#endif

#endif /* SAKURA_VERSION_F8EBA970_BB7A_43D9_B89A_04FB0B586A8A_H_ */
