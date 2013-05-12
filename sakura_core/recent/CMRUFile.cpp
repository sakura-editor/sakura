/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
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

#include "StdAfx.h"
#include <ShlObj.h>
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"
#include "uiparts/CMenuDrawer.h"	//	����ł����̂��H
#include "window/CEditWnd.h"
#include "util/string_ex2.h"

/*!	�R���X�g���N�^
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CMRUFile::CMRUFile()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();
}

/*	�f�X�g���N�^	*/
CMRUFile::~CMRUFile()
{
	m_cRecentFile.Terminate();
}

/*!
	�t�@�C���������j���[�̍쐬
	
	@param pCMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU CMRUFile::CreateMenu( CMenuDrawer* pCMenuDrawer ) const
{
	HMENU	hMenuPopUp;

	//	�󃁃j���[�����
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu( hMenuPopUp, pCMenuDrawer );
}
/*!
	�t�@�C���������j���[�̍쐬
	
	@param �ǉ����郁�j���[�̃n���h��
	@param pCMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU CMRUFile::CreateMenu( HMENU	hMenuPopUp, CMenuDrawer* pCMenuDrawer ) const
{
	TCHAR	szMenu[_MAX_PATH * 2 + 10];				//	���j���[�L���v�V����
	int		i;
	bool	bFavorite;
	const BOOL bMenuIcon = m_pShareData->m_Common.m_sWindow.m_bMenuIcon;

	CFileNameManager::getInstance()->TransformFileName_MakeCache();

	for( i = 0; i < m_cRecentFile.GetItemCount(); ++i )
	{
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecentFile.GetViewCount() ) break;
		
		/* MRU���X�g�̒��ɂ���J����Ă��Ȃ��t�@�C�� */

		const EditInfo	*p = m_cRecentFile.GetItem( i );
		bFavorite = m_cRecentFile.IsFavorite( i );
		bool bFavoriteLabel = bFavorite && !bMenuIcon;
		CFileNameManager::getInstance()->GetMenuFullLabel_MRU( szMenu, _countof(szMenu), p, -1, bFavoriteLabel, i );

		//	���j���[�ɒǉ��B
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMenu, _T(""), TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

BOOL CMRUFile::DestroyMenu( HMENU hMenuPopUp ) const
{
	return ::DestroyMenu( hMenuPopUp );
}

/*!
	�t�@�C�������̈ꗗ��Ԃ�
	
	@param ppszMRU [out] ������ւ̃|�C���^���X�g���i�[����D
	�Ō�̗v�f�̎��ɂ�NULL������D
	�\�ߌĂяo�����ōő�l+1�̗̈���m�ۂ��Ă������ƁD
*/
std::vector<LPCTSTR> CMRUFile::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for( int i = 0; i < m_cRecentFile.GetItemCount(); ++i ){
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( i >= m_cRecentFile.GetViewCount() ) break;
		ret.push_back(m_cRecentFile.GetItemText(i));
	}
	return ret;
}

/*! �A�C�e������Ԃ� */
int CMRUFile::Length(void) const
{
	return m_cRecentFile.GetItemCount();
}

/*!
	�t�@�C�������̃N���A
*/
void CMRUFile::ClearAll(void)
{
	m_cRecentFile.DeleteAllItem();
}

/*!
	�t�@�C�����̎擾
	
	@param num [in] ����ԍ�(0~)
	@param pfi [out] �\���̂ւ̃|�C���^�i�[��
	
	@retval TRUE �f�[�^���i�[���ꂽ
	@retval FALSE �������Ȃ��ԍ����w�肳�ꂽ�D�f�[�^�͊i�[����Ȃ������D
*/
bool CMRUFile::GetEditInfo( int num, EditInfo* pfi ) const
{
	const EditInfo*	p = m_cRecentFile.GetItem( num );
	if( NULL == p ) return false;

	*pfi = *p;

	return true;
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
bool CMRUFile::GetEditInfo( const TCHAR* pszPath, EditInfo* pfi ) const
{
	const EditInfo*	p = m_cRecentFile.GetItem( m_cRecentFile.FindItemByPath( pszPath ) );
	if( NULL == p ) return false;

	*pfi = *p;

	return true;
}

/*!	@brief MRU���X�g�ւ̓o�^

	@param pEditInfo [in] �ǉ�����t�@�C���̏��

	�Y���t�@�C���������[�o�u���f�B�X�N��ɂ���ꍇ�ɂ�MRU List�ւ̓o�^�͍s��Ȃ��B

	@date 2001.03.29 MIK �����[�o�u���f�B�X�N��̃t�@�C����o�^���Ȃ��悤�ɂ����B
	@date 2001.12.26 YAZAKI CShareData::AddMRUList����ړ�
*/
void CMRUFile::Add( EditInfo* pEditInfo )
{
	//	�t�@�C������������Ζ���
	if( NULL == pEditInfo || pEditInfo->m_szPath[0] == L'\0' ){
		return;
	}
	
	// ���łɓo�^����Ă���ꍇ�́A���O�w��𖳎�����
	if( -1 == m_cRecentFile.FindItemByPath( pEditInfo->m_szPath ) ){
		for( int i = 0 ; i < m_pShareData->m_sHistory.m_aExceptMRU.size(); i++ ){
			TCHAR szExceptMRU[_MAX_PATH];
			CFileNameManager::ExpandMetaToFolder( m_pShareData->m_sHistory.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU) );
			if( NULL != _tcsistr( pEditInfo->m_szPath,  szExceptMRU) ){
				return;
			}
		}
	}

	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFolder[_MAX_PATH + 1];	//	�h���C�u�{�t�H���_

	_tsplitpath( pEditInfo->m_szPath, szDrive, szDir, NULL, NULL );	//	�h���C�u�ƃt�H���_�����o���B

	//	Jan.  10, 2006 genta USB��������Removable media�ƔF�������悤�Ȃ̂ŁC
	//	�ꉞ����������D
	//	�����[�o�u���Ȃ��o�^�H
	//if (/* �u�����[�o�u���Ȃ�o�^���Ȃ��v�I�� && */ ! IsLocalDrive( szDrive ) ){
	//	return;
	//}

	//	szFolder�쐬
	_tcscpy( szFolder, szDrive );
	_tcscat( szFolder, szDir );

	//	Folder���ACMRUFolder�ɓo�^
	CMRUFolder cMRUFolder;
	cMRUFolder.Add(szFolder);

	m_cRecentFile.AppendItem( pEditInfo );
	
	::SHAddToRecentDocs( SHARD_PATH, to_wchar(pEditInfo->m_szPath) );
}

/*EOF*/
