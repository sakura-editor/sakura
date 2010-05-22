/*
	2008.05.18 kobake CShareData ���番��
*/

#include "stdafx.h"
#include "DLLSHAREDATA.h"

#include "CSearchKeywordManager.h"
#include "recent/CRecent.h"


/*!	m_aSearchKeys��pszSearchKey��ǉ�����B
	YAZAKI
*/
void CSearchKeywordManager::AddToSearchKeyArr( const wchar_t* pszSearchKey )
{
	CRecentSearch	cRecentSearchKey;
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_aReplaceKeys��pszReplaceKey��ǉ�����
	YAZAKI
*/
void CSearchKeywordManager::AddToReplaceKeyArr( const wchar_t* pszReplaceKey )
{
	CRecentReplace	cRecentReplaceKey;
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_aGrepFiles��pszGrepFile��ǉ�����
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFileArr( const TCHAR* pszGrepFile )
{
	CRecentGrepFile	cRecentGrepFile;
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_aGrepFolders.size()��pszGrepFolder��ǉ�����
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFolderArr( const TCHAR* pszGrepFolder )
{
	CRecentGrepFolder	cRecentGrepFolder;
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}
