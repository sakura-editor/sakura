/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CShareData.h"
#include "CMenuDrawer.h"	//	����ł����̂��H
#include "global.h"
#include "stdio.h"
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/*!	�R���X�g���N�^
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CMRU::CMRU()
{
	//	�������B
	m_pShareData = CShareData::getInstance()->GetShareData();
}

/*	�f�X�g���N�^	*/
CMRU::~CMRU()
{
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
//	HWND	hwndDummy;				//	�_�~�[�i�g��Ȃ���j
	char	szFile2[_MAX_PATH * 2];	//	�S��&�ł����Ȃ��悤�ɁB
//	char	*p;						//	&���X�L��������Ƃ��Ɏg���A��Ɨp�|�C���^�B
	char	szMemu[300];			//	���j���[�L���v�V����
	int		createdMenuItem = 0;	//	���łɍ쐬���ꂽ���j���[�̐��B
	int		i;
	CShareData::getInstance()->TransformFileName_MakeCash();

	//	�󃁃j���[�����
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < Length(); ++i ){
		/* �w��t�@�C�����J����Ă��邩���ׂ� */
//		�J����ĂĂ������񂶂�Ȃ��H
//		if( m_pcShareData->IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
//			continue;
//		}
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( m_pShareData->m_Common.m_nMRUArrNum_MAX <= createdMenuItem ){
			break;
		}
		
		/* MRU���X�g�̒��ɂ���J����Ă��Ȃ��t�@�C�� */
		
		CShareData::getInstance()->GetTransformFileNameFast( m_pShareData->m_fiMRUArr[i].m_szPath, szMemu, _MAX_PATH );
		//	&��&&�ɒu���B
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFile2 );
		
		//	j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�@�C���̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
		wsprintf( szMemu, "&%c %s", (createdMenuItem < 10) ? ('0' + createdMenuItem) : ('A' + createdMenuItem - 10), szFile2 );

		//	�t�@�C�����̂ݕK�v�B
		//	�����R�[�h�\�L
		if( 0 <  m_pShareData->m_fiMRUArr[i].m_nCharCode  &&
				 m_pShareData->m_fiMRUArr[i].m_nCharCode  < CODE_CODEMAX ){
			strcat( szMemu, gm_pszCodeNameArr_3[ m_pShareData->m_fiMRUArr[i].m_nCharCode ] );
		}

		//	���j���[�ɒǉ��B
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu );
		createdMenuItem++;	//	�쐬�������j���[��+1
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
	int copiedItem = 0;
	for( i = 0; i < Length(); ++i ){
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( m_pShareData->m_Common.m_nMRUArrNum_MAX <= copiedItem ){
			break;
		}
		ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
		copiedItem++;
	}
	ppszMRU[i] = NULL;
}

/*! �A�C�e������Ԃ� */
int CMRU::Length(void)
{
	return m_pShareData->m_nMRUArrNum;
}

/*!
	�t�@�C�������̃N���A
*/
void CMRU::ClearAll(void)
{
	int i;
	for( i = 0; i < MAX_MRU; ++i ){
		m_pShareData->m_fiMRUArr[i].m_nViewTopLine = 0;
		m_pShareData->m_fiMRUArr[i].m_nViewLeftCol = 0;
		m_pShareData->m_fiMRUArr[i].m_nX = 0;
		m_pShareData->m_fiMRUArr[i].m_nY = 0;
		m_pShareData->m_fiMRUArr[i].m_bIsModified = 0;
		m_pShareData->m_fiMRUArr[i].m_nCharCode = 0;
		strcpy( m_pShareData->m_fiMRUArr[i].m_szPath, "" );
	}
	m_pShareData->m_nMRUArrNum = 0;
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
	if (num < Length()){
		*pfi = m_pShareData->m_fiMRUArr[num];	//	���ς�炸���h���B�B�B
		return TRUE;
	}
	return FALSE;
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
	int i;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pszPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			*pfi = m_pShareData->m_fiMRUArr[i];	//	���ς�炸���h���B�B�B
			return TRUE;
		}
	}
	return FALSE;
}

/*!	@brief MRU���X�g�ւ̓o�^

	@param pfi [in] �ǉ�����t�@�C���̏��

	�Y���t�@�C���������[�o�u���f�B�X�N��ɂ���ꍇ�ɂ�MRU List�ւ̓o�^�͍s��Ȃ��B

	@date 2001.03.29 MIK �����[�o�u���f�B�X�N��̃t�@�C����o�^���Ȃ��悤�ɂ����B
	@date 2001.12.26 YAZAKI CShareData::AddMRUList����ړ�
*/
void CMRU::Add( FileInfo* pFileInfo )
{
	//	�t�@�C������������Ζ���
	if( 0 == strlen( pFileInfo->m_szPath ) ){
		return;
	}
	
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFolder[_MAX_PATH + 1];	//	�h���C�u�{�t�H���_

	_splitpath( pFileInfo->m_szPath, szDrive, szDir, NULL, NULL );	//	�h���C�u�ƃt�H���_�����o���B

	//	�����[�o�u���Ȃ��o�^�H
	if (/* �u�����[�o�u���Ȃ�o�^���Ȃ��v�I�� && */ IsRemovableDrive( szDrive ) ){
		return;
	}

	//	szFolder�쐬
	strcpy( szFolder, szDrive );
	strcat( szFolder, szDir );
	//	Folder���ACMRUFolder�ɓo�^
	CMRUFolder cMRUFolder;
	cMRUFolder.Add(szFolder);

	//	MRU�ɓo�^�B
	int i, j;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pFileInfo->m_szPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			//	�������ł�m_pShareData->m_fiMRUArr�ɂ������B
			for( j = i; j > 0; j-- ){	//	�����܂ł̃t�@�C�����J�艺���B
				m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];	//	�l�̃R�s�[
			}
			m_pShareData->m_fiMRUArr[0] = *pFileInfo;	//	�擪�Ɋ��蓖�āB
			//	m_pShareData->m_nMRUArrNum�͕ς�炸�B
			return;
		}
	}
	
	//	�܂�m_fiMRUArr�ɂ͖��������B
	//	�ق��ɉ���������B
	for( j = max(Length(), m_pShareData->m_Common.m_nMRUArrNum_MAX) - 1; j > 0; j-- ){
		m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];	//	�l�̃R�s�[
	}
	m_pShareData->m_fiMRUArr[0] = *pFileInfo;	//	�擪�Ɋ��蓖�āB

	m_pShareData->m_nMRUArrNum++;	//	�����𑝂₷�B
	if( m_pShareData->m_nMRUArrNum > MAX_MRU ){	//	������������߂��B
		m_pShareData->m_nMRUArrNum = MAX_MRU;
	}
}

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

/*EOF*/
