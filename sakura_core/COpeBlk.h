//	$Id$
/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

//#include <windows.h>
#include "COpe.h"
//#include "CLayoutMgr.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �ҏW����v�f�u���b�N
	
	COpe �𕡐����˂邽�߂̂��́BUndo, Redo�͂��̃u���b�N�P�ʂōs����B
*/
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
