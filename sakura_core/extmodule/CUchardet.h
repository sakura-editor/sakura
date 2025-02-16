/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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

