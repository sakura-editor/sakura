/*!	@file
	@brief Grep結果ウィンドウへ出力する文字列の整形

	検索そのものは行わず、確定したヒット情報から表示用の文字列を組み立てるだけの
	処理をまとめている。結果ウィンドウへの書き出し(AddTail)と進捗表示は
	CGrepAgent 側に残している。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_GREPMESSAGEFORMAT_812C2479_9955_44DB_8529_75296A7E19E6_H_
#define SAKURA_GREPMESSAGEFORMAT_812C2479_9955_44DB_8529_75296A7E19E6_H_
#pragma once

#include <cassert>

#include "basis/GrepInfo.h"	//GrepInfo
#include "mem/CNativeW.h"	//CNativeW
#include "util/string_ex.h"	//auto_sprintf
#include "util/string_ex2.h"	//int2dec

/*!	@brief マッチした行番号と桁番号をGrep結果に出力する為に文字列化

	auto_sprintf 関数を 書式文字列 "(%I64d,%d)" で実行するのと同等の処理結果を生成
	高速化の為に自前実装に置き換え

	@param[out]	strWork		出力先。必要な長さは static_assert で検証する
	@param[in]	nLine		マッチした行番号(1～)
	@param[in]	nColumn		マッチした桁番号(1～)
	@return 出力先文字列(strWork と同一)

	@note 出力先を配列参照で受け取るのは、テンプレート引数で長さを取得して
	      static_assert に使うため。ポインタで受けると長さ検証ができない。
	@note Debug 版では auto_sprintf の結果と一致することを assert で確認する。
*/
template <size_t nCapacity>
inline
wchar_t* lineColumnToString(
	wchar_t (&strWork)[nCapacity],	/*!< [out] 出力先 */
	LONGLONG	nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColumn				/*!< [in] マッチした桁番号(1～) */
)
{
	// int2dec_destBufferSufficientLength 関数の
	// 戻り値から -1 しているのは終端0文字の分を削っている為
	constexpr size_t requiredMinimumCapacity =
		1		// (
		+ int2dec_destBufferSufficientLength<LONGLONG>() - 1	// I64d
		+ 1		// ,
		+ int2dec_destBufferSufficientLength<int32_t>() - 1	// %d
		+ 1		// )
		+ 1		// \0 終端0文字の分
	;
	static_assert(nCapacity >= requiredMinimumCapacity, "nCapacity not enough.");
	wchar_t* p = strWork;
	*p++ = L'(';
	p += int2dec(nLine, p);
	*p++ = L',';
	p += int2dec(nColumn, p);
	*p++ = L')';
	*p = '\0';
#ifdef _DEBUG
	// Debug 版に限って両方実行して、両者が一致することを確認
	wchar_t strWork2[requiredMinimumCapacity];
	::auto_sprintf( strWork2, L"(%I64d,%d)", nLine, nColumn );
	assert(wcscmp(strWork, strWork2) == 0);
#endif
	return strWork;
}

/*!	@brief Grep結果を構築する

	ヒット1件ぶんの行を cmemMessage の末尾へ追加する。

	@note 出力形式(gi.nGrepOutputStyle)は 1:ノーマル / 2:WZ風 / 3:結果のみ。
	      1 と 2 はファイル名・行桁・文字コード名の見出しを付け、3 は付けない。
	@note 出力する本文は gi.nGrepOutputLineType で切り替わる。
	      0 ならマッチした部分だけ、それ以外なら行全体を出力する。
	@note 1行が長すぎる場合は出力形式ごとの上限(ノーマル2000文字、
	      それ以外2500文字)で切り捨てる。

	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
	@date 2013.11.05 Moca cmemMessageに直接追加するように
*/
void SetGrepResult(
	/* データ格納先 */
	CNativeW& cmemMessage,
	/* マッチしたファイルの情報 */
	const WCHAR*		pszFilePath,	/*!< [in] フルパス or 相対パス*/
	const WCHAR*		pszCodeName,	/*!< [in] 文字コード情報．" [SJIS]"とか */
	/* マッチした行の情報 */
	LONGLONG	nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColumn,			/*!< [in] マッチした桁番号(1～) */
	const wchar_t*	pCompareData,	/*!< [in] 行の文字列 */
	int			nLineLen,			/*!< [in] 行の文字列の長さ */
	int			nEolCodeLen,		/*!< [in] EOLの長さ */
	/* マッチした文字列の情報 */
	const wchar_t*	pMatchData,		/*!< [in] マッチした文字列 */
	int			nMatchLen,			/*!< [in] マッチした文字列の長さ */
	/* オプション */
	const GrepInfo&	gi
);

/*!	@brief ベースフォルダー名・フォルダー名・ファイル名をGrep結果に出力する

	ヒット行の前に置く見出し行を cmemMessage の末尾へ追加する。

	@param[in,out]	cmemMessage			出力先
	@param[in]		gi					Grep実行の入力一式(正規化済み)
	@param[in]		pszFullPath			対象ファイルのフルパス
	@param[in]		pszBaseFolder		検索開始フォルダー
	@param[in]		pszFolder			ベースフォルダーからの相対フォルダー
	@param[in]		pszRelPath			ベースフォルダーからの相対ファイルパス
	@param[in]		pszCodeName			文字コード情報。" [SJIS]" とか
	@param[in,out]	bOutputBaseFolder	ベースフォルダー名を出力済みか
	@param[in,out]	bOutputFolderName	フォルダー名を出力済みか
	@param[in,out]	bOutFileName		ファイル名を出力済みか

	@note 末尾3つのフラグは「既に出力したか」を呼び出し側と共有するためのもの。
	      同じ見出しを二重に出さないよう、出力したら true を立てて返す。
	      DoGrep から DoGrepTree、DoGrepFile を経て渡されてくる。
	@note 出力形式3(結果のみ)のときは何も出力せず、フラグも変化しない。
*/
void OutputPathInfo(
	CNativeW&		cmemMessage,
	const GrepInfo&	gi,
	const WCHAR*	pszFullPath,
	const WCHAR*	pszBaseFolder,
	const WCHAR*	pszFolder,
	const WCHAR*	pszRelPath,
	const WCHAR*	pszCodeName,
	bool&			bOutputBaseFolder,
	bool&			bOutputFolderName,
	BOOL&			bOutFileName
);
#endif /* SAKURA_GREPMESSAGEFORMAT_812C2479_9955_44DB_8529_75296A7E19E6_H_ */
