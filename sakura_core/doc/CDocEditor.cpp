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
#include "CDocEditor.h"
#include "CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CAppMode.h"
#include "CEol.h"
#include "window/CEditWnd.h"
#include "debug/CRunningTimer.h"

CDocEditor::CDocEditor(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_pcOpeBlk( NULL )
, m_bInsMode( true )	// Oct. 2, 2005 genta
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
		if ( pcDoc->m_cDocFile.GetCodeSet() == type.m_encoding.m_eDefaultCodetype ){
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



//	From Here Nov. 20, 2000 genta
/*!	IME状態の設定
	
	@param mode [in] IMEのモード
	
	@date Nov 20, 2000 genta
*/
void CDocEditor::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;
	HWND	hwnd = m_pcDocRef->m_pcEditWnd->GetActiveView().GetHwnd();

	hIme = ImmGetContext( hwnd ); //######大丈夫？ // 2013.06.04 EditWndからViewに変更

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
	ImmReleaseContext( hwnd, hIme ); //######大丈夫？
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


