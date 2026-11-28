/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CDocEditor.h"
#include "CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CAppMode.h"
#include "basis/CEol.h"
#include "window/CEditWnd.h"
#include "debug/CRunningTimer.h"
#include "_os/CClipboard.h"
#include "parse/DetectIndentationStyle.h"

using UndoBufferHolder = CDocEditor::UndoBufferHolder;

/*!
 * @brief アンドゥバッファを後始末する
 */
/* static */ void CDocEditor::ReleaseUndoBuffer(
	CDocEditor* pThis
)
{
	// 現在の操作ブロックがない場合、何もしない
	if (!pThis->m_OpeBlk) return;

	// 現在の操作ブロックの参照カウンタを減らす
	if (const auto refCount = pThis->m_OpeBlk->Release();
		0 < refCount) return;

	// 操作ブロックの格納数を取得する
	if (const auto opeCount = pThis->m_OpeBlk->GetNum();
		!opeCount)
	{
		// 空っぽなので捨てる
		pThis->m_OpeBlk.reset(nullptr);

		return;
	}

	// スマートポインターから切り離す
	auto pOpeBlk = pThis->m_OpeBlk.release();

	// アンドゥバッファに操作を追加する
	pThis->m_cOpeBuf.AppendOpeBlk(pOpeBlk);

	if (!GetEditWnd().UpdateTextWrap() &&	// 折り返し方法関連の更新
		0 < pOpeBlk->GetNum() - pThis->m_nOpeBlkRedawCount)
	{
		// この仕様が必要かどうかは微妙
		GetEditWnd().RedrawAllViews(&GetActiveView());	//	他のペインの表示を更新
	}
}

CDocEditor::CDocEditor(CEditDoc* pcDoc) noexcept
: m_pcDocRef(pcDoc)
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

void CDocEditor::OnBeforeLoad([[maybe_unused]] SLoadInfo* sLoadInfo)
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
		DetectIndentationStyle(pcDoc->m_cDocFile.GetFilePathClass(), pcDoc->m_cDocLineMgr, 256, indentStyle);
		auto& bInsSpace = pcDoc->m_cDocType.GetDocumentAttributeWrite().m_bInsSpace;
		auto setTemporaryTabSpaceKetas = [&]() {
			// タブ幅を一時的に設定
			pcDoc->m_bTabSpaceCurTemp = true;
			auto& layoutMgr = pcDoc->m_cLayoutMgr;
			layoutMgr.SetTabSpaceKetas(CKetaXInt(indentStyle.tabSpace));
		};
		if (indentStyle.character == IndentationStyle::Character::Spaces) { // 半角空白でインデント
			if (indentStyle.tabSpace > 0) { // インデント幅が検出できた場合のみ設定
				// スペースの挿入設定を有効化
				bInsSpace = true;
				setTemporaryTabSpaceKetas();
			}
		}else if (indentStyle.character == IndentationStyle::Character::Tabs) { // タブ文字でインデント
			// スペースの挿入設定を無効化
			bInsSpace = false;
			if (indentStyle.tabSpace == -1) {
				// タブ幅は元のままで変更しない
			}
			else {
				setTemporaryTabSpaceKetas();
			}
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
	cxx::SetCurrentDirectoryW(pcDoc->m_cDocFile.GetFilePathClass().GetDirPath());

	CAppMode::getInstance()->SetViewMode(sLoadInfo.bViewMode);		// ビューモード	##ここも、アリかな
}

void CDocEditor::OnAfterSave([[maybe_unused]] const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	this->SetModified(false,false);	//	Jan. 22, 2002 genta 関数化 更新フラグのクリア

	/* 現在位置で無変更な状態になったことを通知 */
	this->m_cOpeBuf.SetNoModified();

	// カレントディレクトリの変更
	cxx::SetCurrentDirectoryW(pcDoc->m_cDocFile.GetFilePathClass().GetDirPath());
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
		default:
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
 * @brief アンドゥバッファを準備する
 */
UndoBufferHolder CDocEditor::AllocUndoBuffer()
{
	// 現在の操作ブロックがない場合、新しく割り当てる
	if (!GetOpeBlk()) {
		m_OpeBlk = std::make_unique<COpeBlk>();
	}

	// 現在の操作ブロックの参照カウンタを増やす
	GetOpeBlk()->AddRef();

	// アンドゥバッファの後始末を行うためのリソースホルダーを返す
	return UndoBufferHolder{ this };
}

/*!
 * @brief 現在の操作ブロックを取得する
 */
COpeBlk* CDocEditor::GetOpeBlk() const noexcept
{
	return m_OpeBlk
		? m_OpeBlk.get()
		: nullptr;
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
