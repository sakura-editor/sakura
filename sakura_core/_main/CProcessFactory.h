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

	与えられたコマンドライン引数から生成すべきプロセスの種別を判定し，
	対応するオブジェクトを返すFactoryクラス．

	通常のエディタプロセスの起動が指定された場合には，必要に応じてコントロールプロセス
	起動の起動をエディタの起動に先立って行う．
*/
class CProcessFactory {
public:
	CProcess* Create( HINSTANCE hInstance, LPCTSTR lpCmdLine );
protected:
private:
	bool IsValidVersion();
	bool ProfileSelect( HINSTANCE, LPCTSTR );
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
	bool WaitForInitializedControlProcess();	// 2006.04.10 ryoji コントロールプロセスの初期化完了イベントを待つ
	bool TestWriteQuit();	// 2007.09.04 ryoji 「設定を保存して終了する」オプション処理（sakuext連携用）
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESSFACTORY_H_ */


