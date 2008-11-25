// ‚±‚Ìs‚Í•¶Žš‰»‚¯‘Îô‚Ì‚½‚ß‚Ì‚à‚Ì‚Å‚·B
#pragma once

//	Oct. 3, 2002 genta
unsigned short _mbcjmstojis_ex( unsigned char* pszSrc );

/*!
	UTF-16 -> UTF-32
*/
inline wchar32_t DecodeUtf16Surrog( wchar_t wc_hi, wchar_t wc_low ){
	wchar32_t wc32;
	wc_hi &= 0x03ff;
	wc32 = static_cast<wchar32_t>(wc_hi) << 10;
	wc_low &= 0x03ff;
	wc32 |= static_cast<wchar32_t>(wc_low);
	wc32 += 0x10000;
	return wc32;
}
