/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

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
#include <Windows.h>

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>

#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

#include "config/system_constants.h"
#include "env/DLLSHAREDATA.h"

#include "apiwrap/kernel/file_mapping.hpp"
#include "apiwrap/kernel/format_message.hpp"
#include "apiwrap/kernel/handle_closer.hpp"
#include "apiwrap/kernel/message_error.hpp"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

int emulate(std::string_view action, int count);
std::string to_multibyte(std::wstring_view src, UINT codePage);

int WINAPI wWinMain(
	_In_ HINSTANCE	    hInstance,      //!< handle to current instance
	_In_opt_ HINSTANCE  hPrevInstance,  //!< handle to previous instance
	_In_ LPWSTR         lpCmdLine,      //!< pointer to command line
	_In_ int            nCmdShow        //!< show state of window
)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	// ロケールを初期化
	std::locale::global(std::locale(""));

	// std::wcoutにロケールを設定
	std::wcout.imbue(std::locale());

	// std::wcerrにロケールを設定
	std::wcerr.imbue(std::locale());

	// lpCmdLineを解析
	int argc = 0;
	const auto wargv = CommandLineToArgvW(lpCmdLine, &argc);

	std::array<WCHAR, _MAX_PATH> exePath;
	GetModuleFileNameW(nullptr, exePath.data(), DWORD(exePath.size()));

	std::vector<std::string> utf8_buffs;
	utf8_buffs.reserve(argc + 1);
	utf8_buffs.emplace_back(to_multibyte(exePath.data(), CP_UTF8));
	for (int i = 0; i < argc; ++i) {
		utf8_buffs.emplace_back(to_multibyte(wargv[i], CP_UTF8));
	}

	std::vector<LPCSTR> args;
	args.reserve(utf8_buffs.size());
	std::ranges::transform(utf8_buffs, std::back_inserter(args), [](const std::string& s) { return s.c_str(); });

	argc = static_cast<int>(args.size());
	auto argv = args.data();

	try {
        cxxopts::Options options("TrayClickEmulator", "Description of the program");

        options.add_options()
            ("action", "Action to perform (click or context)", cxxopts::value<std::string>()->default_value("click"))
            ("count", "Number of times to repeat the action", cxxopts::value<int>()->default_value("1"));

		options.parse_positional("action", "count");

		auto result = options.parse(argc, argv);

		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			exit(0);
		}

		if (!result.count("action")) {
            std::cerr << "Error: --action is required." << std::endl;
            return 1;
        }

		const auto action = result["action"].as<std::string>();
        const auto count = result["count"].as<int>();

		if ("context"s != action && "click"s != action) {
			std::cerr << "Error: --action must be 'click' or 'context'." << std::endl;
			return 1;
		}

		if (count < 0 || 2 < count) {
			std::cerr << "Error: --count must be 1 or 2." << std::endl;
			return 1;
		}

		return emulate(action, count);

	} catch (const message_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;

	} catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }
}

int emulate(std::string_view action, int count)
{
	file_mapping<DLLSHAREDATA> mapping;
	if (const bool isCreated = mapping.CreateMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof(DLLSHAREDATA),
		L"SakuraShareDataM64WP_DEBUG179" // GSTR_SHAREDATA = (L"SakuraShareData" _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_) _T(STR_SHAREDATA_VERSION))
	))
	{
		format_message(GetLastError(), [](LPCWSTR pszMsg) {
			throw message_error(fmt::format(L"Failed to create file mapping: {}", pszMsg));
		});
	}

	const auto hWndTray = mapping.m_pData->m_sHandles.m_hwndTray;
	if (!hWndTray) {
		throw message_error(L"hWndTray is not found.");
	}

	const auto code1 = "context"s == action ? WM_RBUTTONDOWN : WM_LBUTTONDOWN;
	const auto code2 = "context"s == action ? WM_RBUTTONUP : WM_LBUTTONUP;
	const auto code3 = "context"s == action ? WM_RBUTTONDBLCLK : WM_LBUTTONDBLCLK;

	for (int i = 0; i < count; ++i) {
		Sleep(100);
		PostMessageW(hWndTray, MYWM_NOTIFYICON, 0, code1);
		Sleep(100);
		PostMessageW(hWndTray, MYWM_NOTIFYICON, 0, code2);
	}
	if (2 == count) {
		PostMessageW(hWndTray, MYWM_NOTIFYICON, 0, code3);
	}

	return 0;
}

std::string to_multibyte(std::wstring_view src, UINT codePage) {
	constexpr auto lpDefaultChar = LPCCH(nullptr);
	constexpr auto lpUsedDefaultChar = LPBOOL(nullptr);
	std::string dst;
	if (const auto required = WideCharToMultiByte(codePage, 0, &src[0], int(src.size()), nullptr, 0, lpDefaultChar, lpUsedDefaultChar))
	{
		dst.resize(required, wchar_t());
		WideCharToMultiByte(codePage, 0, &src[0], int(src.size()), dst.data(), required, lpDefaultChar, lpUsedDefaultChar);
	}
	return dst;
}
