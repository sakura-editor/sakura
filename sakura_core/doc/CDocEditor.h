﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_
#define SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_
#pragma once

#include "doc/CDocListener.h"
#include "COpeBuf.h"

class CEditDoc;
class CDocLineMgr;

class CDocEditor : public CDocListenerEx{
public:
	CDocEditor(CEditDoc* pcDoc);

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

public:
	CEditDoc*		m_pcDocRef;
	CEol 			m_cNewLineCode;				//!< Enter押下時に挿入する改行コード種別
	COpeBuf			m_cOpeBuf;					//!< アンドゥバッファ
	COpeBlk*		m_pcOpeBlk;					//!< 操作ブロック
	int				m_nOpeBlkRedawCount;		//!< OpeBlkの再描画非対象数
	bool			m_bInsMode;					//!< 挿入・上書きモード Oct. 2, 2005 genta
	bool			m_bIsDocModified;
};

class CDocEditAgent{
public:
	CDocEditAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           操作                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int );	/* 末尾に行を追加 Ver1.5 */

private:
	CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CDOCEDITOR_87202DA6_D60E_4235_AD2F_02507F4E101A_H_ */
