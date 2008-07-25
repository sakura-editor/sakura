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
#include "etc_uty.h"
#include "charcode.h"  // 2006.06.28 rastiv

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
		 && nCommand != F_CHAR		//	文字入力
		 && nCommand != F_IME_CHAR	//	漢字入力
		 ){
			m_pcEditDoc->m_cHokanMgr.Hide();
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
	if( m_pShareData->m_Common.m_bUseHokan
 	 && FALSE == m_bExecutingKeyMacro	/* キーボードマクロの実行中 */
	){
		CMemory	cmemData;

		/* カーソル直前の単語を取得 */
		if( 0 < GetLeftWord( &cmemData, 100 ) ){
			ShowHokanMgr( cmemData, FALSE );
		}else{
			if( m_bHokan ){
				m_pcEditDoc->m_cHokanMgr.Hide();
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
void CEditView::ShowHokanMgr( CMemory& cmemData, BOOL bAutoDecided )
{
	/* 補完対象ワードリストを調べる */
	CMemory		cmemHokanWord;
	int			nKouhoNum;
	POINT		poWin;
	/* 補完ウィンドウの表示位置を算出 */
	poWin.x = m_nViewAlignLeft
			 + (m_nCaretPosX - m_nViewLeftCol)
			  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	poWin.y = m_nViewAlignTop
			 + (m_nCaretPosY - m_nViewTopLine)
			  * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
	::ClientToScreen( m_hWnd, &poWin );
	poWin.x -= (
		cmemData.GetLength()
		 * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )
	);

	/*	補完ウィンドウを表示
		ただし、bAutoDecided == TRUEの場合は、補完候補が1つのときは、ウィンドウを表示しない。
		詳しくは、Search()の説明を参照のこと。
	*/
	CMemory* pcmemHokanWord;
	if ( bAutoDecided ){
		pcmemHokanWord = &cmemHokanWord;
	}
	else {
		pcmemHokanWord = NULL;
	}
	nKouhoNum = m_pcEditDoc->m_cHokanMgr.Search(
		&poWin,
		m_nCharHeight,
		m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace,
		cmemData.GetPtr(),
		m_pcEditDoc->GetDocumentAttribute().m_szHokanFile,
		m_pcEditDoc->GetDocumentAttribute().m_bHokanLoHiCase,
		m_pcEditDoc->GetDocumentAttribute().m_bUseHokanByFile, // 2003.06.22 Moca
		pcmemHokanWord
	);
	/* 補完候補の数によって動作を変える */
	if (nKouhoNum <= 0) {				//	候補無し
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
			// 2003.06.25 Moca 失敗してたら、ビープ音を出して補完終了。
			::MessageBeep( MB_ICONHAND );
		}
	}
	else if( bAutoDecided && nKouhoNum == 1){ //	候補1つのみ→確定。
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		// 2004.05.14 Moca CHokanMgr::Search側で改行を削除するようにし、直接書き換えるのをやめた
//		pszKouhoWord = cmemHokanWord.GetPtr( &nKouhoWordLen );
//		pszKouhoWord[nKouhoWordLen] = '\0';
		Command_WordDeleteToStart();
		Command_INSTEXT( TRUE, cmemHokanWord.GetPtr(), cmemHokanWord.GetLength(), TRUE );
	}
	else {
		m_bHokan = TRUE;
	}
	
	//	補完終了。
	if ( !m_bHokan ){
		m_pShareData->m_Common.m_bUseHokan = FALSE;	//	入力補完終了の知らせ
	}
}


/*!	入力補完
	Ctrl+Spaceでここに到着。
	CEditView::m_bHokan： 現在補完ウィンドウが表示されているかを表すフラグ。
	m_Common.m_bUseHokan：現在補完ウィンドウが表示されているべきか否かをあらわすフラグ。

    @date 2001/06/19 asa-o 英大文字小文字を同一視する
                     候補が1つのときはそれに確定する
	@date 2001/06/14 asa-o 参照データ変更
	                 開くプロパティシートをタイプ別に変更
	@date 2000/09/15 JEPRO [Esc]キーと[x]ボタンでも中止できるように変更
	@date 2005/01/10 genta CEditView_Commandから移動
*/
void CEditView::Command_HOKAN( void )
{
	if (m_pShareData->m_Common.m_bUseHokan == FALSE){
		m_pShareData->m_Common.m_bUseHokan = TRUE;
	}
retry:;
	/* 補完候補一覧ファイルが設定されていないときは、設定するように促す。 */
	// 2003.06.22 Moca ファイル内から検索する場合には補完ファイルの設定は必須ではない
	if( m_pcEditDoc->GetDocumentAttribute().m_bUseHokanByFile == FALSE &&
		0 == lstrlen( m_pcEditDoc->GetDocumentAttribute().m_szHokanFile 
	) ){
		::MessageBeep( MB_ICONHAND );
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
			"補完候補一覧ファイルが設定されていません。\n今すぐ設定しますか?"
		) ){
			/* タイプ別設定 プロパティシート */
			if( !m_pcEditDoc->OpenPropertySheetTypes( 2, m_pcEditDoc->GetDocumentType() ) ){
				return;
			}
			goto retry;
		}
	}

	CMemory		cmemData;
	/* カーソル直前の単語を取得 */
	if( 0 < GetLeftWord( &cmemData, 100 ) ){
		ShowHokanMgr( cmemData, TRUE );
	}else{
		::MessageBeep( MB_ICONHAND );
		m_pShareData->m_Common.m_bUseHokan = FALSE;	//	入力補完終了のお知らせ
	}
	return;
}

/*!
	編集中データから入力補完キーワードの検索
	CHokanMgrから呼ばれる

	@return 候補数

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta CEditView_Commandから移動
*/
int CEditView::HokanSearchByFile(
		const char* pszKey,
		BOOL		bHokanLoHiCase,	//!< 英大文字小文字を同一視する
		CMemory**	ppcmemKouho,	//!< [IN/OUT] 候補
		int			nKouhoNum,		//!< ppcmemKouhoのすでに入っている数
		int			nMaxKouho		//!< Max候補数(0==無制限)
){
	const int nKeyLen = lstrlen( pszKey );
	int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	int i, j, nWordLen, nLineLen, nRet;
	int nCurX, nCurY; // 物理カーソル位置
	const char* pszLine;
	const char* word;
	nCurX = m_nCaretPosX_PHY;
	nCurY = m_nCaretPosY_PHY;
	
	for( i = 0; i < nLines; i++  ){
		pszLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( i, &nLineLen );
		for( j = 0; j < nLineLen; j++ ){
			if( IS_KEYWORD_CHAR( (unsigned char)(pszLine[j]) ) ){
				word = pszLine + j;
				for( j++, nWordLen = 1;j < nLineLen && IS_KEYWORD_CHAR( (unsigned char)(pszLine[j]) ); j++ ){
					nWordLen++;
				}
				if( nWordLen > 1020 ){ // CDicMgr等の制限により長すぎる単語は無視する
					continue;
				}
				if( nKeyLen <= nWordLen ){
					if( bHokanLoHiCase ){
						nRet = memicmp( pszKey, word, nKeyLen );
					}else{
						nRet = memcmp( pszKey, word, nKeyLen );
					}
					if( 0 == nRet ){
						// カーソル位置の単語は候補からはずす
						if( nCurY == i && nCurX <= j && j - nWordLen <= nCurX ){
							continue;
						}
						if( NULL == *ppcmemKouho ){
							*ppcmemKouho = new CMemory;
							(*ppcmemKouho)->SetData( word, nWordLen );
							(*ppcmemKouho)->AppendSz( "\n" );
							++nKouhoNum;
						}else{
							// 重複していたら追加しない
							int nLen;
							const char* ptr = (*ppcmemKouho)->GetPtr( &nLen );
							int nPosKouho;
							nRet = 1;
							// 2008.07.23 nasukoji	大文字小文字を同一視の場合でも候補の振るい落としは完全一致で見る
							if( nWordLen < nLen ){
								if( '\n' == ptr[nWordLen] && 0 == memcmp( ptr, word, nWordLen )  ){
									nRet = 0;
								}else{
									int nPosKouhoMax = nLen - nWordLen - 1;
									for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
										if( ptr[nPosKouho] == '\n' ){
											if( ptr[nPosKouho + nWordLen + 1] == '\n' ){
												if( 0 == memcmp( &ptr[nPosKouho + 1], word, nWordLen) ){
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
							if( 0 == nRet ){
								continue;
							}
							(*ppcmemKouho)->Append( word, nWordLen );
							(*ppcmemKouho)->Append( "\n", 1 );
							++nKouhoNum;
						}
						if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
							return nKouhoNum;
						}
					}
				}
			}else if( _IS_SJIS_1( (unsigned char)pszLine[j] ) ){
				j++;
				continue;
			}
		}
	}
	return nKouhoNum;
}
/*[EOF]*/
