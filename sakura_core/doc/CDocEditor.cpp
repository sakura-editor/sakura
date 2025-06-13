/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
#include "_os/CClipboard.h"
#include "parse/DetectIndentationStyle.h"

CDocEditor::CDocEditor(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_cNewLineCode( EEolType::cr_and_lf )		//	New Line Type
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
	if( redraw ){
		GetEditWnd().m_cDlgFuncList.NotifyDocModification();
	}

	if( m_bIsDocModified == flag )	//	変更がなければ何もしない
		return;

	m_bIsDocModified = flag;
	if( redraw )
		GetEditWnd().UpdateCaption();
}

void CDocEditor::OnBeforeLoad(SLoadInfo* sLoadInfo)
{
	//ビューのテキスト選択解除
	GetEditWnd().Views_DisableSelectArea(true);
}

void CDocEditor::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();
	const STypeConfig& type = pcDoc->m_cDocType.GetDocumentAttribute();

	if (type.m_bDetectIndentationStyleOnFileLoad) {
		// インデントスタイル検出
		// 現時点の実装ではレイアウトには影響しないのでここで実行する
		IndentationStyle indentStyle{};
		DetectIndentationStyle(pcDoc, 256, indentStyle);
		auto& bInsSpace = pcDoc->m_cDocType.GetDocumentAttributeWrite().m_bInsSpace;
		if (indentStyle.character == IndentationStyle::Character::Spaces) { // 半角空白でインデント
			if (indentStyle.tabSpace > 0) { // インデント幅が検出できた場合のみ設定
				// スペースの挿入設定を有効化
				bInsSpace = true;
				// タブ幅を一時的に設定
				pcDoc->m_bTabSpaceCurTemp = true;
				auto& layoutMgr = pcDoc->m_cLayoutMgr;
				layoutMgr.SetTabSpaceKetas(CKetaXInt(indentStyle.tabSpace));
			}
		}else if (indentStyle.character == IndentationStyle::Character::Tabs) { // タブ文字でインデント
			// スペースの挿入設定を無効化
			bInsSpace = false;
			// タブ幅は元のままで変更しない
		}else {
			// 検出出来なかったので何も設定しない
		}
	}

	//	May 12, 2000 genta
	//	編集用改行コードの設定
	{
		if ( pcDoc->m_cDocFile.GetCodeSet() == type.m_encoding.m_eDefaultCodetype ){
			SetNewLineCode( type.m_encoding.m_eDefaultEoltype );	// 2011.01.24 ryoji デフォルトEOL
		}
		else{
			SetNewLineCode( EEolType::cr_and_lf );
		}
		if( const CDocLine* pFirstline = pcDoc->m_cDocLineMgr.GetLine( CLogicInt(0) ); pFirstline != nullptr ){
			if( const auto cEol = pFirstline->GetEol(); cEol.IsValid() ){
				SetNewLineCode( cEol.GetType() );
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
	HWND	hwnd = GetEditWnd().GetActiveView().GetHwnd();

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

bool CDocEditor::IsEnablePaste() const
{
	return CClipboard::HasValidData();
}

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
	pDocLine->SetDocLineString(pData, nDataLen, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol);
}
