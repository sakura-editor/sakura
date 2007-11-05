/*!	@file
	@brief DLL�̃��[�h�A�A�����[�h

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta

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
#include "CDllHandler.h"

CDllHandler::CDllHandler()
	: m_hInstance( NULL )
{}

/*!
	�I�u�W�F�N�g���őO��DLL���ǂݍ��܂ꂽ��Ԃł����DLL�̉�����s���D
*/
CDllHandler::~CDllHandler()
{
	if( IsAvailable() ){
		FreeLibrary(true);
	}
}

int CDllHandler::LoadLibrary(LPCTSTR str)
{
	if( IsAvailable() ){
		//	���ɗ��p�\�ŗL��Ή������Ȃ��D
		return 0;
	}

	for( int i = 0; m_hInstance == NULL ; i++ ){
		LPCTSTR name = GetDllNameInOrder(str, i);
		if( name == NULL ){
			if( i == 0 ){
				//	1���ڂȂ�t�@�C�����ُ�
				return -1;
			}
			else {
				//	2���ڈȍ~�Ȃ烍�[�h���s
				return -2;
			}
		}

		m_hInstance = ::LoadLibrary( name );
	}

	int ret = InitDll();
	if( ret != 0 ){
		::FreeLibrary( m_hInstance );
		m_hInstance = NULL;
	}
	return ret;
}

int CDllHandler::FreeLibrary(bool force)
{
	if( m_hInstance == NULL || (!IsAvailable()) ){
		//	DLL���ǂݍ��܂�Ă��Ȃ���Ή������Ȃ�
		return 0;
	}

	int ret = DeinitDll();
	if( ret == 0 || force ){
		::FreeLibrary( m_hInstance );
		m_hInstance = NULL;
	}
	return ret;
}
/*!
	�������ȗ��ł���悤�ɂ��邽�߁A��̊֐���p�ӂ��Ă���
*/
int CDllHandler::DeinitDll(void)
{
	return 0;
}

/*!
	�e�[�u���ŗ^����ꂽ�G���g���|�C���^�A�h���X������ꏊ��
	�Ή����镶���񂩂璲�ׂ��G���g���|�C���^��ݒ肷��B
	
	@param table [in] ���O�ƃA�h���X�̑Ή��\�B�Ō��{NULL,0}�ŏI��邱�ƁB
	@retval true �S�ẴA�h���X���ݒ肳�ꂽ�B
	@retval false �A�h���X�̎擾�Ɏ��s�����֐����������B
*/
bool CDllHandler::RegisterEntries(const ImportTable table[])
{
	int i;
	for (i=0; table[i].proc!=NULL; i++) 
	{
		FARPROC proc;
		if ((proc = ::GetProcAddress(GetInstance(), table[i].name)) == NULL) 
		{
			return false;
		}
		*((FARPROC*)table[i].proc) = proc;
	}
	return true;
}

LPCTSTR CDllHandler::GetDllName(LPCTSTR)
{
	return NULL;
}

LPCTSTR CDllHandler::GetDllNameInOrder(LPCTSTR str, int index)
{
	if( index == 0 ){
		return GetDllName(str);
	}
	return NULL;
}
