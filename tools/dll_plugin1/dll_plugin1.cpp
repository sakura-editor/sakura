/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include <windows.h>

#include <atomic>

namespace {
std::atomic<int> g_callCount = 0;
}

extern "C" __declspec(dllexport) void TestDllEditorStart()
{
	++g_callCount;
}

extern "C" __declspec(dllexport) void TestDllCommand()
{
	++g_callCount;
}

extern "C" __declspec(dllexport) int TestDllGetCallCount()
{
	return g_callCount.load();
}

BOOL APIENTRY DllMain(
	[[maybe_unused]] HMODULE hModule,
	[[maybe_unused]] DWORD ul_reason_for_call,
	[[maybe_unused]] LPVOID lpReserved
)
{
	return TRUE;
}
