//	$Id$
/*!	@file
	@brief オープンダイアログ用ファイル拡張子管理

	@author MIK
	$Revision$
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

#include <string.h>
#include <malloc.h>
#include "CFileExt.h"
#include "my_icmp.h"

CFileExt::CFileExt()
{
//	/* 共有データ構造体のアドレスを返す */
//	m_pShareData = CShareData::getInstance()->GetShareData();
//	m_Types = m_pShareData->m_Types;

	m_puFileExtInfo = NULL;
	m_nCount = 0;
	strcpy( m_szFilter, "" );

//	//テキストエディタとして、既定でリストに載ってほしい拡張子
//	AppendExt( "すべてのファイル", "*" );
//	AppendExt( "テキストファイル", "txt" );
}

CFileExt::~CFileExt()
{
	if( m_puFileExtInfo ) free( m_puFileExtInfo );
	m_puFileExtInfo = NULL;
	m_nCount = 0;
}

bool CFileExt::AppendExt( const char *pszName, const char *pszExt )
{
	char	szWork[1024];

	if( false == ConvertTypesExtToDlgExt( pszExt, szWork ) ) return false;
	return AppendExtRaw( pszName, szWork );
}

bool CFileExt::AppendExtRaw( const char *pszName, const char *pszExt )
{
	FileExtInfoTag	*p;

	if( NULL == pszName || 0 == strlen( pszName ) ) return false;
	if( NULL == pszExt  || 0 == strlen( pszExt  ) ) return false;

	if( NULL == m_puFileExtInfo )
	{
		p = (FileExtInfoTag*)malloc( sizeof( FileExtInfoTag ) * 1 );
		if( NULL == p ) return false;
	}
	else
	{
		p = (FileExtInfoTag*)realloc( m_puFileExtInfo, sizeof( FileExtInfoTag ) * ( m_nCount + 1 ) );
		if( NULL == p ) return false;
	}
	m_puFileExtInfo = p;

	strcpy( m_puFileExtInfo[m_nCount].m_szName, pszName );
	strcpy( m_puFileExtInfo[m_nCount].m_szExt, pszExt );
	m_nCount++;

	return true;
}

const char *CFileExt::GetName( int nIndex )
{
	if( nIndex < 0 || nIndex >= m_nCount ) return NULL;

	return m_puFileExtInfo[nIndex].m_szName;
}

const char *CFileExt::GetExt( int nIndex )
{
	if( nIndex < 0 || nIndex >= m_nCount ) return NULL;

	return m_puFileExtInfo[nIndex].m_szExt;
}

const char *CFileExt::GetExtFilter( void )
{
	int		i;
	char	szWork[1024];

	/* 拡張子フィルタの作成 */
	strcpy( m_szFilter, "" );

	for( i = 0; i < m_nCount; i++ )
	{
		wsprintf( szWork,
			"%s (%s)|%s|",
			m_puFileExtInfo[i].m_szName,
			m_puFileExtInfo[i].m_szExt,
			m_puFileExtInfo[i].m_szExt );

		strcat( m_szFilter, szWork );
	}
	strcat( m_szFilter, "|" );
	i = strlen( m_szFilter );

	//区切りは０なので置き換える。
	for( i = 0; m_szFilter[i] != '\0'; i++ )
	{
		if( m_szFilter[i] == '|' ) m_szFilter[i] = '\0';
	}

	return m_szFilter;
}

bool CFileExt::ConvertTypesExtToDlgExt( const char *pszSrcExt, char *pszDstExt )
{
	char	*token;
	char	*p;

	if( NULL == pszSrcExt ) return NULL;
	if( NULL == pszDstExt ) return NULL;

	p = strdup( pszSrcExt );
	strcpy( pszDstExt, "" );

	token = strtok( p, " ;," );
	while( token )
	{
		if( '.' == *token ) strcat( pszDstExt, "*" );
		else                strcat( pszDstExt, "*." );
		strcat( pszDstExt, token );

		token = strtok( NULL, " ;," );
		if( token ) strcat( pszDstExt, ";" );
	}
	free( p );	// 2003.05.20 MIK メモリ解放漏れ
	return true;
}

