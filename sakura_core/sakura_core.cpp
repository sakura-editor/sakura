//	$Id$
/*!	@file
	DLL �A�v���P�[�V�����p�̃G���g���|�C���g���`

	@author Norio Nakatani
	@date 1998/5/15 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_core.h"

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// ����̓G�N�X�|�[�g���ꂽ�ϐ��̗�ł��B
#ifndef __BORLANDC__
SAKURA_CORE_API int nSakura_core=0;
#else
SAKURA_CORE_API int nSakura_core;
#endif

// ����̓G�N�X�|�[�g���ꂽ�֐��̗�ł��B
SAKURA_CORE_API int fnSakura_core(void)
{
	return 42;
}

// ����̓G�N�X�|�[�g���ꂽ�N���X�̃R���X�g���N�^�ł��B
// �N���X�̒�`�ɂ��Ă� sakura_core.h ���Q�Ƃ��Ă��������B
CSakura_core::CSakura_core()
{
	return;
}


/*[EOF]*/
