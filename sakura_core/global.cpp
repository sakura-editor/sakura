/*!	@file
	@brief �����񋤒ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "global.h"
#include "window/CEditWnd.h"
#include "CNormalProcess.h"

//2007.10.02 kobake CEditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
CEditWnd* g_pcEditWnd = NULL;


/*! �I��̈�`��p�p�����[�^ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;






HWND G_GetProgressBar()
{
	if(CEditWnd::Instance()){
		return CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	}
	else{
		return NULL;
	}
}

HINSTANCE G_AppInstance()
{
	return CProcess::Instance()->GetProcessInstance();
}
