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
#pragma once

#include "CDllHandler.h"

typedef struct uchardet * uchardet_t;

/*!
 * uchardet ライブラリ(uchardet.dll) をラップするクラス
 */
class CUchardet : public CDllImp
{
public:
	// DLL関数ポインタ
	uchardet_t (*_uchardet_new)(void) = nullptr;
	void (*_uchardet_delete)(uchardet_t ud) = nullptr;
	int (*_uchardet_handle_data)(uchardet_t ud, const char * data, size_t len) = nullptr;
	void (*_uchardet_data_end)(uchardet_t ud) = nullptr;
	void (*_uchardet_reset)(uchardet_t ud) = nullptr;
	const char * (*_uchardet_get_charset)(uchardet_t ud) = nullptr;

protected:
	// CDllImpインターフェース
	LPCWSTR GetDllNameImp(int nIndex) override;
	bool InitDllImp() override;

public:
	uchardet_t uchardet_new(void) const { return _uchardet_new(); }
	void uchardet_delete(uchardet_t ud) const { _uchardet_delete(ud); }
	int uchardet_handle_data(uchardet_t ud, const char * data, size_t len) const { return _uchardet_handle_data(ud, data, len); }
	void uchardet_data_end(uchardet_t ud) const { _uchardet_data_end(ud); }
	void uchardet_reset(uchardet_t ud) const { _uchardet_reset(ud); }
	const char * uchardet_get_charset(uchardet_t ud) const { return _uchardet_get_charset(ud); }
};

