﻿/*! @file */
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
#ifndef SAKURA_GREPINFO_9A59ABAF_04F9_4D29_B216_0B0784DD2290_H_
#define SAKURA_GREPINFO_9A59ABAF_04F9_4D29_B216_0B0784DD2290_H_
#pragma once

#include "_main/global.h"	//SSearchOption
#include "charset/charcode.h"	//ECodeType
#include "mem/CNativeW.h"	//CNativeW

/*!
 * Grep 検索オプション
 *
 * @date 2002/01/18 aroka
 *
 * @note この構造体は CNativeW をメンバに含むため、
 *   memcmp による比較を行ってはならない。
 */
struct GrepInfo {
	CNativeW		cmGrepKey;				//!< 検索キー
	CNativeW		cmGrepRep;				//!< 置換キー
	CNativeW		cmGrepFile;				//!< 検索対象ファイル
	CNativeW		cmGrepFolder;			//!< 検索対象フォルダー
	SSearchOption	sGrepSearchOption;		//!< 検索オプション
	bool			bGrepCurFolder;			//!< カレントディレクトリを維持
	bool			bGrepStdout;			//!< 標準出力モード
	bool			bGrepHeader;			//!< ヘッダー情報表示
	bool			bGrepSubFolder;			//!< サブフォルダーを検索する
	ECodeType		nGrepCharSet;			//!< 文字コードセット
	int				nGrepOutputStyle;		//!< 結果出力形式
	int				nGrepOutputLineType;	//!< 結果出力：行を出力/該当部分/否マッチ行
	bool			bGrepOutputFileOnly;	//!< ファイル毎最初のみ検索
	bool			bGrepOutputBaseFolder;	//!< ベースフォルダー表示
	bool			bGrepSeparateFolder;	//!< フォルダー毎に表示
	bool			bGrepReplace;			//!< Grep置換
	bool			bGrepPaste;				//!< クリップボードから貼り付け
	bool			bGrepBackup;			//!< 置換でバックアップを保存

	// コンストラクタ
	GrepInfo() noexcept;
};
#endif /* SAKURA_GREPINFO_9A59ABAF_04F9_4D29_B216_0B0784DD2290_H_ */
