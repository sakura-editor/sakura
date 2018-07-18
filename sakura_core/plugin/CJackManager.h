/*!	@file
	@brief ジャック管理クラス

*/
/*
	Copyright (C) 2009, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_
#define SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_

#include "plugin/CPlugin.h"
#include <list>

#define PP_COMMAND_STR	L"Command"

// ジャック（＝プラグイン可能箇所）
enum EJack {
	PP_NONE			= -1,
	PP_COMMAND		= 0,
//	PP_INSTALL,
//	PP_UNINSTALL,
//	PP_APP_START,	// 現状エディタごとにプラグイン管理しているため
//	PP_APP_END,		// アプリレベルのイベントは扱いにくい
	PP_EDITOR_START,
	PP_EDITOR_END,
	PP_DOCUMENT_OPEN,
	PP_DOCUMENT_CLOSE,
	PP_DOCUMENT_BEFORE_SAVE,
	PP_DOCUMENT_AFTER_SAVE,
	PP_OUTLINE,
	PP_SMARTINDENT,
	PP_COMPLEMENT,
	PP_COMPLEMENTGLOBAL,

	//↑ジャックを追加するときはこの行の上に。
	PP_BUILTIN_JACK_COUNT	//組み込みジャック数
};

// ジャック定義構造体
typedef struct tagJackDef {
	EJack			ppId;
	const WCHAR*	szName;
	CPlug::Array		plugs;	//ジャックに関連付けられたプラグ
} JackDef;

// プラグ登録結果
enum ERegisterPlugResult {
	PPMGR_REG_OK,				//プラグイン登録成功
	PPMGR_INVALID_NAME,			//ジャック名が不正
	PPMGR_CONFLICT				//指定したジャックは別のプラグインが接続している
};


//ジャック管理クラス
class CJackManager : public TSingleton<CJackManager>{
	friend class TSingleton<CJackManager>;
	CJackManager();

	typedef std::wstring wstring;

	//操作
public:
	ERegisterPlugResult RegisterPlug( wstring pszJack, CPlug* plug );	//プラグをジャックに関連付ける
	bool UnRegisterPlug( wstring pszJack, CPlug* plug );	//プラグの関連付けを解除する
	bool GetUsablePlug( EJack jack, PlugId plugId, CPlug::Array* plugs );	//利用可能なプラグを検索する
private:
	EJack GetJackFromName( wstring sName );	//ジャック名をジャック番号に変換する

	//属性
public:
	std::vector<JackDef> GetJackDef() const;	//ジャック定義一覧を返す
	EFunctionCode GetCommandCode( int index ) const;		//プラグインコマンドの機能コードを返す
	int GetCommandName( int funccode, WCHAR* buf, int size ) const;	//プラグインコマンドの名前を返す
	int GetCommandCount() const;	//プラグインコマンドの数を返す
	CPlug* GetCommandById( int id ) const;	//IDに合致するコマンドプラグを返す
	const CPlug::Array& GetPlugs( EJack jack ) const;	//プラグを返す
	//TODO: 作りが一貫してないので整理する syat

	//メンバ変数
private:
	DLLSHAREDATA* m_pShareData;
	std::vector<JackDef> m_Jacks;	//ジャック定義の一覧
};

#endif /* SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_ */
/*[EOF]*/
