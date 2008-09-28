#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/CColorStrategy.h"

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
	_tcscpy( pType->m_szTypeName, _T("テキスト") );
	_tcscpy( pType->m_szTypeExts, _T("txt,log,1st,err,ps") );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(120);					/* 折り返し桁数 */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
	pType->m_bKinsokuHead = FALSE;								/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;								/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;								/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;								/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"!%),.:;?]}￠°’”‰′″℃、。々〉》」』】〕゛゜ゝゞ・ヽヾ！％），．：；？］｝｡｣､･ﾞﾟ￠" );		/* 行頭禁則 */	//@@@ 2002.04.13 MIK 
	wcscpy( pType->m_szKinsokuTail, L"$([{￡\\‘“〈《「『【〔＄（［｛｢￡￥" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK 
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
	const wchar_t*	pszStarts = GetDllShareData().m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine )break;

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
		if(NULL==wcschr(pszStarts,pLine[0]))continue;

		//見出し種類の判別 -> szTitle
		if( pLine[i] == L'(' ){
			if(0){}
			else if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //数字
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //英大文字
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //英小文字
			else continue; //※「(」の次が英数字で無い場合、見出しとみなさない
		}
		else if( IsInRange(pLine[i], L'０', L'９') ) wcscpy( szTitle, L"０" ); // 全角数字
		else if( IsInRange(pLine[i], L'①', L'⑳') ) wcscpy( szTitle, L"①" ); // ①～⑳
		else if( IsInRange(pLine[i], L'Ⅰ', L'Ⅹ') ) wcscpy( szTitle, L"Ⅰ" ); // Ⅰ～Ⅹ
		else if( wcschr(L"〇一二三四五六七八九十百零壱弐参伍", pLine[i]) ) wcscpy( szTitle, L"一" ); //漢数字
		else{
			szTitle[0]=pLine[i];
			szTitle[1]=L'\0';
		}

		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

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
		delete [] pszText;

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

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta 判定順序変更
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca 階層が2段位上深くなるときは、無題の要素を追加
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(無題)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // 書き込み先
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

