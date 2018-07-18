// この行は文字化け対策のものです。
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
