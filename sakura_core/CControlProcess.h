//	$Id$
/*!	@file
	@brief コントロールプロセスクラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
	$Revision$
*/
/*
	Copyright (C) 2002, aroka 新規作成

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCONTROLPROCESS_H_
#define _CCONTROLPROCESS_H_

#include "global.h"
#include "CShareData.h"
#include "CProcess.h"

class CEditApp;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはCEditAppクラスのインスタンスを作る。
*/
class SAKURA_CORE_API CControlProcess : public CProcess {
public:
	CControlProcess( HINSTANCE hInstance, LPSTR lpCmdLine ) : 
		CProcess( hInstance, lpCmdLine ),
		m_pcEditApp( 0 ),
		m_pShareData( 0 ){}

	static BOOL CALLBACK ExitingDlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	virtual ~CControlProcess();
protected:
	CControlProcess();
	virtual bool Initialize();
	virtual bool MainLoop();
	virtual void Terminate();

private:
	HANDLE			m_hMutex;
	HANDLE			m_hMutexCP;
	CEditApp*		m_pcEditApp;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CCONTROLPROCESS_H_ */

/*[EOF]*/
