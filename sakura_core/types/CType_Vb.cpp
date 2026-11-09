/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

/* Visual Basic */
//JUl. 10, 2001 JEPRO VB   ユーザーに贈る
//Jul. 09, 2001 JEPRO 追加 //Dec. 16, 2002 MIK追加 // Feb. 19, 2006 genta .vb追加
void CType_Vb::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("Visual Basic") );
	wcscpy( pType->m_szTypeExts, LTEXT("bas,frm,cls,ctl,pag,dob,dsr,vb") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("'"), -1 );				/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_VB;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 13;							/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 14;							/* キーワードセット2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* 半角数値を色分け表示 */
	pType->m_nStringType = STRING_LITERAL_PLSQL;				/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

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
void CDocOutline::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const int	nMaxWordLeng = 255;	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	const WCHAR*	pLine;
	CLogicInt		nLineLen = CLogicInt(0);//: 2002/2/3 aroka 警告対策：初期化
	int			i;
	int			nCharChars;
	WCHAR		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	WCHAR		szWord[256];		// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	int			nWordIdx = 0;
	int			nMode;
	WCHAR		szFuncName[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	CLogicInt	nFuncLine(0);
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// クラスモジュールフラグ
	bool		bProcedure;		// プロシージャフラグ（プロシージャ内ではTrue）
	bool		bDQuote;		// ダブルクォーテーションフラグ（ダブルクォーテーションがきたらTrue）
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	// 調べるファイルがクラスモジュールのときはType、Constの挙動が異なるのでフラグを立てる
	bClass	= false;
	auto filelen = int(wcslen(m_pcDocRef->m_cDocFile.GetFilePath()));
	if ( 4 < filelen ) {
		if ( 0 == _wcsicmp((m_pcDocRef->m_cDocFile.GetFilePath() + filelen - 4), LTEXT(".cls")) ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = LCHAR('\0');
	szWord[nWordIdx] = LCHAR('\0');
	nMode = 0;
	pLine = nullptr;
	bProcedure	= false;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( nullptr != pLine ){
			if( LCHAR('_') != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nFuncId = 0;
		bDQuote	= false;
		for( i = 0; i < nLineLen; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					LCHAR('_') == pLine[i] ||
					LCHAR('~') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
					(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						i += (nCharChars - 1);
						continue;
					}else{
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == _wcsicmp(szWord, LTEXT("Public")) ) {
						// パブリック宣言を見つけた！
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == _wcsicmp(szWord, LTEXT("Private")) ) {
						// プライベート宣言を見つけた！
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == _wcsicmp(szWord, LTEXT("Friend")) ) {
						// フレンド宣言を見つけた！
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == _wcsicmp(szWord, LTEXT("Static")) ) {
						// スタティック宣言を見つけた！
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Function") ) ){
						if ( 0 == _wcsicmp( szWordPrev, LTEXT("End") ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != _wcsicmp( szWordPrev, LTEXT("Exit") ) ){
							if( 0 == _wcsicmp( szWordPrev, LTEXT("Declare") ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x01;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Sub") ) ){
						if ( 0 == _wcsicmp( szWordPrev, LTEXT("End") ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != _wcsicmp( szWordPrev, LTEXT("Exit") ) ){
							if( 0 == _wcsicmp( szWordPrev, LTEXT("Declare") ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x02;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Get") )
					 && 0 == _wcsicmp( szWordPrev, LTEXT("Property") )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId	|= 0x03;		// プロパティ取得
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Let") )
					 && 0 == _wcsicmp( szWordPrev, LTEXT("Property") )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x04;		// プロパティ設定
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Set") )
					 && 0 == _wcsicmp( szWordPrev, LTEXT("Property") )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x05;		// プロパティ参照
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Const") )
					 && 0 != _wcsicmp( szWordPrev, LTEXT("#") )
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
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Enum") )
					){
						nFuncId	|= 0x207;		// 列挙型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Type") )
					){
						if ( bClass ) {
							// クラスモジュールでは強制的にPrivate
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x208;		// ユーザ定義型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Event") )
					){
						nFuncId	|= 0x209;		// イベント宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == _wcsicmp( szWord, LTEXT("Property") )
					 && 0 == _wcsicmp( szWordPrev, LTEXT("End"))
					){
						bProcedure	= false;	// プロシージャフラグをクリア
					}
					else if( 1 == nParseCnt ){
						wcscpy( szFuncName, szWord );
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  → レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)), &ptPosXY );
						pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
						nFuncId	= 0;	// Jul 10, 2003  little YOSHI  論理和を使用するため、必ず初期化
					}

					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = LCHAR('\0');
					nMode = 0;
					i--;
					continue;
				}
			}
			/* 記号列読み込み中 */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// 「#Const」と「Const」を区別するために、「#」も識別するように変更
				if( LCHAR('_') == pLine[i] ||
					LCHAR('~') == pLine[i] ||
					(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
					(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
					(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
					(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 2013.05.08 日本語対応
					LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
					LCHAR('{') == pLine[i] ||
					LCHAR('}') == pLine[i] ||
					LCHAR('(') == pLine[i] ||
					LCHAR(')') == pLine[i] ||
					LCHAR(';') == pLine[i]	||
					LCHAR('\'') == pLine[i] ||
					LCHAR('/') == pLine[i]	||
					LCHAR('-') == pLine[i] ||
					LCHAR('#') == pLine[i] ||
					2 == nCharChars
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = LCHAR('\0');
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && LCHAR('"') == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( LCHAR('\t') == pLine[i] ||
					LCHAR(' ') == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					continue;
				}else
				if( i < nLineLen && LCHAR('\'') == pLine[i] ){
					break;
				} else if (1 == nCharChars && LCHAR('"') == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						LCHAR('_') == pLine[i] ||
						LCHAR('~') == pLine[i] ||
						(LCHAR('a') <= pLine[i] &&	pLine[i] <= LCHAR('z') )||
						(LCHAR('A') <= pLine[i] &&	pLine[i] <= LCHAR('Z') )||
						(LCHAR('0') <= pLine[i] &&	pLine[i] <= LCHAR('9') )||
						(LCHAR('\u00a1') <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = LCHAR('\0');
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* テキストが閉じるまで読み飛ばす */	// Aug 7, 2003 little YOSHI  追加
			if (nMode == 3) {
				// 連続するダブルクォーテーションは無視する
				if (1 == nCharChars && LCHAR('"') == pLine[i]) {
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

//Jul. 10, 2001 JEPRO 追加
const WCHAR* g_ppszKeywordsVB[] = {
	LTEXT("And"),
	LTEXT("As"),
	LTEXT("Attribute"),
	LTEXT("Begin"),
	LTEXT("BeginProperty"),
	LTEXT("Boolean"),
	LTEXT("ByVal"),
	LTEXT("Byte"),
	LTEXT("Call"),
	LTEXT("Case"),
	LTEXT("Const"),
	LTEXT("Currency"),
	LTEXT("Date"),
	LTEXT("Declare"),
	LTEXT("Dim"),
	LTEXT("Do"),
	LTEXT("Double"),
	LTEXT("Each"),
	LTEXT("Else"),
	LTEXT("ElseIf"),
	LTEXT("Empty"),
	LTEXT("End"),
	LTEXT("EndProperty"),
	LTEXT("Error"),
	LTEXT("Eqv"),
	LTEXT("Exit"),
	LTEXT("False"),
	LTEXT("For"),
	LTEXT("Friend"),
	LTEXT("Function"),
	LTEXT("Get"),
	LTEXT("GoTo"),
	LTEXT("If"),
	LTEXT("Imp"),
	LTEXT("Integer"),
	LTEXT("Is"),
	LTEXT("Let"),
	LTEXT("Like"),
	LTEXT("Long"),
	LTEXT("Loop"),
	LTEXT("Me"),
	LTEXT("Mod"),
	LTEXT("New"),
	LTEXT("Next"),
	LTEXT("Not"),
	LTEXT("Null"),
	LTEXT("Object"),
	LTEXT("On"),
	LTEXT("Option"),
	LTEXT("Or"),
	LTEXT("Private"),
	LTEXT("Property"),
	LTEXT("Public"),
	LTEXT("RSet"),
	LTEXT("ReDim"),
	LTEXT("Rem"),
	LTEXT("Resume"),
	LTEXT("Select"),
	LTEXT("Set"),
	LTEXT("Single"),
	LTEXT("Static"),
	LTEXT("Step"),
	LTEXT("Stop"),
	LTEXT("String"),
	LTEXT("Sub"),
	LTEXT("Then"),
	LTEXT("To"),
	LTEXT("True"),
	LTEXT("Type"),
	LTEXT("Wend"),
	LTEXT("While"),
	LTEXT("With"),
	LTEXT("Xor"),
	LTEXT("#If"),
	LTEXT("#Else"),
	LTEXT("#End"),
	LTEXT("#Const"),
	LTEXT("AddressOf"),
	LTEXT("Alias"),
	LTEXT("Append"),
	LTEXT("Array"),
	LTEXT("ByRef"),
	LTEXT("Explicit"),
	LTEXT("Global"),
	LTEXT("In"),
	LTEXT("Lib"),
	LTEXT("Nothing"),
	LTEXT("Optional"),
	LTEXT("Output"),
	LTEXT("Terminate"),
	LTEXT("Until"),
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	LTEXT("DefBool"),
	LTEXT("DefByte"),
	LTEXT("DefCur"),
	LTEXT("DefDate"),
	LTEXT("DefDbl"),
	LTEXT("DefInt"),
	LTEXT("DefLng"),
	LTEXT("DefObj"),
	LTEXT("DefSng"),
	LTEXT("DefStr"),
	LTEXT("DefVar"),
	LTEXT("LSet"),
	LTEXT("GoSub"),
	LTEXT("Return"),
	LTEXT("Variant"),
	//			"Option Base
	//			"As Any
	//=========================================================
	// 以下はVB.NET用キーワードです
	//=========================================================
	//BitAnd
	//BitOr
	//BitNot
	//BitXor
	//Delegate
	//Short
	//Structure
};
int g_nKeywordsVB = int(std::size(g_ppszKeywordsVB));

//Jul. 10, 2001 JEPRO 追加
const WCHAR* g_ppszKeywordsVB2[] = {
	LTEXT("AppActivate"),
	LTEXT("Beep"),
	LTEXT("BeginTrans"),
	LTEXT("ChDir"),
	LTEXT("ChDrive"),
	LTEXT("Close"),
	LTEXT("CommitTrans"),
	LTEXT("CompactDatabase"),
	LTEXT("Date"),
	LTEXT("DeleteSetting"),
	LTEXT("Erase"),
	LTEXT("FileCopy"),
	LTEXT("FreeLocks"),
	LTEXT("Input"),
	LTEXT("Kill"),
	LTEXT("Load"),
	LTEXT("Lock"),
	LTEXT("Mid"),
	LTEXT("MidB"),
	LTEXT("MkDir"),
	LTEXT("Name"),
	LTEXT("Open"),
	LTEXT("Print"),
	LTEXT("Put"),
	LTEXT("Randomize"),
	LTEXT("RegisterDatabase"),
	LTEXT("RepairDatabase"),
	LTEXT("Reset"),
	LTEXT("RmDir"),
	LTEXT("Rollback"),
	LTEXT("SavePicture"),
	LTEXT("SaveSetting"),
	LTEXT("Seek"),
	LTEXT("SendKeys"),
	LTEXT("SetAttr"),
	LTEXT("SetDataAccessOption"),
	LTEXT("SetDefaultWorkspace"),
	LTEXT("Time"),
	LTEXT("Unload"),
	LTEXT("Unlock"),
	LTEXT("Width"),
	LTEXT("Write"),
	LTEXT("Array"),
	LTEXT("Asc"),
	LTEXT("AscB"),
	LTEXT("Atn"),
	LTEXT("CBool"),
	LTEXT("CByte"),
	LTEXT("CCur"),
	LTEXT("CDate"),
	LTEXT("CDbl"),
	LTEXT("CInt"),
	LTEXT("CLng"),
	LTEXT("CSng"),
	LTEXT("CStr"),
	LTEXT("CVErr"),
	LTEXT("CVar"),
	LTEXT("Choose"),
	LTEXT("Chr"),
	LTEXT("ChrB"),
	LTEXT("Command"),
	LTEXT("Cos"),
	LTEXT("CreateDatabase"),
	LTEXT("CreateObject"),
	LTEXT("CurDir"),
	LTEXT("DDB"),
	LTEXT("Date"),
	LTEXT("DateAdd"),
	LTEXT("DateDiff"),
	LTEXT("DatePart"),
	LTEXT("DateSerial"),
	LTEXT("DateValue"),
	LTEXT("Day"),
	LTEXT("Dir"),
	LTEXT("DoEvents"),
	LTEXT("EOF"),
	LTEXT("Environ"),
	LTEXT("Error"),
	LTEXT("Exp"),
	LTEXT("FV"),
	LTEXT("FileAttr"),
	LTEXT("FileDateTime"),
	LTEXT("FileLen"),
	LTEXT("Fix"),
	LTEXT("Format"),
	LTEXT("FreeFile"),
	LTEXT("GetAllSettings"),
	LTEXT("GetAttr"),
	LTEXT("GetObject"),
	LTEXT("GetSetting"),
	LTEXT("Hex"),
	LTEXT("Hour"),
	LTEXT("IIf"),
	LTEXT("IMEStatus"),
	LTEXT("IPmt"),
	LTEXT("IRR"),
	LTEXT("InStr"),
	LTEXT("Input"),
	LTEXT("Int"),
	LTEXT("IsArray"),
	LTEXT("IsDate"),
	LTEXT("IsEmpty"),
	LTEXT("IsError"),
	LTEXT("IsMissing"),
	LTEXT("IsNull"),
	LTEXT("IsNumeric"),
	LTEXT("IsObject"),
	LTEXT("LBound"),
	LTEXT("LCase"),
	LTEXT("LOF"),
	LTEXT("LTrim"),
	LTEXT("Left"),
	LTEXT("LeftB"),
	LTEXT("Len"),
	LTEXT("LoadPicture"),
	LTEXT("Loc"),
	LTEXT("Log"),
	LTEXT("MIRR"),
	LTEXT("Mid"),
	LTEXT("MidB"),
	LTEXT("Minute"),
	LTEXT("Month"),
	LTEXT("MsgBox"),
	LTEXT("NPV"),
	LTEXT("NPer"),
	LTEXT("Now"),
	LTEXT("Oct"),
	LTEXT("OpenDatabase"),
	LTEXT("PPmt"),
	LTEXT("PV"),
	LTEXT("Partition"),
	LTEXT("Pmt"),
	LTEXT("QBColor"),
	LTEXT("RGB"),
	LTEXT("RTrim"),
	LTEXT("Rate"),
	LTEXT("ReadProperty"),
	LTEXT("Right"),
	LTEXT("RightB"),
	LTEXT("Rnd"),
	LTEXT("SLN"),
	LTEXT("SYD"),
	LTEXT("Second"),
	LTEXT("Seek"),
	LTEXT("Sgn"),
	LTEXT("Shell"),
	LTEXT("Sin"),
	LTEXT("Space"),
	LTEXT("Spc"),
	LTEXT("Sqr"),
	LTEXT("Str"),
	LTEXT("StrComp"),
	LTEXT("StrConv"),
	LTEXT("Switch"),
	LTEXT("Tab"),
	LTEXT("Tan"),
	LTEXT("Time"),
	LTEXT("TimeSerial"),
	LTEXT("TimeValue"),
	LTEXT("Timer"),
	LTEXT("Trim"),
	LTEXT("TypeName"),
	LTEXT("UBound"),
	LTEXT("UCase"),
	LTEXT("Val"),
	LTEXT("VarType"),
	LTEXT("Weekday"),
	LTEXT("Year"),
	LTEXT("Hide"),
	LTEXT("Line"),
	LTEXT("Refresh"),
	LTEXT("Show"),
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	//$付き関数各種
	LTEXT("Dir$"),
	LTEXT("LCase$"),
	LTEXT("Left$"),
	LTEXT("LeftB$"),
	LTEXT("Mid$"),
	LTEXT("MidB$"),
	LTEXT("RightB$"),
	LTEXT("Right$"),
	LTEXT("Space$"),
	LTEXT("Str$"),
	LTEXT("String$"),
	LTEXT("Trim$"),
	LTEXT("UCase$"),
	//VB5,6の隠し関数
	LTEXT("VarPtr"),
	LTEXT("StrPtr"),
	LTEXT("ObjPtr"),
	LTEXT("VarPrtArray"),
	LTEXT("VarPtrStringArray")
};
int g_nKeywordsVB2 = int(std::size(g_ppszKeywordsVB2));
