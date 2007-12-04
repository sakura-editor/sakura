//ビルド(コンパイル)設定
//2007.10.18 kobake 作成

#pragma once

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
#ifdef _DEBUG
#define USE_STRICT_INT //←これをコメントアウトすると厳格なintが無効になります。リリースビルドでは常に無効。
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
