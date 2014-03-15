/*!	@file
	@brief プロセス基底クラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESS_H_
#define _CPROCESS_H_

#include "global.h"
#include "CShareData.h"

//#define USE_CRASHDUMP
#ifdef USE_CRASHDUMP
#include <DbgHelp.h> // MINIDUMP_TYPE
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
*/
class CProcess {
public:
	CProcess( HINSTANCE hInstance, LPCTSTR lpCmdLine );
	bool Run();
	virtual ~CProcess(){}
protected:
	CProcess();
	virtual bool InitializeProcess();
	virtual bool MainLoop() = 0;
	virtual void OnExitProcess() = 0;

#ifdef USE_CRASHDUMP
	int				WriteDump( PEXCEPTION_POINTERS pExceptPtrs );
#endif
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
#ifdef USE_CRASHDUMP
	BOOL (WINAPI *m_pfnMiniDumpWriteDump)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);
#endif

	//	唯一のCShareDateとする。（CProcessが責任を持ってnew/deleteする）
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;

private:
};


///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESS_H_ */

/*[EOF]*/
