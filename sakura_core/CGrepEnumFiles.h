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
#ifndef SAKURA_CGREPENUMFILES_0C041F0A_3D17_475F_BA25_26D30E055E9D_H_
#define SAKURA_CGREPENUMFILES_0C041F0A_3D17_475F_BA25_26D30E055E9D_H_
#pragma once

#include "CGrepEnumFileBase.h"

class CGrepEnumFiles : public CGrepEnumFileBase {
private:

public:
	CGrepEnumFiles(){
	}

	virtual ~CGrepEnumFiles(){
	}

	BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCWSTR pFile = NULL ) override {
		if( ! ( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ){
			if( CGrepEnumFileBase::IsValid( w32fd, pFile ) ){
				return TRUE;
			}
		}
		return FALSE;
	}
};
#endif /* SAKURA_CGREPENUMFILES_0C041F0A_3D17_475F_BA25_26D30E055E9D_H_ */
