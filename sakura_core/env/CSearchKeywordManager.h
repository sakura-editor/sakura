/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

#include "DLLSHAREDATA.h"

//共有メモリ内構造体
struct SShare_SearchKeywords{
	// -- -- 検索キー -- -- //
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_SEARCHKEY,  const WCHAR*>	m_aSearchKeys;
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_REPLACEKEY, const WCHAR*>	m_aReplaceKeys;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFILE,   const TCHAR*>	m_aGrepFiles;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>	m_aGrepFolders;
};

//! 検索キーワード管理
class CSearchKeywordManager{
public:
	CSearchKeywordManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//@@@ 2002.2.2 YAZAKI
	void		AddToSearchKeyArr( const wchar_t* pszSearchKey );	//	m_aSearchKeysにpszSearchKeyを追加する
	void		AddToReplaceKeyArr( const wchar_t* pszReplaceKey );	//	m_aReplaceKeysにpszReplaceKeyを追加する
	void		AddToGrepFileArr( const TCHAR* pszGrepFile );		//	m_aGrepFilesにpszGrepFileを追加する
	void		AddToGrepFolderArr( const TCHAR* pszGrepFolder );	//	m_aGrepFolders.size()にpszGrepFolderを追加する
private:
	DLLSHAREDATA* m_pShareData;
};
