//	$Id$
/*!	@file
	@brief �}�N��

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CSMacroMgr.h"
#include "CEditView.h"

CSMacroMgr::CSMacroMgr()
{
	CShareData	m_cShareData;
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
}

CSMacroMgr::~CSMacroMgr()
{
}

/*! �L�[�}�N���̃o�b�t�@���N���A���� */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		m_cKeyMacro[i].ClearAll();
	}
}

/*! @brief�L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�

	@param nFuncID [in] �@�\�ԍ�
	@param lParam1 [in] �p�����[�^�B
	@param mbuf [in] �ǂݍ��ݐ�}�N���o�b�t�@

*/
int CSMacroMgr::Append( int num, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1 )
{
	m_cKeyMacro[num].Append( nFuncID, lParam1 );
	return TRUE;
}


/*!	@brief �L�[�{�[�h�}�N���̎��s

	CShareData����t�@�C�������擾���A���s����B

	@param hInstance [in] �C���X�^���X
	@param hwndParent [in] �e�E�B���h�E��
	@param pViewClass [in] macro���s�Ώۂ�View
	@param idx [in] �}�N���ԍ��B
*/
BOOL CSMacroMgr::Exec( HINSTANCE hInstance, CEditView* pCEditView, int idx )
{
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	�͈̓`�F�b�N
		return FALSE;

	if( !m_cKeyMacro[idx].IsReady() ){
		//	�t�@�C�������Am_pShareData����擾�B
		if( !m_pShareData->m_MacroTable[idx].IsEnabled() )
			return FALSE;
		
		char fbuf[_MAX_PATH * 2];
		char *ptr = m_pShareData->m_MacroTable[idx].m_szFile;

		if( ptr[0] == '\0' )	//	�t�@�C����������
			return FALSE;
		
		if( ptr[0] == '\\' || ( ptr[1] == ':' && ptr[2] == '\\' )){	// ��΃p�X
		}
		else if( m_pShareData->m_szMACROFOLDER[0] != '\0' ){	//	�t�H���_�w�肠��
			//	���΃p�X����΃p�X
			strcpy( fbuf, m_pShareData->m_szMACROFOLDER );
			ptr = fbuf + strlen( fbuf );
			//::MessageBox( pCEditView->m_hwndParent, ptr - 1, "CSMacroMgr::Exec/folder", MB_OK );
			if( ptr[-1] != '\\' ){
				*ptr++ = '\\';
			}
			strcpy( ptr, m_pShareData->m_MacroTable[idx].m_szFile );
			ptr = fbuf;
		}
		
		//::MessageBox( pCEditView->m_hwndParent, ptr, "CSMacroMgr::Exec", MB_OK );
		
		if( !Load( idx, hInstance, ptr ))
			return FALSE;
	}

	m_cKeyMacro[idx].ExecKeyMacro(pCEditView);
	pCEditView->Redraw();	//	�K�v�H
	return TRUE;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���

	@param num [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����

	@author Norio Nakatani
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx < 0 || MAX_CUSTMACRO <= idx ){
		return FALSE;
	}
	/* �L�[�}�N���̃o�b�t�@���N���A���� */
	m_cKeyMacro[idx].ClearAll();
	return m_cKeyMacro[idx].LoadKeyMacro(hInstance, pszPath );
}


/*[EOF]*/
