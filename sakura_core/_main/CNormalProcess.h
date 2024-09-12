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
#include "window/CEditWnd.h"

#include "extmodule/CMigemo.h"
#include "macro/CMacroFactory.h"
#include "CEditApp.h"

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
	using CEditAppHolder = std::shared_ptr<CEditApp>;
	using CEditWndHolder = std::unique_ptr<CEditWnd>;
	using CMacroFactoryHolder = std::unique_ptr<CMacroFactory>;
	using CMigemoHolder = std::unique_ptr<CMigemo>;

public:
	//コンストラクタ・デストラクタ
	explicit CNormalProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow = SW_SHOWDEFAULT) noexcept;
	~CNormalProcess() override = default;

	CEditApp*       GetEditApp() const { return m_pcEditApp.get(); }
	CEditWnd*       GetEditWnd() const { return m_pcEditWnd.get(); }

protected:
	//プロセスハンドラ
	bool InitializeProcess() override;
	bool MainLoop() override;

	void    InitProcess() override;
	bool    InitShareData() override;

protected:
	//実装補助
	void OpenFiles(HWND hwnd);

private:
	/*!
	 * エディターアプリケーションのインスタンス。
	 */
	CEditAppHolder      m_pcEditApp = nullptr;

	CEditWndHolder      m_pcEditWnd = nullptr;

	CMacroFactoryHolder	m_MacroFactory = nullptr;

	CMigemoHolder		m_Migemo = std::make_unique<CMigemo>();
};

using CEditorProcess = CNormalProcess;

CEditorProcess* getEditorProcess() noexcept;

#endif /* SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_ */
