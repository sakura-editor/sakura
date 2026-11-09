/*! @file
	@brief 核心库调用界面层功能的接口（非 Windows 平台）

	核心库里有少数地方需要用到界面：测量文字宽度、显示消息框、读取界面字符串。
	Windows 版直接调用 Win32 API；Mac 版由 App 启动时通过下面的 Set* 函数注册实现
	（第 24 步的桥接层、第 27 步的 Core Text 绘制、第 47 步的本地化）。
	没有注册时使用简单的默认实现，方便单元测试。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#pragma once

#include <string>

namespace sakura::platform {

//! 用指定字体测量文字宽度（像素）
using TextMeasureFunction = int (*)(const LOGFONT& font, const WCHAR* text, size_t length);
void SetTextMeasureFunction(TextMeasureFunction func) noexcept;
int MeasureTextWidth(const LOGFONT& font, const WCHAR* text, size_t length);

//! 显示消息框，返回 IDOK / IDYES 等
using MessageBoxFunction = int (*)(const WCHAR* text, const WCHAR* caption, UINT type);
void SetMessageBoxFunction(MessageBoxFunction func) noexcept;
int ShowMessageBox(const WCHAR* text, const WCHAR* caption, UINT type);

//! 按资源 ID 读取界面字符串。找不到时返回 false
using StringLoaderFunction = bool (*)(UINT id, std::basic_string<WCHAR>& out);
void SetStringLoaderFunction(StringLoaderFunction func) noexcept;
bool LoadResourceString(UINT id, std::basic_string<WCHAR>& out);

} // namespace sakura::platform
