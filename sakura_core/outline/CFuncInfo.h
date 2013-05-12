/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "mem/CMemory.h"

// CDlgFuncList::SetTree()�p m_Info
#define FUNCINFO_INFOMASK	0xFFFF
//	2003.06.27 Moca
#define FUNCINFO_NOCLIPTEXT 0x10000


//! �A�E�g���C�����  �f�[�^�v�f
//@date 2002.04.01 YAZAKI �[������
class CFuncInfo {
	public:
		CFuncInfo( CLogicInt, CLogicInt, CLayoutInt, CLayoutInt, const TCHAR*, int );	/* CFuncInfo�N���X�\�z */
		~CFuncInfo();	/* CFuncInfo�N���X���� */

		//! �N���b�v�{�[�h�ɒǉ�����v�f���H
		//	2003.06.27 Moca
		inline bool IsAddClipText( void ) const{
			return ( FUNCINFO_NOCLIPTEXT != ( m_nInfo & FUNCINFO_NOCLIPTEXT ) );
		}

//	private:
		CLogicInt	m_nFuncLineCRLF;	/*!< �֐��̂���s(CRLF�P��) */
		CLayoutInt	m_nFuncLineLAYOUT;	/*!< �֐��̂���s(�܂�Ԃ��P��) */
		CLogicInt	m_nFuncColCRLF;		/*!< �֐��̂��錅(CRLF�P��) */
		CLayoutInt	m_nFuncColLAYOUT;	/*!< �֐��̂��錅(�܂�Ԃ��P��) */
		CNativeT	m_cmemFuncName;	/*!< �֐��� */
		int			m_nInfo;		/*!< �t����� */
		int			m_nDepth;		/*!< �[�� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */



