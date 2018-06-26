// この行は文字化け対策のためのものです。
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CODEUTIL_E12B274F_2A3B_47A0_AFCB_4BAD1B3308AB_H_
#define SAKURA_CODEUTIL_E12B274F_2A3B_47A0_AFCB_4BAD1B3308AB_H_

#include <Windows.h>

#if 0  // 未使用
//	Oct. 3, 2002 genta
//	2008.11.10 引数と戻り値の型を _mbcjmstojis に似せる
unsigned int _mbcjmstojis_ex( unsigned int );
#endif

unsigned int __fastcall SjisFilter_basis( const unsigned int );
unsigned int __fastcall SjisFilter_ibm2nec( const unsigned int );
unsigned int __fastcall SjisFilter_nec2ibm( const unsigned int );


/*!
	UTF-16 -> UTF-32
*/
inline wchar32_t DecodeUtf16Surrog( const unsigned short wc_hi, const unsigned short wc_low ){
	wchar32_t wc32;
	wc32 = static_cast<wchar32_t>(wc_hi & 0x03ff) << 10;
	wc32 |= static_cast<wchar32_t>(wc_low & 0x03ff);
	return wc32 + 0x10000;
}

/*!
	UTF-32 ->  UTF-16
*/
inline int EncodeUtf16Surrog( wchar32_t wc32, unsigned short* pwc_dest ){
	unsigned short wc1, wc2;
	wc32 -= 0x10000;
	wc2 = static_cast<unsigned short>(wc32 & 0x000003ff) | 0xdc00;
	wc32 >>= 10;
	wc1 = static_cast<unsigned short>(wc32 & 0x000003ff) | 0xd800;
	pwc_dest[0] = wc1;
	pwc_dest[1] = wc2;
	return 2;
}

/*!
	UTF-32 -> UTF-8
*/
inline int EncodeUtf8( const wchar32_t wc32, unsigned char* pDst )
{
	int nret;

	if( wc32 < 0x80 ){
		pDst[0] = static_cast<unsigned char>(wc32 & 0x00ff);
		nret = 1;
	}else if( wc32 < 0x800 ){
		pDst[0] = static_cast<unsigned char>((wc32 & 0x07c0) >> 6) | 0xc0;
		pDst[1] = static_cast<unsigned char>(wc32 & 0x003f) | 0x80;
		nret = 2;
	}else if( wc32 < 0x10000 ){
		pDst[0] = static_cast<unsigned char>((wc32 & 0xf000) >> 12) | 0xe0;
		pDst[1] = static_cast<unsigned char>((wc32 & 0x0fc0) >> 6) | 0x80;
		pDst[2] = static_cast<unsigned char>( wc32 & 0x003f) | 0x80;
		nret = 3;
	}else{
		pDst[0] = static_cast<unsigned char>((wc32 & 0x001c0000) >> 18) | 0xf0;
		pDst[1] = static_cast<unsigned char>((wc32 & 0x0003f000) >> 12) | 0x80;
		pDst[2] = static_cast<unsigned char>((wc32 & 0x00000fc0) >> 6) | 0x80;
		pDst[3] = static_cast<unsigned char>( wc32 & 0x0000003f) | 0x80;
		nret = 4;
	}

	return nret;
}


/*!
	UTF-8 -> UTF-32
*/
inline wchar32_t DecodeUtf8( const unsigned char* pSrc, const int nSrcLen )
{
	wchar32_t wc32 = 0;

	switch( nSrcLen ){
	case 2:
		wc32 |= static_cast<wchar32_t>(pSrc[0] & 0x1f) << 6;
		wc32 |= static_cast<wchar32_t>(pSrc[1] & 0x3f);
		break;
	case 3:
		wc32 |= static_cast<wchar32_t>(pSrc[0] & 0x0f) << 12;
		wc32 |= static_cast<wchar32_t>(pSrc[1] & 0x3f) << 6;
		wc32 |= static_cast<wchar32_t>(pSrc[2] & 0x3f);
		break;
	case 4:
		wc32 |= static_cast<wchar32_t>(pSrc[0] & 0x07) << 18;
		wc32 |= static_cast<wchar32_t>(pSrc[1] & 0x3f) << 12;
		wc32 |= static_cast<wchar32_t>(pSrc[2] & 0x3f) << 6;
		wc32 |= static_cast<wchar32_t>(pSrc[3] & 0x3f);
		break;
	default: // case 1:
		wc32 = pSrc[0];
	}

	return wc32;
}


/*!
	判別テーブル   WinAPI 関数 WideCharToMultiByte の特殊な変換（相互変換できない変換）か
*/
extern const bool TABLE_WctombSpec[];

/*!
	WinAPI 関数 WideCharToMultiByte の特殊な変換を判別

	この関数が true を返す Unicode 文字は、
		@code
		::WideCharToMultiByte( 932, 0, src, srclen, dest, destlen, NULL, &bUsedDefaultChar );
		@endcode
	での変換処理に通過するものの、SJIS(CP 932) への適切な変換がなされない文字。

	Win98 から使える WC_NO_BEST_FIT_CHARS フラグの使用は避けて欲しいらしい。\n
	（http://blogs.msdn.com/shawnste/archive/2006/01/19/515047.aspx）
*/
inline bool IsWctombcNonroundtrip( const unsigned int wc ){
	if( wc == 0x3094/* ←平仮名の う に濁点をつけたやつ。 */ ){
		return true;
	}
	if( 0x00a0 <= wc && wc <= 0x00ff && TABLE_WctombSpec[wc-0x00a0] ){
		return true;
	}
	return false;
}

/*!
	WideCharToMultiByte のラッパー関数

	@param[in] nSrcLen  1 か 2 を渡す
*/
inline int MyWideCharToMultiByte_JP( const unsigned short* pSrc, const int nSrcLen, unsigned char* pDst ){
	int nret;
	BOOL blost;
	int nsrclen;

	// 保護コード
	if( nSrcLen > 2 || nSrcLen < 1 ){
		nsrclen = 1;
	}else{
		nsrclen = nSrcLen;
	}

	if( IsWctombcNonroundtrip(pSrc[0]) != true ){
		nret = ::WideCharToMultiByte( 932, 0, reinterpret_cast<const wchar_t*>(pSrc), nsrclen, reinterpret_cast<char*>(pDst), 4, NULL, &blost );
		if( blost != FALSE ){
			nret = 0;
		}
	}else{
		nret = 0;
	}
	return nret;
}


/*!
	MultiByteToWideChar のラッパー関数

	@param[out] pbNonroundtrip 返還に成功したものの、相互変換性が失われた場合に true

	nSrcLen は 1 か 2
*/
inline int MyMultiByteToWideChar_JP( const unsigned char* pSrc, const int nSrcLen, unsigned short* pDst, bool bKeepRt = true )
{
	int nret;
	unsigned char czenkaku[4];

	nret = ::MultiByteToWideChar( 932, 0, reinterpret_cast<const char*>(pSrc), nSrcLen, reinterpret_cast<wchar_t*>(pDst), 4 );
	if( nret > 0 &&	bKeepRt == true ){
		MyWideCharToMultiByte_JP( pDst, nret, czenkaku );
		if( nSrcLen == 2 ){
			if( pSrc[0] != czenkaku[0] || pSrc[1] != czenkaku[1] ){
				nret = 0;
			}
		}else if( nSrcLen == 1 ){
			if( pSrc[0] != czenkaku[0] ){
				nret = 0;
			}
		}else{
			;
		}
	}

	return nret;
}

#endif /* SAKURA_CODEUTIL_E12B274F_2A3B_47A0_AFCB_4BAD1B3308AB_H_ */
/*[EOF]*/
