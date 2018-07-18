/*
	2008.05.18 kobake CShareData から分離
*/
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
#ifndef SAKURA_CFORMATMANAGER_7FC73E49_281C_4417_BBBC_DE176142E6BC9_H_
#define SAKURA_CFORMATMANAGER_7FC73E49_281C_4417_BBBC_DE176142E6BC9_H_

// 要先行定義
// #include "DLLSHAREDATA.h"

//!書式管理
class CFormatManager{
public:
	CFormatManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//書式 //@@@ 2002.2.9 YAZAKI
	// 共有DLLSHAREDATA依存
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );

	// 共有DLLSHAREDATA非依存
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, const TCHAR* szDateFormat );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, const TCHAR* szTimeFormat );
private:
	DLLSHAREDATA* m_pShareData;
};


#endif /* SAKURA_CFORMATMANAGER_7FC73E49_281C_4417_BBBC_DE176142E6BC9_H_ */
/*[EOF]*/
