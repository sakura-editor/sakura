/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
#ifndef SAKURA_CRECENTExcludeFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_
#define SAKURA_CRECENTExcludeFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, MAX_EXCLUDE_PATH> CExcludeFolderString;

//! Excludeフォルダの履歴を管理 (RECENT_FOR_Exclude_FOLDER)
class CRecentExcludeFolder : public CRecentImp<CExcludeFolderString, LPCTSTR>{
public:
	//生成
	CRecentExcludeFolder();

	//オーバーライド
	int				CompareItem( const CExcludeFolderString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CExcludeFolderString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CExcludeFolderString* src ) const;
	bool			TextToDataType( CExcludeFolderString* dst, LPCTSTR pszText ) const;
	bool			ValidateReceiveType( LPCTSTR p ) const;
	size_t			GetTextMaxLength() const;
};

#endif /* SAKURA_CRECENTExcludeFOLDER_6162D952_F009_44DB_9C13_80E73507D8E7_H_ */
/*[EOF]*/
