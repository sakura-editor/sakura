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
#ifndef SAKURA_CRECENTFOLDER_A671E5A1_CE40_4BEF_BA37_B468B056F081_H_
#define SAKURA_CRECENTFOLDER_A671E5A1_CE40_4BEF_BA37_B468B056F081_H_

#include "CRecentImp.h"
#include "util/StaticType.h"

//StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>

typedef StaticString<TCHAR, _MAX_PATH> CPathString;

//! �t�H���_�̗������Ǘ� (RECENT_FOR_FOLDER)
class CRecentFolder : public CRecentImp<CPathString, LPCTSTR>{
public:
	//����
	CRecentFolder();

	//�I�[�o�[���C�h
	int				CompareItem( const CPathString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CPathString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CPathString* src ) const;
	bool			TextToDataType( CPathString* dst, LPCTSTR pszText ) const;
};

#endif /* SAKURA_CRECENTFOLDER_A671E5A1_CE40_4BEF_BA37_B468B056F081_H_ */
/*[EOF]*/
