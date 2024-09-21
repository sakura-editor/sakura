/*!	@file
	@brief エディタプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#define SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#pragma once

#include "_main/CProcess.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"

#include "_main/CAppMode.h"
#include "uiparts/CSoundSet.h"
#include "types/CType.h"
#include "CLoadAgent.h"
#include "CSaveAgent.h"
#include "uiparts/CVisualProgress.h"
#include "recent/CMruListener.h"
#include "macro/CSMacroMgr.h"
#include "CGrepAgent.h"

#include "extmodule/CMigemo.h"
#include "macro/CMacroFactory.h"

class CEditApp;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはCEditWndクラスのインスタンスを作る。
 */
class CNormalProcess : public CProcess {

	using Me = CNormalProcess;
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;

	using CEditDocHolder = std::shared_ptr<CEditDoc>;
	using CLoadAgentHolder = std::unique_ptr<CLoadAgent>;
	using CSaveAgentHolder = std::unique_ptr<CSaveAgent>;
	using CProgressHolder = std::unique_ptr<CVisualProgress>;
	using CGrepAgentHolder = std::unique_ptr<CGrepAgent>;
	using CAppModeHolder = std::unique_ptr<CAppMode>;
	using CMruListenerHolder = std::unique_ptr<CMruListener>;
	using CSMacroMgrHolder = std::unique_ptr<CSMacroMgr>;

	using CEditAppHolder = std::shared_ptr<CEditApp>;
	using CMacroFactoryHolder = std::unique_ptr<CMacroFactory>;
	using CMigemoHolder = std::unique_ptr<CMigemo>;

public:
	//コンストラクタ・デストラクタ
	explicit CNormalProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow = SW_SHOWDEFAULT) noexcept;
	~CNormalProcess() override = default;

	CEditApp*       GetEditApp() const { return m_pcEditApp.get(); }
	CEditDoc*		GetEditDoc() const { return m_pcEditDoc.get(); }
	CEditWnd*       GetEditWnd() const { return static_cast<CEditWnd*>(GetMainWnd()); }

	CAppMode*           GetAppMode() const { return m_AppMode.get(); }
	CGrepAgent*         GetGrepAgent() const { return m_GrepAgent.get(); }
	CVisualProgress*    GetProgress() const { return m_pcVisualProgress.get(); }
	CSMacroMgr*         GetSMacroMgr() const { return m_SMacroMgr.get(); }

protected:
	//プロセスハンドラ
	bool InitializeProcess() override;

	void    InitProcess() override;
	bool    InitShareData() override;

	//実装補助
	void OpenFiles(HWND hwnd);

private:
	CEditDocHolder		m_pcEditDoc = nullptr;
	CLoadAgentHolder    m_pcLoadAgent = nullptr;
	CSaveAgentHolder    m_pcSaveAgent = nullptr;
	CProgressHolder     m_pcVisualProgress = nullptr;
	CGrepAgentHolder    m_GrepAgent = nullptr;	//GREPモード
	CAppModeHolder		m_AppMode = nullptr;	//編集モード
	CMruListenerHolder  m_pcMruListener = nullptr;		//MRU管理
	CSMacroMgrHolder    m_SMacroMgr = nullptr;	//マクロ管理

	/*!
	 * エディターアプリケーションのインスタンス。
	 */
	CEditAppHolder      m_pcEditApp = nullptr;

	CMacroFactoryHolder	m_MacroFactory = nullptr;

	CMigemoHolder		m_Migemo = std::make_unique<CMigemo>();
};

using CEditorProcess = CNormalProcess;

CEditorProcess* getEditorProcess() noexcept;

#endif /* SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_ */
