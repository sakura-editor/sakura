//	$Id$
/*!	@file
	DLLのロード、アンロード

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
	オブジェクト消滅前にDLLが読み込まれた状態であればDLLの解放を行う．
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
		//	既に利用可能で有れば何もしない．
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
		//	DLLが読み込まれていなければ何もしない
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
	実装を省略できるようにするため、空の関数を用意しておく
*/
int CDllHandler::DeinitDll(void)
{
	return 0;
}
