/*!	@file
	@brief BASE64 Decode

	@author 
*/

/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "convert/CDecode_Base64Decode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "mem/CNativeW.h"

/* Base64デコード */
bool CDecode_Base64Decode::DoDecode( const CNativeW& pcSrc, CMemory* pcDst )
{
	using namespace WCODE;

	const int BUFFER_SIZE = 1024;  // バッファサイズ。１以上の整数かつ４の倍数で。
	const int _BUFSIZE = ((BUFFER_SIZE+3)/4)*4;

	const wchar_t *pSrc;
	int nSrcLen;
	char *pw, *pw_base;
	wchar_t buffer[_BUFSIZE];
	int i, j;
	wchar_t c = 0;

	pSrc = pcSrc.GetStringPtr();
	nSrcLen = pcSrc.GetStringLength();
	pcDst->AllocBuffer( nSrcLen );  // 書き込みバッファを確保
	pw_base = pw = reinterpret_cast<char *>(pcDst->GetRawPtr());

	i = 0;  // pcSrc の添え字
	do{
		j = 0;
		for( ; i < nSrcLen; ++i ){
		// バッファに文字をためるループ
			c = pSrc[i];
			if( IsLineDelimiterBasic(c) || c == TAB || c == SPACE ){
				continue;
			}
			if( j == _BUFSIZE || c == LTEXT('=') ){
				break;
			}
			if( !IsBase64(c) ){
				return false;
			}
			buffer[j] = static_cast<char>(c & 0xff);
			++j;
		}
		pw += _DecodeBase64( &buffer[0], j, pw );
	}while( i < nSrcLen && c != LTEXT('=') );

	//if( CheckBase64Padbit(&buffer[0], j) == false ){
	//	return false;
	//}

	pcDst->_SetRawLength( pw - pw_base );
	return true;
}
