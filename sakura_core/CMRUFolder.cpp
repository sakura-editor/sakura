/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CShareData.h"
#include "CMenuDrawer.h"	//	����ł����̂��H
#include "CMRUFolder.h"
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

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
}

HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	char	szFolder2[_MAX_PATH * 2];	//	�S��&�ł����Ȃ��悤�� :-)
//	char	*p;							//	&���X�L��������Ƃ��Ɏg���A��Ɨp�|�C���^�B
	char	szMemu[300];				//	���j���[�L���v�V����
	int		createdMenuItem = 0;		//	���łɍ쐬���ꂽ���j���[�̐��B
	int		i;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < Length() ; ++i ){
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if ( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= createdMenuItem ){
			break;
		}

		//	&��&&�ɒu���B
		//	Jan. 19, 2002 genta
		dupamp( m_pShareData->m_szOPENFOLDERArr[i], szFolder2 );

		//	j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�H���_�̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
		wsprintf( szMemu, "&%c %s", (createdMenuItem < 10) ? ('0' + createdMenuItem) : ('A' + createdMenuItem - 10), szFolder2 );

		//	���j���[�ɒǉ�
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu );
		createdMenuItem++;
	}
	return hMenuPopUp;
}

void CMRUFolder::GetPathList( char** ppszOPENFOLDER )
{
	int i;
	int copiedItem = 0;
	for( i = 0; i < Length(); ++i ){
		//	�u���ʐݒ�v���u�S�ʁv���u�t�H���_�̗���MAX�v�𔽉f
		if ( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= copiedItem ){
			break;
		}
		ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
		copiedItem++;
	}
	ppszOPENFOLDER[i] = NULL;
}

int CMRUFolder::Length()
{
	return m_pShareData->m_nOPENFOLDERArrNum;
}

void CMRUFolder::ClearAll()
{
	int i;
	for( i = 0; i < MAX_OPENFOLDER; ++i ){
		strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
	}
	m_pShareData->m_nOPENFOLDERArrNum = 0;
}

/*	@brief �J�����t�H���_ ���X�g�ւ̓o�^

	@date 2001.12.26  CShareData::AddOPENFOLDERList����ړ������B�iYAZAKI�j
*/
void CMRUFolder::Add( const char* pszFolder )
{
	if( 0 == strlen( pszFolder ) ){	//	������0�Ȃ�r���B
		return;
	}

	int i, j;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pszFolder, m_pShareData->m_szOPENFOLDERArr[i] ) ){
			//	�������ł�m_pShareData->m_szOPENFOLDERArr[i]�ɂ������B
			for( j = i; j > 0; j-- ){	//	�����܂ł̃t�H���_���J�艺���B
				strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );	//	�R�s�[
			}
			strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );	//	�擪�Ɋ��蓖�āB
			//	m_pShareData->m_nOPENFOLDERArrNum�͕ς�炸�B
			return;
		}
	}

	//	�܂�m_szOPENFOLDERArr�ɂ͖��������B
	for( j = max(Length(), m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX) - 1; j > 0; j-- ){
		strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );	//	�R�s�[
	}
	strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );	//	�擪�ɃR�s�[

	m_pShareData->m_nOPENFOLDERArrNum++;	//	�����𑝂₷�B
	if( m_pShareData->m_nOPENFOLDERArrNum > MAX_OPENFOLDER ){	//	������������߂��B
		m_pShareData->m_nOPENFOLDERArrNum = MAX_OPENFOLDER;
	}
}

const char* CMRUFolder::GetPath(int num)
{
	if ( num < Length() ){
		return m_pShareData->m_szOPENFOLDERArr[num];
	}
	return 0;	//	��O�𓊂����ق����悳�����H
}

