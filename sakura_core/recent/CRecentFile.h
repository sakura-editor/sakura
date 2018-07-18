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
#ifndef SAKURA_CRECENTFILE_EE3F27C6_A91E_426D_8EB9_2E35D191F4199_H_
#define SAKURA_CRECENTFILE_EE3F27C6_A91E_426D_8EB9_2E35D191F4199_H_

#include "CRecentImp.h"
#include "EditInfo.h" //EditInfo

//! EditInfoの履歴を管理 (RECENT_FOR_FILE)
class CRecentFile : public CRecentImp<EditInfo>{
public:
	//生成
	CRecentFile();

	//オーバーライド
	int				CompareItem( const EditInfo* p1, const EditInfo* p2 ) const;
	void			CopyItem( EditInfo* dst, const EditInfo* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( const EditInfo** dst, const EditInfo* src ) const;
	bool			TextToDataType( EditInfo* dst, LPCTSTR pszText ) const;
	bool			ValidateReceiveType( const EditInfo* ) const;
	size_t			GetTextMaxLength() const;
	//固有インターフェース
	int FindItemByPath(const TCHAR* pszPath) const;
};

#endif /* SAKURA_CRECENTFILE_EE3F27C6_A91E_426D_8EB9_2E35D191F4199_H_ */
/*[EOF]*/
