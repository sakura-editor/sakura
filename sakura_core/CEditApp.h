/*! @file */
/*
	Copyright (C) 2007, kobake
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
#ifndef SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#define SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#pragma once

//2007.10.23 kobake 作成

#include "util/design_template.h"

#include "doc/CEditDoc.h"

#include "_main/CAppMode.h"
#include "uiparts/CSoundSet.h"
#include "uiparts/CImageListMgr.h"
#include "types/CType.h"
#include "CPropertyManager.h"
#include "window/CEditWnd.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "uiparts/CVisualProgress.h"
#include "recent/CMruListener.h"
#include "macro/CSMacroMgr.h"
#include "CGrepAgent.h"

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class CEditApp : public TSingleInstance<CEditApp> {
	using CEditDocHolder = std::shared_ptr<CEditDoc>;
	using CLoadAgentHolder = std::unique_ptr<CLoadAgent>;
	using CSaveAgentHolder = std::unique_ptr<CSaveAgent>;
	using CProgressHolder = std::unique_ptr<CVisualProgress>;
	using CGrepAgentHolder = std::unique_ptr<CGrepAgent>;
	using CAppModeHolder = std::unique_ptr<CAppMode>;
	using CMruListenerHolder = std::unique_ptr<CMruListener>;
	using CPropManagerHolder = std::unique_ptr<CPropertyManager>;
	using CSMacroMgrHolder = std::unique_ptr<CSMacroMgr>;

public:
	explicit CEditApp(HINSTANCE hInstance);

	void Create(CEditWnd* pcEditWnd, int nGroupId);

	//モジュール情報
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< インスタンスハンドル取得

	//ウィンドウ情報
	CEditWnd*       GetEditWindow() const;

	CEditDoc*		GetDocument() const { return m_pcEditDoc.get(); }
	CAppMode*       GetAppMode() const { return m_AppMode.get(); }
	CSMacroMgr*     GetSMacroMgr() const { return m_SMacroMgr.get(); }
	CImageListMgr&	GetIcons(){ return m_cIcons; }

	bool OpenPropertySheet( int nPageNum );
	bool OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );

	HINSTANCE			m_hInst;

	//ドキュメント
	CEditDocHolder		m_pcEditDoc = std::make_unique<CEditDoc>(nullptr);

	//ウィンドウ
	CEditWnd*			m_pcEditWnd = nullptr;

	//IO管理
	CLoadAgentHolder    m_pcLoadAgent = std::make_unique<CLoadAgent>();
	CSaveAgentHolder    m_pcSaveAgent = std::make_unique<CSaveAgent>();
	CProgressHolder     m_pcVisualProgress = std::make_unique<CVisualProgress>();

	CGrepAgentHolder    m_GrepAgent = std::make_unique<CGrepAgent>();	//GREPモード

	CAppModeHolder		m_AppMode = std::make_unique<CAppMode>();	//編集モード

	CMruListenerHolder  m_pcMruListener = std::make_unique<CMruListener>();		//MRU管理

	CPropManagerHolder	m_pcPropertyManager = std::make_unique<CPropertyManager>();	//プロパティ管理

	CSoundSet			m_cSoundSet;			//サウンド管理

	//GUIオブジェクト
	CImageListMgr		m_cIcons;					//!< Image List

	CSMacroMgrHolder    m_SMacroMgr = std::make_unique<CSMacroMgr>();	//マクロ管理

	CGrepAgent*         m_pcGrepAgent = m_GrepAgent.get();
	CSMacroMgr*         m_pcSMacroMgr = m_SMacroMgr.get();
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
