//	$Id$
/*!	@file
	@brief �����L�[���[�h�Ǘ�

	@author Norio Nakatani
	$Revision$
	
	@date 2000.12.01 MIK binary search
	@date 2004.07.29-2005.01.27 Moca �L�[���[�h�̉ϒ��L��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK
	Copyright (C) 2004 Moca

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
#include "CKeyWordSetMgr.h"
#include <stdlib.h>
#include <malloc.h>
#include "my_icmp.h" // Nov. 29, 2002 genta/moca

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
		m_nKeyWordNumArr[MAX_SETNUM] = 0;
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
	memcpy( m_szSetNameArr, cKeyWordSetMgr.m_szSetNameArr, sizeof( m_szSetNameArr ) );
	memcpy( m_nKEYWORDCASEArr, cKeyWordSetMgr.m_nKEYWORDCASEArr, sizeof( m_nKEYWORDCASEArr ) );
	memcpy( m_nStartIdx, cKeyWordSetMgr.m_nStartIdx, sizeof( m_nStartIdx ) ); // 2004.07.29 Moca
	memcpy( m_nKeyWordNumArr, cKeyWordSetMgr.m_nKeyWordNumArr, sizeof( m_nKeyWordNumArr ) );
	memcpy( m_szKeyWordArr, cKeyWordSetMgr.m_szKeyWordArr, sizeof( m_szKeyWordArr ) );
	memcpy( m_IsSorted, cKeyWordSetMgr.m_IsSorted, sizeof( m_IsSorted ) );	//MIK 2000.12.01 binary search
	return *this;
}




/*! @brief �L�[���[�h�Z�b�g�̒ǉ�

	@param pszSetName	[in] �Z�b�g��
	@param nKEYWORDCASE	[in] �啶���������̋�ʁDtrue:����, false:����
	@param nSize		[in] �ŏ��ɗ̈���m�ۂ���T�C�Y�D

	@date 2005.01.26 Moca �V�K�쐬
	@date 2005.01.29 genta �T�C�Y0�ō쐬��realloc����悤��
*/
BOOL CKeyWordSetMgr::AddKeyWordSet( const char* pszSetName, BOOL nKEYWORDCASE, int nSize )
{
	if( nSize < 0 ) nSize = nKeyWordSetBlockSize;
	if( MAX_SETNUM <= m_nKeyWordSetNum ){
		return FALSE;
	}
	int nIdx = m_nKeyWordSetNum;	//�ǉ��ʒu
	m_nStartIdx[ ++m_nKeyWordSetNum ] = m_nStartIdx[ nIdx ];// �T�C�Y0�ŃZ�b�g�ǉ�

	if( !KeyWordReAlloc( nIdx, nSize ) ){
		--m_nKeyWordSetNum;	//	�L�[���[�h�Z�b�g�̒ǉ����L�����Z������
		return FALSE;
	}
	strcpy( m_szSetNameArr[nIdx], pszSetName );
	m_nKEYWORDCASEArr[nIdx] = nKEYWORDCASE;
	m_nKeyWordNumArr[nIdx] = 0;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return TRUE;
}

/* ���Ԗڂ̃Z�b�g���폜 */
BOOL CKeyWordSetMgr::DelKeyWordSet( int nIdx )
{
	int		i;
	if( m_nKeyWordSetNum <= nIdx ||
		0 > nIdx
	){
		return FALSE;
	}
	// �L�[���[�h�̈���J��
	KeyWordReAlloc( nIdx, 0 );
	
	for( i = nIdx; i < m_nKeyWordSetNum - 1; ++i ){
		memcpy( m_szSetNameArr[i], m_szSetNameArr[i + 1], sizeof( m_szSetNameArr[0] ) );
		m_nKEYWORDCASEArr[i] = m_nKEYWORDCASEArr[i + 1];
		m_nKeyWordNumArr[i] = m_nKeyWordNumArr[i + 1];
		m_nStartIdx[i] = m_nStartIdx[i + 1];	//	2004.07.29 Moca �ϒ��L��
		m_IsSorted[i] = m_IsSorted[i+1];	//MIK 2000.12.01 binary search
	}
	m_nKeyWordSetNum--;
	if( m_nKeyWordSetNum <= m_nCurrentKeyWordSetIdx ){
		m_nCurrentKeyWordSetIdx = m_nKeyWordSetNum - 1;
//�Z�b�g�������Ȃ����Ƃ��Am_nCurrentKeyWordSetIdx���킴��-1�ɂ��邽�߁A�R�����g��
//		if( 0 > m_nCurrentKeyWordSetIdx ){
//			m_nCurrentKeyWordSetIdx = 0;
//		}
	}
	return TRUE;
}



/*! ���Ԗڂ̃Z�b�g�̃Z�b�g����Ԃ�

	@param nIdx [in] �Z�b�g�ԍ� 0�`�L�[���[�h�Z�b�g��-1
*/
const char* CKeyWordSetMgr::GetTypeName( int nIdx )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	return m_szSetNameArr[nIdx];
}

/*! ���Ԗڂ̃Z�b�g�̃Z�b�g�����Đݒ�

	@date 2005.01.26 Moca �V�K�쐬
*/
const char* CKeyWordSetMgr::SetTypeName( int nIdx, const char* name )
{
	if( NULL == name || nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	strncpy( m_szSetNameArr[nIdx], name, MAX_SETNAMELEN );
	m_szSetNameArr[nIdx][MAX_SETNAMELEN] = '\0';
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
const char* CKeyWordSetMgr::GetKeyWord( int nIdx, int nIdx2 )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	return m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2];
}

/*! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nIdx2 [in] �L�[���[�h�ԍ�
	@param pszKeyWord [in] �ݒ肷��L�[���[�h
*/
const char* CKeyWordSetMgr::UpdateKeyWord( int nIdx, int nIdx2, const char* pszKeyWord )
{
	int i;
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	/* 0�o�C�g�̒����̃L�[���[�h�͕ҏW���Ȃ� */
	if( pszKeyWord[0] == '\0' ){
		return NULL;
	}
	/* �d�������L�[���[�h�͕ҏW���Ȃ� */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == strcmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return NULL;
		}
	}
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return strcpy( m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2], pszKeyWord );
}


/*! ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�

	@param nIdx [in] �Z�b�g�ԍ�
	@param pszKeyWord [in] �L�[���[�h������
	
	@return 0: ����, 1: �Z�b�g�ԍ��G���[�C2: �������m�ۃG���[
		3: �L�[���[�h�s���C4: �L�[���[�h�d��

*/
int CKeyWordSetMgr::AddKeyWord( int nIdx, const char* pszKeyWord )
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
	if( pszKeyWord[0] == '\0' ){
		return 3;
	}
	/* �d�������L�[���[�h�͓o�^���Ȃ� */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == strcmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return 4;
		}
	}
	/* MAX_KEYWORDLEN��蒷���L�[���[�h�͐؂�̂Ă� */
	if( MAX_KEYWORDLEN < strlen( pszKeyWord ) ){
		memcpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord, MAX_KEYWORDLEN );
		m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]][MAX_KEYWORDLEN] = '\0';
	}else{
		strcpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord );
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
		strcpy( m_szKeyWordArr[i], m_szKeyWordArr[i + 1] );
	}
	m_nKeyWordNumArr[nIdx]--;

	// 2005.01.26 Moca 1���炷�����Ȃ̂ŁA�\�[�g�̏�Ԃ͕ێ������
	// m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	KeyWordReAlloc( nIdx, m_nKeyWordNumArr[nIdx] );	// 2004.07.29 Moca
	return 0;
}

#if 0
/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
int CKeyWordSetMgr::SearchKeyWord( int nIdx, const char* pszKeyWord, int nKeyWordLen )
{
	int i;
	if( m_nKEYWORDCASEArr[nIdx] ){	/* �L�[���[�h�̉p�啶����������� */
		for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
			if( (int)strlen( m_szKeyWordArr[i] ) == nKeyWordLen ){
				if( 0 == memcmp( m_szKeyWordArr[i], pszKeyWord, nKeyWordLen ) ){
					return i;
				}
			}
		}
	}else{
		for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
			if( (int)strlen( m_szKeyWordArr[i] ) == nKeyWordLen ){
				if( 0 == memicmp( m_szKeyWordArr[i], pszKeyWord, nKeyWordLen ) ){
					return i;
				}
			}
		}
	}
	return -1;

}
#endif

#if 0
	// 2005.01.29 genta �g���Ă��Ȃ��悤��
/* �ύX�󋵂𒲍� */
BOOL CKeyWordSetMgr::IsModify( CKeyWordSetMgr& cKeyWordSetMgrNew, BOOL* pnModifyFlagArr )
{
	BOOL	bModifyAll;
	int		i;


	for( i = 0; i < MAX_SETNUM; ++i ){
		pnModifyFlagArr[i] = FALSE;
	}
	if( this == &cKeyWordSetMgrNew ){
		return FALSE;
	}

	bModifyAll = FALSE;
	if( m_nCurrentKeyWordSetIdx != cKeyWordSetMgrNew.m_nCurrentKeyWordSetIdx
	 || m_nKeyWordSetNum != cKeyWordSetMgrNew.m_nKeyWordSetNum
	 || 0 != memcmp( &m_szSetNameArr, &cKeyWordSetMgrNew.m_szSetNameArr, sizeof( m_szSetNameArr ) )
	 || 0 != memcmp( &m_nKEYWORDCASEArr, &cKeyWordSetMgrNew.m_nKEYWORDCASEArr, sizeof( m_nKEYWORDCASEArr ) )
	 || 0 != memcmp( &m_nStartIdx, &cKeyWordSetMgrNew.m_nStartIdx, sizeof( m_nStartIdx ) ) // 2004.07.29 Moca
	 || 0 != memcmp( &m_nKeyWordNumArr, &cKeyWordSetMgrNew.m_nKeyWordNumArr, sizeof( m_nKeyWordNumArr ) )
	 || 0 != memcmp( &m_szKeyWordArr, &cKeyWordSetMgrNew.m_szKeyWordArr, sizeof( m_szKeyWordArr ) )
	 ){
		bModifyAll = TRUE;
		for( i = 0; i < MAX_SETNUM; ++i ){
			if( 0 != memcmp( &m_szSetNameArr[i], &cKeyWordSetMgrNew.m_szSetNameArr[i], sizeof( m_szSetNameArr[i] ) )
			 || m_nKEYWORDCASEArr[i] != cKeyWordSetMgrNew.m_nKEYWORDCASEArr[i]
			 || m_nStartIdx[i] != cKeyWordSetMgrNew.m_nStartIdx[i] // 2004.07.29 Moca
			 || m_nKeyWordNumArr[i] != cKeyWordSetMgrNew.m_nKeyWordNumArr[i]
			 || 0 != memcmp( &m_szKeyWordArr[i], &cKeyWordSetMgrNew.m_szKeyWordArr[i], sizeof( m_szKeyWordArr[i] ) )
			){
				pnModifyFlagArr[i] = TRUE;
			}
		}
	}
	return bModifyAll;

}
#endif

//MIK START 2000.12.01 binary search
/*!	�L�[���[�h�̃\�[�g�ƃL�[���[�h���̍ő�l�v�Z

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�

*/
void CKeyWordSetMgr::SortKeyWord( int nIdx )
{
	//nIdx�̃Z�b�g���\�[�g����B
	if( m_nKEYWORDCASEArr[nIdx] ) {
		qsort( m_szKeyWordArr[m_nStartIdx[nIdx]],
				m_nKeyWordNumArr[nIdx],
				sizeof(m_szKeyWordArr[0]),
				(int (__cdecl *)(const void *, const void *))strcmp
			);
	} else {
		qsort( m_szKeyWordArr[m_nStartIdx[nIdx]],
				m_nKeyWordNumArr[nIdx],
				sizeof(m_szKeyWordArr[0]),
				(int (__cdecl *)(const void *, const void *))my_stricmp	//Dec. 24, 2002 Moca
			);
	}
	{
		int i;
		int len;
		m_nKeyWordMaxLenArr[nIdx] = 0;
		for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; i++ ){
			len = strlen( m_szKeyWordArr[i] );
			if( m_nKeyWordMaxLenArr[nIdx] < len ){
				m_nKeyWordMaxLenArr[nIdx] = len;
			}
		}
	}
	m_IsSorted[nIdx] = 1;
	return;
}
/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���o�C�i���T�[�` �����Ƃ���-1��Ԃ� */
int CKeyWordSetMgr::SearchKeyWord2( int nIdx, const char* pszKeyWord, int nKeyWordLen )
{
	int pc, pr, pl, ret, wcase;

	//sort
	if(m_IsSorted[nIdx] == 0) SortKeyWord(nIdx);

	pl = m_nStartIdx[nIdx];
	pr = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx] - 1;
//	if( pr < 0 ) return -1;
	if( nKeyWordLen > m_nKeyWordMaxLenArr[nIdx] ) return -1;
	pc = (pr + 1 - pl) / 2 + pl;
	wcase = m_nKEYWORDCASEArr[nIdx];
	while(pl <= pr) {
		if( wcase ) {
			ret = strncmp( pszKeyWord, m_szKeyWordArr[pc], nKeyWordLen );
		} else {
			ret = strnicmp( pszKeyWord, m_szKeyWordArr[pc], nKeyWordLen );
		}
		if( ret == 0 ) {
			if( (int)strlen( m_szKeyWordArr[pc] ) > nKeyWordLen ) {
				ret = -1;
			} else {
				return pc - m_nStartIdx[nIdx];
			}
		}

		if( ret < 0 ) {
			pr = pc - 1;
		} else {
			pl = pc + 1;
		}

		pc = (pr + 1 - pl) / 2 + pl;
	}
	return -1;
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
		m_nKEYWORDCASEArr[nIdx] = TRUE;
	} else {
		m_nKEYWORDCASEArr[nIdx] = FALSE;
	}

	m_IsSorted[nIdx] = 0;
	return;
}

int CKeyWordSetMgr::GetKeyWordCase( int nIdx )
{
	return 	m_nKEYWORDCASEArr[nIdx];
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
int CKeyWordSetMgr::SetKeyWordArr( int nIdx, int nSize, const char* pszKeyWordArr )
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	const char* ptr = pszKeyWordArr;
	for( cnt = 0, i = m_nStartIdx[nIdx];
		i < m_nStartIdx[nIdx] + nSize && *ptr != '\0'; cnt++, i++ ){
		//	May 25, 2003 �L�[���[�h�̋�؂�Ƃ���\0�ȊO��TAB���󂯕t����悤�ɂ���
		const char* pTop = ptr;	// �L�[���[�h�̐擪�ʒu��ۑ�
		while( *ptr != '\t' && *ptr != '\0' )
			++ptr;
		int kwlen = ptr - pTop;
		memcpy( m_szKeyWordArr[i], pTop, kwlen );
		m_szKeyWordArr[i][kwlen] = '\0';
		++ptr;
	}
	m_nKeyWordNumArr[nIdx] = cnt;
	return nSize;
}

/*!
	�L�[���[�h���X�g��ݒ�

	@param [in] nSize ppszKeyWordArr�̗v�f��
	@param [in] ppszKeyWordArr �L�[���[�h�̔z��(�d���E�����������A�l���ς݂ł��邱��)
	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�

	@return �o�^�����L�[���[�h���D0�͎��s�D
*/
int CKeyWordSetMgr::SetKeyWordArr( int nIdx, int nSize, const char * const *ppszKeyWordArr )
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	for( cnt = 0, i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + nSize; cnt++, i++ ){
		strcpy( m_szKeyWordArr[i], ppszKeyWordArr[cnt] );
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
		const char* p = GetKeyWord( nIdx, i );
		bool bDelKey = false;	//!< true�Ȃ�폜�Ώ�
		// �\���ł��Ȃ��L�[���[�h��
		int k;
		for( k = 0;p[k] != '\0'; k++ ){
			if( IS_KEYWORD_CHAR( p[k] ) ){
			}else{
				bDelKey = true;
				break;
			}
		}
		if( !bDelKey ){
			// �d������L�[���[�h��
			const char* r = GetKeyWord( nIdx, i + 1 );
			unsigned int nKeyWordLen = strlen( p );
			if( nKeyWordLen == strlen( r ) ){
				if( m_nKEYWORDCASEArr[nIdx] ){
					if( 0 == memcmp( p, r, nKeyWordLen ) ){
						bDelKey = true;
					}
				}else{
					if( 0 == memicmp( p, r, nKeyWordLen ) ){
						bDelKey = true;
					}
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


/*[EOF]*/
