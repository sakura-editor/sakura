/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
