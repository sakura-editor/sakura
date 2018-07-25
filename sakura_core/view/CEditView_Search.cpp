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
#include <limits.h>
#include "CEditView.h"
#include "window/CEditWnd.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

const int STRNCMP_MAX = 100;	/* MAXキーワード長：strnicmp文字列比較最大値(CEditView::KeySearchCore) */	// 2006.04.10 fon

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! キーワード辞書検索の前提条件チェックと、検索

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_Rから分離
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CNativeW	cmemCurText;

	/* キーワードヘルプを使用するか？ */
	if( !m_pTypeData->m_bUseKeyWordHelp )	/* キーワードヘルプ機能を使用する */	// 2006.04.10 fon
		goto end_of_search;
	/* フォーカスがあるか？ */
	if( !GetCaret().ExistCaretFocus() ) 
		goto end_of_search;
	/* ウィンドウ内にマウスカーソルがあるか？ */
	GetCursorPos( po );
	GetWindowRect( GetHwnd(), rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* 右コメントの１～３でない場合 */
		if(!( m_bInMenuLoop == FALSE	&&			/* １．メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
			300 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* １．メニュー モーダル ループに入っていない */
		//	0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	default:
		PleaseReportToAuthor( NULL, _T("CEditView::KeyWordHelpSearchDict\nnID=%d"), (int)nID );
	}
	/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
	if( GetSelectedDataOne( cmemCurText, STRNCMP_MAX + 1 ) ){
	}
	/* キャレット位置の単語を取得する処理 */	// 2006.03.24 fon
	else if(GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetParser().GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}
	else
		goto end_of_search;

	if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* 既に検索済みか */
		(!m_cTipWnd.m_KeyWasHit) )								/* 該当するキーがなかった */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* 検索実行 */
	if( !KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* 辞書Tipを表示している */
	m_poTipCurPos = *po;	/* 現在のマウスカーソル位置 */
	return TRUE;			/* ここまで来ていればヒット・ワード */

	/* キーワードヘルプ表示処理終了 */
	end_of_search:
	return FALSE;
}

/*! キーワード辞書検索処理メイン

	@date 2006.04.10 fon KeyWordHelpSearchDictから分離
*/
BOOL CEditView::KeySearchCore( const CNativeW* pcmemCurText )
{
	CNativeW*	pcmemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	m_cTipWnd.m_cInfo.SetString( _T("") );	/* tooltipバッファ初期化 */
	/* 1行目にキーワード表示の場合 */
	if(m_pTypeData->m_bUseKeyHelpKeyDisp){	/* キーワードも表示する */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendString( _T("[ ") );
		m_cTipWnd.m_cInfo.AppendString( pcmemCurText->GetStringT() );
		m_cTipWnd.m_cInfo.AppendString( _T(" ]") );
	}
	/* 途中まで一致を使う場合 */
	if(m_pTypeData->m_bUseKeyHelpPrefix)
		nCmpLen = wcslen( pcmemCurText->GetStringPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i =0 ; i < m_pTypeData->m_nKeyHelpNum; i++){	//最大数：MAX_KEYHELP_FILE
		if( m_pTypeData->m_KeyHelpArr[i].m_bUse ){
			// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)引数を追加
			CNativeW*	pcmemRefText;
			int nSearchResult=m_cDicMgr.CDicMgr::Search(
				pcmemCurText->GetStringPtr(),
				nCmpLen,
				&pcmemRefKey,
				&pcmemRefText,
				m_pTypeData->m_KeyHelpArr[i].m_szPath,
				&nLine
			);
			if(nSearchResult){
				/* 該当するキーがある */
				LPWSTR		pszWork;
				pszWork = pcmemRefText->GetStringPtr();
				/* 有効になっている辞書を全部なめて、ヒットの都度説明の継ぎ増し */
				if(m_pTypeData->m_bUseKeyHelpAllSearch){	/* ヒットした次の辞書も検索 */	// 2006.04.10 fon
					/* バッファに前のデータが詰まっていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( LS(STR_ERR_DLGEDITVW5) );
					else
						m_cTipWnd.m_cInfo.AppendString( LS(STR_ERR_DLGEDITVW6) );	/* 先頭の場合 */
					/* 辞書のパス挿入 */
					{
						TCHAR szFile[MAX_PATH];
						// 2013.05.08 表示するのはファイル名(拡張子なし)のみにする
						_tsplitpath( m_pTypeData->m_KeyHelpArr[i].m_szPath, NULL, NULL, szFile, NULL );
						m_cTipWnd.m_cInfo.AppendString( szFile );
					}
					m_cTipWnd.m_cInfo.AppendString( _T("\n") );
					/* 前方一致でヒットした単語を挿入 */
					if(m_pTypeData->m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					if(!m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* 辞書を開くとき最初にヒットした辞書を開く */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}
				else{	/* 最初のヒット項目のみ返す場合 */
					/* キーワードが入っていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n") );
					
					/* 前方一致でヒットした単語を挿入 */
					if(m_pTypeData->m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}
					
					/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if( m_cTipWnd.m_KeyWasHit != FALSE ){
			return TRUE;
	}
	/* 該当するキーがなかった場合 */
	return FALSE;
}

bool CEditView::MiniMapCursorLineTip( POINT* po, RECT* rc, bool* pbHide )
{
	*pbHide = true;
	if( !m_bMiniMap ){
		return false;
	}
	// ウィンドウ内にマウスカーソルがあるか？
	GetCursorPos( po );
	GetWindowRect( GetHwnd(), rc );
	rc->right -= ::GetSystemMetrics(SM_CXVSCROLL);
	if( !PtInRect( rc, *po ) ){
		return false;
	}
	if(!( m_bInMenuLoop == FALSE	&&			/* １．メニュー モーダル ループに入っていない */
		300 < ::GetTickCount() - m_dwTipTimer	/* ２．一定時間以上、マウスが固定されている */
	) ){
		return false;
	}
	if( WindowFromPoint( *po ) != GetHwnd() ){
		return false;
	}

	CMyPoint ptClient(*po);
	ScreenToClient( GetHwnd(), &ptClient );
	CLayoutPoint ptNew;
	GetTextArea().ClientToLayout( ptClient, &ptNew );
	// 同じ行ならなにもしない
	if( 0 == m_dwTipTimer && m_cTipWnd.m_nSearchLine == (Int)ptNew.y ){
		*pbHide = false; // 表示継続
		return false;
	}
	CNativeW cmemCurText;
	CLayoutYInt nTipBeginLine = ptNew.y;
	CLayoutYInt nTipEndLine = ptNew.y + CLayoutYInt(4);
	for( CLayoutYInt nCurLine = nTipBeginLine; nCurLine < nTipEndLine; nCurLine++ ){
		const CLayout* pcLayout = NULL;
		if( 0 <= nCurLine ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nCurLine );
		}
		if( pcLayout ){
			CNativeW cmemCurLine;
			{
				CLogicInt nLineLen = pcLayout->GetLengthWithoutEOL();
				const wchar_t* pszData = pcLayout->GetPtr();
				int nLimitLength = 80;
				int pre = 0;
				int i = 0;
				int k = 0;
				int charSize = CNativeW::GetSizeOfChar( pszData, nLineLen, i );
				int charWidth = t_max(1, (int)(Int)CNativeW::GetKetaOfChar( pszData, nLineLen, i ));
				int charType = 0;
				// 連続する"\t" " " を " "1つにする
				// 左からnLimitLengthまでの幅を切り取り
				while( i + charSize <= (Int)nLineLen && k + charWidth <= nLimitLength ){
					if( pszData[i] == L'\t' || pszData[i] == L' ' ){
						if( charType == 0 ){
							cmemCurLine.AppendString( pszData + pre , i - pre );
							cmemCurLine.AppendString( L" " );
							charType = 1;
						}
						pre = i + charSize;
						k++;
					}else{
						k += charWidth;
						charType = 0;
					}
					i += charSize;
					charSize = CNativeW::GetSizeOfChar( pszData, nLineLen, i );
					charWidth = t_max(1, (int)(Int)CNativeW::GetKetaOfChar( pszData, nLineLen, i ));
				}
				cmemCurLine.AppendString( pszData + pre , i - pre );
			}
			if( nTipBeginLine != nCurLine ){
				cmemCurText.AppendString( L"\n" );
			}
			cmemCurLine.Replace( L"\\", L"\\\\" );
			cmemCurText.AppendNativeData( cmemCurLine );
		}
	}
	if( cmemCurText.GetStringLength() <= 0 ){
		return false;
	}
	m_cTipWnd.m_cKey = cmemCurText;
	m_cTipWnd.m_cInfo = cmemCurText.GetStringT();
	m_cTipWnd.m_nSearchLine = (Int)ptNew.y;
	m_dwTipTimer = 0;		// 辞書Tipを表示している */
	m_poTipCurPos = *po;	// 現在のマウスカーソル位置 */
	return true;			// ここまで来ていればヒット・ワード
}

/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText, bool bStripMaxPath /* = true */, bool bTrimSpaceTab /* = false */ )
{

	int				i;
	CNativeW		cmemTopic = L"";
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLayoutRange	sRange;

	cmemCurText.SetString(L"");
	if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 選択範囲のデータを取得 */
		if( GetSelectedDataOne( cmemCurText, INT_MAX ) ){
			/* 検索文字列を現在位置の単語で初期化 */
			if( bStripMaxPath ){
				LimitStringLengthW(cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), _MAX_PATH - 1, cmemTopic);
			}else{
				cmemTopic = cmemCurText;
			}
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdx = LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

			/* 現在位置の単語の範囲を調べる */
			bool bWhere = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(),
				nIdx,
				&sRange,
				NULL,
				NULL
			);
			if( bWhere ){
				/* 選択範囲の変更 */
				GetSelectionInfo().m_sSelectBgn = sRange;
				GetSelectionInfo().m_sSelect    = sRange;

				/* 選択範囲のデータを取得 */
				if( GetSelectedDataOne( cmemCurText, INT_MAX ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					if( bStripMaxPath ){
						LimitStringLengthW(cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), _MAX_PATH - 1, cmemTopic);
					}else{
						cmemTopic = cmemCurText;
					}
				}
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( false );
			}
		}
	}

	wchar_t *pTopic2 = cmemTopic.GetStringPtr();
	if( bTrimSpaceTab ){
		// 前のスペース・タブを取り除く
		while( L'\0' != *pTopic2 && ( ' ' == *pTopic2 || '\t' == *pTopic2 ) ){
			pTopic2++;
		}
	}
	int nTopic2Len = (int)wcslen( pTopic2 );
	/* 検索文字列は改行まで */
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	for( i = 0; i < nTopic2Len; ++i ){
		if( WCODE::IsLineDelimiter(pTopic2[i], bExtEol) ){
			break;
		}
	}
	
	if( bTrimSpaceTab ){
		// 後ろのスペース・タブを取り除く
		int m = i - 1;
		while( 0 <= m &&
		    ( L' ' == pTopic2[m] || L'\t' == pTopic2[m] ) ){
			m--;
		}
		if( 0 <= m ){
			i = m + 1;
		}
	}
	cmemCurText.SetString( pTopic2, i );
}


/*!	現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）
	@return 値を設定したか
	@date 2006.08.23 ryoji 新規作成
	@date 2014.07.01 Moca bGetHistory追加、戻り値をboolに変更
*/
bool CEditView::GetCurrentTextForSearchDlg( CNativeW& cmemCurText, bool bGetHistory )
{
	bool bStripMaxPath = false;
	cmemCurText.SetString(L"");

	if( GetSelectionInfo().IsTextSelected() ){	// テキストが選択されている
		GetCurrentTextForSearch( cmemCurText, bStripMaxPath );
	}
	else{	// テキストが選択されていない
		bool bGet = false;
		if( GetDllShareData().m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText, bStripMaxPath );	// カーソル位置単語を取得
			if( cmemCurText.GetStringLength() == 0 && bGetHistory ){
				bGet = true;
			}
		}else{
			bGet = true;
		}
		if( bGet ){
			if( 0 < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size()
					&& m_nCurSearchKeySequence < GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence ){
				cmemCurText.SetString( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	// 履歴からとってくる
				return true; // ""でもtrue
			}else{
				cmemCurText.SetString( m_strCurSearchKey.c_str() );
				return 0 <= m_nCurSearchKeySequence; // ""でもtrue.未設定のときはfalse
			}
		}
	}
	return 0 < cmemCurText.GetStringLength();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        描画用判定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 現在位置が検索文字列に該当するか */
//2002.02.08 hor
//正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
//Jun. 26, 2001 genta	正規表現ライブラリの差し替え
/*
	@retval 0
		(パターン検索時) 指定位置以降にマッチはない。
		(それ以外) 指定位置は検索文字列の始まりではない。
	@retval 1,2,3,...
		(パターン検索時) 指定位置以降にマッチが見つかった。
		(単語検索時) 指定位置が検索文字列に含まれる何番目の単語の始まりであるか。
		(それ以外) 指定位置が検索文字列の始まりだった。
*/
int CEditView::IsSearchString(
	const CStringRef&	cStr,
	/*
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	*/
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
) const
{
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* 行頭ではない? */
		/* 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと */

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if( m_CurRegexp.Match( cStr.GetPtr(), cStr.GetLength(), nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
			return 1;
		}
		else{
			return 0;
		}
	}
	else if( m_sCurSearchOption.bWordOnly ) { // 単語検索
		/* 指定位置の単語の範囲を調べる */
		CLogicInt posWordHead, posWordEnd;
		if( ! CWordParse::WhereCurrentWord_2( cStr.GetPtr(), CLogicInt(cStr.GetLength()), nPos, &posWordHead, &posWordEnd, NULL, NULL ) ) {
			return 0; // 指定位置に単語が見つからなかった。
 		}
		if( nPos != posWordHead ) {
			return 0; // 指定位置は単語の始まりではなかった。
		}
		const CLogicInt wordLength = posWordEnd - posWordHead;
		const wchar_t *const pWordHead = cStr.GetPtr() + posWordHead;

		// 比較関数
		int (*const fcmp)( const wchar_t*, const wchar_t*, size_t ) = m_sCurSearchOption.bLoHiCase ? wcsncmp : wcsnicmp;

		// 検索語を単語に分割しながら指定位置の単語と照合する。
		int wordIndex = 0;
		const wchar_t* const searchKeyEnd = m_strCurSearchKey.data() + m_strCurSearchKey.size();
		for( const wchar_t* p = m_strCurSearchKey.data(); p < searchKeyEnd; ) {
			CLogicInt begin, end; // 検索語に含まれる単語?の位置。WhereCurrentWord_2()の仕様では空白文字列も単語に含まれる。
			if( CWordParse::WhereCurrentWord_2( p, CLogicInt(searchKeyEnd - p), CLogicInt(0), &begin, &end, NULL, NULL )
				&& begin == 0 && begin < end
			) {
				if( ! WCODE::IsWordDelimiter( *p ) ) {
					++wordIndex;
					// p...(p + end) が検索語に含まれる wordIndex番目の単語。(wordIndexの最初は 1)
					if( wordLength == end && 0 == fcmp( p, pWordHead, wordLength ) ) {
						*pnSearchStart = posWordHead;
						*pnSearchEnd = posWordEnd;
						return wordIndex;
					}
				}
				p += end;
			} else {
				p += CNativeW::GetSizeOfChar( p, searchKeyEnd - p, 0 );
			}
		}
		return 0; // 指定位置の単語と検索文字列に含まれる単語は一致しなかった。
	}
	else {
		const wchar_t* pHit = CSearchAgent::SearchString(cStr.GetPtr(), cStr.GetLength(), nPos, m_sSearchPattern);
		if( pHit ){
			*pnSearchStart = pHit - cStr.GetPtr();
			*pnSearchEnd = *pnSearchStart + m_sSearchPattern.GetLen();
			return 1;
		}
		return 0; // この行はヒットしなかった
	}
	return 0;
}
