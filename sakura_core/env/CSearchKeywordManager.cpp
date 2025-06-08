/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "DLLSHAREDATA.h"

#include "CSearchKeywordManager.h"
#include "recent/CRecentSearch.h"
#include "recent/CRecentReplace.h"
#include "recent/CRecentGrepFile.h"
#include "recent/CRecentGrepFolder.h"
#include "recent/CRecentExcludeFile.h"
#include "recent/CRecentExcludeFolder.h"

/*!	m_aSearchKeysにpszSearchKeyを追加する。
	YAZAKI
*/
void CSearchKeywordManager::AddToSearchKeyArr( const wchar_t* pszSearchKey )
{
	CRecentSearch	cRecentSearchKey;
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
	GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence++;
}

/*!	m_aReplaceKeysにpszReplaceKeyを追加する
	YAZAKI
*/
void CSearchKeywordManager::AddToReplaceKeyArr( const wchar_t* pszReplaceKey )
{
	CRecentReplace	cRecentReplaceKey;
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();
	GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence++;

	return;
}

/*!	m_aGrepFilesにpszGrepFileを追加する
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFileArr( const WCHAR* pszGrepFile )
{
	CRecentGrepFile	cRecentGrepFile;
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_aGrepFolders にpszGrepFolder を追加する
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFolderArr( const WCHAR* pszGrepFolder )
{
	CRecentGrepFolder	cRecentGrepFolder;
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}

/*!	m_aExcludeFilesにpszExcludeFileを追加する
*/
void CSearchKeywordManager::AddToExcludeFileArr(const WCHAR* pszExcludeFile)
{
	CRecentExcludeFile	cRecentExcludeFile;
	cRecentExcludeFile.AppendItem(pszExcludeFile);
	cRecentExcludeFile.Terminate();
}

/*!	m_aExcludeFolders.size()にpszExcludeFolderを追加する
*/
void CSearchKeywordManager::AddToExcludeFolderArr(const WCHAR* pszExcludeFolder)
{
	CRecentExcludeFolder	cRecentExcludeFolder;
	cRecentExcludeFolder.AppendItem(pszExcludeFolder);
	cRecentExcludeFolder.Terminate();
}
