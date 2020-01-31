/*!	@file
	@brief エディタプロセスクラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "global.h"
#include "CProcess.h"
#include "extmodule/CMigemo.h"
#include "CEditApp.h"
#include "util/design_template.h"
class CEditWnd;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはCEditWndクラスのインスタンスを作る。
*/
class CNormalProcess final : public CProcess
{
  public:
    //コンストラクタ・デストラクタ
    CNormalProcess(HINSTANCE hInstance, LPCWSTR lpCmdLine);
    ~CNormalProcess();

  protected:
    //プロセスハンドラ
    bool InitializeProcess() override;
    bool MainLoop() override;
    void OnExitProcess() override;

  protected:
    //実装補助
    HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka
    void OpenFiles(HWND hwnd);

  private:
    CEditApp *m_pcEditApp; //2007.10.23 kobake
};
