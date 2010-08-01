/*!	@file
	@brief プラグイン基本クラス

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
#ifndef SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_
#define SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_

#include <algorithm>
#include "macro/CWSHIfObj.h"
#include "CDataProfile.h"

typedef int PlugId;

// Plugin Function番号の計算(クラス外でも使えるバージョン)
// 2010/4/19 Uchi
inline EFunctionCode GetPluginFunctionCode( PlugId nPluginId, int nPlugNo )
{
	return EFunctionCode( nPluginId * MAX_PLUG_CMD + nPlugNo + F_PLUGCOMMAND_FIRST );
}

// PlugId番号の計算(クラス外でも使えるバージョン)
// 2010/4/19 Uchi
inline PlugId GetPluginId( EFunctionCode nFunctionCode )
{
	if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
		return PlugId( (nFunctionCode - F_PLUGCOMMAND_FIRST) / MAX_PLUG_CMD );
	}
	return -1;
}

// PluginNo番号の計算(クラス外でも使えるバージョン)
// 2010/6/24 Uchi
inline PlugId GetPlugNo( EFunctionCode nFunctionCode )
{
	if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
		return PlugId( (nFunctionCode - F_PLUGCOMMAND_FIRST) % MAX_PLUG_CMD );
	}
	return -1;
}

//プラグイン定義ファイル名
#define PII_FILENAME				_T("plugin.def")
//オプションファイル拡張子（オプションファイル＝個別フォルダ名＋拡張子）
#define PII_OPTFILEEXT				_T(".ini")

//プラグイン定義ファイル・キー文字列
#define	PII_PLUGIN					L"Plugin"		//共通情報
#define	PII_PLUGIN_ID				L"Id"			//ID：プラグインID
#define	PII_PLUGIN_NAME				L"Name"			//名前：プラグイン名
#define	PII_PLUGIN_DESCRIPTION		L"Description"	//説明：簡潔な説明
#define	PII_PLUGIN_PLUGTYPE			L"Type"			//種別：wsh / dll
#define	PII_PLUGIN_AUTHOR			L"Author"		//作者：著作権者名
#define	PII_PLUGIN_VERSION			L"Version"		//バージョン：プラグインのバージョン
#define	PII_PLUGIN_URL				L"Url"			//配布URL：配布元URL

#define PII_PLUG					L"Plug"			//プラグ情報

#define PII_COMMAND					L"Command"		//コマンド情報
#define PII_OPTION					L"Option"		//オプション定義情報	// 2010/3/24 Uchi


class CPlugin;

//プラグ（プラグイン内の処理単位）クラス
class CPlug
{
	//型定義
protected:
	typedef std::wstring wstring;
public:
	/*!
	  CPlug::Arrayはstd::vectorなので、要素の追加削除（insert/erase）をすると
	  イテレータが無効になることがある。そのため変数に格納したイテレータを
	  insert/eraseの第一引数に指定すると、VC2005でビルドエラーが出る。
	  かわりにbegin/endからの相対位置指定や、インデックス指定を使うこと。
	*/
	typedef std::vector<CPlug*> Array;			//プラグのリスト
	typedef Array::const_iterator ArrayIter;	//そのイテレータ

	//コンストラクタ
public:
	CPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
		: m_id( id )
		, m_sJack( sJack )
		, m_sHandler( sHandler )
		, m_cPlugin( plugin )
		, m_sLabel( sLabel )
	{
	}
	//デストラクタ
public:
	virtual ~CPlug() {}

	//操作
public:
	bool Invoke( CEditView* view, CWSHIfObj::List& params );	//プラグを実行する

	//属性
public:
	EFunctionCode GetFunctionCode() const;

	//メンバ変数
public:
	const PlugId m_id;					//プラグID
	const wstring m_sJack;				//関連付けるジャック名
	const wstring m_sHandler;			//ハンドラ文字列（関数名）
	const wstring m_sLabel;				//ラベル文字列
	wstring m_sIcon;					//アイコンのファイルパス
	CPlugin& m_cPlugin;					//親プラグイン
};

// オプション定義	// 2010/3/24 Uchi
std::vector<std::wstring> wstring_split( std::wstring, wchar_t );

class CPluginOption
{
	//型定義
protected:
	typedef std::wstring wstring;
public:
	typedef std::vector<CPluginOption*> Array;	// オプションのリスト
	typedef Array::const_iterator ArrayIter;	// そのイテレータ

	//コンストラクタ
public:
	CPluginOption( CPlugin* parent, wstring sLabel, wstring sSection, wstring sKey, wstring sType, wstring sSelects, int index) 
	{
		m_parent	= parent;
		m_sLabel	= sLabel;
		m_sSection	= sSection;
		m_sKey		= sKey;
		// 小文字変換
		std::transform( sType.begin (), sType.end (), sType.begin (), tolower );
		m_sType		= sType;
		m_sSelects	= sSelects;
		m_index		= index;
	}

	//デストラクタ
public:
	~CPluginOption() {}

	//操作
public:
	wstring	GetLabel( void )	{ return m_sLabel; }
	void	GetKey( wstring* sectin, wstring* key )	{ 
		*sectin = m_sSection; 
		*key = m_sKey;
	}
	wstring	GetType( void )		{ return m_sType; }
	int 	GetIndex( void )	{ return m_index; }
	std::vector<wstring>	GetSelects()
	{
		return (wstring_split(m_sSelects, L'|'));
	}

protected:
	CPlugin*	m_parent;
	wstring		m_sLabel;
	wstring		m_sSection;
	wstring		m_sKey;
	wstring		m_sType;
	wstring		m_sSelects;		// 選択候補
	int 		m_index; 
};


//プラグインクラス
typedef int PluginId;

class CPlugin
{
	//型定義
protected:
	typedef std::wstring wstring;
	typedef std::basic_string<TCHAR> tstring;
public:
	typedef std::list<CPlugin*> List;		//プラグインのリスト
	typedef List::const_iterator ListIter;	//そのイテレータ

	//コンストラクタ
public:
	CPlugin( tstring sBaseDir );

	//デストラクタ
public:
	virtual ~CPlugin(void);

	//操作
public:
	tstring GetFilePath( const tstring& sFileName ) const;				//プラグインフォルダ基準の相対パスをフルパスに変換
	virtual int AddCommand( const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister );//コマンドを追加する
	int 	GetCommandCount()	{ return m_nCommandCount; };			// コマンド数を返す	2010/7/4 Uchi

protected:
	bool ReadPluginDefCommon( CDataProfile *cProfile );					//プラグイン定義ファイルのCommonセクションを読み込む
	bool ReadPluginDefPlug( CDataProfile *cProfile );					//プラグイン定義ファイルのPlugセクションを読み込む
	bool ReadPluginDefCommand( CDataProfile *cProfile );				//プラグイン定義ファイルのCommandセクションを読み込む
	bool ReadPluginDefOption( CDataProfile *cProfile );					//プラグイン定義ファイルのOptionセクションを読み込む	// 2010/3/24 Uchi

	//CPlugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
	{
		return new CPlug( plugin, id, sJack, sHandler, sLabel );
	}

//	void NormalizeExtList( const wstring& sExtList, wstring& sOut );	//カンマ区切り拡張子リストを正規化する

	//属性
public:
	tstring GetPluginDefPath() const{ return GetFilePath( PII_FILENAME ); }	//プラグイン定義ファイルのパス
	tstring GetOptionPath() const{ return m_sBaseDir + PII_OPTFILEEXT; }	//オプションファイルのパス
	virtual CPlug::Array GetPlugs() const = 0;								//プラグの一覧

	//メンバ変数
public:
	PluginId m_id;				//!< プラグイン番号（エディタがふる0～MAX_PLUGIN-1の番号）
	wstring m_sId;				//!< プラグインID
	wstring m_sName;			//!< プラグイン和名
	wstring m_sDescription;		//!< プラグインについての簡単な記述
	wstring m_sAuthor;			//!< 作者
	wstring m_sVersion;			//!< バージョン
	wstring m_sUrl;				//!< 配布URL
	tstring m_sBaseDir;
	CPluginOption::Array m_options;		// オプション	// 2010/3/24 Uchi
private:
	bool m_bLoaded;
protected:
	CPlug::Array m_plugs;
	int m_nCommandCount;

	//非実装提供
public:
	virtual bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& param ) =0;	//プラグを実行する
	virtual bool ReadPluginDef( CDataProfile *cProfile ) =0;		//プラグイン定義ファイルを読み込む
	virtual bool ReadPluginOption( CDataProfile *cProfile ) =0;		//オプションファイルを読み込む
};

#endif /* SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_ */
/*[EOF]*/
