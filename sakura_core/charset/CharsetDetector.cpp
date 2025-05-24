/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CharsetDetector.h"

// https://www.freedesktop.org/wiki/Software/uchardet/
// https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
static const std::unordered_map<std::string, uint16_t> map_charsetToCodePage = {
	// International (Unicode) 
	{ "UTF-32BE", 12001, },
	{ "UTF-32LE", 12000, },
	// Arabic
	{ "ISO-8859-6", 28596, },
	{ "WINDOWS-1256", 1256, },
	// Bulgarian
	{ "ISO-8859-5", 28595, },
	{ "WINDOWS-1251", 1251, },
	// Chinese
	{ "ISO-2022-CN", 50227, },
	{ "BIG5", 950, },
	{ "EUC-TW", 51950, },
	{ "GB18030", 54936, },
	{ "HZ-GB-2312", 52936, },
	// Croatian
	{ "ISO-8859-2", 28592, },
	{ "ISO-8859-13", 28603, },
	{ "ISO-8859-16", 28606, },
	{ "WINDOWS-1250", 1250, },
	{ "IBM852", 852, },
	{ "MAC-CENTRALEUROPE", 10029, },
	// Czech
	{ "WINDOWS-1250", 1250, },
	{ "ISO-8859-2", 28592, },
	{ "IBM852", 852, },
	{ "MAC-CENTRALEUROPE", 10029, },
	// Danish
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// English
	{ "ASCII", 20127, },
	// Esperanto
	{ "ISO-8859-3", 28593, },
	// Estonian
	{ "ISO-8859-4", 28594, },
	{ "ISO-8859-13", 28603, },
	{ "ISO-8859-13", 28603, },
	{ "WINDOWS-1252", 1252, },
	{ "WINDOWS-1257", 1257, },
	// Finnish
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-4", 28594, },
	{ "ISO-8859-9", 28599, },
	{ "ISO-8859-13", 28603, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// French
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// German
	{ "ISO-8859-1", 28591, },
	{ "WINDOWS-1252", 1252, },
	// Greek
	{ "ISO-8859-7", 28597, },
	{ "WINDOWS-1253", 1253, },
	// Hebrew
	{ "ISO-8859-8", 28598, },
	{ "WINDOWS-1255", 1255, },
	// Hungarian
	{ "ISO-8859-2", 28592, },
	{ "WINDOWS-1250", 1250, },
	// Irish Gaelic
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-9", 28599, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// Italian
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-3", 28593, },
	{ "ISO-8859-9", 28599, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// Japanese
	{ "ISO-2022-JP", 50220, },
	{ "SHIFT_JIS", 932, },
	{ "EUC-JP", 20932, },
	// Korean
	{ "ISO-2022-KR", 50225, },
	{ "EUC-KR", 51949, },
	// Lithuanian
	{ "ISO-8859-4", 28594, },
	{ "ISO-8859-10", 28600, },
	{ "ISO-8859-13", 28603, },
	// Latvian
	{ "ISO-8859-4", 28594, },
	{ "ISO-8859-10", 28600, },
	{ "ISO-8859-13", 28603, },
	// Maltese
	{ "ISO-8859-3", 28593, },
	// Polish
	{ "ISO-8859-2", 28592, },
	{ "ISO-8859-13", 28603, },
	{ "ISO-8859-16", 28606, },
	{ "WINDOWS-1250", 1250, },
	{ "IBM852", 852, },
	{ "MAC-CENTRALEUROPE", 10029, },
	// Portuguese
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-9", 28599, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// Romanian
	{ "ISO-8859-2", 28592, },
	{ "ISO-8859-16", 28606, },
	{ "WINDOWS-1250", 1250, },
	{ "IBM852", 852, },
	// Russian
	{ "ISO-8859-5", 28595, },
	{ "KOI8-R", 20866, },
	{ "WINDOWS-1251", 1251, },
	{ "MAC-CYRILLIC", 10007, },
	{ "IBM866", 866, },
	{ "IBM855", 855, },
	// Slovak
	{ "WINDOWS-1250", 1250, },
	{ "ISO-8859-2", 28592, },
	{ "IBM852", 852, },
	{ "MAC-CENTRALEUROPE", 10029, },
	// Slovene
	{ "ISO-8859-2", 28592, },
	{ "ISO-8859-16", 28606, },
	{ "WINDOWS-1250", 1250, },
	{ "IBM852", 852, },
	{ "MAC-CENTRALEUROPE", 10029, },
	// Spanish
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// Swedish
	{ "ISO-8859-1", 28591, },
	{ "ISO-8859-4", 28594, },
	{ "ISO-8859-9", 28599, },
	{ "ISO-8859-15", 28605, },
	{ "WINDOWS-1252", 1252, },
	// Thai
	{ "TIS-620", 874, },
	{ "ISO-8859-11", 28601, },
	// Turkish
	{ "ISO-8859-3", 28593, },
	{ "ISO-8859-9", 28599, },
	// Vietnamese
	//{ "VISCII", , },
	{ "Windows-1258", 1258, },
	// Others
	{ "WINDOWS-1252", 1252, },
};

CharsetDetector::CharsetDetector() noexcept
	: _icuin()
	, _csd(nullptr)
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

		// 文字セット名⇒サクラエディタ内部コードの変換
		if (name == "UTF-8") return CODE_UTF8;
		if (name == "SHIFT_JIS") return CODE_SJIS;
		if (name == "UTF-16BE") return CODE_UNICODEBE;
		if (name == "UTF-16LE") return CODE_UNICODE;
		if (name == "EUC-JP") return CODE_EUC;
		if (name == "ISO-2022-JP") return CODE_JIS;
		if (name == "UTF-7") return CODE_UTF7;
		if (name == "ISO-8859-1") return CODE_LATIN1;

		return CODE_ERROR;
	}
	if (_uchardet.IsAvailable()) {
		if (!_ud) {
			_ud = _uchardet.uchardet_new();
		}
		if (!_ud) {
			return CODE_ERROR;
		}
		_uchardet.uchardet_reset(_ud);
		if (_uchardet.uchardet_handle_data(_ud, bytes.data(), bytes.length()) != 0) {
			return CODE_ERROR;
		}
		_uchardet.uchardet_data_end(_ud);
		std::string_view name = _uchardet.uchardet_get_charset(_ud);
		std::string str(name);
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		auto it = map_charsetToCodePage.find(str);
		if (it == map_charsetToCodePage.end()) {
			return CODE_ERROR;
		}else {
			return (ECodeType)it->second;
		}
	}
	return CODE_ERROR;
}
