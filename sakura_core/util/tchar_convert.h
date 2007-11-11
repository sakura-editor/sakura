#pragma once

//WCHAR‚É•ÏŠ·
const WCHAR* to_wchar(const ACHAR* src);
const WCHAR* to_wchar(const ACHAR* pSrcData, int nSrcLength);
inline
const WCHAR* to_wchar(const WCHAR* src){ return src; }

//ACHAR‚É•ÏŠ·
inline
const ACHAR* to_achar(const ACHAR* src){ return src; }
const ACHAR* to_achar(const WCHAR* src);
const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength);

//TCHAR‚É•ÏŠ·
#ifdef _UNICODE
	#define to_tchar     to_wchar
	#define to_not_tchar to_achar
#else
	#define to_tchar     to_achar
	#define to_not_tchar to_wchar
#endif

//‚»‚Ì‘¼
const WCHAR* easy_format(const WCHAR* format, ...);
