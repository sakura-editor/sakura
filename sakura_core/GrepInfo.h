/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
	CNativeW		cmGrepFolder;			//!< 検索対象フォルダ
	CNativeW		cmExcludeFile;			//!< 除外対象ファイル
	CNativeW		cmExcludeFolder;		//!< 除外対象フォルダ
	SSearchOption	sGrepSearchOption;		//!< 検索オプション
	bool			bGrepCurFolder;			//!< カレントディレクトリを維持
	bool			bGrepStdout;			//!< 標準出力モード
	bool			bGrepHeader;			//!< ヘッダ情報表示
	bool			bGrepSubFolder;			//!< サブフォルダを検索する
	ECodeType		nGrepCharSet;			//!< 文字コードセット
	int				nGrepOutputStyle;		//!< 結果出力形式
	int				nGrepOutputLineType;	//!< 結果出力：行を出力/該当部分/否マッチ行
	bool			bGrepOutputFileOnly;	//!< ファイル毎最初のみ検索
	bool			bGrepOutputBaseFolder;	//!< ベースフォルダ表示
	bool			bGrepSeparateFolder;	//!< フォルダ毎に表示
	bool			bGrepReplace;			//!< Grep置換
	bool			bGrepPaste;				//!< クリップボードから貼り付け
	bool			bGrepBackup;			//!< 置換でバックアップを保存
	bool			bUseRipgrep;			//!< Ripgrepを使う

	// コンストラクタ
	GrepInfo() noexcept;
};
