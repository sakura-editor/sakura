/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_
#define SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_
#pragma once

#include <icu.h>

/*!
 * ICU4C のライブラリ(icu.dll) をラップするクラス
 *
 * Windows 10 1703でICU4CがWindowsに統合された。
 * この実装では最新1903以降で導入されたicu.dllを使うようにしている。
 * 1703～1809だと実装DLLが2つに分かれていてicu.dllが存在しない。
 */
class CIcu4cI18n
{
public:
	[[nodiscard]] bool IsAvailable() const { return true; }
	bool InitDll() const { return true; }

	inline UCharsetDetector* ucsdet_open(UErrorCode *status) const {
		return ::ucsdet_open(status);
	}
	inline void ucsdet_setText(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status) const {
		return ::ucsdet_setText(ucsd, textIn, len, status);
	}
	inline const UCharsetMatch* ucsdet_detect(UCharsetDetector *ucsd, UErrorCode *status) const {
		return ::ucsdet_detect(ucsd, status);
	}
	inline const char* ucsdet_getName(const UCharsetMatch *ucsm, UErrorCode *status) const {
		return ::ucsdet_getName(ucsm, status);
	}
	inline void ucsdet_close(UCharsetDetector *ucsd) const {
		return ::ucsdet_close(ucsd);
	}
};
#endif /* SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_ */
