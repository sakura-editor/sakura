//	$Id$
// sakura_core.cpp : DLL �A�v���P�[�V�����p�̃G���g�� �|�C���g���`���܂��B
//	Copyright (C) 1998-2000, Norio Nakatani

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
SAKURA_CORE_API int nSakura_core=0;

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

