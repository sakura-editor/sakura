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
#ifndef SAKURA_TCHAR_CONVERT_12EEF467_2644_4401_92A6_A0EA26FC78F39_H_
#define SAKURA_TCHAR_CONVERT_12EEF467_2644_4401_92A6_A0EA26FC78F39_H_

//WCHARに変換
const WCHAR* to_wchar(const ACHAR* src);
const WCHAR* to_wchar(const ACHAR* pSrcData, int nSrcLength);
inline
const WCHAR* to_wchar(const WCHAR* src){ return src; }

//ACHARに変換
inline
const ACHAR* to_achar(const ACHAR* src){ return src; }
const ACHAR* to_achar(const WCHAR* src);
const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength);

//TCHARに変換
#ifdef _UNICODE
	#define to_tchar     to_wchar
	#define to_not_tchar to_achar
#else
	#define to_tchar     to_achar
	#define to_not_tchar to_wchar
#endif

//その他
const WCHAR* easy_format(const WCHAR* format, ...);

#endif /* SAKURA_TCHAR_CONVERT_12EEF467_2644_4401_92A6_A0EA26FC78F39_H_ */
/*[EOF]*/
