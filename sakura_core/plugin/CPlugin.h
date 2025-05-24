/*!	@file
	@brief プラグイン基本クラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CPLUGIN_9E2D4124_CD2F_46B4_BEFA_4887FCCB2D0A_H_
#define SAKURA_CPLUGIN_9E2D4124_CD2F_46B4_BEFA_4887FCCB2D0A_H_
#pragma once

#include <algorithm>
#include "macro/CWSHIfObj.h"
#include "CDataProfile.h"
#include "util/string_ex.h"

//! プラグインの管理番号index
typedef int PluginId;
//! プラグの管理番号 プラグインのコマンドプラグごとに一意。ほかは0
typedef int PlugId;

//プラグイン定義ファイル名
#define PII_FILENAME				L"plugin.def"
#define PII_L10NDIR					L"local"
#define PII_L10NFILEBASE			L"plugin_"
#define PII_L10NFILEEXT				L".def"
//オプションファイル拡張子（オプションファイル＝個別フォルダー名＋拡張子）
#define PII_OPTFILEEXT				L".ini"

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
#define PII_STRING					L"String"		//文字列情報

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
		, m_sLabel( sLabel )
		, m_cPlugin( plugin )
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

	//補助関数
public:
	// Plug Function番号の計算(クラス外でも使えるバージョン)
	// 2010/4/19 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline EFunctionCode GetPluginFunctionCode( PluginId nPluginId, PlugId nPlugId )
	{
		return static_cast<EFunctionCode>( (nPluginId%20 * 100) + (nPluginId/20 * 50) + nPlugId + F_PLUGCOMMAND_FIRST );
	}

	// PluginId番号の計算(クラス外でも使えるバージョン)
	// 2010/4/19 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline PluginId GetPluginId( EFunctionCode nFunctionCode )
	{
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PluginId( (nFunctionCode - F_PLUGCOMMAND_FIRST)/100 + (nFunctionCode%100/50 * 20) );
		}
		return PluginId(-1);
	}

	// PluginNo番号の計算(クラス外でも使えるバージョン)
	// 2010/6/24 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline PlugId GetPlugId( EFunctionCode nFunctionCode )
	{
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PlugId( nFunctionCode%100 - (nFunctionCode%100/50 * 50) );
		}
		return PlugId(-1);
	}

	/* PluginId, PlugId と 関数コードのマッピング *****************************
	 *   PluginId … プラグインの番号 0～39
	 *     PlugId … プラグイン内のプラグの番号 0～49
	 *
	 *   関数コード 20000～21999   ()内は(PluginId, PlugId)を表す
	 *   +------------+------------+----+------------+
	 *   |20000(0,0)  |20100(1,0)  |    |21900(19,0) |
	 *   |  :         |  :         | … |  :         |
	 *   |20049(0,49) |20149(1,49) |    |21949(19,49)| 
	 *   +------------+------------+----+------------+
	 *   |20050(20,0) |20150(21,0) |    |21950(39,0) |
	 *   |  :         |  :         | … |  :         |
	 *   |20099(20,49)|20199(21,49)|    |21999(39,49)| 
	 *   +------------+------------+----+------------+
	 *   もし足りなければ、22000～23999を払い出して食いつぶす
	 *************************************************************************/
	static EOutlineType GetOutlineType( EFunctionCode nFunctionCode ){
		return static_cast<EOutlineType>(nFunctionCode);
	}

	static ESmartIndentType GetSmartIndentType( EFunctionCode nFunctionCode ){
		return static_cast<ESmartIndentType>(nFunctionCode);
	}

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
	CPluginOption( CPlugin* parent, wstring sLabel, wstring sSection, wstring sKey, wstring sType, wstring sSelects, wstring sDefaultVal, int index) 
		: m_sLabel(sLabel), m_sSection(sSection), m_sKey(sKey)
	{
		m_parent	= parent;
		// 小文字変換
		std::transform( sType.begin (), sType.end (), sType.begin (), my_towlower2 );
		m_sType		= std::move(sType);
		m_sSelects	= std::move(sSelects);
		m_sDefaultVal = std::move(sDefaultVal);
		m_index		= index;
	}

	//デストラクタ
public:
	~CPluginOption() = default;

	//操作
public:
	wstring	GetLabel( void ) const  { return m_sLabel; }
	void	GetKey( wstring* sectin, wstring* key ) const { 
		*sectin = m_sSection; 
		*key = m_sKey;
	}
	wstring	GetType( void ) const	{ return m_sType; }
	int 	GetIndex( void ) const	{ return m_index; }
	std::vector<wstring>	GetSelects() const
	{
		return (wstring_split(m_sSelects, L'|'));
	}
	wstring	GetDefaultVal() const { return m_sDefaultVal; }

protected:
	CPlugin*	m_parent;
	wstring		m_sLabel;
	wstring		m_sSection;
	wstring		m_sKey;
	wstring		m_sType;
	wstring		m_sSelects;		// 選択候補
	wstring		m_sDefaultVal;
	int 		m_index; 
};

//プラグインクラス

class CPlugin
{
	using Me = CPlugin;

	//型定義
protected:
	typedef std::wstring wstring;

public:
	typedef std::list<CPlugin*> List;		//プラグインのリスト
	typedef List::const_iterator ListIter;	//そのイテレータ

	//コンストラクタ
public:
	CPlugin( const wstring& sBaseDir );
	CPlugin(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CPlugin(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;

	//デストラクタ
public:
	virtual ~CPlugin(void);

	//操作
public:
	virtual int AddCommand( const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister );//コマンドを追加する
	int 	GetCommandCount()	{ return m_nCommandCount; }			// コマンド数を返す	2010/7/4 Uchi

protected:
	bool ReadPluginDefCommon( CDataProfile *cProfile, CDataProfile *cProfileMlang );					//プラグイン定義ファイルのCommonセクションを読み込む
	bool ReadPluginDefPlug( CDataProfile *cProfile, CDataProfile *cProfileMlang );					//プラグイン定義ファイルのPlugセクションを読み込む
	bool ReadPluginDefCommand( CDataProfile *cProfile, CDataProfile *cProfileMlang );				//プラグイン定義ファイルのCommandセクションを読み込む
	bool ReadPluginDefOption( CDataProfile *cProfile, CDataProfile *cProfileMlang );					//プラグイン定義ファイルのOptionセクションを読み込む	// 2010/3/24 Uchi
	bool ReadPluginDefString( CDataProfile *cProfile, CDataProfile *cProfileMlang );					//プラグイン定義ファイルのStringセクションを読み込む

	//CPlugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
	{
		return new CPlug( plugin, id, sJack, sHandler, sLabel );
	}

//	void NormalizeExtList( const wstring& sExtList, wstring& sOut );	//カンマ区切り拡張子リストを正規化する

	//属性
public:
	wstring GetFilePath( const wstring& sFileName ) const;				//プラグインフォルダー基準の相対パスをフルパスに変換
	wstring GetPluginDefPath() const{ return GetFilePath( PII_FILENAME ); }	//プラグイン定義ファイルのパス
	wstring GetOptionPath() const{ return m_sOptionDir + PII_OPTFILEEXT; }	//オプションファイルのパス
	wstring GetFolderName() const;	//プラグインのフォルダー名を取得
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
	wstring m_sBaseDir;
	wstring m_sOptionDir;
	wstring m_sLangName;		//!< 言語名
	CPluginOption::Array m_options;		// オプション	// 2010/3/24 Uchi
	std::vector<std::wstring> m_aStrings;	// 文字列
protected:
	CPlug::Array m_plugs;
	int m_nCommandCount;

	//非実装提供
public:
	virtual bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& param ) =0;	//プラグを実行する
	virtual bool ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang ) =0;		//プラグイン定義ファイルを読み込む
	virtual bool ReadPluginOption( CDataProfile *cProfile ) =0;		//オプションファイルを読み込む
};
#endif /* SAKURA_CPLUGIN_9E2D4124_CD2F_46B4_BEFA_4887FCCB2D0A_H_ */
