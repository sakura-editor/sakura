//	$Id$
/************************************************************************
        CEditDoc_new.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

//#include <stdio.h>
#include "CEditDoc.h"
#include "charcode.h"


/* Java関数リスト作成 */
void CEditDoc::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
//	int			nNestLevel2;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[80];
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	char		szClass[1024];
	
//	int			nClassNest;
	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];
	
	BOOL		bClassReading;
	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
//	nNestLevel2 = 0;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = '\0';
	bClassReading = FALSE;
	nClassNestArrNum = 0;
//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* エスケープシーケンスは常に取り除く */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
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
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					/* クラス宣言部分を見つけた */
					if( 0 == strcmp( "class", szWordPrev ) ){
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
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
						char szWork[256];
						wsprintf( szWork, "%s::%s", szClass, "定義位置" );
						pcFuncInfoArr->AppendData( nPosY + 1/*nFuncLine*/, nPosY + 1, szWork, nFuncId );
					
					}

					
//					strcpy( szWordPrev, szWord );
//					nWordIdx = 0;
//					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
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
					nMode = 0;
					i--;
					continue;
				}else{
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
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						if( 0 != strcmp( "sizeof", szFuncName )
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
							m_cLayoutMgr.CaretPos_Phys2Log(
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
					nMode = 0;
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
					nMode = 0;
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
					nMode = 0;
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
						if( pLine2[k] == '{' || __iscsym( pLine2[k] ) ){
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
					nMode = 0;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						if( 0 != strcmp( "sizeof", szFuncName )
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
							m_cLayoutMgr.CaretPos_Phys2Log(
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
					nMode = 0;
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
						nMode = 1;
					}else{
						nMode = 0;
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
	if( m_bReadOnly == FALSE &&	//	読みとり専用でも
		!( 0 != m_nFileShareModeOld && m_hLockedFile == NULL ))	//	上書き禁止でもなければ
		return false;			//	常に書き換え許可
	
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
	case F_INSTEXT:
	case F_ADDTAIL:
	case F_PASTEBOX:
	case F_REPLACE:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
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
	case F_TOZENKAKUKATA:
	case F_TOZENKAKUHIRA:
	case F_HANKATATOZENKAKUKATA:
	case F_HANKATATOZENKAKUHIRA:
	case F_TABTOSPACE:
	case F_HOKAN:
	case F_CHGMOD_INS:
//		::MessageBox( m_hWnd, "Operation is forbidden.", "DEBUG", MB_OK | MB_ICONEXCLAMATION );
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
	if( m_cAutoSave.CheckAction()){
		//	上書き保存

		bool en;
		if( !m_bIsModified )	//	変更無しなら何もしない
			return;				//	ここでは，「無変更でも保存」は無視する

		en = m_cAutoSave.IsEnabled();
		m_cAutoSave.Enable(false);	//	2重呼び出しを防ぐため
		SaveFile(false);	//	保存
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
bool CEditDoc::SaveFile(bool force_rename)
{
	char *path = ( force_rename || m_szFilePath[0] == '\0' ) ? NULL : m_szFilePath;

	if( FileWrite( path ) ){
		m_bIsModified = FALSE;	/* 変更フラグ */

		/* 現在位置で無変更な状態になったことを通知 */
		m_cOpeBuf.SetNoModified();

		SetParentCaption();	/* 親ウィンドウのタイトルを更新 */
		return true;
	}
	return false;
}

//	To Here Aug. 21, 2000 genta

//	From Here Sep 8, 2000 genta
//
//	Perl用アウトライン解析機能（簡易版）
//
//	単純に /^\s*sub\s+(\w+)/ に一致したら $1を取り出す動作を行う
//	ネストとかは面倒くさいので考えない
//	package{ }を使わなければこれで十分．無いよりはまし．
//
//	nModeの意味
//	0: はじめ
//	2: subを見つけた後
//	1: 単語読み出し中
//
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
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
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
				if( strncmp( pLine + i, "sub", 3 ))
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
					('0' <= pLine[i] &&	pLine[i] <= '9' )
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
					m_cLayoutMgr.CaretPos_Phys2Log(
						0,
						nLineCount/*nFuncLine - 1*/,
						&nPosX,
						&nPosY
					);
					pcFuncInfoArr->AppendData( nPosY + 1/*nFuncLine*/, nPosY + 1, szWord, 0 );
					
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
