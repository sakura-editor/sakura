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
	// id==0 �� �󕶎���ɂ���
	m_baseDirArr.push_back(_T(""));
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
	m_baseDirArr.clear();
	m_baseDirArr.push_back(_T(""));
}

/*
	��t�H���_��o�^���A��t�H���_ID���擾
	@date 2010.07.23 Moca �V�K�ǉ�
*/
int CSortedTagJumpList::AddBaseDir( const TCHAR* baseDir )
{
	m_baseDirArr.push_back( baseDir );
	return m_baseDirArr.size() -1;
}

/*
	�A�C�e�����\�[�g���ꂽ��ԂŃ��X�g�ɒǉ�����B
	�A�C�e�����ő吔�𒴂���ꍇ�́A������A�C�e�����폜����B
	������̓R�s�[���쐬����̂ŁA�Ăяo�����͕�����̃A�h���X���ێ�����K�v�͂Ȃ��B
	
	@param[in] keyword	�L�[���[�h
	@param[in] filename	�t�@�C����
	@param[in] no		�s�ԍ�
	@param[in] type		���
	@param[in] note		���l
	@param[in] depth	(�����̂ڂ�)�K�w
	@param[in] baseDirId	��t�H���_ID�B0�ŋ󕶎���w�� (AddBaseDir�̖߂�l)
	@retval TRUE  �ǉ�����
	@retval FALSE �ǉ����s
	@date 2010.07.23 Moca baseDirId �ǉ�
*/
BOOL CSortedTagJumpList::AddParamA( const ACHAR* keyword, const ACHAR* filename, int no,
	ACHAR type, const ACHAR* note, int depth, int baseDirId )
{
	TagJumpInfo*	p;
	TagJumpInfo*	prev;
	TagJumpInfo*	item;
	// 3�߂�SJIS�p�ی�
	ACHAR typeStr[] = {type, '\0', '\0'};

	//�A�C�e�����쐬����B
	item = (TagJumpInfo*)malloc( sizeof( TagJumpInfo ) );
	if( NULL == item ) return FALSE;
	item->keyword  = _tcsdup( to_tchar(keyword) );
	item->filename = _tcsdup( to_tchar(filename) );
	item->no       = no;
	item->type     = to_tchar(typeStr)[0];
	item->note     = _tcsdup( to_tchar(note) );
	item->depth    = depth;
	item->next     = NULL;
	item->baseDirId = baseDirId;

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
	@param[out] baseDir		�t�@�C�����̊�t�H���_
	@return ��������

	@note �s�v�ȏ��̏ꍇ�͈����� NULL ���w�肷��B
*/
BOOL CSortedTagJumpList::GetParam( int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth, TCHAR* baseDir )
{
	if( keyword  ) _tcscpy( keyword, _T("") );
	if( filename ) _tcscpy( filename, _T("") );
	if( no       ) *no    = 0;
	if( type     ) *type  = 0;
	if( note     ) _tcscpy( note, _T("") );
	if( depth    ) *depth = 0;
	if( baseDir  ) _tcscpy( baseDir, _T("") );

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
		if( baseDir ){
			if( 0 <= p->baseDirId && (size_t)p->baseDirId < m_baseDirArr.size() ){
				auto_strcpy( baseDir, m_baseDirArr[p->baseDirId].c_str() );
			}
		}
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
