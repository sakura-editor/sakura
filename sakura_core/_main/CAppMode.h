/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CAPPMODE_797AE845_5323_4D8A_A263_C534249DBB1C_H_
#define SAKURA_CAPPMODE_797AE845_5323_4D8A_A263_C534249DBB1C_H_
#pragma once

#include "util/design_template.h"
#include "doc/CDocListener.h"

class CAppMode : public TSingleton<CAppMode>, public CDocListenerEx{ //###仮
	friend class TSingleton<CAppMode>;
	CAppMode()
	: m_bViewMode( false )	// ビューモード
	, m_bDebugMode( false )		// デバッグモニタモード
	{
		m_szGrepKey[0] = L'\0';
	}

public:
	//インターフェース
	bool	IsViewMode() const				{ return m_bViewMode; }			//!< ビューモードを取得
	void	SetViewMode(bool bViewMode)		{ m_bViewMode = bViewMode; }	//!< ビューモードを設定
	bool	IsDebugMode() const				{ return m_bDebugMode; }
	void	SetDebugModeON();	//!< デバッグモニタモード設定
	void	SetDebugModeOFF();	//!< デバッグモニタモード解除

	//イベント
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

protected:
	void _SetDebugMode(bool bDebugMode){ m_bDebugMode = bDebugMode; }

private:
	bool			m_bViewMode;			//!< ビューモード
	bool			m_bDebugMode;				//!< デバッグモニタモード
public:
	wchar_t			m_szGrepKey[1024];			//!< Grepモードの場合、その検索キー
};
#endif /* SAKURA_CAPPMODE_797AE845_5323_4D8A_A263_C534249DBB1C_H_ */
