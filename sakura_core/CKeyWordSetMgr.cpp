/*!	@file
	@brief �����L�[���[�h�Ǘ�

	@author Norio Nakatani
	
	@date 2000.12.01 MIK binary search
	@date 2004.07.29-2005.01.27 Moca �L�[���[�h�̉ϒ��L��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, genta

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
#include "CKeyWordSetMgr.h"
//#include <stdlib.h>
//#include <malloc.h>
#include <limits>
//#include "charset/charcode.h"

//! 1�u���b�N������̃L�[���[�h��
static const int nKeyWordSetBlockSize = 50;

//! �u���b�N�T�C�Y�Ő���
inline int GetAlignmentSize( int nSize )
{
	return (nKeyWordSetBlockSize - 1 + nSize) / nKeyWordSetBlockSize * nKeyWordSetBlockSize;
}

/*!
	@note CKeyWordSetMgr�͋��L�������\���̂ɖ��ߍ��܂�Ă��邽�߁C
	���̂܂܂ł̓R���X�g���N�^�������Ȃ����Ƃɒ��ӁD
*/
CKeyWordSetMgr::CKeyWordSetMgr( void )
{
	m_nCurrentKeyWordSetIdx = 0;
	m_nKeyWordSetNum = 0;
	m_nStartIdx[0] = 0;
	m_nStartIdx[1] = 0;
	m_nStartIdx[MAX_SETNUM] = 0;
	return;
}

CKeyWordSetMgr::~CKeyWordSetMgr( void)
{
	m_nKeyWordSetNum = 0;
	return;
}

/*!
	@brief �S�L�[���[�h�Z�b�g�̍폜�Ə�����

	�L�[���[�h�Z�b�g�̃C���f�b�N�X��S��0�Ƃ���D
	
	@date 2004.07.29 Moca �ϒ��L��
*/
void CKeyWordSetMgr::ResetAllKeyWordSet( void )
{
	m_nKeyWordSetNum = 0;
	int i;
	for( i = 0; i < MAX_SETNUM+1; i++ ){
		m_nStartIdx[i] = 0;
	}
	for( i = 0; i < MAX_SETNUM; i++ ){
		m_nKeyWordNumArr[i] = 0;
	}
}

const CKeyWordSetMgr& CKeyWordSetMgr::operator=( CKeyWordSetMgr& cKeyWordSetMgr )
{
//	int		nDataLen;
//	char*	pData;
//	int		i;
	if( this == &cKeyWordSetMgr ){
		return *this;
	}
	m_nCurrentKeyWordSetIdx = cKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	m_nKeyWordSetNum = cKeyWordSetMgr.m_nKeyWordSetNum;
	//�z��܂邲�ƃR�s�[
	memcpy_raw( m_szSetNameArr   , cKeyWordSetMgr.m_szSetNameArr   , sizeof( m_szSetNameArr )    );
	memcpy_raw( m_bKEYWORDCASEArr, cKeyWordSetMgr.m_bKEYWORDCASEArr, sizeof( m_bKEYWORDCASEArr ) );
	memcpy_raw( m_nStartIdx      , cKeyWordSetMgr.m_nStartIdx      , sizeof( m_nStartIdx )       );// 2004.07.29 Moca
	memcpy_raw( m_nKeyWordNumArr , cKeyWordSetMgr.m_nKeyWordNumArr , sizeof( m_nKeyWordNumArr )  );
	memcpy_raw( m_szKeyWordArr   , cKeyWordSetMgr.m_szKeyWordArr   , sizeof( m_szKeyWordArr )    );
	memcpy_raw( m_IsSorted       , cKeyWordSetMgr.m_IsSorted       , sizeof( m_IsSorted )        ); //MIK 2000.12.01 binary search
	return *this;
}




/*! @brief �L�[���[�h�Z�b�g�̒ǉ�

	@date 2005.01.26 Moca �V�K�쐬
	@date 2005.01.29 genta �T�C�Y0�ō쐬��realloc����悤��
*/
bool CKeyWordSetMgr::AddKeyWordSet(
	const wchar_t*	pszSetName,		//!< [in] �Z�b�g��
	bool			bKEYWORDCASE,	//!< [in] �啶���������̋�ʁDtrue:����, false:����
	int				nSize			//!< [in] �ŏ��ɗ̈���m�ۂ���T�C�Y�D
)
{
	if( nSize < 0 ) nSize = nKeyWordSetBlockSize;
	if( MAX_SETNUM <= m_nKeyWordSetNum ){
		return false;
	}
	int nIdx = m_nKeyWordSetNum;	//�ǉ��ʒu
	m_nStartIdx[ ++m_nKeyWordSetNum ] = m_nStartIdx[ nIdx ];// �T�C�Y0�ŃZ�b�g�ǉ�

	if( !KeyWordReAlloc( nIdx, nSize ) ){
		--m_nKeyWordSetNum;	//	�L�[���[�h�Z�b�g�̒ǉ����L�����Z������
		return false;
	}
	wcscpy( m_szSetNameArr[nIdx], pszSetName );
	m_bKEYWORDCASEArr[nIdx] = bKEYWORDCASE;
	m_nKeyWordNumArr[nIdx] = 0;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return true;
}

/* ���Ԗڂ̃Z�b�g���폜 */
bool CKeyWordSetMgr::DelKeyWordSet( int nIdx )
{
	int		i;
	if( m_nKeyWordSetNum <= nIdx ||
		0 > nIdx
	){
		return false;
	}
	// �L�[���[�h�̈���J��
	KeyWordReAlloc( nIdx, 0 );
	
	for( i = nIdx; i < m_nKeyWordSetNum - 1; ++i ){
		//�z��܂邲�ƃR�s�[
		memcpy_raw( m_szSetNameArr[i], m_szSetNameArr[i + 1], sizeof( m_szSetNameArr[0] ) );
		m_bKEYWORDCASEArr[i] = m_bKEYWORDCASEArr[i + 1];
		m_nKeyWordNumArr[i] = m_nKeyWordNumArr[i + 1];
		m_nStartIdx[i] = m_nStartIdx[i + 1];	//	2004.07.29 Moca �ϒ��L��
		m_IsSorted[i] = m_IsSorted[i+1];	//MIK 2000.12.01 binary search
	}
	m_nStartIdx[m_nKeyWordSetNum - 1] = m_nStartIdx[m_nKeyWordSetNum];	// 2007.07.14 ryoji ���ꂪ�����Ɩ������ŏI�Z�b�g�̐擪�ɂȂ��Ă��܂�
	m_nKeyWordSetNum--;
	if( m_nKeyWordSetNum <= m_nCurrentKeyWordSetIdx ){
		m_nCurrentKeyWordSetIdx = m_nKeyWordSetNum - 1;
//�Z�b�g�������Ȃ����Ƃ��Am_nCurrentKeyWordSetIdx���킴��-1�ɂ��邽�߁A�R�����g��
//		if( 0 > m_nCurrentKeyWordSetIdx ){
//			m_nCurrentKeyWordSetIdx = 0;
//		}
	}
	return true;
}



/*! ���Ԗڂ̃Z�b�g�̃Z�b�g����Ԃ�

	@param nIdx [in] �Z�b�g�ԍ� 0�`�L�[���[�h�Z�b�g��-1
*/
const wchar_t* CKeyWordSetMgr::GetTypeName( int nIdx )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	return m_szSetNameArr[nIdx];
}

/*! ���Ԗڂ̃Z�b�g�̃Z�b�g�����Đݒ�

	@date 2005.01.26 Moca �V�K�쐬
*/
const wchar_t* CKeyWordSetMgr::SetTypeName( int nIdx, const wchar_t* name )
{
	if( NULL == name || nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	wcsncpy( m_szSetNameArr[nIdx], name, MAX_SETNAMELEN );
	m_szSetNameArr[nIdx][MAX_SETNAMELEN] = L'\0';
	return m_szSetNameArr[nIdx];
}

/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
int CKeyWordSetMgr::GetKeyWordNum( int nIdx )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return 0;
	}
	return m_nKeyWordNumArr[nIdx];
}

/*! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nIdx2 [in] �L�[���[�h�ԍ�
*/
const wchar_t* CKeyWordSetMgr::GetKeyWord( int nIdx, int nIdx2 )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	return m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2];
}

//! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW
const wchar_t* CKeyWordSetMgr::UpdateKeyWord(
	int				nIdx,		//!< [in] �L�[���[�h�Z�b�g�ԍ�
	int				nIdx2,		//!< [in] �L�[���[�h�ԍ�
	const WCHAR*	pszKeyWord	//!< [in] �ݒ肷��L�[���[�h
)
{
	int i;
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	/* 0�o�C�g�̒����̃L�[���[�h�͕ҏW���Ȃ� */
	if( pszKeyWord[0] == L'\0' ){
		return NULL;
	}
	/* �d�������L�[���[�h�͕ҏW���Ȃ� */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == wcscmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return NULL;
		}
	}
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return wcscpy( m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2], pszKeyWord );
}


/*! ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�

	@param nIdx [in] �Z�b�g�ԍ�
	@param pszKeyWord [in] �L�[���[�h������
	
	@return 0: ����, 1: �Z�b�g�ԍ��G���[�C2: �������m�ۃG���[
		3: �L�[���[�h�s���C4: �L�[���[�h�d��

*/
int CKeyWordSetMgr::AddKeyWord( int nIdx, const wchar_t* pszKeyWord )
{
	int  i;
	if( m_nKeyWordSetNum <= nIdx ){
		return 1;
	}
// 2004.07.29 Moca
	if( !KeyWordReAlloc( nIdx, m_nKeyWordNumArr[nIdx] + 1 ) ){
		return 2;
	}
//	if( MAX_KEYWORDNUM <= m_nKeyWordNumArr[nIdx] ){
//		return FALSE;
//	}

	/* 0�o�C�g�̒����̃L�[���[�h�͓o�^���Ȃ� */
	if( pszKeyWord[0] == L'\0' ){
		return 3;
	}
	/* �d�������L�[���[�h�͓o�^���Ȃ� */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == wcscmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return 4;
		}
	}
	/* MAX_KEYWORDLEN��蒷���L�[���[�h�͐؂�̂Ă� */
	if( MAX_KEYWORDLEN < wcslen( pszKeyWord ) ){
		wmemcpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord, MAX_KEYWORDLEN );
		m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]][MAX_KEYWORDLEN] = L'\0';
	}else{
		wcscpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord );
	}
	m_nKeyWordNumArr[nIdx]++;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return 0;
}


/*! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nIdx2 [in] �L�[���[�h�ԍ�
*/
int CKeyWordSetMgr::DelKeyWord( int nIdx, int nIdx2 )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return 1;
	}
	if( nIdx2 < 0 ||  m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return 2;
	}
	if( 0 >= m_nKeyWordNumArr[nIdx]	){
		return 3;	//	�o�^����0�Ȃ��̏����ň���������̂ł����ɂ͗��Ȃ��H
	}
	int  i;
	int  endPos = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx] - 1;
	for( i = m_nStartIdx[nIdx] + nIdx2; i < endPos; ++i ){
		wcscpy( m_szKeyWordArr[i], m_szKeyWordArr[i + 1] );
	}
	m_nKeyWordNumArr[nIdx]--;

	// 2005.01.26 Moca 1���炷�����Ȃ̂ŁA�\�[�g�̏�Ԃ͕ێ������
	// m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	KeyWordReAlloc( nIdx, m_nKeyWordNumArr[nIdx] );	// 2004.07.29 Moca
	return 0;
}


//MIK START 2000.12.01 binary search
/*!	�L�[���[�h�̃\�[�g�ƃL�[���[�h���̍ő�l�v�Z

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�

*/
typedef int (__cdecl *qsort_callback)(const void *, const void *);
void CKeyWordSetMgr::SortKeyWord( int nIdx )
{
	//nIdx�̃Z�b�g���\�[�g����B
	if( m_bKEYWORDCASEArr[nIdx] ) {
		qsort(
			m_szKeyWordArr[m_nStartIdx[nIdx]],
			m_nKeyWordNumArr[nIdx],
			sizeof(m_szKeyWordArr[0]),
			(qsort_callback)wcscmp
		);
	}
	else {
		qsort(
			m_szKeyWordArr[m_nStartIdx[nIdx]],
			m_nKeyWordNumArr[nIdx],
			sizeof(m_szKeyWordArr[0]),
			(qsort_callback)wcsicmp
		);
	}

	{
		int i;
		int len;
		m_nKeyWordMaxLenArr[nIdx] = 0;
		for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; i++ ){
			len = wcslen( m_szKeyWordArr[i] );
			if( m_nKeyWordMaxLenArr[nIdx] < len ){
				m_nKeyWordMaxLenArr[nIdx] = len;
			}
		}
	}
	m_IsSorted[nIdx] = 1;
	return;
}

/** nIdx�Ԗڂ̃L�[���[�h�Z�b�g���� pszKeyWord��T���B
	������� 0�ȏ���A������Ȃ���Ε��̐���Ԃ��B
	@retval 0�ȏ� ���������B
	@retval -1     ������Ȃ������B
	@retval -2     ������Ȃ��������ApszKeyword����n�܂�L�[���[�h�����݂��Ă���B
	@retval intmax �����������ApszKeyword����n�܂�A��蒷���L�[���[�h�����݂��Ă���B
*/
int CKeyWordSetMgr::SearchKeyWord2( int nIdx, const wchar_t* pszKeyWord, int nKeyWordLen )
{
	//sort
	if( m_IsSorted[nIdx] == 0 ) {
		SortKeyWord( nIdx );
	}

	if( m_nKeyWordMaxLenArr[nIdx] < nKeyWordLen ) {
		return -1; // �����I�[�o�[�B
	}

	int result = -1;
	int pl = m_nStartIdx[nIdx];
	int pr = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx] - 1;
	int pc = (pr + 1 - pl) / 2 + pl;
	int (*const cmp)(const wchar_t*, const wchar_t*, size_t) = m_bKEYWORDCASEArr[nIdx] ? wcsncmp : wcsnicmp;
	while( pl <= pr ) {
		const int ret = cmp( pszKeyWord, m_szKeyWordArr[pc], nKeyWordLen );
		if( 0 < ret ) {
			pl = pc + 1;
		} else if( ret < 0 ) {
			pr = pc - 1;
		} else {
			if( wcslen( m_szKeyWordArr[pc] ) > static_cast<size_t>(nKeyWordLen) ) {
				// �n�܂�͈�v����������������Ȃ��B
				if( 0 <= result ) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = -2;
				// �҂������v����L�[���[�h��T�����߂ɑ�����B
				pr = pc - 1;
			} else {
				// ��v����L�[���[�h�����������B
				if( result == -2 ) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = pc - m_nStartIdx[nIdx];
				// ��蒷���L�[���[�h��T�����߂ɑ�����B
				pl = pc + 1;
			}
		}
		pc = (pr + 1 - pl) / 2 + pl;
	}
	return result;
}
//MIK END
//MIK START 2000.12.01 START
void CKeyWordSetMgr::SetKeyWordCase( int nIdx, int nCase )
{
	//�啶�����������f�͂P�r�b�g����Ύ����ł���B
	//����int�^(sizeof(int) * �Z�b�g�� = 4 * 100 = 400)����,
	//char�^(sizeof(char) * �Z�b�g�� = 1 * 100 = 100)�ŏ\������
	//�r�b�g���삵�Ă������B
	if(nCase) {
		m_bKEYWORDCASEArr[nIdx] = true;
	} else {
		m_bKEYWORDCASEArr[nIdx] = false;
	}

	m_IsSorted[nIdx] = 0;
	return;
}

bool CKeyWordSetMgr::GetKeyWordCase( int nIdx )
{
	return 	m_bKEYWORDCASEArr[nIdx];
}
//MIK END


// From Here 2004.07.29 Moca �ϒ��L��
/*!	@brief \\0�܂���TAB�ŋ�؂�ꂽ�����񂩂�L�[���[�h��ݒ�

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nSize [in] �L�[���[�h��
	@param pszKeyWordArr [in]�ukey\\tword\\t\\0�v���́ukey\\0word\\0\\0�v�̌`��
	
	@return �o�^�ɐ��������L�[���[�h��
	
	@author Moca
	@date 2004.07.29 Moca CShareData::ShareData_IO_2���̃R�[�h�����Ɉڒz�E�쐬
*/
int CKeyWordSetMgr::SetKeyWordArr(
	int nIdx,
	int nSize,
	const wchar_t* pszKeyWordArr
)
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	const wchar_t* ptr = pszKeyWordArr;
	for( cnt = 0, i = m_nStartIdx[nIdx];
		i < m_nStartIdx[nIdx] + nSize && *ptr != L'\0'; cnt++, i++ ){
		//	May 25, 2003 �L�[���[�h�̋�؂�Ƃ���\0�ȊO��TAB���󂯕t����悤�ɂ���
		const wchar_t* pTop = ptr;	// �L�[���[�h�̐擪�ʒu��ۑ�
		while( *ptr != L'\t' && *ptr != L'\0' )
			++ptr;
		int kwlen = ptr - pTop;
		wmemcpy( m_szKeyWordArr[i], pTop, kwlen );
		m_szKeyWordArr[i][kwlen] = L'\0';
		++ptr;
	}
	m_nKeyWordNumArr[nIdx] = cnt;
	return nSize;
}

/*!
	�L�[���[�h���X�g��ݒ�

	@param [in] nSize 
	@param [in] ppszKeyWordArr 
	@param nIdx 

	@return �o�^�����L�[���[�h���D0�͎��s�D
*/
int CKeyWordSetMgr::SetKeyWordArr(
	int				nIdx,				//!< [in] �L�[���[�h�Z�b�g�ԍ�
	int				nSize,				//!< [in] ppszKeyWordArr�̗v�f��
	const wchar_t*	ppszKeyWordArr[]	//!< [in] �L�[���[�h�̔z��(�d���E�����������A�l���ς݂ł��邱��)
)
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	for( cnt = 0, i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + nSize; cnt++, i++ ){
		wcscpy( m_szKeyWordArr[i], ppszKeyWordArr[cnt] );
	}
	m_nKeyWordNumArr[nIdx] = nSize;
	return nSize;
}

/*!	@brief �L�[���[�h�̐���

	�d����g�p�s�̃L�[���[�h����菜��

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	
	@return �폜�����L�[���[�h��
*/
int CKeyWordSetMgr::CleanKeyWords( int nIdx )
{
	// ��Ƀ\�[�g���Ă����Ȃ��ƁA��ŏ��Ԃ��ς��Ɠs��������
	if( m_IsSorted[nIdx] == 0 )	SortKeyWord( nIdx );

	int nDelCount = 0;	//!< �폜�L�[���[�h��
	int i = 0;
	while( i < GetKeyWordNum( nIdx ) - 1 ){
		const wchar_t* p = GetKeyWord( nIdx, i );
		bool bDelKey = false;	//!< true�Ȃ�폜�Ώ�
		// �d������L�[���[�h��
		const wchar_t* r = GetKeyWord( nIdx, i + 1 );
		unsigned int nKeyWordLen = wcslen( p );
		if( nKeyWordLen == wcslen( r ) ){
			if( m_bKEYWORDCASEArr[nIdx] ){
				if( 0 == auto_memcmp( p, r, nKeyWordLen ) ){
					bDelKey = true;
				}
			}else{
				if( 0 == auto_memicmp( p, r, nKeyWordLen ) ){
					bDelKey = true;
				}
			}
		}
		if( bDelKey ){
			DelKeyWord( nIdx, i );
			nDelCount++;
			//��낪�����̂ŁAi�𑝂₳�Ȃ�
		}else{
			i++;
		}
	}
	return nDelCount;
}

/*!	@brief �L�[���[�h�ǉ��]�n�̖₢���킹

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@return true: ����1�ǉ��\, false: �ǉ��s�\

	@date 2005.01.26 Moca �V�K�쐬
	@date 2005.01.29 genta ���蓖�čς݂̗̈�ɋ󂫂�����Ίg���s�\�ł��ǉ��\
*/
bool CKeyWordSetMgr::CanAddKeyWord( int nIdx )
{
	//	���蓖�čς݂̗̈�̋󂫂��܂����ׂ�
	int nSizeOld = GetAllocSize( nIdx );
	if( m_nKeyWordNumArr[nIdx] < nSizeOld ){
		return true;
	}

	//	���蓖�čςݗ̈悪�����ς��Ȃ�΁C���蓖�ĉ\�̈�̗L�����m�F
	//	�ꉞ���蓖�čŏ��P�ʕ��c���Ă��邱�Ƃ��m�F�D
	if( GetFreeSize() >= nKeyWordSetBlockSize ){
		return true;
	}

	//	����ł����߂�
	return false;
}

#if 0
/*!	�V�����L�[���[�h�Z�b�g�̃L�[���[�h�̈���m�ۂ���
	m_nKeyWordSetNum�́A�Ăяo�������A�Ăяo�������+1����
*/
bool CKeyWordSetMgr::KeyWordAlloc( int nSize )
{
	// assert( m_nKeyWordSetNum < MAX_SETNUM );
	// assert( 0 <= nSize );

	// �u���b�N�̃T�C�Y�Ő���
	int nAllocSize = GetAlignmentSize( nSize );

	if( GetFreeSize() < nAllocSize ){
		// �������s��
		return false;
	}
	m_nStartIdx[m_nKeyWordSetNum + 1] = m_nStartIdx[m_nKeyWordSetNum] + nAllocSize;
	int i;
	for( i = m_nKeyWordSetNum + 1; i < MAX_SETNUM; i++ ){
		m_nStartIdx[i + 1] = m_nStartIdx[i];
	}
	return true;
}
#endif

/*!	�������ς݂̃L�[���[�h�Z�b�g�̃L�[���[�h�̈�̍Ċ��蓖�āA������s��

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nSize [in] �K�v�ȃL�[���[�h�� (0�`)
*/
bool CKeyWordSetMgr::KeyWordReAlloc( int nIdx, int nSize )
{
	// assert( 0 <= nIdx && nIdx < m_nKeyWordSetNum );

	// �u���b�N�̃T�C�Y�Ő���
	int nAllocSize = GetAlignmentSize( nSize );
	int nSizeOld = GetAllocSize( nIdx );

	if( nSize < 0 ){
		return false;
	}
	if( nAllocSize == nSizeOld ){
		// �T�C�Y�ύX�Ȃ�
		return true;
	}

	int nDiffSize = nAllocSize - nSizeOld;
	if( GetFreeSize() < nDiffSize ){
		// �������s��
		return false;
	}
	// ���̃L�[���[�h�Z�b�g�̃L�[���[�h�����ׂĈړ�����
	int i;
	if( nIdx + 1 < m_nKeyWordSetNum ){
		int nKeyWordIdx = m_nStartIdx[nIdx + 1];
		int nKeyWordNum = m_nStartIdx[m_nKeyWordSetNum] - m_nStartIdx[nIdx + 1];
		memmove( m_szKeyWordArr[nKeyWordIdx + nDiffSize],
			m_szKeyWordArr[nKeyWordIdx],
			nKeyWordNum * sizeof( m_szKeyWordArr[0] ) );
	}
	for( i = nIdx + 1; i <= m_nKeyWordSetNum; i++ ){
		m_nStartIdx[ i ] += nDiffSize;
	}
	return true;
}

/*!	@brief ���蓖�čς݃L�[���[�h�� 

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@return �L�[���[�h�Z�b�g�Ɋ��蓖�čς݂̃L�[���[�h��
*/
int CKeyWordSetMgr::GetAllocSize( int nIdx ) const
{
	return m_nStartIdx[nIdx + 1] - m_nStartIdx[nIdx];
}

/*! ���L�󂫃X�y�[�X

	@date 2004.07.29 Moca �V�K�쐬
	
	@return ���L�󂫗̈�(�L�[���[�h��)
 */
int CKeyWordSetMgr::GetFreeSize( void ) const 
{
	return MAX_KEYWORDNUM - m_nStartIdx[m_nKeyWordSetNum];
}
// To Here 2004.07.29 Moca

// �L�[���[�h�Z�b�g������Z�b�g�ԍ����擾�B������Ȃ���� -1
//	Uchi 2010/4/14
int  CKeyWordSetMgr::SearchKeyWordSet( const wchar_t* pszKeyWord )
{
	int		i;
	int 	nIdx = -1;
	for (i = 0; i < m_nKeyWordSetNum; i++) {
		if ( wcscmp(m_szSetNameArr[i], pszKeyWord) == 0) {
			nIdx = i;
			break;
		}
	}
	return nIdx;
}



