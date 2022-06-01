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
