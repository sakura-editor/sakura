/*!	@file
	@brief �^�O�W�����v���X�g

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta

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
#include "CSortedTagJumpList.h"

/*!
	@date 2005.04.23 genta �Ǘ����̍ő�l���w�肷������ǉ�
*/
CSortedTagJumpList::CSortedTagJumpList(int max)
	: m_MAX_TAGJUMPLIST( max ),
	  m_pTagjump( NULL ),
	  m_nCount( 0 ),
	  m_bOverflow( false )
{
}

CSortedTagJumpList::~CSortedTagJumpList()
{
	Empty();
}

/*
	�w�肳�ꂽ�A�C�e���̃��������������B

	@param[in] item �폜����A�C�e��
*/
void CSortedTagJumpList::Free( TagJumpInfo* item )
{
	free( item->keyword );
	free( item->filename );
	free( item->note );
	free( item );
	return;
}

/*
	���X�g�����ׂĉ������B
*/
void CSortedTagJumpList::Empty( void )
{
	TagJumpInfo*	p;
	TagJumpInfo*	next;
	for( p = m_pTagjump; p; p = next )
	{
		next = p->next;
		Free( p );
	}
	m_pTagjump = NULL;
	m_nCount = 0;
	m_bOverflow = false;
}

/*
	�A�C�e�����\�[�g���ꂽ��ԂŃ��X�g�ɒǉ�����B
	�A�C�e�����ő吔�𒴂���ꍇ�́A������A�C�e�����폜����B
	
	@param[in] keyword	�L�[���[�h
	@param[in] filename	�t�@�C����
	@param[in] no		�s�ԍ�
	@param[in] type		���
	@param[in] note		���l
	@param[in] depth	(�����̂ڂ�)�K�w
	@return ��������
*/
BOOL CSortedTagJumpList::AddParam( TCHAR* keyword, TCHAR* filename, int no, TCHAR type, TCHAR* note, int depth )
{
	TagJumpInfo*	p;
	TagJumpInfo*	prev;
	TagJumpInfo*	item;

	//�A�C�e�����쐬����B
	item = (TagJumpInfo*)malloc( sizeof( TagJumpInfo ) );
	if( NULL == item ) return FALSE;
	item->keyword  = _tcsdup( keyword );
	item->filename = _tcsdup( filename );
	item->no       = no;
	item->type     = type;
	item->note     = _tcsdup( note );
	item->depth    = depth;
	item->next     = NULL;

	//�����񒷃K�[�h
	if( _tcslen( item->keyword  ) >= MAX_TAG_STRING_LENGTH ) item->keyword[  MAX_TAG_STRING_LENGTH-1 ] = 0;
	if( _tcslen( item->filename ) >= MAX_TAG_STRING_LENGTH ) item->filename[ MAX_TAG_STRING_LENGTH-1 ] = 0;
	if( _tcslen( item->note     ) >= MAX_TAG_STRING_LENGTH ) item->note[     MAX_TAG_STRING_LENGTH-1 ] = 0;

	//�A�C�e�������X�g�̓K���Ȉʒu�ɒǉ�����B
	prev = NULL;
	for( p = m_pTagjump; p; p = p->next )
	{
		if( _tcscmp( p->keyword, item->keyword ) > 0 ) break;
		prev = p;
	}
	item->next = p;
	if( prev ) prev->next = item;
	else       m_pTagjump = item;
	m_nCount++;

	//�ő吔�𒴂�����Ō�̃A�C�e�����폜����B
	if( m_nCount > m_MAX_TAGJUMPLIST )
	{
		prev = NULL;
		for( p = m_pTagjump; p->next; p = p->next ) prev = p;
		if( prev ) prev->next = NULL;
		else       m_pTagjump = NULL;
		Free( p );
		m_nCount--;
		m_bOverflow = true;
	}
	return TRUE;
}

/*
	�w��̏����擾����B

	@param[out] keyword		�L�[���[�h
	@param[out] filename	�t�@�C����
	@param[out] no			�s�ԍ�
	@param[out] type		���
	@param[out] note		���l
	@param[out] depth		(�����̂ڂ�)�K�w
	@return ��������

	@note �s�v�ȏ��̏ꍇ�͈����� NULL ���w�肷��B
*/
BOOL CSortedTagJumpList::GetParam( int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth )
{
	if( keyword  ) _tcscpy( keyword, _T("") );
	if( filename ) _tcscpy( filename, _T("") );
	if( no       ) *no    = 0;
	if( type     ) *type  = 0;
	if( note     ) _tcscpy( note, _T("") );
	if( depth    ) *depth = 0;

	CSortedTagJumpList::TagJumpInfo* p;
	p = GetPtr( index );
	if( NULL != p )
	{
		if( keyword  ) _tcscpy( keyword, p->keyword );
		if( filename ) _tcscpy( filename, p->filename );
		if( no       ) *no    = p->no;
		if( type     ) *type  = p->type;
		if( note     ) _tcscpy( note, p->note );
		if( depth    ) *depth = p->depth;
		return TRUE;
	}
	return FALSE;
}

/*
	�w��̏����\���̃|�C���^�Ŏ擾����B
	�擾�������͎Q�ƂȂ̂ŉ�����Ă͂Ȃ�Ȃ��B

	@param[in] index �v�f�ԍ�
	@return �^�O�W�����v���
*/
CSortedTagJumpList::TagJumpInfo* CSortedTagJumpList::GetPtr( int index )
{
	TagJumpInfo*	p;
	int	i;
	i = 0;
	for( p = m_pTagjump; p; p = p->next )
	{
		if( index == i ) return p;
		i++;
	}
	return NULL;
}
