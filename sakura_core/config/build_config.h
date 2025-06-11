/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_BUILD_CONFIG_DB7D8D47_EA6A_4ABF_A081_A31875D78808_H_
#define SAKURA_BUILD_CONFIG_DB7D8D47_EA6A_4ABF_A081_A31875D78808_H_
#pragma once

//ビルド(コンパイル)設定
//2007.10.18 kobake 作成
//2009.09.10 syat メモリリークチェックを追加

/*!
	厳格なintを使うかどうか。

	主にエディタ部分の座標系単位に関して
	コンパイル時に静的な型チェックがされるようになります。
	ただしその分コンパイル時間もかかります。

	実行時挙動は変化無し。
	実行時オーバーヘッド不明。コンパイラが賢ければオーバーヘッドゼロ。

	リリースビルドでは無効にしておくと良い。

	@date 2007.10.18 kobake
*/
#ifdef _MSC_VER
#ifdef _DEBUG
#define USE_STRICT_INT //←これをコメントアウトすると厳格なintが無効になります。リリースビルドでは常に無効.
#endif
#endif

//! USE_UNFIXED_FONT を定義すると、フォント選択ダイアログで等幅フォント以外も選べるようになる
//#define USE_UNFIXED_FONT

//newされた領域をわざと汚すかどうか (デバッグ用)
#if defined(_MSC_VER) &&  defined(_DEBUG)
#define FILL_STRANGE_IN_NEW_MEMORY
#endif

//crtdbg.hによるメモリリークチェックを使うかどうか (デバッグ用)
#if defined(_MSC_VER) &&  defined(_DEBUG)
#define USE_LEAK_CHECK_WITH_CRTDBG
#endif

// -- -- 仕様変更 -- -- //

//全角スペース描画
//#define NEW_ZENSPACE //新しい描画ルーチン (全角スペースを破線矩形で描画) を採用

// -- -- -- -- ↑以上、ビルド設定完了 -- -- -- -- //

//デバッグ検証用：newされた領域をわざと汚す。2007.11.27 kobake
#ifdef FILL_STRANGE_IN_NEW_MEMORY
	void* operator new(
		size_t const size,
		int const    block_use,
		char const*  file_name,
		int const    line_number
		);
	void* operator new[](size_t const size,
		int const    block_use,
		char const*  file_name,
		int const    line_number
		);
#endif

//crtdbg.hによるメモリリークチェックを使うかどうか (デバッグ用)
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	//Cランタイムの機能を使ってメモリリークを検出する
	//  メモリリークチェックの結果出力を得るには
	//    wWinMainの最後で_CrtDumpMemoryLeaks()を呼び出すか
	//    wWinMainの最初で_CrtSetDbgFlag()を呼び出す必要がある。
	//see https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

    #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
    // Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
    // allocations to be of _CLIENT_BLOCK type
#else
    #define DBG_NEW new
#endif

#endif /* SAKURA_BUILD_CONFIG_DB7D8D47_EA6A_4ABF_A081_A31875D78808_H_ */
