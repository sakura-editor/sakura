//	$Id$
/*!	@file
	@brief DLLのロード、アンロード

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

#include "CDllHandler.h"

CDllHandler::CDllHandler()
	: m_hInstance( NULL )
{}

/*!
	オブジェクト消滅前にDLLが読み込まれた状態であればDLLの解放を行う．
*/
CDllHandler::~CDllHandler()
{
	if( IsAvailable() ){
		FreeLibrary(true);
	}
}

int CDllHandler::LoadLibrary(char* str)
{
	if( IsAvailable() ){
		//	既に利用可能で有れば何もしない．
		return 0;
	}

	char *name = GetDllName(str);
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
	if( m_hInstance == NULL || (!IsAvailable()) ){
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

/*!
	テーブルで与えられたエントリポインタアドレスを入れる場所に
	対応する文字列から調べたエントリポインタを設定する。
	
	@param table [in] 名前とアドレスの対応表。最後は{NULL,0}で終わること。
	@retval true 全てのアドレスが設定された。
	@retval false アドレスの取得に失敗した関数があった。
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

/*[EOF]*/
