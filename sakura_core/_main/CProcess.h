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

#include <filesystem>
#include <string>
#include <string_view>

#include "global.h"
#include "util/design_template.h"
#include "env/CShareData.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
*/
class CProcess : public TSingleInstance<CProcess> {
public:
	CProcess( HINSTANCE hInstance, LPCWSTR lpCmdLine );
	bool Run();
	virtual ~CProcess(){}
	virtual void RefreshString();

	virtual std::filesystem::path GetIniFileName() const;

protected:
	CProcess();
	virtual bool InitializeProcess();
	virtual bool MainLoop() = 0;
	virtual void OnExitProcess() = 0;

protected:
	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CShareData&		GetShareData()   { return m_cShareData; }
	HWND			GetMainWindow() const{ return m_hWnd; }

	[[nodiscard]] const CShareData* GetShareDataPtr() const { return &m_cShareData; }
	[[nodiscard]] LPCWSTR	GetAppName( void ) const { return m_strAppName.c_str(); }
	void UpdateAppName( std::wstring_view appName );

private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
	CShareData		m_cShareData;
	std::wstring	m_strAppName;
};
#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
