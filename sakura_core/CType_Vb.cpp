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

#include "StdAfx.h"
#include "CEditDoc.h"
#include "charcode.h"
#include "CFuncInfoArr.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CEditWnd.h"
#include "Debug.h"
#include "etc_uty.h"
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "mymessage.h"	//	Oct. 9, 2004 genta
#include "CControlTray.h"

//	From Here June 23, 2001 N.Nakatani
//!	Visual Basic関数リスト作成（簡易版）
/*!
	Visual Basicのコードから単純にユーザー定義の関数やステートメントを取り出す動作を行う。

    Jul 10, 2003 little YOSHI  細かく解析するように変更
                               すべてのキーワードは自動的に成形されるので、大文字小文字は完全に一致する。
                               フォームやモジュールだけではなく、クラスにも対応。
							   ただし、Constの「,」で連続宣言には未対応
	Jul. 21, 2003 genta キーワードの大文字・小文字を同一視するようにした
	Aug  7, 2003 little YOSHI  ダブルクォーテーションで囲まれたテキストを無視するようにした
	                           関数名などをVBの名前付け規則より255文字に拡張
*/
void CEditDoc::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const int	nMaxWordLeng = 255;	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	const char*	pLine;
	int			nLineLen = 0;//: 2002/2/3 aroka 警告対策：初期化
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	char		szWord[256];		// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	int			nWordIdx = 0;
//	int			nMaxWordLeng = 70;	// Aug 7, 2003 little YOSHI  定数に変更↑
	int			nMode;
	char		szFuncName[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	int			nFuncLine;
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// クラスモジュールフラグ
	bool		bProcedure;		// プロシージャフラグ（プロシージャ内ではTrue）
	bool		bDQuote;		// ダブルクォーテーションフラグ（ダブルクォーテーションがきたらTrue）

	// 調べるファイルがクラスモジュールのときはType、Constの挙動が異なるのでフラグを立てる
	bClass	= false;
	int filelen = _tcslen(GetFilePath());
	if ( 4 < filelen ) {
		if ( 0 == my_stricmp((GetFilePath() + filelen - 4), ".cls") ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	pLine = NULL;
	bProcedure	= false;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( '_' != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		nFuncId = 0;
		bDQuote	= false;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Public") ) {
						// パブリック宣言を見つけた！
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Private") ) {
						// プライベート宣言を見つけた！
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Friend") ) {
						// フレンド宣言を見つけた！
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == my_stricmp(szWord, "Static") ) {
						// スタティック宣言を見つけた！
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Function" ) ){
						if ( 0 == my_stricmp( szWordPrev, "End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != my_stricmp( szWordPrev, "Exit" ) ){
							if( 0 == my_stricmp( szWordPrev, "Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x01;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + 1;
						}
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Sub" ) ){
						if ( 0 == my_stricmp( szWordPrev, "End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != my_stricmp( szWordPrev, "Exit" ) ){
							if( 0 == my_stricmp( szWordPrev, "Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x02;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + 1;
						}
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Get" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId	|= 0x03;		// プロパティ取得
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Let" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x04;		// プロパティ設定
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Set" )
					 && 0 == my_stricmp( szWordPrev, "Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x05;		// プロパティ参照
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Const" )
					 && 0 != my_stricmp( szWordPrev, "#" )
					){
						if ( bClass || bProcedure || 0 == ((nFuncId >> 4) & 0x0f) ) {
							// クラスモジュールでは強制的にPrivate
							// プロシージャ内では強制的にPrivate
							// Publicの指定がないとき、デフォルトでPrivateになる
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x06;		// 定数
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Enum" )
					){
						nFuncId	|= 0x207;		// 列挙型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Type" )
					){
						if ( bClass ) {
							// クラスモジュールでは強制的にPrivate
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x208;		// ユーザ定義型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Event" )
					){
						nFuncId	|= 0x209;		// イベント宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}
					else if( 0 == nParseCnt && 0 == my_stricmp( szWord, "Property" )
					 && 0 == my_stricmp( szWordPrev, "End")
					){
						bProcedure	= false;	// プロシージャフラグをクリア
					}
					else if( 1 == nParseCnt ){
						strcpy( szFuncName, szWord );
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  → レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.LogicToLayout(	0, nFuncLine - 1, &nPosX, &nPosY );
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
						nFuncId	= 0;	// Jul 10, 2003  little YOSHI  論理和を使用するため、必ず初期化
					}

					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}
			/* 記号列読み込み中 */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// 「#Const」と「Const」を区別するために、「#」も識別するように変更
				if( '_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i] ||
					'{' == pLine[i] ||
					'}' == pLine[i] ||
					'(' == pLine[i] ||
					')' == pLine[i] ||
					';' == pLine[i]	||
					'\'' == pLine[i] ||
					'/' == pLine[i]	||
					'-' == pLine[i] ||
					'#' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen && '\'' == pLine[i] ){
					break;
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* テキストが閉じるまで読み飛ばす */	// Aug 7, 2003 little YOSHI  追加
			if (nMode == 3) {
				// 連続するダブルクォーテーションは無視する
				if (1 == nCharChars && '"' == pLine[i]) {
					// ダブルクォーテーションが現れたらフラグを反転する
					bDQuote	= !bDQuote;
				} else if (bDQuote) {
					// ダブルクォーテーションの次に
					// ダブルクォーテーション以外の文字が現れたらノーマルモードに移行
					--i;
					nMode	= 0;
					bDQuote	= false;
					continue;
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}
//	To Here June 23, 2001 N.Nakatani

/*[EOF]*/
