/*!	@file
	@brief オープンダイアログ用ファイル拡張子管理

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK

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
#include <string.h>
#include <stdlib.h>
#include "CFileExt.h"
#include "env/CDocTypeManager.h"

CFileExt::CFileExt()
{
    m_puFileExtInfo = NULL;
    m_nCount        = 0;
    m_vstrFilter.resize(1);
    m_vstrFilter[0] = L'\0';

    //	//テキストエディタとして、既定でリストに載ってほしい拡張子
    //	AppendExt( "すべてのファイル", "*" );
    //	AppendExt( "テキストファイル", "txt" );
}

CFileExt::~CFileExt()
{
    if (m_puFileExtInfo)
        free(m_puFileExtInfo);
    m_puFileExtInfo = NULL;
    m_nCount        = 0;
}

bool CFileExt::AppendExt(const WCHAR *pszName, const WCHAR *pszExt)
{
    WCHAR szWork[_countof(m_puFileExtInfo[0].m_szExt) + 10];

    if (!CDocTypeManager::ConvertTypesExtToDlgExt(pszExt, NULL, szWork))
        return false;
    return AppendExtRaw(pszName, szWork);
}

bool CFileExt::AppendExtRaw(const WCHAR *pszName, const WCHAR *pszExt)
{
    FileExtInfoTag *p;

    if (NULL == pszName || pszName[0] == L'\0')
        return false;
    if (NULL == pszExt || pszExt[0] == L'\0')
        return false;

    if (NULL == m_puFileExtInfo)
    {
        p = (FileExtInfoTag *)malloc(sizeof(FileExtInfoTag) * 1);
        if (NULL == p)
            return false;
    }
    else
    {
        p = (FileExtInfoTag *)realloc(m_puFileExtInfo, sizeof(FileExtInfoTag) * (m_nCount + 1));
        if (NULL == p)
            return false;
    }
    m_puFileExtInfo = p;

    wcscpy(m_puFileExtInfo[m_nCount].m_szName, pszName);
    wcscpy(m_puFileExtInfo[m_nCount].m_szExt, pszExt);
    m_nCount++;

    return true;
}

const WCHAR *CFileExt::GetName(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_nCount)
        return NULL;

    return m_puFileExtInfo[nIndex].m_szName;
}

const WCHAR *CFileExt::GetExt(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_nCount)
        return NULL;

    return m_puFileExtInfo[nIndex].m_szExt;
}

const WCHAR *CFileExt::GetExtFilter(void)
{
    int i;
    std::wstring work;

    /* 拡張子フィルタの作成 */
    m_vstrFilter.resize(0);

    for (i = 0; i < m_nCount; i++)
    {
        // "%s (%s)\0%s\0"
        work = m_puFileExtInfo[i].m_szName;
        work.append(L" (");
        work.append(m_puFileExtInfo[i].m_szExt);
        work.append(L")");
        work.append(L"\0", 1);
        work.append(m_puFileExtInfo[i].m_szExt);
        work.append(L"\0", 1);

        int i = (int)m_vstrFilter.size();
        m_vstrFilter.resize(i + work.length());
        wmemcpy(&m_vstrFilter[i], &work[0], work.length());
    }
    if (0 == m_nCount)
    {
        m_vstrFilter.push_back(L'\0');
    }
    m_vstrFilter.push_back(L'\0');

    return &m_vstrFilter[0];
}
