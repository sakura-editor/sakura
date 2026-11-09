/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "basis/GrepInfo.h"

#include <format>
#include <string_view>

/*!
 * コンストラクタ
 */
GrepInfo::GrepInfo() noexcept
{
}

/*!	正規化した GrepInfo を返す

	@return 出力・置換の挙動が矛盾しないよう補正した複製
	@note Grep置換では「一致しなかった行を出力」が成立しないため、行単位出力に落とす。
*/
GrepInfo GrepInfo::Normalized() const
{
	GrepInfo gi = *this;
	// Grep否定行はGrep置換では無効
	if( gi.bGrepReplace && gi.nGrepOutputLineType == 2 ){
		gi.nGrepOutputLineType = 1; // 行単位
	}
	return gi;
}

namespace {

/*!
	コマンドラインの値を引用符で囲む。値の中の " は "" にする(CCommandLine::ParseCommandLine() が " に戻す)
*/
sakura::wstring QuoteForCommandLine( const CNativeW& cmValue )
{
	sakura::wstring quoted = LTEXT("\"");
	if( cmValue.IsValid() ){
		const sakura::wstring_view value( cmValue.GetStringPtr(), size_t( cmValue.GetStringLength() ) );
		for( const auto ch : value ){
			quoted += ch;
			if( ch == LCHAR('"') ){
				quoted += ch;
			}
		}
	}
	quoted += LCHAR('"');
	return quoted;
}

} // namespace

/*!	Grep を新しいウィンドウで実行するためのコマンドラインを作る

	@return -GREPMODE から始まるコマンドライン。
		CCommandLine::ParseCommandLine() で解析すると、この GrepInfo と同じ内容になる。
	@note 置換後の文字列(-GREPR)は Grep置換のときだけ付ける。
*/
sakura::wstring GrepInfo::MakeCommandLine() const
{
	sakura::wstring cmdLine = LTEXT("-GREPMODE -GKEY=") + QuoteForCommandLine( cmGrepKey );
	if( bGrepReplace ){
		cmdLine += LTEXT(" -GREPR=") + QuoteForCommandLine( cmGrepRep );
	}
	cmdLine += LTEXT(" -GFILE=") + QuoteForCommandLine( cmGrepFile );
	cmdLine += LTEXT(" -GFOLDER=") + QuoteForCommandLine( cmGrepFolder );
	cmdLine += std::format( LTEXT(" -GCODE={}"), int( nGrepCharSet ) );
	if( const auto options = MakeCommandLineOptions(); !options.empty() ){
		cmdLine += LTEXT(" -GOPT=") + options;
	}
	return cmdLine;
}

/*!	-GOPT に指定する文字列を作る

	@return CCommandLine::ParseCommandLine() が解釈する 1 文字のオプションを並べたもの
	@note 結果出力形式は 1〜3 のときだけ付ける(範囲外なら解析側の既定値 1 になる)
*/
sakura::wstring GrepInfo::MakeCommandLineOptions() const
{
	sakura::wstring options;
	if( bGrepSubFolder ){ options += LCHAR('S'); }						// サブフォルダーからも検索する
	if( sGrepSearchOption.bLoHiCase ){ options += LCHAR('L'); }			// 英大文字と英小文字を区別する
	if( sGrepSearchOption.bRegularExp ){ options += LCHAR('R'); }			// 正規表現
	if( nGrepOutputLineType == 1 ){ options += LCHAR('P'); }				// 行を出力する
	else if( nGrepOutputLineType == 2 ){ options += LCHAR('N'); }			// 否ヒット行を出力する
	if( sGrepSearchOption.bWordOnly ){ options += LCHAR('W'); }			// 単語単位で探す
	if( nGrepOutputStyle == 1 ){ options += LCHAR('1'); }					// 結果出力形式
	else if( nGrepOutputStyle == 2 ){ options += LCHAR('2'); }
	else if( nGrepOutputStyle == 3 ){ options += LCHAR('3'); }
	if( bGrepOutputFileOnly ){ options += LCHAR('F'); }					// ファイル毎最初のみ検索
	if( bGrepOutputBaseFolder ){ options += LCHAR('B'); }					// ベースフォルダー表示
	if( bGrepSeparateFolder ){ options += LCHAR('D'); }					// フォルダー毎に表示
	if( bGrepExceptFileRegexp ){ options += LCHAR('E'); }					// 除外ファイルを正規表現で指定する
	if( bGrepPaste ){ options += LCHAR('C'); }							// (置換)クリップボードから貼り付け
	if( bGrepBackup ){ options += LCHAR('O'); }							// (置換)バックアップ作成
	if( bGrepCurFolder ){ options += LCHAR('X'); }						// カレントディレクトリを移動しない
	if( bGrepStdout ){ options += LCHAR('U'); }							// 標準出力
	if( !bGrepHeader ){ options += LCHAR('H'); }							// ヘッダー・フッターを出力しない
	return options;
}
