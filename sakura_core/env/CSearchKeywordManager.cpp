/*
	2008.05.18 kobake CShareData Ç©ÇÁï™ó£
*/

#include "stdafx.h"
#include "CSearchKeywordManager.h"


/*!	m_aSearchKeysÇ…pszSearchKeyÇí«â¡Ç∑ÇÈÅB
	YAZAKI
*/
void CSearchKeywordManager::AddToSearchKeyArr( const wchar_t* pszSearchKey )
{
	CRecentSearch	cRecentSearchKey;
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_aReplaceKeysÇ…pszReplaceKeyÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void CSearchKeywordManager::AddToReplaceKeyArr( const wchar_t* pszReplaceKey )
{
	CRecentReplace	cRecentReplaceKey;
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_aGrepFilesÇ…pszGrepFileÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFileArr( const TCHAR* pszGrepFile )
{
	CRecentGrepFile	cRecentGrepFile;
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_aGrepFolders.size()Ç…pszGrepFolderÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void CSearchKeywordManager::AddToGrepFolderArr( const TCHAR* pszGrepFolder )
{
	CRecentGrepFolder	cRecentGrepFolder;
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}
