/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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

#ifndef	_CRECENTTAGJUMPKEYWORD_H_
#define	_CRECENTTAGJUMPKEYWORD_H_

#include "global.h"
#include "CRecent.h"

//! タグジャンプキーワードの履歴を管理 (RECENT_FOR_TAGJUMP_KEYWORD)
class CRecentTagjumpKeyword : public CRecent
{
public:
	CRecentTagjumpKeyword();
	int FindItem( const char *pszItemData ) const;
};

#endif	//_CRECENTTAGJUMPKEYWORD_H_

/*[EOF]*/
