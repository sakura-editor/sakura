/*!	@file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include <Windows.h>

#if defined(BUILD_PPA)
	#define PPA_API __declspec(dllexport)
#else
	#define PPA_API __declspec(dllimport)
#endif

// PPAのコールバック関数
extern "C" {
	void CALLBACK ppaProc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd);
	void CALLBACK ppaIntFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, int* resultValue);
	void CALLBACK ppaStrFunc(LPCSTR funcName, int index, LPCSTR* argv, int argc, int* errCd, LPSTR* resultValue);
	void CALLBACK ppaStrObj(LPCSTR objName, int index, BYTE GS_Mode, int* errCd, LPSTR* value);
	void CALLBACK ppaErrorProc(int errCd, LPCSTR errMsg);
	void CALLBACK ppaFinishProc();
}

// PPAのAPI関数(サクラエディタでコールバックを実装している分)
extern "C" {
	PPA_API void	APIENTRY SetProc(decltype(&ppaProc) proc);
	PPA_API void	APIENTRY SetIntFunc(decltype(&ppaIntFunc) proc);
	PPA_API void	APIENTRY SetStrFunc(decltype(&ppaStrFunc) proc);
	PPA_API void	APIENTRY SetStrObj(decltype(&ppaStrObj) proc);
	PPA_API void	APIENTRY SetErrProc(decltype(&ppaErrorProc) proc);
	PPA_API void	APIENTRY SetFinishProc(decltype(&ppaFinishProc) proc);
}

// PPAのAPI関数(未使用関数を含む)
extern "C" {
	PPA_API void	APIENTRY AddIntObj(LPCSTR, int, BOOL, int);
	PPA_API void	APIENTRY AddIntVar(LPCSTR, int, int);
	PPA_API void	APIENTRY AddRealObj(LPCSTR, double, BOOL, LONG);
	PPA_API void	APIENTRY AddRealVar(LPCSTR, double, BOOL);
	PPA_API void	APIENTRY AddStrObj(LPCSTR, LPCSTR, BOOL, int);
	PPA_API void	APIENTRY AddStrVar(LPCSTR, LPCSTR, int);
	PPA_API void	APIENTRY DeleteVar(LPCSTR);
	PPA_API void	APIENTRY Execute();
	PPA_API LPSTR	APIENTRY GetArgBStr(int);
	PPA_API int		APIENTRY GetArgInt(int);
	PPA_API DWORD	APIENTRY GetArgReal(int);
	PPA_API LPSTR	APIENTRY GetArgStr(int);
	PPA_API LPSTR	APIENTRY GetBStrVar(LPCSTR);
	PPA_API int		APIENTRY GetIntVar(LPCSTR);
	PPA_API int		APIENTRY GetPpaVersion();
	PPA_API double	APIENTRY GetRealVar(LPCSTR);
	PPA_API LPSTR	APIENTRY GetStrVar(LPCSTR);
	PPA_API BYTE	APIENTRY IsRunning();
	PPA_API void	APIENTRY SetDeclare(LPCSTR);
	PPA_API void	APIENTRY SetDefProc(LPCSTR);
	PPA_API void	APIENTRY SetDefine(LPCSTR);
	PPA_API void	APIENTRY SetIntObj(void*);
	PPA_API BOOL	APIENTRY SetIntVar(LPCSTR, int);
	PPA_API void	APIENTRY SetRealFunc(void*);
	PPA_API void	APIENTRY SetRealObj(void*);
	PPA_API BOOL	APIENTRY SetRealVar(LPCSTR, double);
	PPA_API void	APIENTRY SetSource(LPCSTR);
	PPA_API BOOL	APIENTRY SetStrVar(LPCSTR, LPCSTR);
	PPA_API void	APIENTRY ppaAbort();
}
