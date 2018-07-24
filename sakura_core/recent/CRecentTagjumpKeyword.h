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
#ifndef SAKURA_CRECENTTAGJUMPKEYWORD_A97C71AE_DADC_47EA_B3A4_E3AAB4F6E217_H_
#define SAKURA_CRECENTTAGJUMPKEYWORD_A97C71AE_DADC_47EA_B3A4_E3AAB4F6E217_H_

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CTagjumpKeywordString;

//! タグジャンプキーワードの履歴を管理 (RECENT_FOR_TAGJUMP_KEYWORD)
class CRecentTagjumpKeyword : public CRecentImp<CTagjumpKeywordString, LPCWSTR>{
public:
	//生成
	CRecentTagjumpKeyword();

	//オーバーライド
	int				CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCWSTR* dst, const CTagjumpKeywordString* src ) const;
	bool			TextToDataType( CTagjumpKeywordString* dst, LPCTSTR pszText ) const;
	bool			ValidateReceiveType( LPCWSTR p ) const;
	size_t			GetTextMaxLength() const;
};

#endif /* SAKURA_CRECENTTAGJUMPKEYWORD_A97C71AE_DADC_47EA_B3A4_E3AAB4F6E217_H_ */
/*[EOF]*/
