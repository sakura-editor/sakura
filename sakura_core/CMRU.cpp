/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CShareData.h"
#include "CMenuDrawer.h"	//	����ł����̂��H
#include "global.h"
#include <stdio.h>
#include "CMRU.h"
#include "CRecent.h"	//���C�ɓ���	//@@@ 2003.04.08 MIK
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/*!	�R���X�g���N�^
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CMRU::CMRU()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();

	//���C�ɓ���	//@@@ 2003.04.08 MIK
	(void)m_cRecent.EasyCreate( RECENT_FOR_FILE );
}

/*	�f�X�g���N�^	*/
CMRU::~CMRU()
{
	m_cRecent.Terminate();
}

/*!
	�t�@�C���������j���[�̍쐬
	
	@param pCMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return �����������j���[�̃n���h��
*/
HMENU CMRU::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	char	szFile2[_MAX_PATH * 2];	//	�S��&�ł����Ȃ��悤�ɁB
	char	szMemu[300];			//	���j���[�L���v�V����
	int		i;
	bool	bFavorite;
	FileInfo	*p;

	CShareData::getInstance()->TransformFileName_MakeCache();

	//	�󃁃j���[�����
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecent.GetViewCount() ) break;
		
		/* MRU���X�g�̒��ɂ���J����Ă��Ȃ��t�@�C�� */

		p = (FileInfo*)m_cRecent.GetItem( i );
		
		CShareData::getInstance()->GetTransformFileNameFast( p->m_szPath, szMemu, _MAX_PATH );
		//	&��&&�ɒu���B
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFile2 );
		
		bFavorite = m_cRecent.IsFavorite( i );
		//	j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�@�C���̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
		wsprintf( szMemu, "&%c %s%s", 
			(i < 10) ? ('0' + i) : ('A' + i - 10), 
			(FALSE == m_pShareData->m_Common.m_bMenuIcon && bFavorite) ? "�� " : "",
			szFile2 );

		//	�t�@�C�����̂ݕK�v�B
		//	�����R�[�h�\�L
		if( 0 <  p->m_nCharCode  &&
				 p->m_nCharCode  < CODE_CODEMAX ){
			strcat( szMemu, gm_pszCodeNameArr_3[ p->m_nCharCode ] );
		}

		//	���j���[�ɒǉ��B
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu, TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

BOOL CMRU::DestroyMenu( HMENU hMenuPopUp )
{
	return ::DestroyMenu( hMenuPopUp );
}

/*!
	�t�@�C�������̈ꗗ��Ԃ�
	
	@param ppszMRU [out] ������ւ̃|�C���^���X�g���i�[����D
	�Ō�̗v�f�̎��ɂ�NULL������D
	�\�ߌĂяo�����ōő�l+1�̗̈���m�ۂ��Ă������ƁD
*/
void CMRU::GetPathList( char** ppszMRU )
{
	int i;

	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecent.GetViewCount() ) break;
		ppszMRU[i] = (char*)m_cRecent.GetDataOfItem( i );
	}
	ppszMRU[i] = NULL;
}

/*! �A�C�e������Ԃ� */
int CMRU::Length(void)
{
	return m_cRecent.GetItemCount();
}

/*!
	�t�@�C�������̃N���A
*/
void CMRU::ClearAll(void)
{
	m_cRecent.DeleteAllItem();
}

/*!
	�t�@�C�����̎擾
	
	@param num [in] ����ԍ�(0~)
	@param pfi [out] �\���̂ւ̃|�C���^�i�[��
	
	@retval TRUE �f�[�^���i�[���ꂽ
	@retval FALSE �������Ȃ��ԍ����w�肳�ꂽ�D�f�[�^�͊i�[����Ȃ������D
*/
BOOL CMRU::GetFileInfo( int num, FileInfo* pfi )
{
	FileInfo	*p;

	p = (FileInfo*)m_cRecent.GetItem( num );
	if( NULL == p ) return FALSE;

	*pfi = *p;

	return TRUE;
}

/*!
	�w�肳�ꂽ���O�̃t�@�C����MRU���X�g�ɑ��݂��邩���ׂ�B���݂���Ȃ�΃t�@�C������Ԃ��B

	@param pszPath [in] ��������t�@�C����
	@param pfi [out] �f�[�^�����������Ƃ��Ƀt�@�C�������i�[����̈�B
		�Ăяo�����ŗ̈�����炩���ߗp�ӂ���K�v������B
	@retval TRUE  �t�@�C�������������Bpfi�Ƀt�@�C����񂪊i�[����Ă���B
	@retval FALSE �w�肳�ꂽ�t�@�C����MRU List�ɖ����B

	@date 2001.12.26 CShareData::IsExistInMRUList����ړ������B�iYAZAKI�j
*/
BOOL CMRU::GetFileInfo( const char* pszPath, FileInfo* pfi )
{
	FileInfo	*p;

	p = (FileInfo*)m_cRecent.GetItem( m_cRecent.FindItem( pszPath ) );
	if( NULL == p ) return FALSE;

	*pfi = *p;

	return TRUE;
}

/*!	@brief MRU���X�g�ւ̓o�^

	@param pFileInfo [in] �ǉ�����t�@�C���̏��

	�Y���t�@�C���������[�o�u���f�B�X�N��ɂ���ꍇ�ɂ�MRU List�ւ̓o�^�͍s��Ȃ��B

	@date 2001.03.29 MIK �����[�o�u���f�B�X�N��̃t�@�C����o�^���Ȃ��悤�ɂ����B
	@date 2001.12.26 YAZAKI CShareData::AddMRUList����ړ�
*/
void CMRU::Add( FileInfo* pFileInfo )
{
	//	�t�@�C������������Ζ���
	if( NULL == pFileInfo
	 || 0 == strlen( pFileInfo->m_szPath ) )
	{
		return;
	}

	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFolder[_MAX_PATH + 1];	//	�h���C�u�{�t�H���_

	_splitpath( pFileInfo->m_szPath, szDrive, szDir, NULL, NULL );	//	�h���C�u�ƃt�H���_�����o���B

	//	Jan.  10, 2006 genta USB��������Removable media�ƔF�������悤�Ȃ̂ŁC
	//	�ꉞ����������D
	//	�����[�o�u���Ȃ��o�^�H
	//if (/* �u�����[�o�u���Ȃ�o�^���Ȃ��v�I�� && */ ! IsLocalDrive( szDrive ) ){
	//	return;
	//}

	//	szFolder�쐬
	strcpy( szFolder, szDrive );
	strcat( szFolder, szDir );
	//	Folder���ACMRUFolder�ɓo�^
	CMRUFolder cMRUFolder;
	cMRUFolder.Add(szFolder);

	m_cRecent.AppendItem( (char*)pFileInfo );
}

#if 0
// etc_uty.cpp��IsLocalDrive()�֓���

/*!
	�����[�o�u���h���C�u�̔���

	@param pszDrive [in] �h���C�u�����܂ރp�X��
	
	@retval true �����[�o�u���h���C�u
	@retval false �Œ�h���C�u�D�l�b�g���[�N�h���C�u�D
	
	@author MIK
	@date 2001.03.29 �V�K�쐬
	@date 2001.12.23 YAZAKI MRU�̕ʃN���X���ɔ����֐���
	@date 2002.01.28 genta �߂�l�̌^��BOOL����bool�ɕύX�D
*/
bool CMRU::IsRemovableDrive( const char* pszDrive )
{
	char	szDriveType[_MAX_DRIVE+1];	// "A:\"�o�^�p
	long	lngRet;
	char	c;

	c = pszDrive[0];
	if( c >= 'a' && c <= 'z' ){
		c = c - ('a' - 'A');
	}
	if( c >= 'A' && c <= 'Z' ){
		sprintf( szDriveType, "%c:\\", c );
		lngRet = GetDriveType( szDriveType );
		if( DRIVE_REMOVABLE	== lngRet
		 || DRIVE_CDROM		== lngRet){
			return true;
		}
	}
	return false;
}
#endif
/*EOF*/
