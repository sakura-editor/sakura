/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
