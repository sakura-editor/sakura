/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CRECENTGREPFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_
#define SAKURA_CRECENTGREPFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, MAX_GREP_PATH> CGrepFolderString;

//! GREPフォルダの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder : public CRecentImp<CGrepFolderString, LPCTSTR>{
public:
	//生成
	CRecentGrepFolder();

	//オーバーライド
	int				CompareItem( const CGrepFolderString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFolderString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CGrepFolderString* src ) const;
	bool			TextToDataType( CGrepFolderString* dst, LPCTSTR pszText ) const;
	bool			ValidateReceiveType( LPCTSTR p ) const;
	size_t			GetTextMaxLength() const;
};

#endif /* SAKURA_CRECENTGREPFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_ */
/*[EOF]*/
