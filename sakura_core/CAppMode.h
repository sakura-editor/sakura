#pragma once

#include "util/design_template.h"
#include "CDocListener.h"

class CAppMode : public TSingleton<CAppMode>, public CDocListenerEx{ //###仮
public:
	CAppMode()
	: m_bViewMode( false )	// ビューモード
	, m_bDebugMode( false )		// デバッグモニタモード
	{
		wcscpy( m_szGrepKey, L"" );
	}

	//インターフェース
	bool	IsViewMode() const				{ return m_bViewMode; }			//!< ビューモードを取得
	void	SetViewMode(bool bViewMode)		{ m_bViewMode = bViewMode; }	//!< ビューモードを設定
	bool	IsDebugMode() const				{ return m_bDebugMode; }
	void	SetDebugModeON();	//!< デバッグモニタモード設定
	void	SetDebugModeOFF();	//!< デバッグモニタモード解除

	//イベント
	void OnAfterSave(const SSaveInfo& sSaveInfo)
	{
		m_bViewMode = false;	/* ビューモード */
	}

protected:
	void _SetDebugMode(bool bDebugMode){ m_bDebugMode = bDebugMode; }

private:
	bool			m_bViewMode;			//!< ビューモード
	bool			m_bDebugMode;				//!< デバッグモニタモード
public:
	wchar_t			m_szGrepKey[1024];			//!< Grepモードの場合、その検索キー
};
