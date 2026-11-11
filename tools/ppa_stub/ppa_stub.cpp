/*!	@file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "ppa_stub.h"

#include <cassert>

//! PPAスタブのインスタンス(static)
std::unique_ptr<CPpaStub> CPpaStub::gm_Instance = nullptr;

/*!
	DLLのエントリポイント

	プロセスのアタッチ時にPPAスタブのインスタンスを生成し、
	プロセスのデタッチ時にPPAスタブのインスタンスを破棄する。
 */
BOOL APIENTRY DllMain(
	[[maybe_unused]] HMODULE hModule,
	DWORD ul_reason_for_call,
	[[maybe_unused]] LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CPpaStub::createInstance();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
	default:
		CPpaStub::releaseInstance();
		break;
	}
	return TRUE;
}

//! PPAスタブのインスタンスを生成する
void CPpaStub::createInstance() {
	assert(!gm_Instance);

	gm_Instance = std::make_unique<CPpaStub>();
}

//! PPAスタブのインスタンスを取得する
CPpaStub* CPpaStub::getInstance() noexcept {
	assert(gm_Instance);

	return gm_Instance.get();
}

//! PPAスタブのインスタンスを解放する
void CPpaStub::releaseInstance() noexcept {
	gm_Instance.reset();
}

void APIENTRY SetProc(decltype(&ppaProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetProc(proc);
	}
}

void CPpaStub::SetProc(decltype(&ppaProc) proc) {
	m_pfnProc = proc;
}

void APIENTRY SetIntFunc(decltype(&ppaIntFunc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetIntFunc(proc);
	}
}

void CPpaStub::SetIntFunc(decltype(&ppaIntFunc) proc) {
	m_pfnIntFunc = proc;
}

void APIENTRY SetStrFunc(decltype(&ppaStrFunc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetStrFunc(proc);
	}
}

void CPpaStub::SetStrFunc(decltype(&ppaStrFunc) proc) {
	m_pfnStrFunc = proc;
}

void APIENTRY SetStrObj(decltype(&ppaStrObj) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetStrObj(proc);
	}
}

void CPpaStub::SetStrObj(decltype(&ppaStrObj) proc) {
	m_pfnStrObj = proc;
}

void APIENTRY SetErrProc(decltype(&ppaErrorProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetErrorProc(proc);
	}
}

void CPpaStub::SetErrorProc(decltype(&ppaErrorProc) proc) {
	m_pfnErrorProc = proc;
}

void APIENTRY SetFinishProc(decltype(&ppaFinishProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetFinishProc(proc);
	}
}

void CPpaStub::SetFinishProc(decltype(&ppaFinishProc) proc) {
    m_pfnFinishProc = proc;
}
