﻿/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CGREPENUMFOLDERS_78931E72_6F22_4576_B760_34E2F6C87786_H_
#define SAKURA_CGREPENUMFOLDERS_78931E72_6F22_4576_B760_34E2F6C87786_H_
#pragma once

#include "CGrepEnumFileBase.h"

class CGrepEnumFolders : public CGrepEnumFileBase {
private:

public:
	CGrepEnumFolders(){
	}

	virtual ~CGrepEnumFolders(){
	}

	virtual BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = NULL ){
		if( ( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		 && ( 0 != wcscmp( w32fd.cFileName, L"." ) )
		 && ( 0 != wcscmp( w32fd.cFileName, L".." ) ) ){
			if( CGrepEnumFileBase::IsValid( w32fd, pFile ) ){
				return TRUE;
			}
		}
		return FALSE;
	}
};
#endif /* SAKURA_CGREPENUMFOLDERS_78931E72_6F22_4576_B760_34E2F6C87786_H_ */
