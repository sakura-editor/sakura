/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CHELPMANAGER_57445D73_99E9_4B85_A905_47685753D1DF_H_
#define SAKURA_CHELPMANAGER_57445D73_99E9_4B85_A905_47685753D1DF_H_
#pragma once

#include "env/CSakuraEnvironment.h"	//env::ShareDataClient

//!ヘルプ管理
class CHelpManager : private env::ShareDataClient {
public:
	//ヘルプ関連	//@@@ 2002.2.3 YAZAKI
	bool			ExtWinHelpIsSet( const STypeConfig* pType = nullptr );		//	タイプがnTypeのときに、外部ヘルプが設定されているか。
	const WCHAR*	GetExtWinHelp( const STypeConfig* pType = nullptr );		//	タイプがnTypeのときの、外部ヘルプファイル名を取得。
	bool			ExtHTMLHelpIsSet( const STypeConfig* pType = nullptr );	//	タイプがnTypeのときに、外部HTMLヘルプが設定されているか。
	const WCHAR*	GetExtHTMLHelp( const STypeConfig* pType = nullptr );		//	タイプがnTypeのときの、外部HTMLヘルプファイル名を取得。
	bool			HTMLHelpIsSingle( const STypeConfig* pType = nullptr );	//	タイプがnTypeのときの、外部HTMLヘルプ「ビューアを複数起動しない」がONかを取得。
};

#endif /* SAKURA_CHELPMANAGER_57445D73_99E9_4B85_A905_47685753D1DF_H_ */
