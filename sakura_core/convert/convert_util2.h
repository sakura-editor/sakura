/*!	@file
	@brief 変換ユーティリティ2 - BASE64 Ecode/Decode, UUDecode, Q-printable decode

	@author 
*/

/*
	Copyright (C)

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
#ifndef SAKURA_CONVERT_UTIL2_0DBB4338_3B8D_4466_A20D_638B847EB6C9_H_
#define SAKURA_CONVERT_UTIL2_0DBB4338_3B8D_4466_A20D_638B847EB6C9_H_

#include "parse/CWordParse.h"
#include "mem/CMemory.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//    Quoted-Printable デコード
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//


inline ACHAR _GetHexChar( ACHAR c )
{
	if( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ){
		return c;
	}else if( c >= 'a' && c <= 'f' ){
		return  c - ('a' - 'A');
	}else{
		return '\0';
	}
}
inline WCHAR _GetHexChar( WCHAR c )
{
	if( (c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'F') ){
		return c;
	}else if( c >= L'a' && c <= L'f' ){
		return  c - (L'a' - L'A');
	}else{
		return L'\0';
	}
}


/*
	c の入力値： 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
*/
inline int _HexToInt( ACHAR c )
{
	if( c <= '9' ){
		return c - '0';
	}else{
		return c - 'A' + 10;
	}
}
inline int _HexToInt( WCHAR c )
{
	if( c <= L'9' ){
		return c - L'0';
	}else{
		return c - L'A' + 10;
	}
}


template< class CHAR_TYPE >
int _DecodeQP( const CHAR_TYPE* pS, const int nLen, char* pDst )
{
	const CHAR_TYPE *pr;
	char *pw;
	int ninc_len;

	pr = pS;
	pw = pDst;

	while( pr < pS + nLen ){
		/* =XX の形式でない部分をデコード */
		if( sizeof(CHAR_TYPE) == 2 ){
			if( *pr != L'=' ){
				*pw = static_cast<char>( *pr );
				pw += 1;
				pr += 1;
				continue;
			}
		}else{
			if( *pr != '=' ){
				*pw = static_cast<char>( *pr );
				pw += 1;
				pr += 1;
				continue;
			}
		}

		/* =XX の部分をデコード */
		ninc_len = 1;   // '=' の部分のインクリメント。
		if( pr + 2 < pS + nLen ){
			// デコード実行部分
			CHAR_TYPE c1, c2;
			c1 = _GetHexChar(pr[1]);
			c2 = _GetHexChar(pr[2]);
			if( c1 != 0 && c2 != 0 ){
				*pw = static_cast<char>(_HexToInt(c1) << 4) | static_cast<char>(_HexToInt(c2));
				++pw;
			}else{
				pw[0] = '=';
				pw[1] = static_cast<char>(pr[1] & 0x00ff);
				pw[2] = static_cast<char>(pr[2] & 0x00ff);
				pw += 3;
			}
			ninc_len += 2;
			// ここまで。
		}
		pr += ninc_len;
	}

	return pw - pDst;
}







// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// BAASE64 のエンコード/デコード
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

extern const uchar_t TABLE_BASE64CharToValue[];
extern const char TABLE_ValueToBASE64Char[];


// BASE64文字 <-> 数値
template< class CHAR_TYPE >
inline uchar_t Base64ToVal( const CHAR_TYPE c ){
	int c_ = c;
	return static_cast<uchar_t>((c_ < 0x80)? TABLE_BASE64CharToValue[c_] : -1);
}
template< class CHAR_TYPE >
inline CHAR_TYPE ValToBase64( const char v ){
	int v_ = v;
	return static_cast<CHAR_TYPE>((v_ < 64)? TABLE_ValueToBASE64Char[v_] : -1);
}


#if 0
/*
	Bas64文字列の末尾が適切かどうかをチェック
	
	入力：BASE64 文字列。
*/
template< class CHAR_TYPE >
bool CheckBase64Padbit( const CHAR_TYPE *pSrc, const int nSrcLen )
{
	bool bret = true;

	if( nSrcLen < 1 ){
		return false;
	}

	/* BASE64文字の末尾について：
		ooxx xxxx   ooxx oooo                   -> 1 byte(s)
		ooxx xxxx   ooxx xxxx   ooxx xxoo          -> 2 byte(s)
		ooxx xxxx   ooxx xxxx   ooxx xxxx   ooxx xxxx -> 3 byte(s)
	*/
	
	switch( nSrcLen % 4 ){
	case 0:
		break;
	case 1:
		bret = false;
		break;
	case 2:
		if( (Base64ToVal(pSrc[nSrcLen-1]) & 0x0f) != 0 ){
			bret = false;
		}
		break;
	case 3:
		if( (Base64ToVal(pSrc[nSrcLen-1]) & 0x03) != 0 ){
			bret = false;
		}
		break;
	}
	return bret;
}
#endif

/*!
	BASE64 デコード実行関数

	前の実装を参考に。
	正しい BASE64 入力文字列を仮定している。
*/
template< class CHAR_TYPE >
int _DecodeBase64( const CHAR_TYPE *pSrc, const int nSrcLen, char *pDest )
{
	long lData;
	int nDesLen;
	int sMax;
	int nsrclen = nSrcLen;

	// 文字列の最後のパッド文字 '=' を文字列長に含めないようにする処理
	{
		int i = 0;
		bool bret;
		for( ; i < nsrclen; i++ ){
			if( sizeof(CHAR_TYPE) == 2 ){
				bret = ( pSrc[nsrclen-1-i] == L'=' );
			}else{
				bret = ( pSrc[nsrclen-1-i] == '=' );
			}
			if( bret != true ){
				break;
			}
		}
		nsrclen -= i;
	}

	nDesLen = 0;
	for( int i = 0; i < nsrclen; i++ ){
		if( i < nsrclen - (nsrclen % 4) ){
			sMax = 4;
		}else{
			sMax = (nsrclen % 4);
		}
		lData = 0;
		for( int j = 0; j < sMax; j++ ){
			long k = Base64ToVal( pSrc[i + j] );
			lData |= k << ((4 - j - 1) * 6);
		}
		for( int j = 0; j < (sMax * 6)/ 8 ; j++ ){
			pDest[nDesLen] = static_cast<char>((lData >> (8 * (2 - j))) & 0x0000ff);
			nDesLen++;
		}
		i+= 3;
	}
	return nDesLen;
}


/*!
	BASE64 エンコード実行関数

	前の実装を参考に。
	パッド文字などは付加しない。エラーチェックなし。
*/
template< class CHAR_TYPE >
int _EncodeBase64( const char *pSrc, const int nSrcLen, CHAR_TYPE *pDest )
{
	const unsigned char *psrc;
	unsigned long lDataSrc;
	int i, j, k, n;
	char v;
	int nDesLen;

	psrc = reinterpret_cast<const unsigned char *>(pSrc);
	nDesLen = 0;
	for( i = 0; i < nSrcLen; i += 3 ){
		lDataSrc = 0;
		if( nSrcLen - i < 3 ){
			n = nSrcLen % 3;
			j = (n * 4 + 2) / 3;  // 端数切り上げ
		}else{
			n = 3;
			j = 4;
		}
		// n 今回エンコードする長さ
		// j エンコード後のBASE64文字数
		for( k = 0; k < n; k++ ){
			lDataSrc |=
				static_cast<unsigned long>(psrc[i + k]) << ((n - k - 1) * 8);
		}
		// パッドビット付加。lDataSrc の長さが 6*j になるように調節する。
		lDataSrc <<= j * 6 - n * 8;
		// エンコードして書き込む。
		for( k = 0; k < j; k++ ){
			v = static_cast<char>((lDataSrc >> (6 * (j - k - 1))) & 0x0000003f);
			pDest[nDesLen] = static_cast<CHAR_TYPE>(ValToBase64<CHAR_TYPE>( v ));
			nDesLen++;
		}
	}
	return nDesLen;
}






// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// UU デコード
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//




/*
	Unix-to-Unix のこと

egin <permission> <file name>
begin
<encoded data>

end

<permission>：
	ファイル生成時に使うパーミッションの値
	（Windowsではパーミッションが存在しない？ので、600 または 666 を用いる）

<file name>：
	ファイル生成時に使いファイル名

<encoded data>：
	・バイナリデータを3バイトずつ取り出し、それら3バイトをMSBからLSBへと並べた
	　24ビット幅のデータをさらに4分割し、MSBから順に6ビットずつ取り出す。
	　取り出したそれぞれの値に 0x20(空白文字)を加算し7ビットASCII文字に変換し、
	　取り出した順に書き込んでいく。
	・データ長が3の倍数になっていない場合は、0でパディングして3の倍数となるよう調節する。
	・行の最初には、その行に何バイト分のデータがあるかの情報を書き込む。
	・1行には45バイト分のデータ（60文字）を書き込むのが慣例で（決まり？）、最後の行以外は、
	　"M"（45+0x20）が行の先頭となる。
	・符号化されたデータは、0バイトの行で終了する。
	・行末の空白を削除するゲートウェイに対処するため、空白は、"~"(0x7E)または"`"(0x60)を換わりに使う。
*/




inline BYTE _UUDECODE_CHAR( WCHAR c )
{
	BYTE c_ = (c & 0xff);
	if( c_ == L'`' || c_ == L'~' ){
		c_ = L' ';
	}
	return static_cast<BYTE>((static_cast<BYTE>(c_) - 0x20) & 0x3f);
}
inline BYTE _UUDECODE_CHAR( ACHAR c )
{
	if( c == '`' || c == '~' ){
		c = ' ';
	}
	return static_cast<BYTE>((static_cast<BYTE>(c) - 0x20) & 0x3f);
}



/*
	UU デコーダー（一行だけ実行するバージョン）

	@param[in] nSrcLen	必ず、4の倍数であること。
	@param[in] pDest	必ず、(nSrcLen / 4) * 3 以上のバッファが確保されていること。

	@return 一行分をデコードした結果得られた生データのバイト長
	        書き込んだデータが戻り値よりも大きいときがあるので注意。
*/
template< class CHAR_TYPE >
int _DecodeUU_line( const CHAR_TYPE *pSrc, const int nSrcLen, char *pDest )
{
	unsigned long lDataDes;
	const CHAR_TYPE *pr;

	if( nSrcLen < 1 ){
		return 0;
	}

	pr = pSrc+1;  // 先頭の文字（M(0x20+45)など）を飛ばす
	int i = 0;
	int j = 0;
	int k = 0;
	for( ; i < nSrcLen; i += 4 ){
		lDataDes = 0;
		for( j = 0; j < 4; ++j ){
			lDataDes |= _UUDECODE_CHAR(pr[i+j]) << ((4 - j - 1) * 6);
		}
		for( j = 0; j < 3; ++j ){
			pDest[k + j] = (char)((lDataDes >> ((3 - j - 1) * 8)) & 0x000000ff);
		}
		k += 3;
	}

	return _UUDECODE_CHAR(pSrc[0]); // 1行分をデコードしたときに得られる生データのバイト長を取得
}

/*!
	UUエンコードのヘッダー部分を解析
*/
template< class CHAR_TYPE >
bool CheckUUHeader( const CHAR_TYPE *pSrc, const int nLen, TCHAR *pszFilename )
{
//	using namespace WCODE;

	const CHAR_TYPE *pr, *pr_end;
	CHAR_TYPE *pwstart;
	int nwlen, nstartidx;
	CHAR_TYPE pszSplitChars[16];

	if( sizeof(CHAR_TYPE) == 2 ){
		// スペースまたはタブが区切り文字
		pszSplitChars[0] = L' ';
		pszSplitChars[1] = L'\t';
		pszSplitChars[2] = L'\0';
	}else{
		// スペースまたはタブが区切り文字
		pszSplitChars[0] = ' ';
		pszSplitChars[1] = '\t';
		pszSplitChars[2] = '\0';
	}
	

	if( nLen < 1 ){
		if( pszFilename ){
			pszFilename[0] = _WINT('\0');
		}
		return false;
	}

	// 先頭の空白・改行文字をスキップ
	for( nstartidx = 0; nstartidx < nLen; ++nstartidx ){
		CHAR_TYPE c = pSrc[nstartidx];
		if( sizeof(CHAR_TYPE) == 2 ){
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}

	pr = pSrc + nstartidx;
	pr_end = pSrc + nLen;

	// ヘッダーの構成
	// begin  755  <filename>

	/* begin を取得 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	if( nwlen != 5 ){
		// error.
		return false;
	}
	if( sizeof(CHAR_TYPE) == 2 ){
		if( wcsncmp(pwstart, L"begin", 5) != 0 ){
			// error.
			return false;
		}
	}else{
		if( strncmp(reinterpret_cast<const char*>(pwstart), "begin", 5) != 0 ){
			// error.
			return false;
		}
	}

	/* 3桁の8進数（Unix システムのパーミッション）を取得 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	if( nwlen != 3 ){
		// error.
		return false;
	}
	for( int i = 0; i < nwlen; i++ ){
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR の場合の処理
			if( !iswdigit(pwstart[i]) || (pwstart[i] == L'8' || pwstart[i] == L'9') ){
				// error.
				return false;
			}
		}else{
			// ACHAR の場合の処理
			if( !isdigit(pwstart[i]) || (pwstart[i] == '8' || pwstart[i] == '9') ){
				// error.
				return false;
			}
		}
	}

	/* 書き出し用のファイル名を取得 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	// 末尾の空白・改行文字をスキップ
	for( ; nwlen > 0; --nwlen ){
		CHAR_TYPE c = pwstart[nwlen-1];
		if( sizeof(CHAR_TYPE) == 2 ){
			if( !WCODE::IsLineDelimiterBasic(c) && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}
	if( nwlen < 1 || nwlen + 1  > _MAX_PATH ){
		// error.
		return false;
	}
	// ファイル名を格納
	if( pszFilename ){
		strtotcs( pszFilename, pwstart, (size_t)nwlen );
		pszFilename[nwlen] = _WINT('\0');
	}

	return true;
}


/*!
	UU フッターを確認
*/
template< class CHAR_TYPE >
bool CheckUUFooter( const CHAR_TYPE *pS, const int nLen )
{
	int nstartidx;
	const CHAR_TYPE* psrc;
	int nsrclen;
	int i;

	// フッターの構成
	// end
	// ※ 空行はフッターに含めない。

	// 先頭の改行・空白文字をスキップ
	for( nstartidx = 0; nstartidx < nLen; ++nstartidx ){
		CHAR_TYPE c = pS[nstartidx];
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR の場合の処理
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			// ACHAR の場合の処理
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}

	psrc = pS + nstartidx;
	nsrclen = nLen - nstartidx;
	i = 0;

	if( nsrclen < 3 ){
		return false;
	}
	if( sizeof(CHAR_TYPE) == 2 ){
		if( wcsncmp(&pS[nstartidx], L"end", 3) != 0 ){
			// error.
			return false;
		}
	}else{
		if( strncmp(reinterpret_cast<const char*>(&pS[nstartidx]), "end", 3) != 0 ){
			// error.
			return false;
		}
	}
	i += 3;

	// end の後が空白文字ばかりであることを確認
	for( ; i < nsrclen; ++i ){
		CHAR_TYPE c = psrc[i];
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR の場合の処理
			if( !WCODE::IsLineDelimiterBasic(c) && c != L' ' && c != L'\t' ){
				return false;
			}
		}else{
			// ACHAR の場合の処理
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				return false;
			}
		}
	}

	return true;
}





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//    MIME ヘッダーデコード
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//


enum EEncodingMethod {
	EM_NONE,
	EM_QP,
	EM_BASE64,
};

/*!
	MIMEヘッダーデコード補助関数

	@return  CMemory と置き換えられる入力文字列長 (nSkipLen)
*/
template< class CHAR_TYPE >
int _DecodeMimeHeader( const CHAR_TYPE* pSrc, const int nSrcLen, CMemory* pcMem_alt, ECodeType* peCodetype )
{
	ECodeType ecode = CODE_NONE;
	EEncodingMethod emethod = EM_NONE;
	int nLen_part1, nLen_part2, nskipped_len;
	int ncmpresult1, ncmpresult2, ncmpresult;

	const CHAR_TYPE *pr, *pr_base;
	char* pdst;
	int ndecoded_len;


	// MIME の該当部分を検出。----------------------------------------
	//


	//   part1 部分
	//
	//   "=?ISO-2022-JP?", "=?UTF-8?" などの部分を検出
	//

	if( pSrc+14 < pSrc+nSrcLen ){
		// JIS の場合
		if( sizeof(CHAR_TYPE) == 2 ){
			ncmpresult = wcsnicmp( reinterpret_cast<const wchar_t*>(&pSrc[0]), L"=?ISO-2022-JP?", 14 );
		}else{
			ncmpresult = strnicmp( &pSrc[0], "=?ISO-2022-JP?", 14 );
		}
		if( ncmpresult == 0 ){  // 
			ecode = CODE_JIS;
			nLen_part1 = 14;
			goto finish_first_detect;
		}
	}
	if( pSrc+8 < pSrc+nSrcLen ){
		// UTF-8 の場合
		if( sizeof(CHAR_TYPE) == 2 ){
			ncmpresult = wcsnicmp( reinterpret_cast<const wchar_t*>(&pSrc[0]), L"=?UTF-8?", 8 );
		}else{
			ncmpresult = strnicmp( &pSrc[0], "=?UTF-8?", 8 );
		}
		if( ncmpresult == 0 ){
			ecode = CODE_UTF8;
			nLen_part1 = 8;
			goto finish_first_detect;
		}
	}
	// マッチしなかった場合
	pcMem_alt->SetRawData( "", 0 );
	if( peCodetype ){
		*peCodetype = CODE_NONE;
	}
	return 0;

finish_first_detect:;

	if( peCodetype ){
		*peCodetype = ecode;
	}

	//
	//    part2 部分
	//
	//   "B?" または "Q?" の部分を検出
	//

	if( pSrc+nLen_part1+2 >= pSrc+nSrcLen ){
		pcMem_alt->SetRawData( "", 0 );
		return 0;
	}
	if( sizeof(CHAR_TYPE) == 2 ){
		ncmpresult1 = wcsnicmp( reinterpret_cast<const wchar_t*>(&pSrc[nLen_part1]), L"B?", 2 );
		ncmpresult2 = wcsnicmp( reinterpret_cast<const wchar_t*>(&pSrc[nLen_part1]), L"Q?", 2 );
	}else{
		ncmpresult1 = strnicmp( &pSrc[nLen_part1], "B?", 2 );
		ncmpresult2 = strnicmp( &pSrc[nLen_part1], "Q?", 2 );
	}
	if( ncmpresult1 == 0 ){
		emethod = EM_BASE64;
	}else if( ncmpresult2 == 0 ){
		emethod = EM_QP;
	}else{
		pcMem_alt->SetRawData( "", 0 );
		return 0;
	}
	nLen_part2 = 2;

	//
	//   エンコード文字列の部分を検出
	//

	pr_base = pSrc + nLen_part1 + nLen_part2;
	pr = pSrc + nLen_part1 + nLen_part2;
	for( ; pr < pSrc+nSrcLen-1; ++pr ){
		if( sizeof(CHAR_TYPE) == 2 ){
			ncmpresult = wcsncmp( reinterpret_cast<const wchar_t*>(pr), L"?=", 2 );
		}else{
			ncmpresult = strncmp( pr, "?=", 2 );
		}
		if( ncmpresult == 0 ){
			break;
		}
	}
	if( pr == pSrc+nSrcLen-1 ){
		pcMem_alt->SetRawData( "", 0 );
		return 0;
	}

	nskipped_len = pr - pSrc + 2;  // =? から ?= までの、全体の長さを記録

	//   デコード ----------------------------------------------------
	//

	pcMem_alt->AllocBuffer( pr - pr_base );
	pdst = reinterpret_cast<char*>( pcMem_alt->GetRawPtr() );
	if( pdst == NULL ){
		pcMem_alt->SetRawData( "", 0 );
		return 0;
	}

	if( emethod == EM_BASE64 ){
		ndecoded_len = _DecodeBase64( pr_base, pr-pr_base, pdst );
	}else{
		ndecoded_len = _DecodeQP( pr_base, pr-pr_base, pdst );
	}

	pcMem_alt->_SetRawLength( ndecoded_len );

	return nskipped_len;
}

#endif /* SAKURA_CONVERT_UTIL2_0DBB4338_3B8D_4466_A20D_638B847EB6C9_H_ */
/*[EOF]*/
