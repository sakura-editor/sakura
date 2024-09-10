/*!	@file
	@brief プロセス基底クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#define SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#pragma once

#include "_main/CCommandLine.h"

#include "apiwrap/kernel/handle_closer.hpp"

#include "util/design_template.h"
#include "env/CShareData.h"

#include "CSelectLang.h"
#include "sakura_rc.h"
#include "String_define.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
 */
class CProcess : public TSingleInstance<CProcess> {

	using Me = CProcess;
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;

public:
	explicit CProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow) noexcept;
	~CProcess() override = default;

	int Run() noexcept;

	virtual void RefreshString();

protected:
	virtual bool InitializeProcess();
	virtual bool MainLoop() = 0;
	virtual void OnExitProcess() = 0;

	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CCommandLine&   GetCCommandLine() const { return *m_pCommandLine; }
	CShareData&	    GetCShareData() { return m_cShareData; }
	DLLSHAREDATA&   GetShareData() const { return m_cShareData.GetShareData(); }
	HWND			GetMainWindow() const{ return m_hWnd; }

private:
	HINSTANCE           m_hInstance;
	CCommandLineHolder  m_pCommandLine;
	int                 m_nCmdShow;
	CShareData          m_cShareData;
	HWND                m_hWnd         = nullptr;
};

#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
