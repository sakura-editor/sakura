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
#include "CMRUFolder.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "CMenuDrawer.h"	//	����ł����̂��H
#include "recent/CRecent.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "util/string_ex2.h"

/*!	�R���X�g���N�^

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CMRUFolder::CMRUFolder()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();
}

/*	�f�X�g���N�^	*/
CMRUFolder::~CMRUFolder()
{
	m_cRecentFolder.Terminate();
}

HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	TCHAR	szFolder2[_MAX_PATH * 2];	//	�S��&�ł����Ȃ��悤�� :-)
	TCHAR	szMemu[300];				//	���j���[�L���v�V����
	int		i;
	bool	bFavorite;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	CFileNameManager::Instance()->TransformFileName_MakeCache();
	for( i = 0; i < m_cRecentFolder.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecentFolder.GetViewCount() ) break;

		CFileNameManager::Instance()->GetTransformFileNameFast( m_cRecentFolder.GetItemText( i ), szMemu, _MAX_PATH );
		//	&��&&�ɒu���B
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFolder2 );

		bFavorite = m_cRecentFolder.IsFavorite( i );
		//	j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�H���_�̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
		auto_sprintf( szMemu, _T("&%tc %ts%ts"), 
			(i < 10) ? (_T('0') + i) : (_T('A') + i - 10), 
			(!m_pShareData->m_Common.m_sWindow.m_bMenuIcon && bFavorite) ? _T("�� ") : _T(""),
			szFolder2 );

		//	���j���[�ɒǉ�
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu, TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

std::vector<LPCTSTR> CMRUFolder::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for( int i = 0; i < m_cRecentFolder.GetItemCount(); ++i ){
		//	�u���ʐݒ�v���u�S�ʁv���u�t�H���_�̗���MAX�v�𔽉f
		if ( i >= m_cRecentFolder.GetViewCount() ) break;
		ret.push_back(m_cRecentFolder.GetItemText(i));
	}
	return ret;
}

int CMRUFolder::Length()
{
	return m_cRecentFolder.GetItemCount();
}

void CMRUFolder::ClearAll()
{
	m_cRecentFolder.DeleteAllItem();
}

/*	@brief �J�����t�H���_ ���X�g�ւ̓o�^

	@date 2001.12.26  CShareData::AddOPENFOLDERList����ړ������B�iYAZAKI�j
*/
void CMRUFolder::Add( const TCHAR* pszFolder )
{
	if( NULL == pszFolder
	 || 0 == _tcslen( pszFolder ) )
	{	//	������0�Ȃ�r���B
		return;
	}

	m_cRecentFolder.AppendItem( pszFolder );
}

const TCHAR* CMRUFolder::GetPath(int num)
{
	return m_cRecentFolder.GetItemText( num );
}

