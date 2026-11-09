/*!	@file
	@brief インポート、エクスポートマネージャ

	@author Uchi
	@date 2010/4/22 新規作成
*/
/*
	Copyright (C) 2010, Uchi, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CIMPEXPMANAGER_12EC6C8E_1661_485E_8972_A7A9AE419BC8_H_
#define SAKURA_CIMPEXPMANAGER_12EC6C8E_1661_485E_8972_A7A9AE419BC8_H_
#pragma once

#include "basis/primitive.h"
#include "env/CDataProfile.h"
#include "env/CSakuraEnvironment.h"	//env::ShareDataClient
#include "env/DLLSHAREDATA.h"

class CImpExpManager
{
public:
	virtual ~CImpExpManager() = default;

	bool ImportUI(HINSTANCE hInstance, HWND hwndParent);
	bool ExportUI(HINSTANCE hInstance, HWND hwndParent);
	virtual bool ImportAscertain(HINSTANCE hInstance, HWND hwndParent, const sakura::wstring& sFileName, sakura::wstring& sErrMsg);
	virtual bool Import(const sakura::wstring& sFileName, sakura::wstring& sErrMsg) = 0;
	virtual bool Export(const sakura::wstring& sFileName, sakura::wstring& sErrMsg) = 0;

	// ファイル名の初期値を設定
	void SetBaseName(const sakura::wstring& sBase);

	// ファイル名の初期値を設定
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<std::remove_cvref_t<A>, sakura::wstring>)
	void SetBaseName(A baseName)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _BaseName{ baseName };

		// 文字列を渡すバージョンを呼び出す
		return SetBaseName(
			sakura::wstring{ _BaseName.str() }
		);
	}

	// フルパス名を取得
	inline sakura::wstring GetFullPath() const
	{
		return MakeFullPath(m_sOriginName);
	}
	// フルパス名を取得
	inline sakura::wstring MakeFullPath(sakura::wstring_view sFileName) const
	{
		return (std::filesystem::path{ GetDllShareData().m_sHistory.m_szIMPORTFOLDER } / sFileName).native();
	}
	// ファイル名を取得
	inline sakura::wstring GetFileName() const { return m_sOriginName; }

protected:
	// Import Folderの設定
	inline void SetImportFolder(sakura::wstring_view importPath) const
	{
		/* ファイルのフルパスをフォルダーとファイル名に分割 */
		/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
		const auto szPath = std::data(importPath);
		::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, nullptr);

		wcscat_s(GetDllShareData().m_sHistory.m_szIMPORTFOLDER, LTEXT("\\"));
	}

	// デフォルト拡張子の取得(「*.txt」形式)
	virtual const WCHAR* GetDefaultExtension();
	// デフォルト拡張子の取得(「txt」形式)
	virtual const WCHAR* GetOriginExtension();

protected:
	sakura::wstring		m_sBase;
	sakura::wstring		m_sOriginName;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          タイプ別設定                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpType : public CImpExpManager, private env::ShareDataClient
{
public:
	// Constructor
	CImpExpType( int nIdx, STypeConfig& types, HWND hwndList )
		: m_nIdx( nIdx )
		, m_Types( types )
		, m_hwndList( hwndList )
	{
	}

	bool ImportAscertain( HINSTANCE, HWND, const sakura::wstring&, sakura::wstring& ) override;
	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.ini"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("ini"); }

	bool	IsAddType() const noexcept { return m_bAddType; }

private:
	// インターフェース用
	int 			m_nIdx;
	STypeConfig&	m_Types;
	HWND			m_hwndList;

	// 内部使用
	int				m_nColorType = -1;
	sakura::wstring 	m_sColorFile;
	bool			m_bAddType = false;
	CDataProfile	m_cProfile;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpColors : public CImpExpManager
{
public:
	// Constructor
	CImpExpColors( ColorInfo * psColorInfoArr )
		: m_ColorInfoArr( psColorInfoArr )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.col"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("col"); }

private:
	ColorInfo*		m_ColorInfoArr;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpRegex : public CImpExpManager
{
public:
	// Constructor
	CImpExpRegex( STypeConfig& types )
		: m_Types( types )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.rkw"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("rkw"); }

private:
	STypeConfig&	m_Types;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードヘルプ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeyHelp : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeyHelp( STypeConfig& types )
		: m_Types( types )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.txt"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("txt"); }

private:
	STypeConfig&	m_Types;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キー割り当て                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeybind : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeybind( CommonSetting& common )
		: m_Common( common )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.key"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("key"); }

private:
	CommonSetting&		m_Common;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpCustMenu : public CImpExpManager
{
public:
	// Constructor
	CImpExpCustMenu( CommonSetting& common )
		: m_Common( common )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.mnu"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("mnu"); }

private:
	CommonSetting&		m_Common;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     強調キーワード                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpKeyWord : public CImpExpManager
{
public:
	// Constructor
	CImpExpKeyWord( CommonSetting& common, int nKeyWordSetIdx, bool& bCase )
		: m_Common( common )
		, m_nIdx( nKeyWordSetIdx )
		, m_bCase( bCase )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.kwd"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("kwd"); }

private:
	CommonSetting&		m_Common;
	int 				m_nIdx;
	bool&				m_bCase;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpMainMenu : public CImpExpManager
{
public:
	// Constructor
	CImpExpMainMenu( CommonSetting& common )
		: m_Common( common )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.ini"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("ini"); }

private:
	CommonSetting&		m_Common;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpFileTree : public CImpExpManager
{
public:
	// Constructor
	CImpExpFileTree( std::vector<SFileTreeItem>& items )
		: m_aFileTreeItems( items )
	{
	}

	bool Import( const sakura::wstring&, sakura::wstring& ) override;
	bool Export( const sakura::wstring&, sakura::wstring& ) override;
	static void IO_FileTreeIni( CDataProfile&, std::vector<SFileTreeItem>& );

	// デフォルト拡張子の取得
	const WCHAR* GetDefaultExtension() override	{ return LTEXT("*.ini"); }
	const WCHAR* GetOriginExtension() override	{ return LTEXT("ini"); }

private:
	std::vector<SFileTreeItem>&		m_aFileTreeItems;
};

#endif /* SAKURA_CIMPEXPMANAGER_12EC6C8E_1661_485E_8972_A7A9AE419BC8_H_ */
