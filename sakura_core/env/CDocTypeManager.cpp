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

#include "StdAfx.h"
#include "CDocTypeManager.h"
#include "_main/CMutex.h"
#include "CFileExt.h"
#include <Shlwapi.h>	// PathMatchSpec

const TCHAR* CDocTypeManager::m_typeExtSeps = _T(" ;,");	// �^�C�v�ʊg���q ��؂蕶��
const TCHAR* CDocTypeManager::m_typeExtWildcards = _T("*?");	// �^�C�v�ʊg���q ���C���h�J�[�h

static CMutex g_cDocTypeMutex( FALSE, GSTR_MUTEX_SAKURA_DOCTYPE );


/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeOfExt �ɓn�������D
	@date 2014.12.06 syat ���C���h�J�[�h�Ή��B�Q�d�g���q�Ή�����߂�
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfPath( const TCHAR* pszFilePath )
{
	int		i;

	// �t�@�C�����𒊏o
	const TCHAR* pszFileName = pszFilePath;
	const TCHAR* pszSep = _tcsrchr(pszFilePath, _T('\\'));
	if (pszSep) {
		pszFileName = pszSep + 1;
	}

	for (i = 0; i < m_pShareData->m_nTypesCount; ++i){
		const STypeConfigMini* mini;
		GetTypeConfigMini(CTypeConfig(i), &mini);
		if (IsFileNameMatch(mini->m_szTypeExts, pszFileName)) {
			return CTypeConfig(i);	//	�ԍ�
		}
	}
	return CTypeConfig(0);
}


/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��.�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
	@date 2012.10.22 Moca �Q�d�g���q, �g���q�Ȃ��ɑΉ�
	@date 2014.12.06 syat GetDocumentTypeOfPath�ɓ���
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfExt( const TCHAR* pszExt )
{
	return GetDocumentTypeOfPath(pszExt);
}

CTypeConfig CDocTypeManager::GetDocumentTypeOfId( int id )
{
	int		i;

	for( i = 0; i < m_pShareData->m_nTypesCount; ++i ){
		const STypeConfigMini* mini;
		GetTypeConfigMini( CTypeConfig(i), &mini );
		if( mini->m_id == id ){
			return CTypeConfig(i);
		}
	}
	return CTypeConfig(-1);	//	�n�Y��
}

bool CDocTypeManager::GetTypeConfig(CTypeConfig cDocumentType, STypeConfig& type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		if( 0 == n ){
			type = m_pShareData->m_TypeBasis;
			return true;
		}else{
			LockGuard<CMutex> guard( g_cDocTypeMutex );
			 if( SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_GET_TYPESETTING, (WPARAM)n, 0 ) ){
				type = m_pShareData->m_sWorkBuffer.m_TypeConfig;
				return true;
			}
		}
	}
	return false;
}

bool CDocTypeManager::SetTypeConfig(CTypeConfig cDocumentType, const STypeConfig& type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		LockGuard<CMutex> guard( g_cDocTypeMutex );
		m_pShareData->m_sWorkBuffer.m_TypeConfig = type;
		if( SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_SET_TYPESETTING, (WPARAM)n, 0 ) ){
			return true;
		}
	}
	return false;
}

bool CDocTypeManager::GetTypeConfigMini(CTypeConfig cDocumentType, const STypeConfigMini** type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		*type = &m_pShareData->m_TypeMini[n];
		return true;
	}
	return false;
}

bool CDocTypeManager::AddTypeConfig(CTypeConfig cDocumentType)
{
	LockGuard<CMutex> guard( g_cDocTypeMutex );
	return FALSE != SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_ADD_TYPESETTING, (WPARAM)cDocumentType.GetIndex(), 0 );
}

bool CDocTypeManager::DelTypeConfig(CTypeConfig cDocumentType)
{
	LockGuard<CMutex> guard( g_cDocTypeMutex );
	return FALSE != SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_DEL_TYPESETTING, (WPARAM)cDocumentType.GetIndex(), 0 );
}

/*!
	�^�C�v�ʊg���q�Ƀt�@�C�������}�b�`���邩
	
	@param pszTypeExts [in] �^�C�v�ʊg���q�i���C���h�J�[�h���܂ށj
	@param pszFileName [in] �t�@�C����
*/
bool CDocTypeManager::IsFileNameMatch(const TCHAR* pszTypeExts, const TCHAR* pszFileName)
{
	TCHAR szWork[MAX_TYPES_EXTS];

	_tcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	TCHAR* token = _tcstok(szWork, m_typeExtSeps);
	while (token) {
		if (_tcspbrk(token, m_typeExtWildcards) == NULL) {
			if (_tcsicmp(token, pszFileName) == 0) {
				return true;
			}
			const TCHAR* pszExt = _tcsrchr(pszFileName, _T('.'));
			if (pszExt != NULL && _tcsicmp(token, pszExt + 1) == 0) {
				return true;
			}
		} else {
			if (PathMatchSpec(pszFileName, token) == TRUE) {
				return true;
			}
		}
		token = _tcstok(NULL, m_typeExtSeps);
	}
	return false;
}

/*!
	�^�C�v�ʊg���q�̐擪�g���q���擾����
	
	@param pszTypeExts [in] �^�C�v�ʊg���q�i���C���h�J�[�h���܂ށj
	@param szFirstExt  [out] �擪�g���q
	@param nBuffSize   [in] �擪�g���q�̃o�b�t�@�T�C�Y
*/
void CDocTypeManager::GetFirstExt(const TCHAR* pszTypeExts, TCHAR szFirstExt[], int nBuffSize)
{
	TCHAR szWork[MAX_TYPES_EXTS];

	_tcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	TCHAR* token = _tcstok(szWork, m_typeExtSeps);
	while (token) {
		if (_tcspbrk(token, m_typeExtWildcards) == NULL) {
			_tcsncpy(szFirstExt, token, nBuffSize);
			szFirstExt[nBuffSize - 1] = _T('\0');
			return;
		}
	}
	szFirstExt[0] = _T('\0');
	return;
}

/*! �^�C�v�ʐݒ�̊g���q���X�g���_�C�A���O�p���X�g�ɕϊ�����
	@param pszSrcExt [in]  �g���q���X�g ��u.c .cpp;.h�v
	@param pszDstExt [out] �g���q���X�g ��u*.c;*.cpp;*.h�v
	@param szExt [in] ���X�g�̐擪�ɂ���g���q ��u.h�v

	@date 2014.12.06 syat CFileExt����ړ�
*/
bool CDocTypeManager::ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, const TCHAR* szExt, TCHAR *pszDstExt )
{
	TCHAR	*token;
	TCHAR	*p;

	//	2003.08.14 MIK NULL����Ȃ���false
	if( NULL == pszSrcExt ) return false;
	if( NULL == pszDstExt ) return false;

	p = _tcsdup( pszSrcExt );
	_tcscpy( pszDstExt, _T("") );

	if (szExt != NULL && szExt[0] != _T('\0')) {
		// �t�@�C���p�X������A�g���q����̏ꍇ�A�g�b�v�Ɏw��
		_tcscpy(pszDstExt, _T("*"));
		_tcscat(pszDstExt, szExt);
	}

	token = _tcstok(p, m_typeExtSeps);
	while( token )
	{
		if (szExt == NULL || szExt[0] == _T('\0') || auto_stricmp(token, szExt + 1) != 0) {
			if( pszDstExt[0] != '\0' ) _tcscat( pszDstExt, _T(";") );
			// �g���q�w��Ȃ��A�܂��̓}�b�`�����g���q�łȂ�
			if (_tcspbrk(token, m_typeExtWildcards) == NULL) {
				if (_T('.') == *token) _tcscat(pszDstExt, _T("*"));
				else                 _tcscat(pszDstExt, _T("*."));
			}
			_tcscat(pszDstExt, token);
		}

		token = _tcstok( NULL, m_typeExtSeps );
	}
	free( p );	// 2003.05.20 MIK ����������R��
	return true;
}
