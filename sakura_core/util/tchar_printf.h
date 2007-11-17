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

#pragma once

// vsprintf_sラップ
int tchar_vsprintf_s (ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v);
int tchar_vswprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v);

// vsprintfラップ
int tchar_vsprintf (ACHAR* buf, const ACHAR* format, va_list& v);
int tchar_vswprintf(WCHAR* buf, const WCHAR* format, va_list& v);

// sprintf_sラップ
int tchar_sprintf_s (ACHAR* buf, size_t nBufCount, const ACHAR* format, ...);
int tchar_swprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...);

// sprintfラップ
int tchar_sprintf(ACHAR* buf, const ACHAR* format, ...);
int tchar_swprintf(WCHAR* buf, const WCHAR* format, ...);

// _snprintfラップ
int tchar_snprintf (ACHAR* buf, size_t count, const ACHAR* format, ...);
int tchar_snwprintf(WCHAR* buf, size_t count, const WCHAR* format, ...);
