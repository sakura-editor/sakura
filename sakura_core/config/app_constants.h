/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_APP_CONSTANTS_30AEF8B3_BD9C_404E_B5CE_CDFE9E8FE451_H_
#define SAKURA_APP_CONSTANTS_30AEF8B3_BD9C_404E_B5CE_CDFE9E8FE451_H_
#pragma once

#include "build_config.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCWSTR GetAppName( void );

#define GSTR_APPNAME_W  GetAppName()		//!< アプリ名の文字列
#define GSTR_APPNAME    GSTR_APPNAME_W

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキストエリア                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta 最大値の定数化と値変更
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;

//	Aug. 14, 2005 genta 定数定義追加
// 2007.09.07 kobake 定数名変更: MAXLINESIZE→MAXLINEKETAS
// 2007.09.07 kobake 定数名変更: MINLINESIZE→MINLINEKETAS
const int MAXLINEKETAS		= 10240;	//!< 1行の桁数の最大値
const int MINLINEKETAS		= 10;		//!< 1行の桁数の最小値

// 2014.08.02 定数定義追加 katze
const int LINENUMWIDTH_MIN = 2;
const int LINENUMWIDTH_MAX = 11;

constexpr auto TABSPACE_MAX = 64;

#endif /* SAKURA_APP_CONSTANTS_30AEF8B3_BD9C_404E_B5CE_CDFE9E8FE451_H_ */
