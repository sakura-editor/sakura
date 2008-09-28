/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

#include "DLLSHAREDATA.h"

//���L���������\����
struct SShare_SearchKeywords{
	// -- -- �����L�[ -- -- //
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_SEARCHKEY,  const WCHAR*>	m_aSearchKeys;
	StaticVector< StaticString<WCHAR, _MAX_PATH>, MAX_REPLACEKEY, const WCHAR*>	m_aReplaceKeys;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFILE,   const TCHAR*>	m_aGrepFiles;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>	m_aGrepFolders;
};

//! �����L�[���[�h�Ǘ�
class CSearchKeywordManager{
public:
	CSearchKeywordManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//@@@ 2002.2.2 YAZAKI
	void		AddToSearchKeyArr( const wchar_t* pszSearchKey );	//	m_aSearchKeys��pszSearchKey��ǉ�����
	void		AddToReplaceKeyArr( const wchar_t* pszReplaceKey );	//	m_aReplaceKeys��pszReplaceKey��ǉ�����
	void		AddToGrepFileArr( const TCHAR* pszGrepFile );		//	m_aGrepFiles��pszGrepFile��ǉ�����
	void		AddToGrepFolderArr( const TCHAR* pszGrepFolder );	//	m_aGrepFolders.size()��pszGrepFolder��ǉ�����
private:
	DLLSHAREDATA* m_pShareData;
};
