//	$Id$
/*!	@file
	@brief �o�[�W�������_�C�A���O

	@author Norio Nakatani
	@date 1998/05/22 �쐬
	@date 1999/12/05 �č쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "CDialog.h"
/*!
	@brief About Box�Ǘ�
	
	DispatchEvent��Ǝ��ɒ�`���邱�ƂŁCCDialog�ŃT�|�[�g����Ă��Ȃ�
	���b�Z�[�W��ߑ�����D
*/
class CDlgAbout : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND );	/* ���[�_���_�C�A���O�̕\�� */
	//	Nov. 7, 2000 genta	�W���ȊO�̃��b�Z�[�W��ߑ�����
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
private:
	int	 nCursorState;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */


/*[EOF]*/
