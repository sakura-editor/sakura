/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author YAZAKI
	@date 2002�N1��26��
*/
/*
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CPPAMacroMgr.h"
#include "mem/CMemory.h"
#include "CMacroFactory.h"
#include <string.h>
#include "io/CTextStream.h"
using namespace std;

CPPA CPPAMacroMgr::m_cPPA;

CPPAMacroMgr::CPPAMacroMgr()
{
}

CPPAMacroMgr::~CPPAMacroMgr()
{
}

/** PPA�}�N���̎��s

	PPA.DLL�ɁA�o�b�t�@���e��n���Ď��s�B

	@date 2007.07.20 genta flags�ǉ�
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	m_cPPA.SetSource( to_achar(m_cBuffer.GetStringPtr()) );
	m_cPPA.Execute(pcEditView, flags);
}

/*! �L�[�{�[�h�}�N���̓ǂݍ��݁i�t�@�C������j
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath )
{
	CTextInputStream in( pszPath );
	if(!in){
		m_nReady = false;
		return FALSE;
	}

	CNativeW cmemWork;

	// �o�b�t�@�icmemWork�j�Ƀt�@�C�����e��ǂݍ��݁Am_cPPA�ɓn���B
	while( in ){
		wstring szLine = in.ReadLineW();
		szLine += L"\n";
		cmemWork.AppendString(szLine.c_str());
	}
	in.Close();

	m_cBuffer.SetNativeData( cmemWork );	//	m_cBuffer�ɃR�s�[

	m_nReady = true;
	return TRUE;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ��݁i�����񂩂�j
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CPPAMacroMgr::LoadKeyMacroStr( HINSTANCE hInstance, const TCHAR* pszCode )
{
	m_cBuffer.SetNativeData( to_wchar( pszCode ) );	//	m_cBuffer�ɃR�s�[

	m_nReady = true;
	return TRUE;
}

//	From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
		���̂��߁C�߂����I�u�W�F�N�g�������s��Ȃ����߂Ɋg���q�`�F�b�N�͕K�{�D

*/
CMacroManagerBase* CPPAMacroMgr::Creator(const TCHAR* ext)
{
	if( _tcscmp( ext, _T("ppa") ) == 0 ){
		return new CPPAMacroMgr;
	}
	return NULL;
}

/*!	CPPAMacroManager�̓o�^

	PPA�����p�ł��Ȃ��Ƃ��͉������Ȃ��B

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
*/
void CPPAMacroMgr::declare (void)
{
	if( DLL_SUCCESS == m_cPPA.InitDll() ){
		CMacroFactory::getInstance()->RegisterCreator( Creator );
	}
}
//	To Here Apr. 29, 2002 genta


