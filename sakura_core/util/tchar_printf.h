// printf系ラップ関数群
// 2007.09.20 kobake 作成。
//
// 重要な特徴として、独自のフィールド "%ts" および "%tc" を認識して処理する、という点があります。
// UNICODEビルドでは "%ts", "%tc" はそれぞれ "%ls", %lc" として認識され、
// ANSIビルドでは    "%ts", "%tc" はそれぞれ "%hs", %hc" として認識されます。
//
// "%s", "%c" は使用関数により型が変わり、char, wchar_t が混在するコーディング環境ではバグの元となりやすいので、
// できるだけ、上に記したような明示的な型指定をしたフィールドを用いてください。
//
// 注意：%10ts %.12ts のようなものは未サポート
//
// ++ ++ 改善案 ++ ++
//
// あくまでも標準ライブラリ動作を「ラップ」しているだけなので、
// そのラップ処理分、パフォーマンスは悪いです。
// 標準ライブラリに頼らずに全て自前で実装すれば、標準ライブラリ並みのパフォーマンスが得られるはずです。
//
// ちょっと関数名が分かりにくいので、もっと良い名前募集。
// 今のままだと、上記説明を読まなければ、_tsprintf とかと何が違うの？と思われちゃいそう。。。
//
// プロジェクト全体がTCHARに頼らないのであれば、これらの関数群は不要。
//
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_TCHAR_PRINTF_DAD4722C_BE9A_420C_BB75_311B6B1EC14E9_H_
#define SAKURA_TCHAR_PRINTF_DAD4722C_BE9A_420C_BB75_311B6B1EC14E9_H_

// vsprintf_sラップ
int tchar_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v);
int tchar_vsprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v);

// vsprintfラップ
int tchar_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v);
int tchar_vsprintf(WCHAR* buf, const WCHAR* format, va_list& v);

// vsnprintf_sラップ
int tchar_vsnprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v);
int tchar_vsnprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v);

// sprintf_sラップ
int tchar_sprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, ...);
int tchar_sprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...);

// sprintfラップ
int tchar_sprintf(ACHAR* buf, const ACHAR* format, ...);
int tchar_sprintf(WCHAR* buf, const WCHAR* format, ...);

// _snprintf_sラップ
int tchar_snprintf_s(ACHAR* buf, size_t count, const ACHAR* format, ...);
int tchar_snprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...);

#endif /* SAKURA_TCHAR_PRINTF_DAD4722C_BE9A_420C_BB75_311B6B1EC14E9_H_ */
/*[EOF]*/
