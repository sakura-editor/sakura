/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author YAZAKI

*/
/*
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CPPAMacroMgr.h"
#include "CPPA.h"
#include "CMemory.h"

CPPA CPPAMacroMgr::m_cPPA;

CPPAMacroMgr::CPPAMacroMgr()
: CKeyMacroMgr()
{
}

CPPAMacroMgr::~CPPAMacroMgr()
{
}

/*! �L�[�{�[�h�}�N���̎��s
	CMacro�ɈϏ��B
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	m_cPPA.Execute(pcEditView);
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const char* pszPath )
{
	FILE* hFile = fopen( pszPath, "r" );
	if( NULL == hFile ){
		m_nReady = FALSE;
		return FALSE;
	}

	CMemory cmemWork;

	// ��s���ǂ݂��݁A�R�����g�s��r��������ŁAmacro�R�}���h���쐬����B
	char	szLine[10240];	//	1�s��10240�ȏゾ�����疳�����ɃA�E�g
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		cmemWork.Append(szLine, nLineLen);
	}
	fclose( hFile );

	m_cPPA.SetSource( cmemWork.GetPtr2() );

	m_nReady = TRUE;
	return TRUE;
}


/*[EOF]*/
