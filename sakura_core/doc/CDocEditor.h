/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_
#define SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_
#pragma once

#include "basis/CEol.h"
#include "cmd/COpeBuf.h"
#include "doc/CDocListener.h"

class CEditDoc;
class CDocLineMgr;

/*!
 * @brief エディタークラス
 *
 * 編集ドキュメントの編集機能を提供する
 * ・・・はずだが、編集機能は実装されていない。
 *
 * 現状、編集機能はCViewCommanderクラスに実装されており、
 * このクラスは「編集プロパティの保持」を担当している。
 */
class CDocEditor : public CDocListenerEx{
public:
	explicit CDocEditor(CEditDoc* pcDoc) noexcept;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//ロード前後
	void OnBeforeLoad(SLoadInfo* sLoadInfo) override;
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;

	//セーブ前後
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           状態                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Jan. 22, 2002 genta Modified Flagの設定
	void SetModified( bool flag, bool redraw);
	//! ファイルが修正中かどうか
	bool IsModified() const { return m_bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME状態の設定

	//	May 15, 2000 genta
	CEol  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode( EEolType t ) noexcept { m_cNewLineCode = t; }
	void  SetNewLineCode( const CEol& cEol ) noexcept { SetNewLineCode( cEol.GetType() ); }

	//	Oct. 2, 2005 genta 挿入モードの設定
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	//! Undo(元に戻す)可能な状態か？ */
	bool IsEnableUndo( void ) const
	{
		return m_cOpeBuf.IsEnableUndo();
	}

	//! Redo(やり直し)可能な状態か？
	bool IsEnableRedo( void ) const
	{
		return m_cOpeBuf.IsEnableRedo();
	}

	//! クリップボードから貼り付け可能か？
	bool IsEnablePaste( void ) const;

	CEditDoc*		m_pcDocRef;

	CEol 			m_cNewLineCode{ EEolType::cr_and_lf };	//!< Enter押下時に挿入する改行コード種別

	COpeBuf			m_cOpeBuf{};				//!< アンドゥバッファ
	COpeBlk*		m_pcOpeBlk = nullptr;		//!< 操作ブロック
	int				m_nOpeBlkRedawCount = 0;	//!< OpeBlkの再描画非対象数

	bool			m_bInsMode = true;			//!< 挿入・上書きモード

	bool			m_bIsDocModified = false;
};

class CDocEditAgent{
public:
	explicit CDocEditAgent(CDocLineMgr* pcDocLineMgr) noexcept
		: m_pcDocLineMgr(pcDocLineMgr)
	{
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           操作                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int );	/* 末尾に行を追加 Ver1.5 */

private:
	CDocLineMgr* m_pcDocLineMgr;
};

#endif /* SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_ */
