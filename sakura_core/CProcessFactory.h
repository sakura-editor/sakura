//	$Id$
/*!	@file
	@brief プロセス生成クラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
	$Revision$
*/
/*
	Copyright (C) 2002, aroka 新規作成

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
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESSFACTORY_H_ */

/*[EOF]*/
