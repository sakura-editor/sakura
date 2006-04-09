/*!	@file
	@brief �}���`���j�^�֐����I���[�h

	�}���`���j�^�֐��Q�ւ̓��I�A�N�Z�X�N���X

	@author genta
	@date May 01, 2004
*/
/*
	Copyright (C) 2004, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "stdafx.h"
#include <windows.h>
#include "CMultiMonitor.h"
#include "debug.h"

//-----------------------------------------
//	DLL �������֐�
//-----------------------------------------

int CMultiMonitor::InitDll(void)
{
	//	static�ɂ��Ă͂����Ȃ�
	//	�G���g���[����Windows SDK��MultiMon.h���Q��
	
	const ImportTable table[] = {
		&m_MonitorFromWindow, "MonitorFromWindow",
		&m_GetMonitorInfo, "GetMonitorInfoA",
		NULL, 0
	};

	if( ! RegisterEntries(table) )
		return 1;

	return 0;
}

char* CMultiMonitor::GetDllName(char *)
{
	return "USER32";
}

CMultiMonitor::~CMultiMonitor()
{
}

//-----------------------------------------
//	Monitor���擾�֐�
//-----------------------------------------
/*!
	�w�肵���E�B���h�E�̂��郂�j�^�̍�Ɨ̈�𓾂�

	@param hWnd [in] ���j�^���w�肷�邽�߂̃E�B���h�E
	@param rcDesktop [out]	���j�^�͈̔�
	
	@return true:�v���C�}�����j�^ / false: ����ȊO�̃��j�^
*/
bool CMultiMonitor::GetMonitorWorkRect(HWND hWnd, LPRECT rcDesktop)
{
	if( MyInit() ){	//	�}���`���j�^�[�T�|�[�g�L��
		HMONITOR hMon = this->m_MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
		
		if( hMon != NULL ){
			MONITORINFO mi;
			::ZeroMemory( &mi, sizeof( mi ));
			mi.cbSize = sizeof( mi );

			if( this->m_GetMonitorInfo( hMon, &mi ) != 0 ){
				*rcDesktop = mi.rcWork;
				return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
			}
			
		}
	}
	//	�}���`���j�^�[�T�|�[�g�����D�܂��̓}���`���j�^�ł��܂������Ȃ������ꍇ�D
	::SystemParametersInfo( SPI_GETWORKAREA, NULL, rcDesktop, 0 );
	return true;
}
