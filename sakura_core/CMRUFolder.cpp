/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CShareData.h"
#include "CMenuDrawer.h"	//	����ł����̂��H
#include "CMRUFolder.h"
#include "CRecent.h"	//���C�ɓ���	//@@@ 2003.04.08 MIK
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/*!	�R���X�g���N�^

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CMRUFolder::CMRUFolder()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();

	//���C�ɓ���	//@@@ 2003.04.08 MIK
	(void)m_cRecent.EasyCreate( RECENT_FOR_FOLDER );
}

/*	�f�X�g���N�^	*/
CMRUFolder::~CMRUFolder()
{
	m_cRecent.Terminate();
}

HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	char	szFolder2[_MAX_PATH * 2];	//	�S��&�ł����Ȃ��悤�� :-)
	char	szMemu[300];				//	���j���[�L���v�V����
	int		i;
	bool	bFavorite;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	CShareData::getInstance()->TransformFileName_MakeCache();
	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecent.GetViewCount() ) break;

		CShareData::getInstance()->GetTransformFileNameFast( m_cRecent.GetDataOfItem( i ), szMemu, _MAX_PATH );
		//	&��&&�ɒu���B
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFolder2 );

		bFavorite = m_cRecent.IsFavorite( i );
		//	j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�H���_�̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
		wsprintf( szMemu, "&%c %s%s", 
			(i < 10) ? ('0' + i) : ('A' + i - 10), 
			(FALSE == m_pShareData->m_Common.m_bMenuIcon && bFavorite) ? "�� " : "",
			szFolder2 );

		//	���j���[�ɒǉ�
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu, TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

void CMRUFolder::GetPathList( char** ppszOPENFOLDER )
{
	int	i;

	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�H���_�̗���MAX�v�𔽉f
		if ( i >= m_cRecent.GetViewCount() ) break;
		ppszOPENFOLDER[i] = (char*)m_cRecent.GetDataOfItem( i );
	}
	ppszOPENFOLDER[i] = NULL;
}

int CMRUFolder::Length()
{
	return m_cRecent.GetItemCount();
}

void CMRUFolder::ClearAll()
{
	m_cRecent.DeleteAllItem();
}

/*	@brief �J�����t�H���_ ���X�g�ւ̓o�^

	@date 2001.12.26  CShareData::AddOPENFOLDERList����ړ������B�iYAZAKI�j
*/
void CMRUFolder::Add( const char* pszFolder )
{
	if( NULL == pszFolder
	 || 0 == strlen( pszFolder ) )
	{	//	������0�Ȃ�r���B
		return;
	}

	(void)m_cRecent.AppendItem( pszFolder );
}

const char* CMRUFolder::GetPath(int num)
{
	return m_cRecent.GetDataOfItem( num );
}

