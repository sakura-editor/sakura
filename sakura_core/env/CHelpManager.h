/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

// 要先行定義
// #include "DLLSHAREDATA.h"


//!ヘルプ管理
class CHelpManager{
public:
	CHelpManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//ヘルプ関連	//@@@ 2002.2.3 YAZAKI
	bool			ExtWinHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );		//	タイプがnTypeのときに、外部ヘルプが設定されているか。
	const TCHAR*	GetExtWinHelp( CTypeConfig nType = CTypeConfig(-1) );		//	タイプがnTypeのときの、外部ヘルプファイル名を取得。
	bool			ExtHTMLHelpIsSet( CTypeConfig nType = CTypeConfig(-1) );	//	タイプがnTypeのときに、外部HTMLヘルプが設定されているか。
	const TCHAR*	GetExtHTMLHelp( CTypeConfig nType = CTypeConfig(-1) );		//	タイプがnTypeのときの、外部HTMLヘルプファイル名を取得。
	bool			HTMLHelpIsSingle( CTypeConfig nType = CTypeConfig(-1) );	//	タイプがnTypeのときの、外部HTMLヘルプ「ビューアを複数起動しない」がONかを取得。
private:
	DLLSHAREDATA* m_pShareData;
};
