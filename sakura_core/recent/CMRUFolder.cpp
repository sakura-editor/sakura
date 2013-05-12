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

#include "StdAfx.h"
#include "CMRUFolder.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "uiparts/CMenuDrawer.h"	//	����ł����̂��H
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

/*!
	�t�H���_�������j���[�̍쐬
	
	@param pCMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer ) const
{
	HMENU	hMenuPopUp;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu( hMenuPopUp, pCMenuDrawer );
}

/*!
	�t�H���_�������j���[�̍쐬
	
	@param �ǉ����郁�j���[�̃n���h��
	@param pCMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return ���j���[�̃n���h��
*/
HMENU CMRUFolder::CreateMenu( HMENU	hMenuPopUp, CMenuDrawer* pCMenuDrawer ) const
{
	TCHAR	szMenu[_MAX_PATH * 2 + 10];				//	���j���[�L���v�V����
	int		i;
	bool	bFavorite;

	CFileNameManager::getInstance()->TransformFileName_MakeCache();
	for( i = 0; i < m_cRecentFolder.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecentFolder.GetViewCount() ) break;

		const TCHAR* pszFolder = m_cRecentFolder.GetItemText( i );
		bFavorite = m_cRecentFolder.IsFavorite( i );
		bool bFavoriteLabel = bFavorite && !m_pShareData->m_Common.m_sWindow.m_bMenuIcon;
		CFileNameManager::getInstance()->GetMenuFullLabel( szMenu, _countof(szMenu), true, pszFolder, -1, false, CODE_NONE, bFavoriteLabel, i, true );

		//	���j���[�ɒǉ�
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMenu, _T(""), TRUE,
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

int CMRUFolder::Length() const
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
	 || pszFolder[0] == _T('\0') )
	{	//	������0�Ȃ�r���B
		return;
	}

	// ���łɓo�^����Ă���ꍇ�́A���O�w��𖳎�����
	if( -1 == m_cRecentFolder.FindItemByText( pszFolder ) ){
		for( int i = 0 ; i < m_pShareData->m_sHistory.m_aExceptMRU.size(); i++ ){
			TCHAR szExceptMRU[_MAX_PATH];
			CFileNameManager::ExpandMetaToFolder( m_pShareData->m_sHistory.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU) );
			if( NULL != _tcsistr( pszFolder, szExceptMRU ) ){
				return;
			}
		}
	}

	m_cRecentFolder.AppendItem( pszFolder );
}

const TCHAR* CMRUFolder::GetPath(int num) const
{
	return m_cRecentFolder.GetItemText( num );
}

