//	$Id$
/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f

	@author Norio Nakatani
	@date	1998/06/23 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"

//! �A�E�g���C�����  �f�[�^�v�f
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfo�N���X�\�z */
		~CFuncInfo();	/* CFuncInfo�N���X���� */

//	private:
		int			m_nFuncLineCRLF;	/*!< �֐��̂���s(CRLF�P��) */
		int			m_nFuncLineLAYOUT;	/*!< �֐��̂���s(�܂�Ԃ��P��) */
		CMemory		m_cmemFuncName;	/*!< �֐��� */
		int			m_nInfo;		/*!< �t����� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */


/*[EOF]*/
