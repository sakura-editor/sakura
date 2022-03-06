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
#pragma once

#include "ppa.h"

#include <memory>

// PPAスタブのAPI関数
extern "C" {
	void	APIENTRY CallIntFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, int* resultValue);
	void	APIENTRY CallStrFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, LPSTR* resultValue);
	void	APIENTRY CallProc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd);
	void	APIENTRY CallErrorProc(int errCd, LPCSTR errMsg);
	void	APIENTRY CallStrObj(LPCSTR ObjName, int index, BYTE GS_Mode, int* errCd, LPSTR* value);
	void	APIENTRY CallFinishProc();
}

/*!
	PPAスタブ

	PPAから呼ばれるコールバック関数をテストするためのもの。
	DLLロード時に生成し、アンロード時に破棄する。
	テスト目的なのでマルチスレッドは考慮していない。
	マルチスレッド考慮が必要と判明したら拡張すること。
 */
class CPpaStub {
private:
	static std::unique_ptr<CPpaStub> gm_Instance;

	decltype(&ppaProc)		m_pfnProc = nullptr;
	decltype(&ppaIntFunc)		m_pfnIntFunc = nullptr;
	decltype(&ppaStrFunc)		m_pfnStrFunc = nullptr;
	decltype(&ppaStrObj)		m_pfnStrObj = nullptr;
	decltype(&ppaErrorProc)		m_pfnErrorProc = nullptr;
	decltype(&ppaFinishProc)	m_pfnFinishProc = nullptr;

public:
	static void createInstance();
	static CPpaStub* getInstance() noexcept;
	static void releaseInstance() noexcept;

	void	SetProc(decltype(&ppaProc) p);
	void	CallProc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd) const;
	void	SetIntFunc(decltype(&ppaIntFunc) p);
	void	CallIntFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, int* resultValue) const;
	void	SetStrFunc(decltype(&ppaStrFunc) p);
	void	CallStrFunc(LPCSTR funcName, int index, LPCSTR* argv, const int argc, int* errCd, LPSTR* resultValue) const;
	void	SetStrObj(decltype(&ppaStrObj) p);
	void	CallStrObj(LPCSTR ObjName, int index, BYTE GS_Mode, int* errCd, LPSTR* value) const;
	void	SetErrorProc(decltype(&ppaErrorProc) p);
	void	CallErrorProc(int errCd, LPCSTR errMsg) const;
	void	SetFinishProc(decltype(&ppaFinishProc) p);
	void	CallFinishProc() const;
};
