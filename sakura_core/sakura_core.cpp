//	$Id$
/*!	@file
	DLL �A�v���P�[�V�����p�̃G���g���|�C���g���`

	@author Norio Nakatani
	@date 1998/5/15 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
