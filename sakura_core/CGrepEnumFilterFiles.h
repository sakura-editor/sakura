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
#ifndef SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_
#define SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_
#pragma once

#include "CGrepEnumFiles.h"

class CGrepEnumFilterFiles final : public CGrepEnumFiles {
private:

public:
	CGrepEnumFiles m_cGrepEnumExceptFiles;

public:
	CGrepEnumFilterFiles(){
	}

	virtual ~CGrepEnumFilterFiles(){
	}

	BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = NULL  ) override {
		if( CGrepEnumFiles::IsValid( w32fd, pFile ) ){
			if( m_cGrepEnumExceptFiles.IsValid( w32fd, pFile ) ){
				return TRUE;
			}
		}
		return FALSE;
	}

	int Enumerates( LPCWSTR lpBaseFolder, CGrepEnumKeys& cGrepEnumKeys, CGrepEnumOptions option, CGrepEnumFiles& pExcept ){
		m_cGrepEnumExceptFiles.Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecExceptFileKeys, option, NULL );
		return CGrepEnumFiles::Enumerates( lpBaseFolder, cGrepEnumKeys.m_vecSearchFileKeys, option, &pExcept );
	}
};
#endif /* SAKURA_CGREPENUMFILTERFILES_48762BD9_D3E4_4D32_8C3A_502A7A6AE85A_H_ */
