/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "grep/GrepMessageFormat.h"

#include "charset/charcode.h"	//WCODE::IsLineDelimiter
#include "env/CShareData.h"	//GetDllShareData

void SetGrepResult(
	/* データ格納先 */
	CNativeW& cmemMessage,
	/* マッチしたファイルの情報 */
	const WCHAR*		pszFilePath,
	const WCHAR*		pszCodeName,
	/* マッチした行の情報 */
	LONGLONG	nLine,
	int			nColumn,
	const wchar_t*	pCompareData,
	int			nLineLen,
	int			nEolCodeLen,
	/* マッチした文字列の情報 */
	const wchar_t*	pMatchData,
	int			nMatchLen,
	/* オプション */
	const GrepInfo&	gi
)
{
	CNativeW cmemBuf(L"");				// 見出し部分(ファイル名・行桁・文字コード名)
	wchar_t strWork[64];				// 行桁の文字列化に使う作業領域
	const wchar_t * pDispData;			// 本文として出力する文字列の先頭
	int k;								// 本文として出力する文字数
	bool bEOL = true;					// 末尾に改行を足すか
	int nMaxOutStr = 0;					// 本文の最大出力文字数(出力形式ごとに決まる)

	/* ノーマル */
	if( 1 == gi.nGrepOutputStyle ){
		if( gi.bGrepOutputBaseFolder || gi.bGrepSeparateFolder ){
			cmemBuf.AppendString( L"・" );
		}
		cmemBuf.AppendString( pszFilePath );
		cmemBuf.AppendString( lineColumnToString(strWork, nLine, nColumn) );
		cmemBuf.AppendString( pszCodeName );
		cmemBuf.AppendString( L": " );
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
	}
	/* WZ風 */
	else if( 2 == gi.nGrepOutputStyle ){
		::auto_sprintf( strWork, L"・(%6I64d,%-5d): ", nLine, nColumn );
		cmemBuf.AppendString( strWork );
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
	}
	// 結果のみ
	else if( 3 == gi.nGrepOutputStyle ){
		nMaxOutStr = 2500;
	}

	/* 該当行 */
	if( gi.nGrepOutputLineType != 0 ){
		// 行全体を出力する。改行コードは含めない
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( nMaxOutStr < k ){
			// 極端に長い行で結果ウィンドウが埋まらないよう切り捨てる
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
	}
	/* 該当部分 */
	else{
		// マッチした部分だけを出力する
		pDispData = pMatchData;
		k = nMatchLen;
		if( nMaxOutStr < k ){
			// 極端に長いマッチで結果ウィンドウが埋まらないよう切り捨てる
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
		// 該当部分に改行を含む場合はその改行コードをそのまま利用する(次の行に空行を作らない)
		// 2003.06.10 Moca k==0のときにバッファアンダーランしないように
		if( 0 < k && WCODE::IsLineDelimiter(pMatchData[ k - 1 ], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			bEOL = false;
		}
	}

	cmemMessage.AllocStringBuffer( cmemMessage.GetStringLength() + cmemBuf.GetStringLength() + 2 );
	cmemMessage.AppendNativeData( cmemBuf );
	cmemMessage.AppendString( pDispData, k );
	if( bEOL ){
		cmemMessage.AppendString( L"\r\n", 2 );
	}
}

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
)
{
	{
		// バッファを2^n 分確保する
		// 見出し行は長くても300文字程度に収まるので、それを足した長さを
		// 超える最小の2のべき乗まで広げておく。再確保の回数を抑えるため。
		int n = 1024;
		int size = cmemMessage.GetStringLength() + 300;
		while( n < size ){
			n *= 2;
		}
		cmemMessage.AllocStringBuffer( n );
	}
	if( 3 == gi.nGrepOutputStyle ){
		return;
	}

	if( !bOutputBaseFolder && gi.bGrepOutputBaseFolder ){
		if( !gi.bGrepSeparateFolder && 1 == gi.nGrepOutputStyle ){
			cmemMessage.AppendString( L"■\"" );
		}else{
			cmemMessage.AppendString( L"◎\"" );
		}
		cmemMessage.AppendString( pszBaseFolder );
		cmemMessage.AppendString( L"\"\r\n" );
		bOutputBaseFolder = true;
	}
	if( !bOutputFolderName && gi.bGrepSeparateFolder ){
		if( pszFolder[0] ){
			cmemMessage.AppendString( L"■\"" );
			cmemMessage.AppendString( pszFolder );
			cmemMessage.AppendString( L"\"\r\n" );
		}else{
			cmemMessage.AppendString( L"■\r\n" );
		}
		bOutputFolderName = true;
	}
	// WZ風では見出しにファイル名を出す。ノーマルはヒット行ごとに出すので不要
	if( 2 == gi.nGrepOutputStyle && !bOutFileName ){
		// 上位のフォルダー名を既に出しているならフルパスは冗長なので相対パスにする
		const WCHAR* pszDispFilePath = ( gi.bGrepSeparateFolder || gi.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;
		if( gi.bGrepSeparateFolder ){
			// フォルダー毎表示のときはフォルダーの見出し(■)と区別する
			cmemMessage.AppendString( L"◆\"" );
		}else{
			cmemMessage.AppendString( L"■\"" );
		}
		cmemMessage.AppendString( pszDispFilePath );
		cmemMessage.AppendString( L"\"" );
		cmemMessage.AppendString( pszCodeName );
		cmemMessage.AppendString( L"\r\n" );
		bOutFileName = TRUE;
	}
}
