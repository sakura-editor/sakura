//	$Id$
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

#ifndef _SAKURA_MUTIMONITOR_H_
#define _SAKURA_MUTIMONITOR_H_

#include "CDllHandler.h"

/*!
	@brief �}���`���j�^�֐����I���[�h

	�}���`���j�^�֘A���\�b�h�̓��I���[�h�ƃ��j�^�֘A�֐�

	@note ���j�^�̐��̓G�f�B�^���s���ɕω�����\��������D

*/
class SAKURA_CORE_API CMultiMonitor : public CDllHandler {
	enum MonDllState { Munavailable = 0, Mavailable = 1, Munknown = 2};

	//!	���x��DLL��ǂݍ������Ƃ��Ȃ��悤�ɂ��邽�߂̃t���O
	MonDllState		m_bEnabled;

public:
	CMultiMonitor() : m_bEnabled( Munknown ){};
	virtual ~CMultiMonitor();

	bool GetMonitorWorkRect(HWND, LPRECT);

	/*!
		Init��1�񂾂����s���鏉�����֐�

		Init�̎��s���ʂ�m_bEnabled�ɕۑ����C2��ڈȍ~�͂����Ԃ��D
	*/
	bool MyInit(void) {
		if( m_bEnabled != Munknown )
			return (m_bEnabled == Mavailable);
		
		if( Init() ){
			m_bEnabled = Mavailable;
			return true;
		}
		else {
			m_bEnabled = Munavailable;
			return false;
		}
	}

protected:
	//	Entry Point
	typedef HMONITOR (WINAPI* Proc_MonitorFromWindow)(HWND, DWORD);
	typedef BOOL (WINAPI* Proc_GetMonitorInfo)(HMONITOR, LPMONITORINFO);
	
	Proc_MonitorFromWindow	m_MonitorFromWindow;
	Proc_GetMonitorInfo		m_GetMonitorInfo;

	virtual int InitDll(void);
	virtual char* GetDllName(char *);

};

#endif


/*[EOF]*/
