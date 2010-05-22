/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

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

