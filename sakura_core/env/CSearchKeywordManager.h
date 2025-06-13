/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSEARCHKEYWORDMANAGER_AFD28203_4738_46B7_9A7F_E758A94DB290_H_
#define SAKURA_CSEARCHKEYWORDMANAGER_AFD28203_4738_46B7_9A7F_E758A94DB290_H_
#pragma once

//共有メモリ内構造体
struct SShare_SearchKeywords{
	// -- -- 検索キー -- -- //
	StaticVector< StaticString<_MAX_PATH>, MAX_SEARCHKEY,  const WCHAR*>	m_aSearchKeys;
	StaticVector< StaticString<_MAX_PATH>, MAX_REPLACEKEY, const WCHAR*>	m_aReplaceKeys;
	StaticVector< StaticString<MAX_GREP_PATH>, MAX_GREPFILE,   const WCHAR*>	m_aGrepFiles;
	StaticVector< StaticString<MAX_GREP_PATH>, MAX_GREPFOLDER, const WCHAR*>	m_aGrepFolders;
	StaticVector< StaticString<MAX_EXCLUDE_PATH>, MAX_EXCLUDEFILE,   const WCHAR*>	m_aExcludeFiles;
	StaticVector< StaticString<MAX_EXCLUDE_PATH>, MAX_EXCLUDEFOLDER, const WCHAR*>	m_aExcludeFolders;
};

struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

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
	void		AddToGrepFileArr( const WCHAR* pszGrepFile );		//	m_aGrepFilesにpszGrepFileを追加する
	void		AddToGrepFolderArr( const WCHAR* pszGrepFolder );	//	m_aGrepFolders にpszGrepFolder を追加する
	void		AddToExcludeFileArr( const WCHAR* pszExcludeFile );		//	m_aExcludeFiles に pszExcludeFile を追加する
	void		AddToExcludeFolderArr( const WCHAR* pszExcludeFolder );	//	m_aExcludeFolders に pszExcludeFolder を追加する
private:
	DLLSHAREDATA* m_pShareData;
};
#endif /* SAKURA_CSEARCHKEYWORDMANAGER_AFD28203_4738_46B7_9A7F_E758A94DB290_H_ */
