//	$Id$
/************************************************************************

	COpeBlk.h

	�ҏW����v�f�u���b�N
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  �V�K�쐬

************************************************************************/

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

#include <windows.h>
#include "COpe.h"
#include "CLayoutMgr.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �ҏW����v�f�u���b�N�@COpeBlk */
class COpeBlk {
	public:
		COpeBlk();	/* COpeBlk�N���X�\�z */
		~COpeBlk();	/* COpeBlk�N���X���� */

		int GetNum( void ){ return m_nCOpeArrNum; };	/* ����̐���Ԃ� */
//		int AppendOpe( COpe*, CLayoutMgr* );	/* ����̒ǉ� */
		int AppendOpe( COpe* );	/* ����̒ǉ� */
		COpe* GetOpe( int );	/* �����Ԃ� */

		void DUMP( void );	/* �ҏW����v�f�u���b�N�̃_���v */
	private:
		void Init( void );
		void Empty( void );

		/* �f�[�^ */
		int		m_nCOpeArrNum;	/* ����̐� */
		COpe**	m_ppCOpeArr;	/* ����̔z�� */
};



//////////////////////////////////////////////////////////////////////12
#endif /* _COPEBLK_H_ */

/*[EOF]*/
