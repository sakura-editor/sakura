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
	{ "UTF-32BE",			(WORD)12001, },
	{ "UTF-32LE",			(WORD)12000, },
	// Arabic
	{ "ISO-8859-6",			(WORD)28596, },
	{ "WINDOWS-1256",		(WORD)1256, },
	// Bulgarian
	{ "ISO-8859-5",			(WORD)28595, },
	{ "WINDOWS-1251",		(WORD)1251, },
	// Chinese
	{ "ISO-2022-CN",		(WORD)50227, },
	{ "BIG5",				(WORD)950, },
	{ "EUC-TW",				(WORD)51950, },
	{ "GB18030",			(WORD)54936, },
	{ "HZ-GB-2312",			(WORD)52936, },
	// Croatian
	{ "ISO-8859-2",			(WORD)28592, },
	{ "ISO-8859-13",		(WORD)28603, },
	{ "ISO-8859-16",		(WORD)28606, },
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "IBM852",				(WORD)852, },
	{ "MAC-CENTRALEUROPE",	(WORD)10029, },
	// Czech
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "ISO-8859-2",			(WORD)28592, },
	{ "IBM852",				(WORD)852, },
	{ "MAC-CENTRALEUROPE",	(WORD)10029, },
	// Danish
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// English
	{ "ASCII",				(WORD)20127, },
	// Esperanto
	{ "ISO-8859-3",			(WORD)28593, },
	// Estonian
	{ "ISO-8859-4",			(WORD)28594, },
	{ "ISO-8859-13",		(WORD)28603, },
	{ "ISO-8859-13",		(WORD)28603, },
	{ "WINDOWS-1252",		(WORD)1252, },
	{ "WINDOWS-1257",		(WORD)1257, },
	// Finnish
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-4",			(WORD)28594, },
	{ "ISO-8859-9",			(WORD)28599, },
	{ "ISO-8859-13",		(WORD)28603, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// French
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// German
	{ "ISO-8859-1",			(WORD)28591, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Greek
	{ "ISO-8859-7",			(WORD)28597, },
	{ "WINDOWS-1253",		(WORD)1253, },
	// Hebrew
	{ "ISO-8859-8",			(WORD)28598, },
	{ "WINDOWS-1255",		(WORD)1255, },
	// Hungarian
	{ "ISO-8859-2",			(WORD)28592, },
	{ "WINDOWS-1250",		(WORD)1250, },
	// Irish Gaelic
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-9",			(WORD)28599, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Italian
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-3",			(WORD)28593, },
	{ "ISO-8859-9",			(WORD)28599, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Japanese
	{ "ISO-2022-JP",		(WORD)50220, },
	{ "SHIFT_JIS",			(WORD)932, },
	{ "EUC-JP",				(WORD)20932, },
	// Korean
	{ "ISO-2022-KR",		(WORD)50225, },
	{ "EUC-KR",				(WORD)51949, },
	// Lithuanian
	{ "ISO-8859-4",			(WORD)28594, },
	{ "ISO-8859-10",		(WORD)28600, },
	{ "ISO-8859-13",		(WORD)28603, },
	// Latvian
	{ "ISO-8859-4",			(WORD)28594, },
	{ "ISO-8859-10",		(WORD)28600, },
	{ "ISO-8859-13",		(WORD)28603, },
	// Maltese
	{ "ISO-8859-3",			(WORD)28593, },
	// Polish
	{ "ISO-8859-2",			(WORD)28592, },
	{ "ISO-8859-13",		(WORD)28603, },
	{ "ISO-8859-16",		(WORD)28606, },
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "IBM852",				(WORD)852, },
	{ "MAC-CENTRALEUROPE",	(WORD)10029, },
	// Portuguese
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-9",			(WORD)28599, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Romanian
	{ "ISO-8859-2",			(WORD)28592, },
	{ "ISO-8859-16",		(WORD)28606, },
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "IBM852",				(WORD)852, },
	// Russian
	{ "ISO-8859-5",			(WORD)28595, },
	{ "KOI8-R",				(WORD)20866, },
	{ "WINDOWS-1251",		(WORD)1251, },
	{ "MAC-CYRILLIC",		(WORD)10007, },
	{ "IBM866",				(WORD)866, },
	{ "IBM855",				(WORD)855, },
	// Slovak
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "ISO-8859-2",			(WORD)28592, },
	{ "IBM852",				(WORD)852, },
	{ "MAC-CENTRALEUROPE",	(WORD)10029, },
	// Slovene
	{ "ISO-8859-2",			(WORD)28592, },
	{ "ISO-8859-16",		(WORD)28606, },
	{ "WINDOWS-1250",		(WORD)1250, },
	{ "IBM852",				(WORD)852, },
	{ "MAC-CENTRALEUROPE",	(WORD)10029, },
	// Spanish
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Swedish
	{ "ISO-8859-1",			(WORD)28591, },
	{ "ISO-8859-4",			(WORD)28594, },
	{ "ISO-8859-9",			(WORD)28599, },
	{ "ISO-8859-15",		(WORD)28605, },
	{ "WINDOWS-1252",		(WORD)1252, },
	// Thai
	{ "TIS-620",			(WORD)874, },
	{ "ISO-8859-11",		(WORD)28601, },
	// Turkish
	{ "ISO-8859-3",			(WORD)28593, },
	{ "ISO-8859-9",			(WORD)28599, },
	// Vietnamese
	//{ "VISCII", , },
	{ "Windows-1258",		(WORD)1258, },
	// Others
	{ "WINDOWS-1252",		(WORD)1252, },
};

CharsetDetector::CharsetDetector() noexcept
{
	_uchardet.InitDll();
}

CharsetDetector::~CharsetDetector() noexcept
{
	if (_uchardet.IsAvailable()) {
		_uchardet.uchardet_delete(_ud);
	}
}

ECodeType CharsetDetector::Detect(const std::string_view& bytes)
{
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
		std::ranges::transform(str, str.begin(), [] (const auto& c) { return (char)::toupper(c); });
		auto it = map_charsetToCodePage.find(str);
		if (it == map_charsetToCodePage.end()) {
			return CODE_ERROR;
		}else {
			return (ECodeType)it->second;
		}
	}
	return CODE_ERROR;
}
