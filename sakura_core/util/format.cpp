#include "stdafx.h"
#include "format.h"


/*!	日時をフォーマット

	@param[out] 書式変換後の文字列
	@param[in] バッファサイズ
	@param[in] format 書式
	@param[in] systime 書式化したい日時
	@return bool true

	@note  %Y %y %m %d %H %M %S の変換に対応

	@author aroka
	@date 2005.11.21 新規
	
	@todo 出力バッファのサイズチェックを行う
*/
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime )
{
	TCHAR szTime[10];
	const TCHAR *p = format;
	TCHAR *q = szResult;
	int len;
	
	while( *p ){
		if( *p == _T('%') ){
			++p;
			switch(*p){
			case _T('Y'):
				len = wsprintf(szTime,_T("%d"),systime.wYear);
				_tcscpy( q, szTime );
				break;
			case _T('y'):
				len = wsprintf(szTime,_T("%02d"),(systime.wYear%100));
				_tcscpy( q, szTime );
				break;
			case _T('m'):
				len = wsprintf(szTime,_T("%02d"),systime.wMonth);
				_tcscpy( q, szTime );
				break;
			case _T('d'):
				len = wsprintf(szTime,_T("%02d"),systime.wDay);
				_tcscpy( q, szTime );
				break;
			case _T('H'):
				len = wsprintf(szTime,_T("%02d"),systime.wHour);
				_tcscpy( q, szTime );
				break;
			case _T('M'):
				len = wsprintf(szTime,_T("%02d"),systime.wMinute);
				_tcscpy( q, szTime );
				break;
			case _T('S'):
				len = wsprintf(szTime,_T("%02d"),systime.wSecond);
				_tcscpy( q, szTime );
				break;
				// A Z
			case _T('%'):
			default:
				*q = *p;
				len = 1;
				break;
			}
			q+=len;//q += strlen(szTime);
			++p;
			
		}
		else{
			*q = *p;
			q++;
			p++;
		}
	}
	*q = *p;
	return true;
}
