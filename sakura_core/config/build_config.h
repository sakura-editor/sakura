/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_
#define SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_

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
#if _MSC_VER>=1400 //VS2005以降なら
#ifdef _DEBUG
#define USE_STRICT_INT //←これをコメントアウトすると厳格なintが無効になります。リリースビルドでは常に無効。
#endif
#endif


//! USE_UNFIXED_FONT を定義すると、フォント選択ダイアログで等幅フォント以外も選べるようになる
//#define USE_UNFIXED_FONT


//UNICODE BOOL定数
#ifdef _UNICODE
static const bool UNICODE_BOOL=true;
#else
static const bool UNICODE_BOOL=false;
#endif


//DebugMonitorLib(仮)を使うかどうか
//#define USE_DEBUGMON


//newされた領域をわざと汚すかどうか (デバッグ用)
#ifdef _DEBUG
#define FILL_STRANGE_IN_NEW_MEMORY
#endif


//crtdbg.hによるメモリーリークチェックを使うかどうか（デバッグ用）
#ifdef _DEBUG
//#define USE_LEAK_CHECK_WITH_CRTDBG
#endif

// -- -- 仕様変更 -- -- //

//全角スペース描画
//#define NEW_ZENSPACE //新しい描画ルーチン (全角スペースを破線矩形で描画) を採用



// -- -- -- -- ↑以上、ビルド設定完了 -- -- -- -- //


//デバッグ検証用：newされた領域をわざと汚す。2007.11.27 kobake
#ifdef FILL_STRANGE_IN_NEW_MEMORY
	#include <stdlib.h> //malloc,free
	inline void _fill_new_memory(void* p, size_t nSize, const char* pSrc, size_t nSrcLen)
	{
		char* s = (char*)p;
		size_t i;
		for(i = 0; i < nSize; i++)
		{
			*s++ = pSrc[i%nSrcLen];
		}
	}
	inline void* operator new(size_t nSize)
	{
		void* p = ::malloc(nSize);
		_fill_new_memory(p,nSize,"ﾆｭｰ",3); //確保されたばかりのメモリ状態は「ﾆｭｰﾆｭｰﾆｭｰ…」となります
		return p;
	}
	inline void* operator new[](size_t nSize)
	{
		void* p = ::malloc(nSize);
		_fill_new_memory(p,nSize,"ｷﾞｭｰ",4); //確保されたばかりのメモリ状態は「ｷﾞｭｰｷﾞｭｰｷﾞｭｰ…」となります
		return p;
	}
	inline void operator delete(void* p)
	{
		::free(p);
	}
	inline void operator delete[](void* p)
	{
		::free(p);
	}
#endif


//crtdbg.hによるメモリーリークチェックを使うかどうか（デバッグ用）
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	//new演算子をオーバーライドするヘッダはcrtdbg.hの前にincludeしないとコンパイルエラーとなる	
	//参考：http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=99818
	#include <xiosbase>
	#include <xlocale>
	#include <xmemory>
	#include <xtree>

	#include <crtdbg.h>
	#define new DEBUG_NEW
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	//それと、WinMainの先頭で _CrtSetDbgFlag() を呼ぶ
#endif

#endif /* SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_ */
/*[EOF]*/
