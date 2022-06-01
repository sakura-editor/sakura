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
#ifndef SAKURA_CGREPENUMFILTERFOLDERS_4EF27C6D_F01D_495F_94A0_E78CC756B09E_H_
#define SAKURA_CGREPENUMFILTERFOLDERS_4EF27C6D_F01D_495F_94A0_E78CC756B09E_H_
#pragma once

#include "CGrepEnumFolders.h"

class CGrepEnumFilterFolders final : public CGrepEnumFolders {
private:

public:
	CGrepEnumFolders m_cGrepEnumExceptFolders;

public:
	CGrepEnumFilterFolders(){
	}

	virtual ~CGrepEnumFilterFolders(){
	}

	BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = NULL ) override{
		if( CGrepEnumFolders::IsValid( w32fd, pFile ) ){
			if( m_cGrepEnumExceptFolders.IsValid( w32fd, pFile ) ){
				return TRUE;
			}
		}
		return FALSE;
	}

	int Enumerates( LPCWSTR lpBaseFolder, CGrepEnumKeys& cGrepEnumKeys, CGrepEnumOptions option, CGrepEnumFolders& except ){
		m_cGrepEnumExceptFolders.Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecExceptFolderKeys, option, NULL );
		return CGrepEnumFolders::Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecSearchFolderKeys, option, &except );
	}
};
#endif /* SAKURA_CGREPENUMFILTERFOLDERS_4EF27C6D_F01D_495F_94A0_E78CC756B09E_H_ */
