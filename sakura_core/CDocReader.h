#pragma once

class CDocLineMgr;

//as decorator
class CDocReader{
public:
	CDocReader(const CDocLineMgr& pcDocLineMgr) : m_pcDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(int* pnDataLen);	/* 全行データを返す */
	const wchar_t* GetLineStr( CLogicInt , CLogicInt* );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt , int* ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const wchar_t* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */

private:
	const CDocLineMgr* m_pcDocLineMgr;
};
