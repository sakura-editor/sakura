/*
	2008.05.18 kobake CShareData ���番��
*/
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CSEARCHKEYWORDMANAGER_6D883D8B_3076_423C_BADA_B4C5DCB3E6E5_H_
#define SAKURA_CSEARCHKEYWORDMANAGER_6D883D8B_3076_423C_BADA_B4C5DCB3E6E5_H_

// �v��s��`
// #include "DLLSHAREDATA.h"


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

#endif /* SAKURA_CSEARCHKEYWORDMANAGER_6D883D8B_3076_423C_BADA_B4C5DCB3E6E5_H_ */
/*[EOF]*/
