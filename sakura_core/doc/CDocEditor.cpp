#include "StdAfx.h"
#include "CDocEditor.h"
#include "CClipboard.h"
#include "env/DLLSHAREDATA.h"
#include "doc/CEditDoc.h"
#include "doc/CDocLine.h"
#include "doc/CDocLineMgr.h"
#include "CAppMode.h"
#include "CEol.h"
#include "window/CEditWnd.h"
#include "debug/CRunningTimer.h"

CDocEditor::CDocEditor(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_bIsDocModified( false )	/* 変更フラグ */ // Jan. 22, 2002 genta 型変更
{
	//	Oct. 2, 2005 genta 挿入モード
	this->SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
}


/*! 変更フラグの設定

	@param flag [in] 設定する値．true: 変更有り / false: 変更無し
	@param redraw [in] true: タイトルの再描画を行う / false: 行わない
	
	@author genta
	@date 2002.01.22 新規作成
*/
void CDocEditor::SetModified( bool flag, bool redraw)
{
	if( m_bIsDocModified == flag )	//	変更がなければ何もしない
		return;

	m_bIsDocModified = flag;
	if( redraw )
		m_pcDocRef->m_pcEditWnd->UpdateCaption();
}

void CDocEditor::OnBeforeLoad(SLoadInfo* sLoadInfo)
{
	//ビューのテキスト選択解除
	GetListeningDoc()->m_pcEditWnd->Views_DisableSelectArea(true);
}

void CDocEditor::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	May 12, 2000 genta
	//	編集用改行コードの設定
	{
		const STypeConfig& type = pcDoc->m_cDocType.GetDocumentAttribute();
		if ( pcDoc->m_cDocFile.m_sFileInfo.eCharCode == type.m_encoding.m_eDefaultCodetype ){
			SetNewLineCode( type.m_encoding.m_eDefaultEoltype );	// 2011.01.24 ryoji デフォルトEOL
		}
		else{
			SetNewLineCode( EOL_CRLF );
		}
		CDocLine*	pFirstlineinfo = pcDoc->m_cDocLineMgr.GetLine( CLogicInt(0) );
		if( pFirstlineinfo != NULL ){
			EEolType t = pFirstlineinfo->GetEol();
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	//	Nov. 20, 2000 genta
	//	IME状態の設定
	this->SetImeMode( pcDoc->m_cDocType.GetDocumentAttribute().m_nImeState );

	// カレントディレクトリの変更
	::SetCurrentDirectory( pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str() );

	CAppMode::getInstance()->SetViewMode(sLoadInfo.bViewMode);		// ビューモード	##ここも、アリかな
}

void CDocEditor::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	this->SetModified(false,false);	//	Jan. 22, 2002 genta 関数化 更新フラグのクリア

	/* 現在位置で無変更な状態になったことを通知 */
	this->m_cOpeBuf.SetNoModified();

	// カレントディレクトリの変更
	::SetCurrentDirectory( pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str() );
}



/* Undo(元に戻す)可能な状態か？ */
bool CDocEditor::IsEnableUndo( void )
{
	return m_cOpeBuf.IsEnableUndo();
}


/*! Redo(やり直し)可能な状態か？ */
bool CDocEditor::IsEnableRedo( void )
{
	return m_cOpeBuf.IsEnableRedo();
}

/*! クリップボードから貼り付け可能か？ */
bool CDocEditor::IsEnablePaste( void )
{
	return CClipboard::HasValidData();
}




//	From Here Nov. 20, 2000 genta
/*!	IME状態の設定
	
	@param mode [in] IMEのモード
	
	@date Nov 20, 2000 genta
*/
void CDocEditor::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( CEditWnd::getInstance()->GetHwnd() ); //######大丈夫？

	//	最下位ビットはIME自身のOn/Off制御
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( CEditWnd::getInstance()->GetHwnd(), hIme ); //######大丈夫？
}
//	To Here Nov. 20, 2000 genta










/*!
	末尾に行を追加

	@version 1.5

	@param pData    [in] 追加する文字列へのポインタ
	@param nDataLen [in] 文字列の長さ。文字単位。
	@param cEol     [in] 行末コード

*/


void CDocEditAgent::AddLineStrX( const wchar_t* pData, int nDataLen )
{
	//チェーン適用
	CDocLine* pDocLine = m_pcDocLineMgr->AddNewLine();

	//インスタンス設定
	pDocLine->SetDocLineString(pData, nDataLen);
}




/* データの削除 */
/*
|| 指定行内の文字しか削除できません
|| データ変更によって影響のあった、変更前と変更後の行の範囲を返します
|| この情報をもとに、レイアウト情報などを更新してください。
||
	@date 2002/03/24 YAZAKI bUndo削除
*/
void CDocEditAgent::DeleteData_CDocLineMgr(
	CLogicInt	nLine,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLogicInt*	pnModLineOldFrom,	//!< 影響のあった変更前の行(from)
	CLogicInt*	pnModLineOldTo,		//!< 影響のあった変更前の行(to)
	CLogicInt*	pnDelLineOldFrom,	//!< 削除された変更前論理行(from)
	CLogicInt*	pnDelLineOldNum,	//!< 削除された行数
	CNativeW*	cmemDeleted			//!< [out] 削除されたデータ
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	CLogicInt	nDeleteLength;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;

	*pnModLineOldFrom = nLine;	/* 影響のあった変更前の行(from) */
	*pnModLineOldTo = nLine;	/* 影響のあった変更前の行(to) */
	*pnDelLineOldFrom = CLogicInt(0);		/* 削除された変更前論理行(from) */
	*pnDelLineOldNum = CLogicInt(0);		/* 削除された行数 */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted->Clear();

	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( NULL == pDocLine ){
		return;
	}

	CModifyVisitor().SetLineModified(pDocLine,true);		/* 変更フラグ */

	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	if( nDelPos >= nLineLen ){
		return;
	}
	/* 「改行」を削除する場合は、次の行と連結する */
//	if( ( nDelPos == nLineLen -1 && ( pLine[nDelPos] == CR || pLine[nDelPos] == LF ) )
//	 || nDelPos + nDelLen >= nLineLen
	if( ( EOL_NONE != pDocLine->GetEol() && nDelPos == nLineLen - pDocLine->GetEol().GetLen() )
	 || ( EOL_NONE != pDocLine->GetEol() && nDelPos + nDelLen >  nLineLen - pDocLine->GetEol().GetLen() )
	 || ( EOL_NONE == pDocLine->GetEol() && nDelPos + nDelLen >= nLineLen - pDocLine->GetEol().GetLen() )
	){
		/* 実際に削除するバイト数 */
		nDeleteLength = nLineLen - nDelPos;

		/* 削除されるデータ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		/* 次の行の情報 */
		pDocLine2 = pDocLine->GetNextLine();
		if( !pDocLine2 ){
			wchar_t*	pData = new wchar_t[nLineLen + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = L'\0';

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength );
			}else{
				// 行の削除
				// 2004.03.18 Moca 関数を使う
				m_pcDocLineMgr->DeleteLine( pDocLine );
				pDocLine = NULL;
				*pnDelLineOldFrom = nLine;	/* 削除された変更前論理行(from) */
				*pnDelLineOldNum = CLogicInt(1);		/* 削除された行数 */
			}
			delete [] pData;
		}
		else{
			*pnModLineOldTo = nLine + CLogicInt(1);	/* 影響のあった変更前の行(to) */
			pLine2 = pDocLine2->GetDocLineStrWithEOL( &nLineLen2 );
			wchar_t*	pData = new wchar_t[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* 次の行のデータを連結 */
			wmemcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = L'\0';
			pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength + nLineLen2 );

			/* 次の行を削除 && 次次行とのリストの連結*/
			// 2004.03.18 Moca DeleteLine を使う
			m_pcDocLineMgr->DeleteLine( pDocLine2 );
			pDocLine2 = NULL;
			*pnDelLineOldFrom = nLine + CLogicInt(1);	/* 削除された変更前論理行(from) */
			*pnDelLineOldNum = CLogicInt(1);			/* 削除された行数 */
			delete [] pData;
		}
	}
	else{
		/* 実際に削除するバイト数 */
		nDeleteLength = nDelLen;

		/* 削除されるデータ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		wchar_t*	pData = new wchar_t[nLineLen + 1];
		if( nDelPos > 0 ){
			wmemcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			wmemcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = L'\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength );
		}
		delete [] pData;
	}
}





/*!	データの挿入

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CDocEditAgent::InsertData_CDocLineMgr(
	CLogicInt		nLine,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLogicInt*		pnInsLineNum,	// 挿入によって増えた行の数
	CLogicPoint*	pptNewPos		// 挿入された部分の次の位置
)
{
	CNativeW	cmemPrevLine;
	CNativeW	cmemNextLine;
	CLogicInt	nAllLinesOld = m_pcDocLineMgr->GetLineCount();

	bool		bBookMarkNext;	// 2001.12.03 hor 挿入によるマーク行の制御

	pptNewPos->y = nLine;	/* 挿入された部分の次の位置の行 */

	//	Jan. 25, 2004 genta
	//	挿入文字列長が0の場合に最後までpnNewPosが設定されないので
	//	初期値として0ではなく開始位置と同じ値を入れておく．
	pptNewPos->x  = nInsPos;		/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	*pnInsLineNum = CLogicInt(0);
	CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( !pDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		bBookMarkNext=false;	// 2001.12.03 hor
	}
	else{
		CModifyVisitor().SetLineModified(pDocLine,true);		/* 変更フラグ */

		CLogicInt		nLineLen;
		const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
		cmemPrevLine.SetString( pLine, nInsPos );
		cmemNextLine.SetString( &pLine[nInsPos], nLineLen - nInsPos );

		bBookMarkNext= CBookmarkGetter(pDocLine).IsBookmarked();	// 2001.12.03 hor
	}

	int			nCount = 0;
	CLogicInt	nBgn   = CLogicInt(0);
	CLogicInt	nPos   = CLogicInt(0);
	for( nPos = CLogicInt(0); nPos < nInsDataLen; ){
		if( WCODE::IsLineDelimiter(pInsData[nPos]) ){
			/* 行終端子の種類を調べる */
			CEol 	cEOLType;
			cEOLType.SetTypeByString( &pInsData[nPos], nInsDataLen - nPos );

			/* 行終端子も含めてテキストをバッファに格納 */
			CNativeW	cmemCurLine;
			cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + CLogicInt(cEOLType.GetLen());
			nPos = nBgn;
			if( NULL == pDocLine ){
				CDocLine* pDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pDocLineNew->SetDocLineString(cmemPrevLine + cmemCurLine);
				}
				else{
					pDocLineNew->SetDocLineString(cmemCurLine);
				}
				pDocLine = NULL;
			}
			else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );

					// 2001.12.13 hor
					// 行頭で改行したら元の行のマークを新しい行に移動する
					// それ以外なら元の行のマークを維持して新しい行にはマークを付けない
					if(nInsPos==CLogicInt(0)){
						CBookmarkSetter(pDocLine).SetBookmark(false);
					}
					else{
						bBookMarkNext=false;
					}

					pDocLine = pDocLine->GetNextLine();
				}
				else{
					CDocLine* pDocLineNew = m_pcDocLineMgr->InsertNewLine(pDocLine); //pDocLineの直前に挿入
					
					pDocLineNew->SetDocLineString( cmemCurLine );
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++pptNewPos->y;	/* 挿入された部分の次の位置の行 */
		}
		else{
			++nPos;
		}
	}

	if( CLogicInt(0) < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		CNativeW	cmemCurLine;
		cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			CDocLine* pDocLineNew = m_pcDocLineMgr->AddNewLine();
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
			}
			else{
				pDocLineNew->SetDocLineString( cmemCurLine );
			}
			pDocLine = NULL;
			pptNewPos->x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}
		else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );
				pDocLine = pDocLine->GetNextLine();
				pptNewPos->x = CLogicInt(cmemPrevLine.GetStringLength()) + nPos - nBgn;		/* 挿入された部分の次の位置のデータ位置 */
			}
			else{
				CDocLine* pDocLineNew = m_pcDocLineMgr->InsertNewLine(pDocLine); //pDocLineの直前に挿入

				pDocLineNew->SetDocLineString( cmemCurLine );

				CBookmarkSetter(pDocLineNew).SetBookmark(bBookMarkNext);	// 2001.12.03 hor ブックマークを復元

				pptNewPos->x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	*pnInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
}

