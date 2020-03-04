/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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

#include "StdAfx.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include "CRipgrep.h"
#include "charset/charcode.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


CRipgrep::CRipgrep()
{
}

CRipgrep::~CRipgrep()
{
}


//rg.exeの存在チェック
bool ExistRipgrep()
{
	WCHAR cmdline[1024];
	WCHAR szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, RIPGREP_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	return ( (DWORD)-1 != ::GetFileAttributes( cmdline ) );
}
