//	$Id$
/*!	@file
	DLL�̃��[�h�A�A�����[�h

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
	
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

#include "CDllHandler.h"

CDllHandler::CDllHandler()
	: m_hInstance( NULL )
{}

/*!
	�I�u�W�F�N�g���őO��DLL���ǂݍ��܂ꂽ��Ԃł����DLL�̉�����s���D
*/
CDllHandler::~CDllHandler()
{
	if( IsAvailable()){
		FreeLibrary(true);
	}
}

int CDllHandler::LoadLibrary(void)
{
	if( IsAvailable()){
		//	���ɗ��p�\�ŗL��Ή������Ȃ��D
		return 0;
	}

	char *name = GetDllName();
	if( name == NULL )	return -1;
	
	m_hInstance = ::LoadLibrary( name );
	if( m_hInstance == NULL )	return -2;

	int ret = InitDll();
	if( ret != 0 ){
		::FreeLibrary( m_hInstance );
		m_hInstance = NULL;
	}
	return ret;
}

int CDllHandler::FreeLibrary(bool force)
{
	if( m_hInstance == NULL || (!IsAvailable())){
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
