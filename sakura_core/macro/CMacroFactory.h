/*!	@file
	@brief マクロ種別管理

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2004, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMACROFACTORY_67B6F8F6_0951_4717_84AD_C67E6D5F68AB_H_
#define SAKURA_CMACROFACTORY_67B6F8F6_0951_4717_84AD_C67E6D5F68AB_H_
#pragma once

#include <map>
#include <list>
#include <string>
#include "util/design_template.h"

class CMacroManagerBase;

/*!
	@brief マクロHandler生成クラス
	
	@par 初期化
	CMacroManagerBase::declare() により，MacroEngineのCreaterの登録
	RegisterEngine() 及び 対応拡張子の登録 RegisterExt() が呼び出される．
	
	@par 呼び出し
	CMacroFactory::Create()を拡張子を引数にして呼び出すと対応する
	マクロエンジンが返される．得られたEngineに対してLoadKeyMacro()及び
	ExecKeyMacro() を呼び出すことでマクロの読み込み・実行が行われる．

	Singleton
*/
class CMacroFactory : public TSingleton<CMacroFactory> {
	friend class TSingleton<CMacroFactory>;
	CMacroFactory();

public:
	typedef CMacroManagerBase* (*Creator)(const WCHAR*);

	bool RegisterCreator(Creator f);
	// Jan. 31, 2004 genta
	// バイナリサイズ削減のためm_mMacroExtsを削除
	//bool RegisterExt( const char*, Creator );
	bool Unregister(Creator f);

	CMacroManagerBase* Create(const WCHAR* ext);

private:
	std::wstring Ext2Key(const WCHAR *ext);

	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため拡張子保持用mapを削除
	//	typedef std::map<std::string, Creator> MacroTypeRep;
	typedef std::list<Creator> MacroEngineRep;

	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため
	//MacroTypeRep m_mMacroExts;	/*!< 拡張子対応表 */
	/*!
		Creatorリスト
		@date 2002.08.25 genta 追加
	*/
	MacroEngineRep m_mMacroCreators;
};
#endif /* SAKURA_CMACROFACTORY_67B6F8F6_0951_4717_84AD_C67E6D5F68AB_H_ */
