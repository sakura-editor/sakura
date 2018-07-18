/*
	Copyright (C) 2007, kobake

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

#include "StdAfx.h"
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

/*!	バージョン番号の解析

	@param[in] バージョン番号文字列
	@return UINT32 8bit（符号1bit+数値7bit）ずつメジャー、マイナー、ビルド、リビジョンを格納

	@author syat
	@date 2011.03.18 新規
	@note 参考 PHP version_compare http://php.s3.to/man/function.version-compare.html
*/
UINT32 ParseVersion( const TCHAR* sVer )
{
	int nVer;
	int nShift = 0;	//特別な文字列による下駄
	int nDigit = 0;	//連続する数字の数
	UINT32 ret = 0;

	const TCHAR *p = sVer;
	int i;

	for( i=0; *p && i<4; i++){
		//特別な文字列の処理
		if( *p == _T('a') ){
			if( _tcsncmp( _T("alpha"), p, 5 ) == 0 )p += 5;
			else p++;
			nShift = -0x60;
		}
		else if( *p == _T('b') ){
			if( _tcsncmp( _T("beta"), p, 4 ) == 0 )p += 4;
			else p++;
			nShift = -0x40;
		}
		else if( *p == _T('r') || *p == _T('R') ){
			if( _tcsnicmp( _T("rc"), p, 2 ) == 0 )p += 2;
			else p++;
			nShift = -0x20;
		}
		else if( *p == _T('p') ){
			if( _tcsncmp( _T("pl"), p, 2 ) == 0 )p += 2;
			else p++;
			nShift = 0x20;
		}
		else if( !_istdigit(*p) ){
			nShift = -0x80;
		}
		else{
			nShift = 0;
		}
		while( *p && !_istdigit(*p) ){ p++; }
		//数値の抽出
		for( nVer = 0, nDigit = 0; _istdigit(*p); p++ ){
			if( ++nDigit > 2 )break;	//数字は2桁までで止める
			nVer = nVer * 10 + *p - _T('0');
		}
		//区切り文字の処理
		while( *p && _tcschr( _T(".-_+"), *p ) ){ p++; }

		DEBUG_TRACE(_T("  VersionPart%d: ver=%d,shift=%d\n"), i, nVer, nShift);
		ret |= ( (nShift + nVer + 128) << (24-8*i) );
	}
	for( ; i<4; i++ ){	//残りの部分はsigned 0 (=0x80)を埋める
		ret |= ( 128 << (24-8*i) );
	}

#ifdef _UNICODE
	DEBUG_TRACE(_T("ParseVersion %ls -> %08x\n"), sVer, ret);
#endif
	return ret;
}

/*!	バージョン番号の比較

	@param[in] バージョンA
	@param[in] バージョンB
	@return int 0: バージョンは等しい、1以上: Aが新しい、-1以下: Bが新しい

	@author syat
	@date 2011.03.18 新規
*/
int CompareVersion( const TCHAR* verA, const TCHAR* verB )
{
	UINT32 nVerA = ParseVersion(verA);
	UINT32 nVerB = ParseVersion(verB);

	return nVerA - nVerB;
}
