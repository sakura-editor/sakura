//	$Id$
/************************************************************************

	CDlgJump.h

    UPDATE: 1999.12/05  �č쐬
    CREATE: 1998.05/31  �V�K�쐬
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

class CDlgJump;

#ifndef _CDLGJUMP_H_
#define _CDLGJUMP_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgJump : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, BOOL );	/* ���[�_���_�C�A���O�̕\�� */

	int			m_nLineNum;		/* �s�ԍ� */
	BOOL		m_bPLSQL;		/* PL/SQL�\�[�X�̗L���s�� */
	int			m_nPLSQL_E1;
	int			m_nPLSQL_E2;
	BOOL		m_bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnNotify( WPARAM,  LPARAM );	//	Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGJUMP_H_ */

/*[EOF]*/
