/*!	@file
	@brief CEditViewクラスの補完関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "debug.h"
#include "charcode.h"  // 2006.06.28 rastiv
#include "CEditWnd.h"

/*!
	@brief コマンド受信前補完処理
	
	補完ウィンドウの非表示

	@date 2005.01.10 genta 関数化
*/
void CEditView::PreprocessCommand_hokan( int nCommand )
{
	/* 補完ウィンドウが表示されているとき、特別な場合を除いてウィンドウを非表示にする */
	if( m_bHokan ){
		if( nCommand != F_HOKAN		//	補完開始・終了コマンド
		 && nCommand != F_WCHAR		//	文字入力
		 && nCommand != F_IME_CHAR	//	漢字入力
		 ){
			m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
	}
}

/*!
	コマンド実行後補完処理

	@author Moca
	@date 2005.01.10 genta 関数化
*/
void CEditView::PostprocessCommand_hokan(void)
{
	if( m_pShareData->m_Common.m_sHelper.m_bUseHokan && !m_bExecutingKeyMacro ){ /* キーボードマクロの実行中 */
		CNativeW2	cmemData;

		/* カーソル直前の単語を取得 */
		if( 0 < GetParser().GetLeftWord( &cmemData, 100 ) ){
			ShowHokanMgr( cmemData, FALSE );
		}else{
			if( m_bHokan ){
				m_pcEditWnd->m_cHokanMgr.Hide();
				m_bHokan = FALSE;
			}
		}
	}
}

/*!	補完ウィンドウを表示する
	ウィンドウを表示した後は、HokanMgrに任せるので、ShowHokanMgrの知るところではない。
	
	@param cmemData [in] 補完する元のテキスト 「Ab」などがくる。
	@param bAutoDecided [in] 候補が1つだったら確定する

	@date 2005.01.10 genta CEditView_Commandから移動
*/
void CEditView::ShowHokanMgr( CNativeW2& cmemData, BOOL bAutoDecided )
{
	/* 補完対象ワードリストを調べる */
	CNativeW2	cmemHokanWord;
	int			nKouhoNum;
	POINT		poWin;
	/* 補完ウィンドウの表示位置を算出 */
	poWin.x = GetTextArea().GetAreaLeft()
			 + (Int)(GetCaret().GetCaretLayoutPos().GetX2() - GetTextArea().GetViewLeftCol())
			  * GetTextMetrics().GetHankakuDx();
	poWin.y = GetTextArea().GetAreaTop()
			 + (Int)(GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine())
			  * GetTextMetrics().GetHankakuDy();
	::ClientToScreen( m_hWnd, &poWin );
	poWin.x -= (
		cmemData.GetStringLength()
		 * GetTextMetrics().GetHankakuDx()
	);

	/*	補完ウィンドウを表示
		ただし、bAutoDecided == TRUEの場合は、補完候補が1つのときは、ウィンドウを表示しない。
		詳しくは、Search()の説明を参照のこと。
	*/
	CNativeW2* pcmemHokanWord;
	if ( bAutoDecided ){
		pcmemHokanWord = &cmemHokanWord;
	}
	else {
		pcmemHokanWord = NULL;
	}
	nKouhoNum = m_pcEditWnd->m_cHokanMgr.CHokanMgr::Search(
		&poWin,
		GetTextMetrics().GetHankakuHeight(),
		GetTextMetrics().GetHankakuDx(),
		cmemData.GetStringPtr(),
		m_pcEditDoc->GetDocumentAttribute().m_szHokanFile,
		m_pcEditDoc->GetDocumentAttribute().m_bHokanLoHiCase,
		m_pcEditDoc->GetDocumentAttribute().m_bUseHokanByFile, // 2003.06.22 Moca
		pcmemHokanWord
	);
	/* 補完候補の数によって動作を変える */
	if (nKouhoNum <= 0) {				//	候補無し
		if( m_bHokan ){
			m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
			// 2003.06.25 Moca 失敗してたら、ビープ音を出して補完終了。
			::MessageBeep( MB_ICONHAND );
		}
	}
	else if( bAutoDecided && nKouhoNum == 1){ //	候補1つのみ→確定。
		if( m_bHokan ){
			m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		// 2004.05.14 Moca CHokanMgr::Search側で改行を削除するようにし、直接書き換えるのをやめた
//		pszKouhoWord = cmemHokanWord.GetPtr( &nKouhoWordLen );
//		pszKouhoWord[nKouhoWordLen] = L'\0';
		GetCommander().Command_WordDeleteToStart();
		GetCommander().Command_INSTEXT( TRUE, cmemHokanWord.GetStringPtr(), cmemHokanWord.GetStringLength(), TRUE );
	}
	else {
		m_bHokan = TRUE;
	}
	
	//	補完終了。
	if ( !m_bHokan ){
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;	//	入力補完終了の知らせ
	}
}



/*!
	編集中データから入力補完キーワードの検索
	CHokanMgrから呼ばれる

	@return 候補数

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta  CEditView_Commandから移動
	@date 2007/10/17 kobake 読みやすいようにネストを浅くしました。
*/
int CEditView::HokanSearchByFile(
	const wchar_t*	pszKey,			//!< [in]
	BOOL			bHokanLoHiCase,	//!< [in] 英大文字小文字を同一視する
	CNativeW2**		ppcmemKouho,	//!< [in/out] 候補
	int				nKouhoNum,		//!< [in] ppcmemKouhoのすでに入っている数
	int				nMaxKouho		//!< [in] Max候補数(0==無制限)
){
	const int nKeyLen = wcslen( pszKey );
	int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	int j, nWordLen, nLineLen, nRet;

	const wchar_t* pszLine;
	const wchar_t* word;

	CLogicPoint ptCur = GetCaret().GetCaretLogicPos(); //物理カーソル位置
	
	for( CLogicInt i = CLogicInt(0); i < nLines; i++  ){
		pszLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( i, &nLineLen );
		for( j = 0; j < nLineLen; j++ ){
			//
			if( !IS_KEYWORD_CHAR( pszLine[j] ) )continue;

			//
			word = pszLine + j;
			for( j++, nWordLen = 1;j < nLineLen && IS_KEYWORD_CHAR( pszLine[j] ); j++ ){
				nWordLen++;
			}

			//
			if( nWordLen > 1020 ){ // CDicMgr等の制限により長すぎる単語は無視する
				continue;
			}
			if( nKeyLen > nWordLen )continue;

			//
			if( bHokanLoHiCase ){
				nRet = auto_memicmp( pszKey, word, nKeyLen );
			}else{
				nRet = auto_memcmp( pszKey, word, nKeyLen );
			}
			if( nRet!=0 )continue;

			// カーソル位置の単語は候補からはずす
			if( ptCur.y == i && ptCur.x <= j && j - nWordLen <= ptCur.x ){
				continue;
			}

			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CNativeW2;
				(*ppcmemKouho)->SetString( word, nWordLen );
				(*ppcmemKouho)->AppendString( L"\n" );
				++nKouhoNum;
			}
			else{
				// 重複していたら追加しない
				int nLen;
				const wchar_t* ptr = (*ppcmemKouho)->GetStringPtr( &nLen );
				int nPosKouho;
				nRet = 1;
				if( bHokanLoHiCase ){
					if( nWordLen < nLen ){
						if( L'\n' == ptr[nWordLen] && 0 == auto_memicmp( ptr, word, nWordLen )  ){
							nRet = 0;
						}
						else{
							int nPosKouhoMax = nLen - nWordLen - 1;
							for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
								if( ptr[nPosKouho] == L'\n' ){
									if( ptr[nPosKouho + nWordLen + 1] == L'\n' ){
										if( 0 == auto_memicmp( &ptr[nPosKouho + 1], word, nWordLen) ){
											nRet = 0;
											break;
										}else{
											nPosKouho += nWordLen;
										}
									}
								}
							}
						}
					}
				}else{
					if( nWordLen < nLen ){
						if( L'\n' == ptr[nWordLen] && 0 == auto_memcmp( ptr, word, nWordLen )  ){
							nRet = 0;
						}else{
							int nPosKouhoMax = nLen - nWordLen - 1;
							for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
								if( ptr[nPosKouho] == L'\n' ){
									if( ptr[nPosKouho + nWordLen + 1] == L'\n' ){
										if( 0 == auto_memcmp( &ptr[nPosKouho + 1], word, nWordLen) ){
											nRet = 0;
											break;
										}else{
											nPosKouho += nWordLen;
										}
									}
								}
							}
						}
					}
				}
				if( 0 == nRet ){
					continue;
				}
				//2007.10.17 kobake メモリリークしてました。修正。
				(*ppcmemKouho)->AppendString( word, nWordLen );
				(*ppcmemKouho)->AppendString( L"\n" );
				++nKouhoNum;
			}
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				return nKouhoNum;
			}
			
		}
	}
	return nKouhoNum;
}
/*[EOF]*/
