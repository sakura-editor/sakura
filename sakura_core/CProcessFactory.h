/*!	@file
	@brief プロセス生成クラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESSFACTORY_H_
#define _CPROCESSFACTORY_H_

#include "global.h"

class CProcess;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス生成クラス
*/
class SAKURA_CORE_API CProcessFactory {
public:
	CProcess* Create( HINSTANCE hInstance, LPSTR lpCmdLine );
protected:
private:
	bool IsValidVersion();
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
	bool WaitForInitializedControlProcess();	// 2006.04.10 ryoji コントロールプロセスの初期化完了イベントを待つ
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESSFACTORY_H_ */

/*[EOF]*/
