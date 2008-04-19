/*!	@file
	@brief プロセス基底クラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESS_H_
#define _CPROCESS_H_

#include "global.h"
#include "CShareData.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
*/
class SAKURA_CORE_API CProcess {
public:
	CProcess( HINSTANCE hInstance, LPTSTR lpCmdLine );
	bool Run();
	virtual ~CProcess(){};
protected:
	CProcess();
	virtual bool Initialize();
	virtual bool MainLoop() = 0;
	virtual void Terminate() = 0;

protected:
	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }
public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CShareData&		GetShareData()   { return m_cShareData; }
	DLLSHAREDATA&	GetDllShareData(){ return *m_cShareData.GetShareData(); }
	HWND			GetMainWindow() const{ return m_hWnd; }

private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
	LPTSTR		m_CommandLine;
	
	//	唯一のCShareDateとする。（CProcessが責任を持ってnew/deleteする）
	CShareData		m_cShareData;

private:
};


///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESS_H_ */


