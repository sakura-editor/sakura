/*! @file
	@brief 核心库中依赖界面的部分的 Mac 版实现

	- sakura::platform 的接口（文字宽度、消息框、界面字符串）和默认实现
	- MessageBoxS（util/MessageBoxF.h）
	- CSelectLang::LoadStringW / CLoadString（CSelectLang.h）的字符串读取部分
	  语言资源 DLL 的选择功能在 Mac 版不使用（界面语言跟随系统设置）。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "sakura_platform_hooks.h"
#include "CSelectLang.h"
#include "util/MessageBoxF.h"

#include <atomic>
#include <cstdio>
#include <mutex>
#include <unordered_map>

namespace sakura::platform {

namespace {

std::atomic<TextMeasureFunction> g_measure{ nullptr };
std::atomic<MessageBoxFunction> g_messageBox{ nullptr };
std::atomic<StringLoaderFunction> g_stringLoader{ nullptr };

//! 东亚宽字符（全角）判定。只在没有注册 Core Text 测量函数时使用。
bool IsWideCodePoint(char32_t c) noexcept
{
	return (c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0x303E) || (c >= 0x3041 && c <= 0x33FF)
		|| (c >= 0x3400 && c <= 0x4DBF) || (c >= 0x4E00 && c <= 0x9FFF) || (c >= 0xA000 && c <= 0xA4CF)
		|| (c >= 0xAC00 && c <= 0xD7A3) || (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFE30 && c <= 0xFE4F)
		|| (c >= 0xFF00 && c <= 0xFF60) || (c >= 0xFFE0 && c <= 0xFFE6) || (c >= 0x1F300 && c <= 0x1F64F)
		|| (c >= 0x20000 && c <= 0x3FFFD);
}

//! 默认实现：按等宽字体估算（半角 = 字高的一半，全角 = 两倍）
int DefaultMeasure(const LOGFONT& font, const WCHAR* text, size_t length)
{
	const int height = font.lfHeight < 0 ? -font.lfHeight : font.lfHeight;
	const int cell = height > 1 ? (height + 1) / 2 : 8;
	int width = 0;
	for (const WCHAR* p = text, *e = text + length; p < e;) {
		const char32_t c = u16::decode_one(p, e);
		width += IsWideCodePoint(c) ? cell * 2 : cell;
	}
	return width;
}

} // namespace

void SetTextMeasureFunction(TextMeasureFunction func) noexcept { g_measure = func; }
int MeasureTextWidth(const LOGFONT& font, const WCHAR* text, size_t length)
{
	const auto f = g_measure.load();
	return f ? f(font, text, length) : DefaultMeasure(font, text, length);
}

void SetMessageBoxFunction(MessageBoxFunction func) noexcept { g_messageBox = func; }
int ShowMessageBox(const WCHAR* text, const WCHAR* caption, UINT type)
{
	if (const auto f = g_messageBox.load()) return f(text, caption, type);
	// 默认实现：写到标准错误输出，按“确定 / 是”处理
	std::fprintf(stderr, "[%s] %s\n", u16::to_utf8(caption ? caption : u"").c_str(), u16::to_utf8(text ? text : u"").c_str());
	switch (type & 0x0F) {
	case MB_YESNO: case MB_YESNOCANCEL: return IDYES;
	default: return IDOK;
	}
}

void SetStringLoaderFunction(StringLoaderFunction func) noexcept { g_stringLoader = func; }
bool LoadResourceString(UINT id, std::basic_string<WCHAR>& out)
{
	if (const auto f = g_stringLoader.load()) return f(id, out);
	out.clear();
	return false;
}

} // namespace sakura::platform

// ---------------------------------------------------------------------------
// util/MessageBoxF.h
// ---------------------------------------------------------------------------
int MessageBoxS(
	const sakura::wstring& text,
	UINT uType,
	HWND /* hWndOwner */,
	const std::optional<sakura::wstring>& optCaption
)
{
	const sakura::wstring caption = optCaption.value_or(LTEXT("Sakura Editor"));
	return sakura::platform::ShowMessageBox(text.c_str(), caption.c_str(), uType);
}

// ---------------------------------------------------------------------------
// CSelectLang.h（文字列の読み込み部分のみ）
// ---------------------------------------------------------------------------
sakura::wstring_view CSelectLang::LoadStringW(UINT id)
{
	// 返したビューが無効にならないよう、読み込んだ文字列は保持し続ける
	static std::mutex mtx;
	static std::unordered_map<UINT, sakura::wstring> cache;
	std::lock_guard lock(mtx);
	if (const auto it = cache.find(id); it != cache.end()) return it->second;
	sakura::wstring s;
	sakura::platform::LoadResourceString(id, s);
	return cache.emplace(id, std::move(s)).first->second;
}

thread_local std::array<CLoadString::CLoadStrBuffer, 16> CLoadString::gm_Buffers{};
thread_local size_t CLoadString::gm_LastUsedIndex = std::size(CLoadString::gm_Buffers);

LPCWSTR CLoadString::LoadStringSt(UINT uid)
{
	++gm_LastUsedIndex;
	if (std::size(gm_Buffers) <= gm_LastUsedIndex) {
		gm_LastUsedIndex = 0;
	}
	auto& workString = gm_Buffers[gm_LastUsedIndex];
	workString.LoadStringW(uid);
	return workString.GetStringPtr();
}

LPCWSTR CLoadString::LoadStringW(UINT uid)
{
	m_Buffer.LoadStringW(uid);
	return m_Buffer.GetStringPtr();
}

size_t CLoadString::CLoadStrBuffer::LoadStringW(UINT uid)
{
	m_ResString = CSelectLang::LoadStringW(uid);
	m_String.assign(m_ResString);
	return m_String.length();
}
