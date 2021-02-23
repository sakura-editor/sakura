/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "CDecode_UuDecode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "util/string_ex2.h"
#include "CEol.h"

/* Uudecode (デコード）*/
bool CDecode_UuDecode::DoDecode( const CNativeW& pcSrc, CMemory* pcDst )
{
	const WCHAR *psrc, *pline;
	int nsrclen;
	char *pw, *pw_base;
	int nlinelen, ncuridx;
	CEol ceol;
	bool bsuccess = false;

	pcDst->Clear();
	psrc = pcSrc.GetStringPtr();
	nsrclen = pcSrc.GetStringLength();

	if( nsrclen < 1 ){
		pcDst->_AppendSz("");
		return false;
	}
	pcDst->AllocBuffer( (nsrclen / 4) * 3 + 10 );
	pw_base = pw = static_cast<char *>( pcDst->GetRawPtr() );

	// 先頭の改行・空白文字をスキップ
	for( ncuridx = 0; ncuridx < nsrclen; ++ncuridx ){
		WCHAR c = psrc[ncuridx];
		if( !WCODE::IsLineDelimiterBasic(c) && c != L' ' && c != L'\t' ){
			break;
		}
	}

	// ヘッダーを解析
	pline = GetNextLineW( psrc, nsrclen, &nlinelen, &ncuridx, &ceol, false );
	if( !CheckUUHeader(pline, nlinelen, m_aFilename) ){
		pcDst->_AppendSz("");
		return false;
	}

	// ボディーを処理
	while( (pline = GetNextLineW(psrc, nsrclen, &nlinelen, &ncuridx, &ceol, false)) != NULL ){
		if( ceol.GetType() != EOL_CRLF ){
			pcDst->_AppendSz("");
			return false;
		}
		if( nlinelen < 1 ){
			pcDst->_AppendSz("");
			return false;
		}
		if( nlinelen == 1 ){
			// データの最後である場合
			if( pline[0] == L' ' || pline[0] == L'`' || pline[0] == L'~' ){
				bsuccess = true;
				break;
			}
		}
		pw += _DecodeUU_line( pline, nlinelen, pw );
	}
	if( bsuccess == false ){
		return false;
	}

	pline += 3;  // '`' 'CR' 'LF' の分をスキップ

	// フッターを解析
	if( !CheckUUFooter(pline, nsrclen-ncuridx) ){
		pcDst->_AppendSz("");
		return false;
	}

	pcDst->_SetRawLength( pw - pw_base );
	return true;
}
