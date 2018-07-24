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
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

/* Visual Basic */
//JUl. 10, 2001 JEPRO VB   ユーザに贈る
//Jul. 09, 2001 JEPRO 追加 //Dec. 16, 2002 MIK追加 // Feb. 19, 2006 genta .vb追加
void CType_Vb::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Visual Basic") );
	_tcscpy( pType->m_szTypeExts, _T("bas,frm,cls,ctl,pag,dob,dsr,vb") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"'", -1 );				/* 行コメントデリミタ */
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
	const wchar_t*	pLine;
	CLogicInt		nLineLen = CLogicInt(0);//: 2002/2/3 aroka 警告対策：初期化
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	wchar_t		szWord[256];		// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	int			nWordIdx = 0;
	int			nMode;
	wchar_t		szFuncName[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	CLogicInt	nFuncLine(0);
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// クラスモジュールフラグ
	bool		bProcedure;		// プロシージャフラグ（プロシージャ内ではTrue）
	bool		bDQuote;		// ダブルクォーテーションフラグ（ダブルクォーテーションがきたらTrue）
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;


	// 調べるファイルがクラスモジュールのときはType、Constの挙動が異なるのでフラグを立てる
	bClass	= false;
	int filelen = _tcslen(m_pcDocRef->m_cDocFile.GetFilePath());
	if ( 4 < filelen ) {
		if ( 0 == _tcsicmp((m_pcDocRef->m_cDocFile.GetFilePath() + filelen - 4), _FT(".cls")) ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	pLine = NULL;
	bProcedure	= false;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( L'_' != pLine[nLineLen-1]){
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
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						i += (nCharChars - 1);
						continue;
					}else{
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Public") ) {
						// パブリック宣言を見つけた！
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Private") ) {
						// プライベート宣言を見つけた！
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Friend") ) {
						// フレンド宣言を見つけた！
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Static") ) {
						// スタティック宣言を見つけた！
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Function" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x01;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Sub" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x02;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Get" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId	|= 0x03;		// プロパティ取得
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Let" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x04;		// プロパティ設定
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Set" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x05;		// プロパティ参照
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Const" )
					 && 0 != wcsicmp( szWordPrev, L"#" )
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
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Enum" )
					){
						nFuncId	|= 0x207;		// 列挙型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Type" )
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
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Event" )
					){
						nFuncId	|= 0x209;		// イベント宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Property" )
					 && 0 == wcsicmp( szWordPrev, L"End")
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
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}
			/* 記号列読み込み中 */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// 「#Const」と「Const」を区別するために、「#」も識別するように変更
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 2013.05.08 日本語対応
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'/' == pLine[i]	||
					L'-' == pLine[i] ||
					L'#' == pLine[i] ||
					2 == nCharChars
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				){
					continue;
				}else
				if( i < nLineLen && L'\'' == pLine[i] ){
					break;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 2013.05.08 日本語対応
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* テキストが閉じるまで読み飛ばす */	// Aug 7, 2003 little YOSHI  追加
			if (nMode == 3) {
				// 連続するダブルクォーテーションは無視する
				if (1 == nCharChars && L'"' == pLine[i]) {
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
const wchar_t* g_ppszKeywordsVB[] = {
	L"And",
	L"As",
	L"Attribute",
	L"Begin",
	L"BeginProperty",
	L"Boolean",
	L"ByVal",
	L"Byte",
	L"Call",
	L"Case",
	L"Const",
	L"Currency",
	L"Date",
	L"Declare",
	L"Dim",
	L"Do",
	L"Double",
	L"Each",
	L"Else",
	L"ElseIf",
	L"Empty",
	L"End",
	L"EndProperty",
	L"Error",
	L"Eqv",
	L"Exit",
	L"False",
	L"For",
	L"Friend",
	L"Function",
	L"Get",
	L"GoTo",
	L"If",
	L"Imp",
	L"Integer",
	L"Is",
	L"Let",
	L"Like",
	L"Long",
	L"Loop",
	L"Me",
	L"Mod",
	L"New",
	L"Next",
	L"Not",
	L"Null",
	L"Object",
	L"On",
	L"Option",
	L"Or",
	L"Private",
	L"Property",
	L"Public",
	L"RSet",
	L"ReDim",
	L"Rem",
	L"Resume",
	L"Select",
	L"Set",
	L"Single",
	L"Static",
	L"Step",
	L"Stop",
	L"String",
	L"Sub",
	L"Then",
	L"To",
	L"True",
	L"Type",
	L"Wend",
	L"While",
	L"With",
	L"Xor",
	L"#If",
	L"#Else",
	L"#End",
	L"#Const",
	L"AddressOf",
	L"Alias",
	L"Append",
	L"Array",
	L"ByRef",
	L"Explicit",
	L"Global",
	L"In",
	L"Lib",
	L"Nothing",
	L"Optional",
	L"Output",
	L"Terminate",
	L"Until",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	L"DefBool",
	L"DefByte",
	L"DefCur",
	L"DefDate",
	L"DefDbl",
	L"DefInt",
	L"DefLng",
	L"DefObj",
	L"DefSng",
	L"DefStr",
	L"DefVar",
	L"LSet",
	L"GoSub",
	L"Return",
	L"Variant",
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
int g_nKeywordsVB = _countof(g_ppszKeywordsVB);

//Jul. 10, 2001 JEPRO 追加
const wchar_t* g_ppszKeywordsVB2[] = {
	L"AppActivate",
	L"Beep",
	L"BeginTrans",
	L"ChDir",
	L"ChDrive",
	L"Close",
	L"CommitTrans",
	L"CompactDatabase",
	L"Date",
	L"DeleteSetting",
	L"Erase",
	L"FileCopy",
	L"FreeLocks",
	L"Input",
	L"Kill",
	L"Load",
	L"Lock",
	L"Mid",
	L"MidB",
	L"MkDir",
	L"Name",
	L"Open",
	L"Print",
	L"Put",
	L"Randomize",
	L"RegisterDatabase",
	L"RepairDatabase",
	L"Reset",
	L"RmDir",
	L"Rollback",
	L"SavePicture",
	L"SaveSetting",
	L"Seek",
	L"SendKeys",
	L"SetAttr",
	L"SetDataAccessOption",
	L"SetDefaultWorkspace",
	L"Time",
	L"Unload",
	L"Unlock",
	L"Width",
	L"Write",
	L"Array",
	L"Asc",
	L"AscB",
	L"Atn",
	L"CBool",
	L"CByte",
	L"CCur",
	L"CDate",
	L"CDbl",
	L"CInt",
	L"CLng",
	L"CSng",
	L"CStr",
	L"CVErr",
	L"CVar",
	L"Choose",
	L"Chr",
	L"ChrB",
	L"Command",
	L"Cos",
	L"CreateDatabase",
	L"CreateObject",
	L"CurDir",
	L"DDB",
	L"Date",
	L"DateAdd",
	L"DateDiff",
	L"DatePart",
	L"DateSerial",
	L"DateValue",
	L"Day",
	L"Dir",
	L"DoEvents",
	L"EOF",
	L"Environ",
	L"Error",
	L"Exp",
	L"FV",
	L"FileAttr",
	L"FileDateTime",
	L"FileLen",
	L"Fix",
	L"Format",
	L"FreeFile",
	L"GetAllSettings",
	L"GetAttr",
	L"GetObject",
	L"GetSetting",
	L"Hex",
	L"Hour",
	L"IIf",
	L"IMEStatus",
	L"IPmt",
	L"IRR",
	L"InStr",
	L"Input",
	L"Int",
	L"IsArray",
	L"IsDate",
	L"IsEmpty",
	L"IsError",
	L"IsMissing",
	L"IsNull",
	L"IsNumeric",
	L"IsObject",
	L"LBound",
	L"LCase",
	L"LOF",
	L"LTrim",
	L"Left",
	L"LeftB",
	L"Len",
	L"LoadPicture",
	L"Loc",
	L"Log",
	L"MIRR",
	L"Mid",
	L"MidB",
	L"Minute",
	L"Month",
	L"MsgBox",
	L"NPV",
	L"NPer",
	L"Now",
	L"Oct",
	L"OpenDatabase",
	L"PPmt",
	L"PV",
	L"Partition",
	L"Pmt",
	L"QBColor",
	L"RGB",
	L"RTrim",
	L"Rate",
	L"ReadProperty",
	L"Right",
	L"RightB",
	L"Rnd",
	L"SLN",
	L"SYD",
	L"Second",
	L"Seek",
	L"Sgn",
	L"Shell",
	L"Sin",
	L"Space",
	L"Spc",
	L"Sqr",
	L"Str",
	L"StrComp",
	L"StrConv",
	L"Switch",
	L"Tab",
	L"Tan",
	L"Time",
	L"TimeSerial",
	L"TimeValue",
	L"Timer",
	L"Trim",
	L"TypeName",
	L"UBound",
	L"UCase",
	L"Val",
	L"VarType",
	L"Weekday",
	L"Year",
	L"Hide",
	L"Line",
	L"Refresh",
	L"Show",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	//$付き関数各種
	L"Dir$",
	L"LCase$",
	L"Left$",
	L"LeftB$",
	L"Mid$",
	L"MidB$",
	L"RightB$",
	L"Right$",
	L"Space$",
	L"Str$",
	L"String$",
	L"Trim$",
	L"UCase$",
	//VB5,6の隠し関数
	L"VarPtr",
	L"StrPtr",
	L"ObjPtr",
	L"VarPrtArray",
	L"VarPtrStringArray"
};
int g_nKeywordsVB2 = _countof(g_ppszKeywordsVB2);
