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
	PPA.DLL�ɁA�o�b�t�@���e��n���Ď��s�B
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	m_cPPA.SetSource( m_cBuffer.GetPtr2() );
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

	// �o�b�t�@�icmemWork�j�Ƀt�@�C�����e��ǂݍ��݁Am_cPPA�ɓn���B
	char	szLine[10240];	//	1�s��10240�ȏゾ�����疳�����ɃA�E�g
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		cmemWork.Append(szLine, nLineLen);
	}
	fclose( hFile );

	m_cBuffer.SetData( &cmemWork );	//	m_cBuffer�ɃR�s�[

	m_nReady = TRUE;
	return TRUE;
}


/*[EOF]*/
