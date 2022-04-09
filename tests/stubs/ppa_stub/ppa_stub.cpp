/*!	@file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#include "ppa_stub.h"

void APIENTRY SetProc(decltype(&ppaProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetProc(proc);
	}
}

void CPpaStub::SetProc(decltype(&ppaProc) proc) {
	m_pfnProc = proc;
}

void CPpaStub::CallProc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd) const {
	if (m_pfnProc) {
		m_pfnProc(funcName, index, argv, argc, errCd);
	}
}

void WINAPI CallProc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd) {
	if (const auto stub = CPpaStub::getInstance()) {
		stub->CallProc(funcName, index, argv, argc, errCd);
	}
}

void APIENTRY SetIntFunc(decltype(&ppaIntFunc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetIntFunc(proc);
	}
}

void CPpaStub::SetIntFunc(decltype(&ppaIntFunc) proc) {
	m_pfnIntFunc = proc;
}

void CPpaStub::CallIntFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, int* resultValue) const {
	if (m_pfnIntFunc) {
		m_pfnIntFunc(funcName, index, argv, argc, errCd, resultValue);
	}
}

void WINAPI CallIntFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, int* resultValue) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->CallIntFunc(funcName, index, argv, argc, errCd, resultValue);
	}
}

void APIENTRY SetStrFunc(decltype(&ppaStrFunc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetStrFunc(proc);
	}
}

void CPpaStub::SetStrFunc(decltype(&ppaStrFunc) proc) {
	m_pfnStrFunc = proc;
}

void CPpaStub::CallStrFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, LPSTR* resultValue) const {
	if (m_pfnStrFunc) {
		m_pfnStrFunc(funcName, index, argv, argc, errCd, resultValue);
	}
}

void WINAPI CallStrFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, LPSTR* resultValue) {
	if (const auto stub = CPpaStub::getInstance()) {
		stub->CallStrFunc(funcName, index, argv, argc, errCd, resultValue);
	}
}

void APIENTRY SetStrObj(decltype(&ppaStrObj) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetStrObj(proc);
	}
}

void CPpaStub::SetStrObj(decltype(&ppaStrObj) proc) {
	m_pfnStrObj = proc;
}

void CPpaStub::CallStrObj(LPCSTR ObjName, int index, BYTE GS_Mode, int* errCd, LPSTR* value) const {
	if (m_pfnStrObj) {
		m_pfnStrObj(ObjName, index, GS_Mode, errCd, value);
	}
}

void WINAPI CallStrObj(LPCSTR ObjName, int index, BYTE GS_Mode, int* errCd, LPSTR* value) {
	if (const auto stub = CPpaStub::getInstance()) {
		stub->CallStrObj(ObjName, index, GS_Mode, errCd, value);
	}
}

void APIENTRY SetErrProc(decltype(&ppaErrorProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetErrorProc(proc);
	}
}

void CPpaStub::SetErrorProc(decltype(&ppaErrorProc) proc) {
	m_pfnErrorProc = proc;
}

void CPpaStub::CallErrorProc(int errCd, LPCSTR errMsg) const {
	if (m_pfnErrorProc) {
		m_pfnErrorProc(errCd, errMsg);
	}
}

void WINAPI CallErrorProc(int errCd, LPCSTR errMsg) {
	if (const auto stub = CPpaStub::getInstance()) {
		stub->CallErrorProc(errCd, errMsg);
	}
}

void APIENTRY SetFinishProc(decltype(&ppaFinishProc) proc) {
	if (auto stub = CPpaStub::getInstance()) {
		stub->SetFinishProc(proc);
	}
}

void CPpaStub::SetFinishProc(decltype(&ppaFinishProc) proc) {
    m_pfnFinishProc = proc;
}

void CPpaStub::CallFinishProc() const {
	if (m_pfnFinishProc) {
		m_pfnFinishProc();
	}
}

void WINAPI CallFinishProc() {
	if (const auto stub = CPpaStub::getInstance()) {
		stub->CallFinishProc();
	}
}
