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
#ifndef SAKURA_CRECENTGREPFILE_6DFF8FB2_B7D0_4828_8191_744A9580C467_H_
#define SAKURA_CRECENTGREPFILE_6DFF8FB2_B7D0_4828_8191_744A9580C467_H_

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, MAX_GREP_PATH> CGrepFileString;

//! GREPファイルの履歴を管理 (RECENT_FOR_GREP_FILE)
class CRecentGrepFile : public CRecentImp<CGrepFileString, LPCTSTR>{
public:
	//生成
	CRecentGrepFile();

	//オーバーライド
	int				CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFileString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CGrepFileString* src ) const;
	bool			TextToDataType( CGrepFileString* dst, LPCTSTR pszText ) const;
	bool			ValidateReceiveType( LPCTSTR p ) const;
	size_t			GetTextMaxLength() const;
};

#endif /* SAKURA_CRECENTGREPFILE_6DFF8FB2_B7D0_4828_8191_744A9580C467_H_ */
/*[EOF]*/
