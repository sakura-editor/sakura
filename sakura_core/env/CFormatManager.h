/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

//!�����Ǘ�
class CFormatManager{
public:
	CFormatManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//���� //@@@ 2002.2.9 YAZAKI
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, const TCHAR* szDateFormat );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, const TCHAR* szTimeFormat );
private:
	DLLSHAREDATA* m_pShareData;
};

