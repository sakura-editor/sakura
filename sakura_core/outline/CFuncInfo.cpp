/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CFuncInfo.h"

/*! CFuncInfo�N���X�\�z */
CFuncInfo::CFuncInfo(
	CLogicInt		nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	CLogicInt		nFuncColCRLF,		//!< �֐��̂��錅(CRLF�P��)
	CLayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	CLayoutInt		nFuncColLAYOUT,		//!< �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	int				nInfo				//!< �t�����
)
: m_nDepth(0) // �[��
{
	m_nFuncLineCRLF = nFuncLineCRLF;		/* �֐��̂���s(CRLF�P��) */
	m_nFuncColCRLF = nFuncColCRLF;			/* �֐��̂��錅(CRLF�P��) */
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	/* �֐��̂���s(�܂�Ԃ��P��) */
	m_nFuncColLAYOUT = nFuncColLAYOUT;		/* �֐��̂��錅(�܂�Ԃ��P��) */
	m_cmemFuncName.SetString( pszFuncName );

	m_nInfo = nInfo;
	return;
}




/* CFuncInfo�N���X���� */
CFuncInfo::~CFuncInfo()
{

}



