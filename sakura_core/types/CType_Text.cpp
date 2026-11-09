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
#include "env/DLLSHAREDATA.h"
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/EColorIndexType.h"
#include "CSelectLang.h"

/* テキスト */
//Sep. 20, 2000 JEPRO テキストの規定値を80→120に変更(不具合一覧.txtがある程度読みやすい桁数)
//Nov. 15, 2000 JEPRO PostScriptファイルも読めるようにする
//Jan. 12, 2001 JEPRO readme.1st も読めるようにする
//Feb. 12, 2001 JEPRO .err エラーメッセージ
//Nov.  6, 2002 genta docはMS Wordに譲ってここからは外す（関連づけ防止のため）
//Nov.  6, 2002 genta log を追加
void CType_Text::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("テキスト") );
	wcscpy( pType->m_szTypeExts, LTEXT("txt,log,1st,err,ps") );

	//設定
	pType->m_nMaxLineKetas = CKetaXInt(120);					/* 折り返し桁数 */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
	pType->m_bKinsokuHead = false;								// 行頭禁則				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = false;								// 行末禁則				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = false;								// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = false;								// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, LTEXT("!%),.:;?]}\xa2°’”‰′″℃、。々〉》」』】〕゛゜ゝゞ・ヽヾ！％），．：；？］｝｡｣､･ﾞﾟ￠") );		/* 行頭禁則 */	//@@@ 2002.04.13 MIK 
	wcscpy( pType->m_szKinsokuTail, LTEXT("$([\\{\xa3\xa5‘“〈《「『【〔＄（［｛｢￡￥") );		/* 行末禁則 */	//@@@ 2002.04.08 MIK 
	// pType->m_szKinsokuKuto（句読点ぶら下げ文字）はここではなく全タイプにデフォルト設定	// 2009.08.07 ryoji 

	//※小さな親切として、C:\～～ や \\～～ などのファイルパスをクリッカブルにする設定を「テキスト」に既定で仕込む
	//※""で挟まれる設定は挟まれない設定よりも上に無ければならない
	//※""で挟まれる設定を複製してちょっと修正すれば、<>や[]に挟まれたものにも対応できる（ユーザーに任せる）

	//正規表現キーワード
	int keywordPos = 0;
	WCHAR* pKeyword = pType->m_RegexKeywordList;
	pType->m_bUseRegexKeyword = true;							// 正規表現キーワードを使うか
	pType->m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_URL;	// 色指定番号
	::wcsncpy_s(&pKeyword[keywordPos],			// 正規表現キーワード
		std::size(pType->m_RegexKeywordList) - keywordPos,
		LTEXT("/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k"),			//   ""で挟まれた C:\～, \\～ にマッチするパターン
		_TRUNCATE
	);
	keywordPos += int(wcslen(&pKeyword[keywordPos]) + 1);
	pType->m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_URL;	// 色指定番号
	::wcsncpy_s(&pKeyword[keywordPos],			// 正規表現キーワード
		std::size(pType->m_RegexKeywordList) - keywordPos,
		LTEXT("/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k"),		//   C:\～, \\～ にマッチするパターン
		_TRUNCATE
	);
	keywordPos += int(wcslen(&pKeyword[keywordPos]) + 1);
	pKeyword[keywordPos] = LCHAR('\0');
}

/*!	テキスト・トピックリスト作成
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()を使用するように改訂。
	@date 2002.11.03 Moca	階層が最大値を超えるとバッファオーバーランするのを修正
							最大値以上は追加せずに無視する
	@date 2007.8頃   kobake 機械的にUNICODE化
	@date 2007.11.29 kobake UNICODE対応できてなかったので修正
*/
void CDocOutline::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//見出し記号
	const WCHAR*	pszStarts = GetDllShareData().m_Common.m_sFormat.m_szMidashiKigou;
	auto nStartsLen = int(wcslen(pszStarts));

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダーは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	WCHAR pszStack[nMaxStack][32];
	WCHAR szTitle[32];			//	一時領域
	CLogicInt				nLineCount;
	bool b278a = false;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const WCHAR*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( nullptr == pLine )break;

		//行頭の空白飛ばし
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::IsBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		int j;
		int nCharChars;
		int nCharChars2;
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
		for( j = 0; j < nStartsLen; j += nCharChars2 ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars2 = CNativeW::GetSizeOfChar( pszStarts, nStartsLen, j );
			if( nCharChars == nCharChars2 ){
				if( 0 == wmemcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}

		//見出し種類の判別 -> szTitle
		if( pLine[i] == LCHAR('(') ){
			     if ( IsInRange(pLine[i + 1], LCHAR('0'), LCHAR('9')) ) wcscpy( szTitle, LTEXT("(0)") ); //数字
			else if ( IsInRange(pLine[i + 1], LCHAR('A'), LCHAR('Z')) ) wcscpy( szTitle, LTEXT("(A)") ); //英大文字
			else if ( IsInRange(pLine[i + 1], LCHAR('a'), LCHAR('z')) ) wcscpy( szTitle, LTEXT("(a)") ); //英小文字
			else continue; //※「(」の次が英数字で無い場合、見出しとみなさない
		}
		else if( IsInRange(pLine[i], LCHAR('０'), LCHAR('９')) ) wcscpy( szTitle, LTEXT("０") ); // 全角数字
		else if( IsInRange(pLine[i], LCHAR('①'), LCHAR('⑳')) || pLine[i] == LCHAR('\u24ea')
			|| IsInRange(pLine[i], LCHAR('\u3251'), LCHAR('\u325f')) || IsInRange(pLine[i], LCHAR('\u32b1'), LCHAR('\u32bf')) ) wcscpy( szTitle, LTEXT("①") ); // ①～⑳ ○0　○21○35　○36○50
		else if( IsInRange(pLine[i], LCHAR('Ⅰ'), LCHAR('\u216f')) ) wcscpy( szTitle, LTEXT("Ⅰ") ); // Ⅰ～Ⅹ　XIXIILCDM
		else if( IsInRange(pLine[i], LCHAR('ⅰ'), LCHAR('\u217f')) ) wcscpy( szTitle, LTEXT("Ⅰ") ); // Ⅰ～Ⅹ　xixiilcdm
		else if( IsInRange(pLine[i], LCHAR('\u2474'), LCHAR('\u2487')) ) wcscpy( szTitle, LTEXT("\u2474") ); // (1)-(20)
		else if( IsInRange(pLine[i], LCHAR('\u2488'), LCHAR('\u249b')) ) wcscpy( szTitle, LTEXT("\u2488") ); // 1.-20.
		else if( IsInRange(pLine[i], LCHAR('\u249c'), LCHAR('\u24b5')) ) wcscpy( szTitle, LTEXT("\u249c") ); // (a)-(z)
		else if( IsInRange(pLine[i], LCHAR('\u24b6'), LCHAR('\u24cf')) ) wcscpy( szTitle, LTEXT("\u24b6") ); // ○A-○Z
		else if( IsInRange(pLine[i], LCHAR('\u24d0'), LCHAR('\u24e9')) ) wcscpy( szTitle, LTEXT("\u24d0") ); // ○a-○z
		else if( IsInRange(pLine[i], LCHAR('\u24eb'), LCHAR('\u24f4')) ){ // ●11-●20
			if(b278a){ wcscpy( szTitle, LTEXT("\u278a") ); }
			else{ wcscpy( szTitle, LTEXT("\u2776") ); } }
		else if( IsInRange(pLine[i], LCHAR('\u24f5'), LCHAR('\u24fe')) ) wcscpy( szTitle, LTEXT("\u24f5") ); // ◎1-◎10
		else if( IsInRange(pLine[i], LCHAR('\u2776'), LCHAR('\u277f')) ) wcscpy( szTitle, LTEXT("\u2776") ); // ●1-●10
		else if( IsInRange(pLine[i], LCHAR('\u2780'), LCHAR('\u2789')) ) wcscpy( szTitle, LTEXT("\u2780") ); // ○1-○10
		else if( IsInRange(pLine[i], LCHAR('\u278a'), LCHAR('\u2793')) ){ wcscpy( szTitle, LTEXT("\u278a") ); b278a = true; } // ●1-●10(SANS-SERIF)
		else if( IsInRange(pLine[i], LCHAR('\u3220'), LCHAR('\u3229')) ) wcscpy( szTitle, LTEXT("\ua3220") ); // (一)-(十)
		else if( IsInRange(pLine[i], LCHAR('\u3280'), LCHAR('\u3289')) ) wcscpy( szTitle, LTEXT("\u3220") ); // ○一-○十
		else if( IsInRange(pLine[i], LCHAR('\u32d0'), LCHAR('\u32fe')) ) wcscpy( szTitle, LTEXT("\u32d0") ); // ○ア-○ヲ
		else if( wcschr(LTEXT("〇一二三四五六七八九十百零壱弐参伍"), pLine[i]) ) wcscpy( szTitle, LTEXT("一") ); //漢数字
		else{
			wcsncpy( szTitle, &pLine[i], nCharChars );	//	先頭文字をszTitleに保持。
			szTitle[nCharChars] = LCHAR('\0');
		}

		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		const WCHAR*	pszText = &pLine[i];
		nLineLen -= i;
		const bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::IsLineDelimiter(pszText[i], bExtEol) ){
				break;
			}
		}
		sakura::wstring strText( pszText, i );
		pszText = strText.c_str();

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーラン
			// nDepth = nMaxStack;
			bAppend = false;
		}

		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
	}
	return;
}

/*! 階層付きテキスト アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2003.05.25 genta 実装方法一部修正
	@date 2003.06.21 Moca 階層が2段以上深くなる場合を考慮
*/
void CDocOutline::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const WCHAR*	pLine;
		CLogicInt		nLineLen;

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta 判定順序変更
		if( *pLine == LCHAR('.') )
		{
			const WCHAR* pPos;	//	May 25, 2003 genta
			int			nLength;
			WCHAR		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for( pPos = pLine + 1 ; *pPos == LCHAR('.') ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = int(pPos - pLine);

			// 2003.06.27 Moca 階層が2段位上深くなるときは、無題の要素を追加
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						LS(STR_NO_TITLE1),
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,LTEXT("%d - "), level );
			
			WCHAR *pDest = szTitle + nLength; // 書き込み先
			WCHAR *pDestEnd = szTitle + int(std::size(szTitle)) - 2;
			
			while( pDest < pDestEnd )
			{
				if( WCODE::IsLineDelimiter(*pPos, bExtEol) || *pPos == LCHAR('\0'))
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = LCHAR('\0');
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}
