﻿/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
