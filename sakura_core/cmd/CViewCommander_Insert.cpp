/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�}���n)�֐��Q

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "dlg/CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "env/CFormatManager.h"

//���t�}��
void CViewCommander::Command_INS_DATE( void )
{
	// ���t���t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT( true, to_wchar(szText), CLogicInt(-1), TRUE );
}



//�����}��
void CViewCommander::Command_INS_TIME( void )
{
	// �������t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT( true, to_wchar(szText), CLogicInt(-1), TRUE );
}



//	from CViewCommander_New.cpp
/*!	�R���g���[���R�[�h�̓���(�_�C�A���O)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//�R���g���[���R�[�h���̓_�C�A���O��\������
	if( cDlgCtrlCode.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		//�R���g���[���R�[�h����͂���
		Command_WCHAR( cDlgCtrlCode.GetCharCode() );
	}
}
