/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	
	@date aroka 警告対策で変数除去
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, hor, genta, aroka, Moca, MIK, ai
	Copyright (C) 2003, MIK, zenryaku, genta, little YOSHI
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, maru, Moca, genta
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
#include "CEditApp.h"	//	Oct. 9, 2004 genta

/* Java解析モード */
enum EFuncListJavaMode {
	FL_JAVA_MODE_NORMAL = 0,
	FL_JAVA_MODE_WORD = 1,
	FL_JAVA_MODE_SYMBOL = 2,
	FL_JAVA_MODE_COMMENT = 8,
	FL_JAVA_MODE_SINGLE_QUOTE = 20,
	FL_JAVA_MODE_DOUBLE_QUOTE = 21,
	FL_JAVA_MODE_TOO_LONG_WORD = 999
};

/* Java関数リスト作成 */
void CEditDoc::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[100];
	int			nFuncLine = 0;
	int			nFuncId;
	int			nFuncNum;
	char		szClass[1024];

	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];

	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = FL_JAVA_MODE_NORMAL;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = '\0';
	nClassNestArrNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			//nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* エスケープシーケンスは常に取り除く */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( FL_JAVA_MODE_SINGLE_QUOTE == nMode ){
				if( '\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( FL_JAVA_MODE_DOUBLE_QUOTE == nMode ){
				if( '"' == pLine[i] ){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( FL_JAVA_MODE_COMMENT == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
				}
			}
			/* 単語読み込み中 */
			else if( FL_JAVA_MODE_WORD == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = FL_JAVA_MODE_TOO_LONG_WORD;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					/* クラス宣言部分を見つけた */
					//	Oct. 10, 2002 genta interfaceも対象に
					if( 0 == strcmp( "class", szWordPrev ) ||
						0 == strcmp( "interface", szWordPrev )
					 ){
						nClassNestArr[nClassNestArrNum] = nNestLevel;
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							strcat( szClass, "\\" );
						}
						strcat( szClass, szWord );

						nFuncId = 0;
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.LogicToLayout(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
						char szWork[256];
						wsprintf( szWork, "%s::%s", szClass, "定義位置" );
						pcFuncInfoArr->AppendData( nPosY + 1/*nFuncLine*/, nPosY + 1, szWork, nFuncId );

					}

					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( FL_JAVA_MODE_SYMBOL == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
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
					'"' == pLine[i] ||
					'/' == pLine[i] ||
					'.' == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					i--;
					continue;
				}else{
				}
			}else
			/* 長過ぎる単語無視中 */
			if( FL_JAVA_MODE_TOO_LONG_WORD == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}
			}else
			/* ノーマルモード */
			if( FL_JAVA_MODE_NORMAL == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = FL_JAVA_MODE_COMMENT;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = FL_JAVA_MODE_SINGLE_QUOTE;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = FL_JAVA_MODE_DOUBLE_QUOTE;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	メソッド中でさらにメソッドを定義することはないので
						//	ネストレベル判定追加 class/interfaceの直下の場合のみ判定する
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = 2;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.LogicToLayout(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( '}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArrNum--;
						int k;
						for( k = lstrlen( szClass ) - 1; k >= 0; k-- ){
							if( '\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = '\0';
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( '(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != strcmp( "new", szWordPrev )
					){
						strcpy( szFuncName, szWord );
						nFuncLine = nLineCount + 1;
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( ')' == pLine[i] ){
					int			k;
					const char*	pLine2;
					int			nLineLen2;
					int			nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != ' ' && pLine2[k] != TAB && pLine2[k] != CR && pLine2[k] != LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == '*' && pLine2[k + 1] == '/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_cDocLineMgr.GetLineStr( nLineCount2, &nLineLen2 );
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abstract にも対応
						if( pLine2[k] == '{' || pLine2[k] == ';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// 関数の中で別の関数の宣言部を使うことって，Javaであるの？
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = 1;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.LogicToLayout(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = FL_JAVA_MODE_NORMAL;
					continue;
				}else{
					if( '_' == pLine[i] ||
						':' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )||
						'.' == pLine[i]
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = FL_JAVA_MODE_WORD;
					}else{
						nMode = FL_JAVA_MODE_NORMAL;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}

//	From Here Aug. 14, 2000 genta
//
//	書き換えが禁止されているかどうか
//	戻り値: true: 禁止 / false: 許可
//
bool CEditDoc::IsModificationForbidden( int nCommand )
{
	//	編集可能の場合
	if( IsEditable() )
		return false; // 常に書き換え許可

	//	上書き禁止モードの場合
	//	暫定Case文: 実際にはもっと効率の良い方法を使うべき
	switch( nCommand ){
	//	ファイルを書き換えるコマンドは使用禁止
	case F_CHAR:
	case F_IME_CHAR:
	case F_DELETE:
	case F_DELETE_BACK:
	case F_WordDeleteToEnd:
	case F_WordDeleteToStart:
	case F_WordDelete:
	case F_WordCut:
	case F_LineDeleteToStart:
	case F_LineDeleteToEnd:
	case F_LineCutToStart:
	case F_LineCutToEnd:
	case F_DELETE_LINE:
	case F_CUT_LINE:
	case F_DUPLICATELINE:
	case F_INDENT_TAB:
	case F_UNINDENT_TAB:
	case F_INDENT_SPACE:
	case F_UNINDENT_SPACE:
	case F_CUT:
	case F_PASTE:
	case F_INS_DATE:
	case F_INS_TIME:
	case F_CTRL_CODE_DIALOG:	//@@@ 2002.06.02 MIK
	case F_INSTEXT:
	case F_ADDTAIL:
	case F_PASTEBOX:
	case F_REPLACE_DIALOG:
	case F_REPLACE:
	case F_REPLACE_ALL:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
	case F_CODECNV_SJIS2UTF7:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_AUTO2SJIS:
	case F_TOLOWER:
	case F_TOUPPER:
	case F_TOHANKAKU:
	case F_TOHANKATA:				// 2002/08/29 ai
	case F_TOZENEI:					// 2001/07/30 Misaka
	case F_TOHANEI:
	case F_TOZENKAKUKATA:
	case F_TOZENKAKUHIRA:
	case F_HANKATATOZENKATA:
	case F_HANKATATOZENHIRA:
	case F_TABTOSPACE:
	case F_SPACETOTAB:  //#### Stonee, 2001/05/27
	case F_HOKAN:
	case F_CHGMOD_INS:
	case F_LTRIM:		// 2001.12.03 hor
	case F_RTRIM:		// 2001.12.03 hor
	case F_SORT_ASC:	// 2001.12.11 hor
	case F_SORT_DESC:	// 2001.12.11 hor
	case F_MERGE:		// 2001.12.11 hor
	case F_UNDO:		// 2007.10.12 genta
	case F_REDO:		// 2007.10.12 genta
		return true;
	}
	return false;	//	デフォルトで書き換え許可
}
//	To Here Aug. 14, 2000 genta

//	From Here Aug. 21, 2000 genta
//
//	自動保存を行うかどうかのチェック
//
void CEditDoc::CheckAutoSave(void)
{
	if( m_cAutoSave.CheckAction() ){
		//	上書き保存

		if( !IsModified() )	//	変更無しなら何もしない
			return;				//	ここでは，「無変更でも保存」は無視する

		//	2003.10.09 zenryaku 保存失敗エラーの抑制
		if( !IsFilePathAvailable() )	//	まだファイル名が設定されていなければ保存しない
			return;

		bool en = m_cAutoSave.IsEnabled();
		m_cAutoSave.Enable(false);	//	2重呼び出しを防ぐため
		SaveFile( GetFilePath() );	//	保存（m_nCharCode, m_cSaveLineCodeを変更しない）
		m_cAutoSave.Enable(en);
	}
}

//
//	設定変更を自動保存動作に反映する
//
void CEditDoc::ReloadAutoSaveParam(void)
{
	m_cAutoSave.SetInterval( m_pShareData->m_Common.GetAutoBackupInterval() );
	m_cAutoSave.Enable( m_pShareData->m_Common.IsAutoBackupEnabled() );
}


//	ファイルの保存機能をEditViewから移動
//
bool CEditDoc::SaveFile( const char* pszPath )
{
	// 2006.09.01 ryoji 保存前自動実行マクロを実行する
	RunAutoMacro( m_pShareData->m_nMacroOnSave, pszPath );

	if( FileWrite( pszPath, m_cSaveLineCode ) ){
		SetModified(false,true);	//	Jan. 22, 2002 genta

		/* 現在位置で無変更な状態になったことを通知 */
		m_cOpeBuf.SetNoModified();
		return true;
	}
	return false;
}

//	To Here Aug. 21, 2000 genta

//	From Here Sep 8, 2000 genta
//
//!	Perl用アウトライン解析機能（簡易版）
/*!
	単純に /^\\s*sub\\s+(\\w+)/ に一致したら $1を取り出す動作を行う。
	ネストとかは面倒くさいので考えない。
	package{ }を使わなければこれで十分．無いよりはまし。

	@par nModeの意味
	@li 0: はじめ
	@li 2: subを見つけた後
	@li 1: 単語読み出し中

	@date 2005.06.18 genta パッケージ区切りを表す ::と'を考慮するように
*/
void CEditDoc::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* 単語読み込み中 */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub の一文字目かもしれない
				if( nLineLen - i < 4 )
					break;
				if( strncmp( pLine + i, "sub", 3 ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == ' ' || c == '\t' ){
					nMode = 2;	//	発見
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}
				if( '_' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
				){
					//	関数名の始まり
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = '\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( '_' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					//	Jun. 18, 2005 genta パッケージ修飾子を考慮
					//	コロンは2つ連続しないといけないのだが，そこは手抜き
					':' == pLine[i] || '\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					//	関数名取得
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					int		nPosX;
					int		nPosY;
					m_cLayoutMgr.LogicToLayout(
						0,
						nLineCount/*nFuncLine - 1*/,
						&nPosX,
						&nPosY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1, szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta








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


// From Here 2001.12.03 hor
/*! ブックマークリスト作成（無理矢理！）

	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void CEditDoc::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int		nLineLen;
	int		nLineCount;
	int		leftspace, pos_wo_space, k,nX,nY;
	char*	pszText;
	BOOL	bMarkUpBlankLineEnable = m_pShareData->m_Common.m_bMarkUpBlankLineEnable;	//! 空行をマーク対象にするフラグ 20020119 aroka
	int		nNewLineLen	= m_cNewLineCode.GetLen();
	int		nLineLast	= m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = 0; nLineCount <  nLineLast; ++nLineCount ){
		if(!m_cDocLineMgr.GetLine(nLineCount)->IsBookMarked())continue;
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( pLine[leftspace] == ' ' ||
				pLine[leftspace] == '\t'){
				continue;
			}else if( (unsigned char)pLine[leftspace] == (unsigned char)0x81
				&& (unsigned char)pLine[leftspace + 1] == (unsigned char)0x40 ){
				++leftspace;
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(pLine[k] == CR ||
						pLine[k] == LF ||
						pLine[k] == SPACE ||
						pLine[k] == TAB ||
						pLine[k] == '\0') )
					pos_wo_space = k + nCharChars;
			}
			else if( 2 == nCharChars ){
				if( !((unsigned char)pLine[k] == (unsigned char)0x81 && (unsigned char)pLine[k + 1] == (unsigned char)0x40) )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta 文字列長計算式の修正
		nLineLen = pos_wo_space - leftspace;
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[leftspace], nLineLen );
		pszText[nLineLen] = '\0';
		m_cLayoutMgr.LogicToLayout(	0, nLineCount, &nX, &nY );
		pcFuncInfoArr->AppendData( nLineCount+1, nY+1 , (char *)pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor

// From Here Jan. 22, 2002 genta
/*! 変更フラグの設定

	@param flag [in] 設定する値．true: 変更有り / false: 変更無し
	@param redraw [in] true: タイトルの再描画を行う / false: 行わない
	
	@author genta
	@date 2002.01.22 新規作成
*/
void CEditDoc::SetModified( bool flag, bool redraw)
{
	if( m_bIsModified == flag )	//	変更がなければ何もしない
		return;

	m_bIsModified = flag;
	if( redraw )
		SetParentCaption();
}
// From Here Jan. 22, 2002 genta

/*!
	ファイル名の設定
	
	ファイル名を設定すると同時に，ウィンドウアイコンを適切に設定する．
	
	@param szFile [in] ファイルのパス名
	
	@author genta
	@date 2002.09.09
*/
void CEditDoc::SetFilePath(const char* szFile)
{
	strcpy( m_szFilePath, szFile );
	SetDocumentIcon();

	//@@@ From Here 2003.05.31 MIK
	//タブウインドウ	//SetParentCaptionで実施
	//m_pcEditWnd->ChangeFileNameNotify( szFile );
	//@@@ To Here 2003.05.31 MIK
}

/*! ファイル名(パスなし)を取得する
	@author Moca
	@date 2002.10.13
*/
const char * CEditDoc::GetFileName( void ) const
{
	const char *p, *pszName;
	pszName = p = GetFilePath();
	while( *p != '\0'  ){
		if( _IS_SJIS_1( (unsigned char)*p ) && _IS_SJIS_2( (unsigned char)p[1] ) ){
			p+=2;
		}else if( *p == '\\' ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return pszName;
}

/*!
	アイコンの設定
	
	タイプ別設定に応じてウィンドウアイコンをファイルに関連づけられた物，
	または標準のものに設定する．
	
	@author genta
	@date 2002.09.10
*/
void CEditDoc::SetDocumentIcon(void)
{
	HICON	hIconBig, hIconSmall;
	
	if( m_bGrepMode )	// Grepモードの時はアイコンを変更しない
		return;
	
	if( GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcEditWnd->GetRelatedIcon( GetFilePath(), &hIconBig, &hIconSmall );
	else
		m_pcEditWnd->GetDefaultIcon( &hIconBig, &hIconSmall );

	m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}

/*!
	カレントファイルをMRUに登録する。
	ブックマークも一緒に登録する。

	@date 2003.03.30 genta 作成

*/
void CEditDoc::AddToMRU(void)
{
	EditInfo	fi;
	CMRU		cMRU;

	SetFileInfo( &fi );
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );
}


/*!	@brief 指定されたファイルを開く

	現在の編集状況に応じて，現在のウィンドウに読み込むか，新しいウィンドウを開くか
	あるいは既に開かれているウィンドウにフォーカスを移すだけにするかを決定し，
	実行する．

	対象ファイル，エディタウィンドウの状況に依らず新しいファイルを開きたい場合に
	使用する．

	@date 2003.03.30 genta 「閉じて開く」から利用するために引数追加
	@date 2004.10.09 CEditViewより移動
	@date 2007.03.12 maru 重複コード(多重オープン処理部分など)をCShareData::IsPathOpenedに移動
*/
void CEditDoc::OpenFile( const char *filename, ECodeType nCharCode, BOOL bReadOnly )
{
	char		pszPath[_MAX_PATH];
	BOOL		bOpened;
	HWND		hWndOwner;

	/* 「ファイルを開く」ダイアログ */
	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}
	else {
		//	2007.10.01 genta 相対パスを絶対パスに変換
		//	変換しないとIsPathOpenedで正しい結果が得られず，
		//	同一ファイルを複数開くことがある．
		if( ! GetLongFileName( filename, pszPath )){
			//	ファイル名の変換に失敗
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME, 
				_T("ファイル名の変換に失敗しました [%s]"), filename );
			return;
		}
	}
	/* 指定ファイルが開かれているか調べる */
	if( CShareData::getInstance()->IsPathOpened(pszPath, &hWndOwner, nCharCode) ){		// 開いていればアクティブにする
		/* 2007.03.12 maru 開いていたときの処理はCShareData::IsPathOpenedに移動 */
	}else{
		/* ファイルが開かれていない */
		/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
//@@@ 2001.12.26 YAZAKI Grep結果で無い場合も含める。
		if( IsFileOpenInThisWindow()
		){
			BOOL bRet;
			/* ファイル読み込み */
			//	Oct. 03, 2004 genta コード確認は設定に依存
			bRet = FileRead( pszPath, &bOpened, nCharCode, bReadOnly,
							m_pShareData->m_Common.m_bQueryIfCodeChange );

			// 2006.09.01 ryoji オープン後自動実行マクロを実行する
			// 2007.06.27 maru すでに編集ウィンドウは開いているので、FileReadがキャンセルされた場合は開くマクロは実行不要
			if(TRUE==bRet) RunAutoMacro( m_pShareData->m_nMacroOnOpened );
		}else{
			if( strchr( pszPath, ' ' ) ){
				char	szFile2[_MAX_PATH + 3];
				wsprintf( szFile2, "\"%s\"", pszPath );
				strcpy( pszPath, szFile2 );
			}
			/* 新たな編集ウィンドウを起動 */
			CEditApp::OpenNewEditor( m_hInstance, m_hWnd, pszPath, nCharCode, bReadOnly );
		}
	}
	return;
}

/*!	レイアウトパラメータの変更

	具体的にはタブ幅と折り返し位置を変更する．
	現在のドキュメントのレイアウトのみを変更し，共通設定は変更しない．

	@date 2005.08.14 genta 新規作成
	@date 2008.06.18 ryoji レイアウト変更途中はカーソル移動の画面スクロールを見せない（画面のちらつき抑止）
*/
void CEditDoc::ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineSize )
{
	HWND		hwndProgress = NULL;
	if( bShowProgress && NULL != m_pcEditWnd ){
		hwndProgress = m_pcEditWnd->m_hwndProgressBar;
		//	Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	//	座標の保存
	int* posSave = SavePhysPosOfAllView();

	//	レイアウトの更新
	m_cLayoutMgr.ChangeLayoutParam( NULL, nTabSize, nMaxLineSize );

	//	座標の復元
	//	レイアウト変更途中はカーソル移動の画面スクロールを見せない	// 2008.06.18 ryoji
	SetDrawSwitchOfAllViews( FALSE );
	RestorePhysPosOfAllView( posSave );
	SetDrawSwitchOfAllViews( TRUE );

	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( m_cEditViewArr[i].m_hWnd ){
			InvalidateRect( m_cEditViewArr[i].m_hWnd, NULL, TRUE );
			m_cEditViewArr[i].AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}
	if( !GetDocumentAttribute().m_bLineNumIsCRLF ){
		ActiveView().DrawCaretPosInfo();	// 2009.07.25 ryoji
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}

/* 閉じて(無題)

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CEditDoc::FileClose( void )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !OnFileClose() ){
		return;
	}
	/* 既存データのクリア */
	InitDoc();

	/* 全ビューの初期化 */
	InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

	// 2006.09.01 ryoji オープン後自動実行マクロを実行する
	RunAutoMacro( m_pShareData->m_nMacroOnOpened );

	return;
}

/* 閉じて開く

	@param filename	[in] ファイル名
	@param nCharCode	[in] 文字コード
	@param bReadOnly	[in] 読み取り専用か

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CEditDoc::FileCloseOpen( const char *filename, ECodeType nCharCode, BOOL bReadOnly )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !OnFileClose() ){
		return;
	}

	// Mar. 30, 2003 genta
	char	pszPath[_MAX_PATH];

	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}

	/* 既存データのクリア */
	InitDoc();

	/* 全ビューの初期化 */
	InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

	/* ファイルを開く */
	// Mar. 30, 2003 genta
	// Oct.  9, 2004 genta CEditDocへ移動したことによる変更
	OpenFile(( filename ? filename : pszPath ), nCharCode, bReadOnly );
}

/*! 上書き保存

	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す

	@date 2006.12.30 ryoji CEditView::Command_FILESAVE()から処理本体を切り出し
*/
BOOL CEditDoc::FileSave( bool warnbeep, bool askname )
{

	/* 無変更でも上書きするか */
	if( FALSE == m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite
	 && !IsModified()	// 変更フラグ
	 ){
	 	//	Feb. 28, 2004 genta
	 	//	保存不要でも警告音を出して欲しくない場合がある
	 	if( warnbeep ){
			ErrorBeep();
		}
		return TRUE;
	}

	if( !IsFilePathAvailable() ){
		if( ! askname ){
			return FALSE;
		}
		//	Feb. 28, 2004 genta SAVEASの結果が正しく返されていなかった
		//	次の処理と組み合わせるときに問題が生じる
		//return Command_FILESAVEAS_DIALOG();
		FileSaveAs_Dialog();
	}
	else {
		//	Jun.  5, 2004 genta
		//	読み取り専用のチェックをCEditDocから上書き保存処理に移動
		if( m_bReadOnly ){	/* 読み取り専用モード */
			if( warnbeep ){
				ErrorBeep();
				MYMESSAGEBOX(
					m_hWnd,
					MB_OK | MB_ICONSTOP | MB_TOPMOST,
					GSTR_APPNAME,
					_T("%s\n\nは読み取り専用モードで開いています。 上書き保存はできません。\n\n")
					_T("名前を付けて保存をすればいいと思います。"),
					IsFilePathAvailable() ? GetFilePath() : _T("（無題）")
				);
			}
			return FALSE;
		}

		if( SaveFile( GetFilePath() ) ){	//	m_nCharCode, m_cSaveLineCodeを変更せずに保存
			/* キャレットの行桁位置を表示する */
			m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
			return TRUE;
		}
	}
	return FALSE;
}

/*! 名前を付けて保存ダイアログ

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()から処理本体を切り出し
*/
BOOL CEditDoc::FileSaveAs_Dialog( void )
{
	//	Aug. 16, 2000 genta
	//	現在のファイル名を初期値で与えない
	//	May 18, 2001 genta
	//	現在のファイル名を与えないのは上書き禁止の時のみ
	//	そうでない場合には現在のファイル名を初期値として設定する。
	char szPath[_MAX_PATH + 1];
	if( IsReadOnly() )
		szPath[0] = '\0';
	else
		strcpy( szPath, GetFilePath() );

	//	Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOMの有無を与えるパラメータ
	if( SaveFileDialog( szPath, &m_nCharCode, &m_cSaveLineCode, &m_bBomExist ) ){
		//	Jun.  5, 2004 genta
		//	読み取り専用のチェックをCEditDocから上書き保存処理に移動
		//	同名で上書きされるのを防ぐ
		if( m_bReadOnly && strcmp( szPath, GetFilePath()) == 0 ){
			ErrorBeep();
			MYMESSAGEBOX(
				m_hWnd,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				GSTR_APPNAME,
				_T("読み取り専用モードでは同一ファイルへの上書き保存はできません。")
			);
		}
		else {
			//Command_FILESAVEAS( szPath );
			FileSaveAs( szPath );
			m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji 改行コードは引き継がない（上書き保存では常に「変換なし」）
			return TRUE;
		}
	}
	m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji 改行コードは引き継がない（上書き保存では常に「変換なし」）
	return FALSE;
}

/* 名前を付けて保存

	@param filename	[in] ファイル名

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
BOOL CEditDoc::FileSaveAs( const char *filename )
{
	if( SaveFile( filename ) ){
		/* キャレットの行桁位置を表示する */
		m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
		OnChangeSetting();	//	タイプ別設定の変更を指示。
		//	再オープン
		//	Jul. 26, 2003 ryoji 現在開いているのと同じコードで開き直す
		ReloadCurrentFile( m_nCharCode, FALSE );
		return TRUE;
	}
	return FALSE;
}


/*!
	CEditViewの画面バッファを削除
	@date 2007.09.09 Moca 新規作成
*/
void CEditDoc::DeleteCompatibleBitmap()
{
	// CEditView群へ転送する
	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( m_cEditViewArr[i].m_hWnd ){
			m_cEditViewArr[i].DeleteCompatibleBitmap();
		}
	}
}
/*[EOF]*/
