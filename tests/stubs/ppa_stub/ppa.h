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

#include <Windows.h>

// PPAのコールバック関数
extern "C" {
	void CALLBACK ppaProc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd);
	void CALLBACK ppaIntFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, int* resultValue);
	void CALLBACK ppaStrFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, LPSTR* resultValue);
	void CALLBACK ppaStrObj(LPCSTR objName, int index, BYTE GS_Mode, int* errCd, LPSTR* value);
	void CALLBACK ppaErrorProc(int errCd, LPCSTR errMsg);
	void CALLBACK ppaFinishProc();
}

// PPAのAPI関数
extern "C" {
	void	APIENTRY SetProc(decltype(&ppaProc) p);
	void	APIENTRY SetIntFunc(decltype(&ppaIntFunc) proc);
	void	APIENTRY SetRealFunc(void* p);
	void	APIENTRY SetStrFunc(decltype(&ppaStrFunc) proc);
	void	APIENTRY SetIntObj(void* proc);
	void	APIENTRY SetRealObj(void* p);
	void	APIENTRY SetStrObj(decltype(&ppaStrObj) proc);
	void	APIENTRY SetErrProc(decltype(&ppaErrorProc) proc);
	void	APIENTRY SetFinishProc(decltype(&ppaFinishProc) proc);

	void	APIENTRY AddIntObj(LPCSTR, int, BOOL, int);
	void	APIENTRY AddIntVar(LPCSTR, int, int);
	void	APIENTRY AddRealObj(LPCSTR, double, BOOL, LONG);
	void	APIENTRY AddRealVar(LPCSTR, double, BOOL);
	void	APIENTRY AddStrObj(LPCSTR, LPCSTR, BOOL, int);
	void	APIENTRY AddStrVar(LPCSTR, LPCSTR, int);
	void	APIENTRY DeleteVar(LPCSTR);
	void	APIENTRY Execute();
	LPSTR	APIENTRY GetArgBStr(int);
	int		APIENTRY GetArgInt(int);
	DWORD	APIENTRY GetArgReal(int);
	LPSTR	APIENTRY GetArgStr(int);
	LPSTR	APIENTRY GetBStrVar(LPCSTR ss);
	int		APIENTRY GetIntVar(LPCSTR ss);
	int		APIENTRY GetPpaVersion();
	double	APIENTRY GetRealVar(LPCSTR);
	LPSTR	APIENTRY GetStrVar(LPCSTR ss);
	BYTE	APIENTRY IsRunning();
	void	APIENTRY SetDeclare(LPCSTR ss);
	void	APIENTRY SetDefProc(LPCSTR ss);
	void	APIENTRY SetDefine(LPCSTR ss);
	BOOL	APIENTRY SetIntVar(LPCSTR, int);
	BOOL	APIENTRY SetRealVar(LPCSTR, double);
	void	APIENTRY SetSource(LPCSTR ss);
	BOOL	APIENTRY SetStrVar(LPCSTR, LPCSTR);
	void	APIENTRY ppaAbort();

}
