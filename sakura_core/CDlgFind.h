//	$Id$
/************************************************************************

	CDlgFind.h

	1999.12.2  �č쐬 
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
#include "CDialog.h"

#ifndef _CDLGFIND_H_
#define _CDLGFIND_H_


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, LPARAM );	/* ���[�h���X�_�C�A���O�̕\�� */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	void ChangeView( LPARAM );

	int		m_bLoHiCase;	/* �p�啶���Ɖp����������ʂ��� */
	int		m_bWordOnly;	/* ��v����P��̂݌������� */
	int		m_bRegularExp;	/* ���K�\�� */
	int		m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	char	m_szText[_MAX_PATH + 1];	/* ���������� */


protected:
	void AddToSearchKeyArr( const char* );
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );
	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFIND_H_ */

/*[EOF]*/
