//	$Id$
/************************************************************************

	CDlgPrintPage.h
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
#include "CDialog.h"

#ifndef _CDLGPRINTPAGE_H_
#define _CDLGPRINTPAGE_H_

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgPrintPage : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgPrintPage();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */


	int	m_nPageMin;
	int	m_nPageMax;

	BOOL m_bAllPage;
	int	m_nPageFrom;
	int	m_nPageTo;



protected:
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTPAGE_H_ */

/*[EOF]*/
