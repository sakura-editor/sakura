//	$Id$
/************************************************************************

	CDlgAbout.h

    �o�[�W�������_�C�A���O

    UPDATE: 1999.12/05 �č쐬
    CREATE: 1998.05/22 �V�K�쐬
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgAbout;

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgAbout : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND );	/* ���[�_���_�C�A���O�̕\�� */
	//	Nov. 7, 2000 genta	�W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
private:
	int	 nCursorState;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */

/*[EOF]*/
