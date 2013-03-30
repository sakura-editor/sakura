/*!	@file
	@brief コントロールプロセスクラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成, YAZAKI
	Copyright (C) 2006, ryoji

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
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlProcess : public CProcess {
public:
	CControlProcess( HINSTANCE hInstance, LPCTSTR lpCmdLine ) : 
		CProcess( hInstance, lpCmdLine ),
		m_pcEditApp( 0 ),
		// 2006.04.10 ryoji 同期オブジェクトのハンドルを初期化
		m_hMutex( NULL ),
		m_hMutexCP( NULL ),
		m_hEventCPInitialized( NULL )
	{}

	virtual ~CControlProcess();
protected:
	CControlProcess();
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

private:
	HANDLE			m_hMutex;				//!< アプリケーション実行検出用ミューテックス
	HANDLE			m_hMutexCP;				//!< コントロールプロセスミューテックス
	HANDLE			m_hEventCPInitialized;	//!< コントロールプロセス初期化完了イベント 2006.04.10 ryoji
	CEditApp*		m_pcEditApp;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CCONTROLPROCESS_H_ */

/*[EOF]*/
