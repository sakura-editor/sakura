/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
