/*!	@file
	@brief �I�[�v���_�C�A���O�p�t�@�C���g���q�Ǘ�

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

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include "CFileExt.h"

CFileExt::CFileExt()
{
//	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	m_pShareData = CShareData::getInstance()->GetShareData();
//	m_Types = m_pShareData->m_Types;

	m_puFileExtInfo = NULL;
	m_nCount = 0;
	_tcscpy( m_szFilter, _T("") );

//	//�e�L�X�g�G�f�B�^�Ƃ��āA����Ń��X�g�ɍڂ��Ăق����g���q
//	AppendExt( "���ׂẴt�@�C��", "*" );
//	AppendExt( "�e�L�X�g�t�@�C��", "txt" );
}

CFileExt::~CFileExt()
{
	if( m_puFileExtInfo ) free( m_puFileExtInfo );
	m_puFileExtInfo = NULL;
	m_nCount = 0;
}

bool CFileExt::AppendExt( const TCHAR *pszName, const TCHAR *pszExt )
{
	TCHAR	szWork[_countof(m_puFileExtInfo[0].m_szExt) + 10];

	if( !ConvertTypesExtToDlgExt( pszExt, szWork ) ) return false;
	return AppendExtRaw( pszName, szWork );
}

bool CFileExt::AppendExtRaw( const TCHAR *pszName, const TCHAR *pszExt )
{
	FileExtInfoTag	*p;

	if( NULL == pszName || 0 == _tcslen( pszName ) ) return false;
	if( NULL == pszExt  || 0 == _tcslen( pszExt  ) ) return false;

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

	_tcscpy( m_puFileExtInfo[m_nCount].m_szName, pszName );
	_tcscpy( m_puFileExtInfo[m_nCount].m_szExt, pszExt );
	m_nCount++;

	return true;
}

const TCHAR *CFileExt::GetName( int nIndex )
{
	if( nIndex < 0 || nIndex >= m_nCount ) return NULL;

	return m_puFileExtInfo[nIndex].m_szName;
}

const TCHAR *CFileExt::GetExt( int nIndex )
{
	if( nIndex < 0 || nIndex >= m_nCount ) return NULL;

	return m_puFileExtInfo[nIndex].m_szExt;
}

const TCHAR *CFileExt::GetExtFilter( void )
{
	int		i;
	TCHAR	szWork[_countof(m_puFileExtInfo[0].m_szName) + _countof(m_puFileExtInfo[0].m_szExt)*2 + 10];

	/* �g���q�t�B���^�̍쐬 */
	_tcscpy( m_szFilter, _T("") );

	for( i = 0; i < m_nCount; i++ )
	{
		auto_sprintf( szWork,
			_T("%ts (%ts)|%ts|"),
			m_puFileExtInfo[i].m_szName,
			m_puFileExtInfo[i].m_szExt,
			m_puFileExtInfo[i].m_szExt );

		_tcscat( m_szFilter, szWork );
	}
	_tcscat( m_szFilter, _T("|") );

	//��؂�͂O�Ȃ̂Œu��������B
	for( i = 0; m_szFilter[i] != _T('\0'); i++ )
	{
		if( m_szFilter[i] == _T('|') ) m_szFilter[i] = _T('\0');
	}

	return m_szFilter;
}

/*! �^�C�v�ʐݒ�̊g���q���X�g���_�C�A���O�p���X�g�ɕϊ�����
	@param pszSrcExt [in]  �g���q���X�g ��u.c .cpp;.h�v
	@param pszDstExt [out] �g���q���X�g ��u*.c;*.cpp;*.h�v
*/
bool CFileExt::ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, TCHAR *pszDstExt )
{
	TCHAR	*token;
	TCHAR	*p;

	//	2003.08.14 MIK NULL����Ȃ���false
	if( NULL == pszSrcExt ) return false;
	if( NULL == pszDstExt ) return false;

	p = _tcsdup( pszSrcExt );
	_tcscpy( pszDstExt, _T("") );

	token = _tcstok( p, _T(" ;,") );
	while( token )
	{
		if( _T('.') == *token ) _tcscat( pszDstExt, _T("*") );
		else                 _tcscat( pszDstExt, _T("*.") );
		_tcscat( pszDstExt, token );

		token = _tcstok( NULL, _T(" ;,") );
		if( token ) _tcscat( pszDstExt, _T(";") );
	}
	free( p );	// 2003.05.20 MIK ����������R��
	return true;
}

