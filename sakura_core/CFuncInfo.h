//	$Id$
/************************************************************************

	CFuncInfo.h

	�A�E�g���C����́@�f�[�^�v�f
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  �V�K�쐬

************************************************************************/

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �A���h�D�E���h�D�o�b�t�@ */
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfo�N���X�\�z */
		~CFuncInfo();	/* CFuncInfo�N���X���� */

//	private:
		int			m_nFuncLineCRLF;	/* �֐��̂���s(CRLF�P��) */
		int			m_nFuncLineLAYOUT;	/* �֐��̂���s(�܂�Ԃ��P��) */
		CMemory		m_cmemFuncName;	/* �֐��� */
		int			m_nInfo;		/* �t����� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */

/*[EOF]*/
