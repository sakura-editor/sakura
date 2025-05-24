/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_
#define SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_
#pragma once

#include "CDllHandler.h"

//ICU4Cの型定義
class UCharsetDetector;
class UCharsetMatch;

typedef enum UErrorCode {
	U_ZERO_ERROR = 0,     /**< No error, no warning. */
} UErrorCode;

/*!
 * ICU4C の i18n ライブラリ(icuin.dll) をラップするクラス
 */
class CIcu4cI18n : public CDllImp
{
	// DLL関数型定義
	typedef UCharsetDetector*		(__cdecl *ucsdet_open_t)(UErrorCode *status);
	typedef void					(__cdecl *ucsdet_setText_t)(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status);
	typedef const UCharsetMatch *	(__cdecl *ucsdet_detect_t)(UCharsetDetector *ucsd, UErrorCode *status);
	typedef const char*				(__cdecl *ucsdet_getName_t)(const UCharsetMatch *ucsm, UErrorCode *status);
	typedef void					(__cdecl *ucsdet_close_t)(UCharsetDetector *ucsd);

	// メンバ定義
	ucsdet_open_t		_ucsdet_open;
	ucsdet_setText_t	_ucsdet_setText;
	ucsdet_detect_t		_ucsdet_detect;
	ucsdet_getName_t	_ucsdet_getName;
	ucsdet_close_t		_ucsdet_close;

public:
	CIcu4cI18n() noexcept;

protected:
	// CDllImpインターフェース
	LPCWSTR GetDllNameImp(int nIndex) override;
	bool InitDllImp() override;

public:
	inline UCharsetDetector* ucsdet_open(UErrorCode *status) const {
		return _ucsdet_open(status);
	}
	inline void ucsdet_setText(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status) const {
		return _ucsdet_setText(ucsd, textIn, len, status);
	}
	inline const UCharsetMatch* ucsdet_detect(UCharsetDetector *ucsd, UErrorCode *status) const {
		return _ucsdet_detect(ucsd, status);
	}
	inline const char* ucsdet_getName(const UCharsetMatch *ucsm, UErrorCode *status) const {
		return _ucsdet_getName(ucsm, status);
	}
	inline void ucsdet_close(UCharsetDetector *ucsd) const {
		return _ucsdet_close(ucsd);
	}
};
#endif /* SAKURA_CICU4CI18N_16EE9B14_2791_4C5D_AE1E_E78FBF5CB083_H_ */
