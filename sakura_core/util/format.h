// 2007.10.20 kobake �����֘A

#pragma once


// 20051121 aroka
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const TCHAR* ver );	//�o�[�W�����ԍ��̉��
int CompareVersion( const TCHAR* verA, const TCHAR* verB );	//�o�[�W�����ԍ��̔�r
