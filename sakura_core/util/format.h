// 2007.10.20 kobake 書式関連

#pragma once


// 20051121 aroka
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const TCHAR* ver );	//バージョン番号の解析
int CompareVersion( const TCHAR* verA, const TCHAR* verB );	//バージョン番号の比較
