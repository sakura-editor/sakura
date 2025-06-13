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
