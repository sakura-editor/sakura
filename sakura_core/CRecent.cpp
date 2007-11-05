/*!	@file
	@brief �ŋߎg�������X�g

	���C�ɓ�����܂ލŋߎg�������X�g���Ǘ�����B

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005

*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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
#include "global.h"
#include "CShareData.h"
#include "CRecent.h"
#include <string.h>
#include "my_icmp.h"

CRecent::CRecent()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_bCreate = false;

	m_nArrayCount  = 0;
	m_nItemSize    = 0;
	m_nOffset      = 0;
	m_nCmpSize     = 0;
	m_nCmpType     = 0;
	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;
}

CRecent::~CRecent()
{
	Terminate();
}

/*
	�������ς݂����ׂ�B
*/
bool CRecent::IsAvailable( void )
{
	if( ! m_bCreate ) return false;

	if( NULL == m_puUserItemData
	 || NULL == m_pnUserItemCount )
	{
		Terminate();
		return false;
	}

	//�f�[�^�j�󎞂̃��J�o��������Ă݂��肷��
	if( *m_pnUserItemCount < 0             ) *m_pnUserItemCount = 0;
	if( *m_pnUserItemCount > m_nArrayCount ) *m_pnUserItemCount = m_nArrayCount;

	if( m_pnUserViewCount )
	{
		if( *m_pnUserViewCount < 0             ) *m_pnUserViewCount = 0;
		if( *m_pnUserViewCount > m_nArrayCount ) *m_pnUserViewCount = m_nArrayCount;
	}

	return true;
}

/*
	������������

	@param	pszItemArray	�A�C�e���z��ւ̃|�C���^
	@param	pnItemCount		�A�C�e�����ւ̃|�C���^
	@param	pbItemFavorite	���C�ɓ���ւ̃|�C���^(NULL����)
	@param	nArrayCount		�ő�Ǘ��\�ȃA�C�e����
	@param	*pnViewCount	�\����(NULL����)
	@param	nItemSize		���[�U�f�[�^�̃o�C�g�T�C�Y
	@param	nOffset			��r�ʒu
	@param	nCmpSize		��r�T�C�Y(strn*cmp, mem*cmp �ŕK�{)
	@param	nCmpType		��r�^�C�v

	@note
	nCmpType = strcmp, stricmp �̂Ƃ��� nCmpSize = 0 ���w�肷��ƁAAppendItem 
	�ł̃f�[�^��������ł���ƔF������ strcpy ������B
	���̏ꍇ�� memcpy �� nItemSize �����R�s�[����B
	
	pnViewCount = NULL �ɂ���ƁA�[���I�� nViewCount == nArrayCount �ɂȂ�B
*/
bool CRecent::Create( 
		char	*pszItemArray,	//�A�C�e���z��ւ̃|�C���^
		int		*pnItemCount,	//�A�C�e�����ւ̃|�C���^
		bool	*pbItemFavorite,	//���C�ɓ���ւ̃|�C���^
		int		nArrayCount, 
		int		*pnViewCount, 
		int		nItemSize, 
		int		nOffset, 
		int		nCmpSize, 
		int		nCmpType 
	)
{
	Terminate();

	//�p�����[�^�`�F�b�N
	if( NULL == pszItemArray ) return false;
	if( NULL == pnItemCount ) return false;
	//if( NULL == pnViewCount ) return false;
	//if( NULL == pbItemFavorite ) return false;

	if( nArrayCount <= 0 ) return false;
	if( pnViewCount )
	{
		if( *pnViewCount < 0 || nArrayCount < *pnViewCount ) return false;
	}
	if( nItemSize <= 0 || nItemSize < nOffset ) return false;
	if( nCmpSize < 0 || nItemSize < nOffset + nCmpSize ) return false;

	m_puUserItemData     = pszItemArray;
	m_pnUserItemCount    = pnItemCount;
	m_pnUserViewCount    = pnViewCount;
	m_pbUserItemFavorite = pbItemFavorite;

	m_nArrayCount = nArrayCount;
	m_nItemSize   = nItemSize;
	m_nOffset     = nOffset;
	m_nCmpSize    = nCmpSize;
	m_nCmpType    = nCmpType;

	m_bCreate = true;

	//�ʂɑ��삳��Ă����Ƃ��̂��߂̑Ή�
	UpdateView();

	return true;
}

/*
	�I������
*/
void CRecent::Terminate( void )
{
	m_bCreate = false;

	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;

	m_nArrayCount  = 0;
	m_nItemSize    = 0;
	m_nOffset      = 0;
	m_nCmpSize     = 0;
	m_nCmpType     = 0;

	return;
}

/*
	�Ǘ�����Ă���A�C�e���̂����̕\������ύX����B

	@note	���C�ɓ���͉\�Ȍ���\�����Ɉړ�������B
*/
bool CRecent::ChangeViewCount( int nViewCount )
{
	int	i;
	int	nIndex;

	//�͈͊O�Ȃ�G���[
	if( ! IsAvailable() ) return false;
	if( nViewCount < 0 || nViewCount > m_nArrayCount ) return false;

	//�\�������X�V����B
	if( m_pnUserViewCount )
	{
		*m_pnUserViewCount = nViewCount;
	}

	//�͈͓��ɂ��ׂĎ��܂��Ă���̂ŉ������Ȃ��Ă悢�B
	if( nViewCount >= *m_pnUserItemCount ) return true;

	//�ł��Â����C�ɓ����T���B
	i = GetOldestItem( *m_pnUserItemCount - 1, true );
	if( -1 == i ) return true;	//�Ȃ��̂ŉ������Ȃ��ŏI��

	//�\���O�A�C�e����\�����Ɉړ�����B
	for( ; i >= nViewCount; i-- )
	{
		if( IsFavorite( i ) )
		{
			//�J�����g�ʒu�����ɒʏ�A�C�e����T��
			nIndex = GetOldestItem( i - 1, false );
			if( -1 == nIndex ) break;	//����1���Ȃ�

			//���������A�C�e�����J�����g�ʒu�Ɉړ�����
			MoveItem( nIndex, i );
		}
	}

	return true;
}

/*
	���X�g���X�V����B
*/
bool CRecent::UpdateView( void )
{
	int	nViewCount;

	//�͈͊O�Ȃ�G���[
	if( ! IsAvailable() ) return false;

	if( m_pnUserViewCount ) nViewCount = *m_pnUserViewCount;
	else                    nViewCount = m_nArrayCount;

	return ChangeViewCount( nViewCount );
}

/*
	���C�ɓ����Ԃ�ݒ肷��B

	true	�ݒ�
	false	����
*/
bool CRecent::SetFavorite( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	m_pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	���ׂĂ̂��C�ɓ����Ԃ���������B
*/
void CRecent::ResetAllFavorite( void )
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < *m_pnUserItemCount; i++ )
	{
		(void)SetFavorite( i, false );
	}

	return;
}

/*
	���C�ɓ����Ԃ��ǂ������ׂ�B

	true	���C�ɓ���
	false	�ʏ�
*/
bool CRecent::IsFavorite( int nIndex )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	return m_pbUserItemFavorite[nIndex];
}

/*
	�A�C�e�����X�g��������Ƃ��Â��o���C�ɓ���E�ʏ�p�̃A�C�e����T���B

	bFavorite=true	���C�ɓ���̒�����T��
	bFavorite=false	�ʏ�̒�����T��
*/
int CRecent::GetOldestItem( int nIndex, bool bFavorite )
{
	int	i;

	if( ! IsAvailable() ) return -1;
	if( nIndex >= *m_pnUserItemCount ) nIndex = *m_pnUserItemCount - 1;

	for( i = nIndex; i >= 0; i-- )
	{
		if( IsFavorite( i ) == bFavorite ) return i;
	}

	return -1;
}

/*
	�A�C�e�����ړ�����B
*/
bool CRecent::MoveItem( int nSrcIndex, int nDstIndex )
{
	char	*pri;
	int	i;
	bool	bFavorite;

	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= *m_pnUserItemCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= *m_pnUserItemCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	pri = new char[ m_nItemSize ];
	if( NULL == pri ) return false;

	//�ړ��������ޔ�
	memcpy( pri, GetArrayOffset( nSrcIndex ), m_nItemSize );
	bFavorite = IsFavorite( nSrcIndex );

	if( nSrcIndex < nDstIndex )
	{
		for( i = nSrcIndex; i < nDstIndex; i++ )
		{
			CopyItem( i + 1, i );
		}
	}
	else
	{
		for( i = nSrcIndex; i > nDstIndex; i-- )
		{
			CopyItem( i - 1, i );
		}
	}

	//�V�����ʒu�Ɋi�[
	memcpy( GetArrayOffset( nDstIndex ), pri, m_nItemSize );
	(void)SetFavorite( nDstIndex, bFavorite );

	delete [] pri;

	return true;
}

bool CRecent::CopyItem( int nSrcIndex, int nDstIndex )
{
	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= m_nArrayCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= m_nArrayCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	memcpy( GetArrayOffset( nDstIndex ), GetArrayOffset( nSrcIndex ), m_nItemSize );

	//(void)SetFavorite( nDstIndex, IsFavorite( nSrcIndex ) );
	//�����������Ȃ��Ƃ��߁B
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nDstIndex] = m_pbUserItemFavorite[nSrcIndex];

	return true;
}

/*
	�A�C�e�����[���N���A����B
*/
void CRecent::ZeroItem( int nIndex )
{
	if( ! IsAvailable() ) return;
	if( nIndex < 0 || nIndex >= m_nArrayCount ) return;

	memset( GetArrayOffset( nIndex ), 0, m_nItemSize );

	//(void)SetFavorite( nIndex, false );
	//�����������Ȃ��Ƃ��߁B
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nIndex] = false;

	return;
}

/*
	�A�C�e�����폜����B
*/
bool CRecent::DeleteItem( int nIndex )
{
	int	i;

	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;

	ZeroItem( nIndex );

	//�ȍ~�̃A�C�e����O�ɋl�߂�B
	for( i = nIndex; i < *m_pnUserItemCount - 1; i++ )
	{
		CopyItem( i + 1, i );
	}
	ZeroItem( i );

	*m_pnUserItemCount -= 1;

	return true;
}

/*
	���ׂẴA�C�e�����폜����B

	@note	�[���N���A���\�Ƃ��邽�߁A���ׂĂ��ΏۂɂȂ�B
*/
void CRecent::DeleteAllItem( void )
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < m_nArrayCount; i++ )
	{
		ZeroItem( i );
	}

	*m_pnUserItemCount = 0;

	return;
}

/*
	�A�C�e�����擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const char *CRecent::GetItem( int nIndex )
{
	if( ! IsAvailable() ) return NULL;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return NULL;

	return GetArrayOffset( nIndex );
}

/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const char *CRecent::GetDataOfItem( int nIndex )
{
	const char	*p;

	if( ! IsAvailable() ) return NULL;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return NULL;

	p = GetItem( nIndex );
	if( NULL == p ) return NULL;

	return &p[ m_nOffset ];
}

/*
	�A�C�e����擪�ɒǉ�����B

	@note	���łɓo�^�ς݂̏ꍇ�͐擪�Ɉړ�����B
	@note	�����ς��̂Ƃ��͍ŌẪA�C�e�����폜����B
	@note	���C�ɓ���͍폜����Ȃ��B
*/
bool CRecent::AppendItem( const char *pszItemData )
{
	int		nIndex;
	int		i;
	char	*p;

	if( ! IsAvailable() ) return false;
	if( NULL == pszItemData ) return false;

	//�o�^�ς݂����ׂ�B
	nIndex = FindItem( &pszItemData[ m_nOffset ] );
	if( nIndex >= 0 )
	{
		//�ŐV�̏��ɍX�V����B
		if( m_nCmpSize > 0 )
			memcpy( GetArrayOffset( nIndex ), pszItemData, m_nItemSize );
		else	//������Ƃ��Ď󂯎��B
		{
			p = GetArrayOffset( nIndex );
			strncpy( p, pszItemData, m_nItemSize );
			p[ m_nItemSize - 1 ] = '\0';
			
		}

		//�擪�Ɏ����Ă���B
		(void)MoveItem( nIndex, 0 );
		goto reconfigure;
	}

	//�����ς��̂Ƃ��͍ŌÂ̒ʏ�A�C�e�����폜����B
	if( m_nArrayCount <= *m_pnUserItemCount )
	{
		nIndex = GetOldestItem( *m_pnUserItemCount - 1, false );
		if( -1 == nIndex )
		{
			return false;
		}

		(void)DeleteItem( nIndex );
	}

	for( i = *m_pnUserItemCount; i > 0; i-- )
	{
		CopyItem( i - 1, i );
	}

	if( m_nCmpSize > 0 )
		memcpy( GetArrayOffset( 0 ), pszItemData, m_nItemSize );
	else	//������Ƃ��Ď󂯎��B
	{
		p = GetArrayOffset( 0 );
		strncpy( p, pszItemData, m_nItemSize );
		p[ m_nItemSize - 1 ] = '\0';
	}

	//(void)SetFavorite( 0, true );
	//�����������Ȃ��Ƃ��߁B
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[0] = false;

	*m_pnUserItemCount += 1;


reconfigure:
	//���C�ɓ����\�����Ɉړ�����B
	if( m_pnUserViewCount )
	{
		(void)ChangeViewCount( *m_pnUserViewCount );
	}
	return true;
}

/*
	�A�C�e������������B
*/
int CRecent::FindItem( const char *pszItemData )
{
	int	i;
	const char	*p, *q;

	if( ! IsAvailable() ) return -1;
	if( NULL == pszItemData ) return -1;

	//p = &pszItemData[m_nOffset];
	p = pszItemData;

	for( i = 0; i < *m_pnUserItemCount; i++ )
	{
		q = GetArrayOffset( i ) + m_nOffset;

		switch( m_nCmpType )
		{
		case RECENT_CMP_STRICMP:	//stricmp
			if( stricmp( p, q ) == 0 ) return i;
			break;

		case RECENT_CMP_STRNCMP:	//strncmp
			if( strncmp( p, q, m_nCmpSize ) == 0 ) return i;
			break;

		case RECENT_CMP_STRNICMP:	//strnicmp
			if( strnicmp( p, q, m_nCmpSize ) == 0 ) return i;
			break;

		case RECENT_CMP_MEMCMP:		//memcmp
			if( memcmp( p, q, m_nCmpSize ) == 0 ) return i;
			break;

		case RECENT_CMP_MEMICMP:	//memicmp
			if( memicmp( p, q, m_nCmpSize ) == 0 ) return i;
			break;

		case RECENT_CMP_STRCMP:		//strcmp
		default:
			if( strcmp( p, q ) == 0 ) return i;
			break;
		}
	}

	return -1;
}

bool CRecent::EasyCreate( int nRecentType )
{
	switch( nRecentType )
	{
	case RECENT_FOR_FILE:	//�t�@�C��
		return Create(
			(char*)m_pShareData->m_fiMRUArr,
			&m_pShareData->m_nMRUArrNum,
			m_pShareData->m_bMRUArrFavorite,
			MAX_MRU,
			&(m_pShareData->m_Common.m_nMRUArrNum_MAX),
			sizeof( FileInfo ),
			(int)(&((FileInfo*)0)->m_szPath[0]),
			sizeof( ((FileInfo*)0)->m_szPath ),	//_MAX_PATH
			RECENT_CMP_STRICMP
		);

	case RECENT_FOR_FOLDER:	//�t�H���_
		return Create(
			(char*)m_pShareData->m_szOPENFOLDERArr,
			&m_pShareData->m_nOPENFOLDERArrNum,
			m_pShareData->m_bOPENFOLDERArrFavorite,
			MAX_OPENFOLDER,
			&(m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX),
			sizeof( m_pShareData->m_szOPENFOLDERArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRICMP
		);

	case RECENT_FOR_SEARCH:	//����
		return Create(
			(char*)m_pShareData->m_szSEARCHKEYArr,
			&m_pShareData->m_nSEARCHKEYArrNum,
			NULL /*m_pShareData->m_bSEARCHKEYArrFavorite*/,
			MAX_SEARCHKEY,
			NULL,
			sizeof( m_pShareData->m_szSEARCHKEYArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRCMP
		);

	case RECENT_FOR_REPLACE:	//�u��
		return Create(
			(char*)m_pShareData->m_szREPLACEKEYArr,
			&m_pShareData->m_nREPLACEKEYArrNum,
			NULL /*m_pShareData->m_bREPLACEKEYArrFavorite*/,
			MAX_REPLACEKEY,
			NULL,
			sizeof( m_pShareData->m_szREPLACEKEYArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRCMP
		);

	case RECENT_FOR_GREP_FILE:	//GREP�t�@�C��
		return Create(
			(char*)m_pShareData->m_szGREPFILEArr,
			&m_pShareData->m_nGREPFILEArrNum,
			NULL /*m_pShareData->m_bGREPFILEArrFavorite*/,
			MAX_GREPFILE,
			NULL,
			sizeof( m_pShareData->m_szGREPFILEArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRICMP
		);

	case RECENT_FOR_GREP_FOLDER:	//GREP�t�H���_
		return Create(
			(char*)m_pShareData->m_szGREPFOLDERArr,
			&m_pShareData->m_nGREPFOLDERArrNum,
			NULL /*m_pShareData->m_bGREPFOLDERArrFavorite*/,
			MAX_GREPFOLDER,
			NULL,
			sizeof( m_pShareData->m_szGREPFOLDERArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRICMP
		);

	case RECENT_FOR_CMD:	//�R�}���h
		return Create(
			(char*)m_pShareData->m_szCmdArr,
			&m_pShareData->m_nCmdArrNum,
			NULL /*m_pShareData->m_bCmdArrFavorite*/,
			MAX_CMDARR,
			NULL,
			sizeof( m_pShareData->m_szCmdArr[0] ),	//MAX_CMDLEN
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRCMP
		);

	//@@@ 2003.06.28 MIK �ǉ�
	case RECENT_FOR_EDITNODE:	//�E�C���h�E���X�g
		return Create(
			(char*)m_pShareData->m_pEditArr,
			&m_pShareData->m_nEditArrNum,
			NULL,
			MAX_EDITWINDOWS,
			NULL,
			sizeof( m_pShareData->m_pEditArr[0] ),
			(int)(&((EditNode*)0)->m_hWnd),
			sizeof( ((EditNode*)0)->m_hWnd ),	//sizeof(HWND)
			RECENT_CMP_MEMCMP
		);

	case RECENT_FOR_TAGJUMP_KEYWORD:	//�^�O�W�����v�L�[���[�h 2005.04.03 MIK
		return Create(
			(char*)m_pShareData->m_szTagJumpKeywordArr,
			&m_pShareData->m_nTagJumpKeywordArrNum,
			NULL /*m_pShareData->m_bTagJumpKeywordArrFavorite*/,
			MAX_TAGJUMP_KEYWORD,
			NULL,
			sizeof( m_pShareData->m_szTagJumpKeywordArr[0] ),	//_MAX_PATH
			0,
			0,	//AppendItem�̃f�[�^�͕�����
			RECENT_CMP_STRCMP
		);

	default:
		return false;
	}
}

