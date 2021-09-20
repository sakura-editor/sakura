/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "CharsetDetector.h"

CharsetDetector::CharsetDetector() noexcept
	: _icuin()
	, _csd(nullptr)
	, _ud(nullptr)
{
	_icuin.InitDll();
	_uchardet.InitDll();
}

CharsetDetector::~CharsetDetector() noexcept
{
	if (_icuin.IsAvailable()) {
		_icuin.ucsdet_close(_csd);
	}
	if (_uchardet.IsAvailable()) {
		_uchardet.uchardet_delete(_ud);
	}
}

static ECodeType name2code(std::string_view name)
{
	// 文字セット名⇒サクラエディタ内部コードの変換
	if (name == "UTF-8") return CODE_UTF8;
	if (name == "SHIFT_JIS") return CODE_SJIS;
	if (name == "UTF-16BE") return CODE_UNICODEBE;
	if (name == "UTF-16LE") return CODE_UNICODE;
	if (name == "EUC-JP") return CODE_EUC;
	if (name == "ISO-2022-JP") return CODE_JIS;
	if (name == "UTF-7") return CODE_UTF7;
	if (name == "ISO-8859-1") return CODE_LATIN1;
	// ここから下は数が多いのでどうしたものか…
	// https://www.freedesktop.org/wiki/Software/uchardet/
	// https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
	if (name == "GB2312") return (ECodeType)936;
	if (name == "BIG5") return (ECodeType)950;
	if (name == "ISO-2022-KR") return (ECodeType)50225;
	if (name == "GB18030") return (ECodeType)54936;
	return CODE_ERROR;
}

ECodeType CharsetDetector::Detect(const std::string_view& bytes)
{
	if (_icuin.IsAvailable()) {
		UErrorCode status = U_ZERO_ERROR;
		_csd = _icuin.ucsdet_open(&status);
		if (status != U_ZERO_ERROR) {
			return CODE_ERROR;
		}

		_icuin.ucsdet_setText(_csd, bytes.data(), bytes.length(), &status);
		if (status != U_ZERO_ERROR) {
			return CODE_ERROR;
		}

		const auto csm = _icuin.ucsdet_detect(_csd, &status);
		if (status != U_ZERO_ERROR) {
			return CODE_ERROR;
		}

		std::string_view name = _icuin.ucsdet_getName(csm, &status);
		if (status != U_ZERO_ERROR) {
			return CODE_ERROR;
		}
		return name2code(name);
	}
	if (_uchardet.IsAvailable()) {
		if (!_ud) {
			_ud = _uchardet.uchardet_new();
		}
		if (!_ud) {
			return CODE_ERROR;
		}
		int ret = _uchardet.uchardet_handle_data(_ud, bytes.data(), bytes.length());
		if (ret != 0) {
			return CODE_ERROR;
		}
		_uchardet.uchardet_data_end(_ud);
		std::string_view name = _uchardet.uchardet_get_charset(_ud);
		auto code = name2code(name);
		_uchardet.uchardet_reset(_ud);
		return code;
	}
	return CODE_ERROR;
}
