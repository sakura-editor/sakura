/*!	@file
	@brief インポート、エクスポートマネージャ

	@author Uchi
	@date 2010/4/22 新規作成
*/
/*
	Copyright (C) 2010, Uchi, Moca

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

#ifndef SAKURA_CIMPEXPMANAGER_H_
#define SAKURA_CIMPEXPMANAGER_H_

#include "CDataProfile.h"
#include "env/DLLSHAREDATA.h"

using std::wstring;

class CImpExpManager
{
public:
	bool ImportUI( HINSTANCE, HWND );
	bool ExportUI( HINSTANCE, HWND );
	virtual bool ImportAscertain( HINSTANCE, HWND, const wstring&, wstring& );
	virtual bool Import( const wstring&, wstring& ) = 0;
	virtual bool Export( const wstring&, wstring& ) = 0;
	// ファイル名の初期値を設定
	void SetBaseName( const wstring& );
	// フルパス名を取得
	inline wstring GetFullPath()
	{
		return to_wchar( GetDllShareData().m_sHistory.m_szIMPORTFOLDER ) + m_sOriginName;
	}
	// フルパス名を取得
	inline wstring MakeFullPath( wstring sFileName )
	{
		return to_wchar( GetDllShareData().m_sHistory.m_szIMPORTFOLDER ) + sFileName;
	}
	// ファイル名を取得
	inline wstring GetFileName()	{ return m_sOriginName; }

protected:
	// Import Folderの設定
	inline void SetImportFolder( const TCHAR* szPath ) 
	{
		/* ファイルのフルパスをフォルダとファイル名に分割 */
		/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
		::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
		_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
	}

	// デフォルト拡張子の取得(「*.txt」形式)
	virtual const TCHAR* GetDefaultExtension();
	// デフォルト拡張子の取得(「txt」形式)
	virtual const wchar_t* GetOriginExtension();

protected:
	wstring		m_sBase;
	wstring		m_sOriginName;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          タイプ別設定                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CImpExpType : public CImpExpManager
{
public:
	// Constructor
	CImpExpType( int nIdx, STypeConfig& types, HWND hwndList )
		: m_nIdx( nIdx )
		, m_Types( types )
		, m_hwndList( hwndList )
	{
		/* 共有データ構造体のアドレスを返す */
		m_pShareData = &GetDllShareData();
	}

public:
	bool ImportAscertain( HINSTANCE, HWND, const wstring&, wstring& );
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }
	bool IsAddType(){ return m_bAddType; }

private:
	// インターフェース用
	int 			m_nIdx;
	STypeConfig&	m_Types;
	HWND			m_hwndList;

	// 内部使用
	DLLSHAREDATA*	m_pShareData;
	int				m_nColorType;
	wstring 		m_sColorFile;
	bool			m_bAddType;
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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.col"); }
	const wchar_t* GetOriginExtension()	{ return L"col"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.rkw"); }
	const wchar_t* GetOriginExtension()	{ return L"rkw"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.txt"); }
	const wchar_t* GetOriginExtension()	{ return L"txt"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.key"); }
	const wchar_t* GetOriginExtension()	{ return L"key"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.mnu"); }
	const wchar_t* GetOriginExtension()	{ return L"mnu"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.kwd"); }
	const wchar_t* GetOriginExtension()	{ return L"kwd"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

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

public:
	bool Import( const wstring&, wstring& );
	bool Export( const wstring&, wstring& );
	static void IO_FileTreeIni( CDataProfile&, std::vector<SFileTreeItem>& );

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	std::vector<SFileTreeItem>&		m_aFileTreeItems;
};


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CIMPEXPMANAGER_H_ */
