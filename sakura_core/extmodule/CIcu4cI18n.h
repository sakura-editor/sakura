/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
#pragma once

#include "CDllHandler.h"

//ICU4Cの型定義
class UCharsetDetector;
class UCharsetMatch;

typedef enum UErrorCode
{
    U_ZERO_ERROR = 0, /**< No error, no warning. */
} UErrorCode;

/*!
 * ICU4C の i18n ライブラリ(icuin.dll) をラップするクラス
 */
class CIcu4cI18n final : public CDllImp
{
    // DLL関数型定義
    typedef UCharsetDetector *(__cdecl *ucsdet_open_t)(UErrorCode *status);
    typedef void(__cdecl *ucsdet_setText_t)(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status);
    typedef const UCharsetMatch *(__cdecl *ucsdet_detect_t)(UCharsetDetector *ucsd, UErrorCode *status);
    typedef const char *(__cdecl *ucsdet_getName_t)(const UCharsetMatch *ucsm, UErrorCode *status);
    typedef void(__cdecl *ucsdet_close_t)(UCharsetDetector *ucsd);

    // メンバ定義
    ucsdet_open_t _ucsdet_open;
    ucsdet_setText_t _ucsdet_setText;
    ucsdet_detect_t _ucsdet_detect;
    ucsdet_getName_t _ucsdet_getName;
    ucsdet_close_t _ucsdet_close;

  public:
    CIcu4cI18n() noexcept;
    virtual ~CIcu4cI18n() noexcept;

  protected:
    // CDllImpインタフェース
    LPCWSTR GetDllNameImp(int nIndex) override;
    bool InitDllImp() override;

  public:
    inline UCharsetDetector *ucsdet_open(UErrorCode *status) const
    {
        return _ucsdet_open(status);
    }
    inline void ucsdet_setText(UCharsetDetector *ucsd, const char *textIn, int32_t len, UErrorCode *status) const
    {
        return _ucsdet_setText(ucsd, textIn, len, status);
    }
    inline const UCharsetMatch *ucsdet_detect(UCharsetDetector *ucsd, UErrorCode *status) const
    {
        return _ucsdet_detect(ucsd, status);
    }
    inline const char *ucsdet_getName(const UCharsetMatch *ucsm, UErrorCode *status) const
    {
        return _ucsdet_getName(ucsm, status);
    }
    inline void ucsdet_close(UCharsetDetector *ucsd) const
    {
        return _ucsdet_close(ucsd);
    }
};
