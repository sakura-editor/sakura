//	$Id$
/************************************************************************

	CFuncInfo.cpp

	�A�E�g���C����́@�f�[�^�v�f
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  �V�K�쐬

************************************************************************/
#include "CFuncInfo.h"
#include <windows.h>
//#include <string.h>

/* CFuncInfo�N���X�\�z */
CFuncInfo::CFuncInfo(
	int		nFuncLineCRLF,	/* �֐��̂���s(CRLF�P��) */
	int		nFuncLineLAYOUT,/* �֐��̂���s(�܂�Ԃ��P��) */
//	int		nFuncLine,		/* �֐��̂���s */
	char*	pszFuncName,	/* �֐��� */
	int		nInfo			/* �t����� */
)
{
	m_nFuncLineCRLF = nFuncLineCRLF;	/* �֐��̂���s(CRLF�P��) */
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	/* �֐��̂���s(�܂�Ԃ��P��) */
//	m_nFuncLine = nFuncLine;;
//    m_cmemFuncName = pszFuncName;
//	m_cmemFuncName.SetData( pszFuncName, lstrlen( pszFuncName ) );
	m_cmemFuncName.SetDataSz( pszFuncName );

    m_nInfo = nInfo;
	return;
}




/* CFuncInfo�N���X���� */
CFuncInfo::~CFuncInfo()
{

}


