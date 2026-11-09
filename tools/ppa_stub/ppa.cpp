/*!	@file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "ppa.h"

extern "C" {
	PPA_API void	APIENTRY AddIntObj(LPCSTR, int, BOOL, int) {}
	PPA_API void	APIENTRY AddIntVar(LPCSTR, int, int) {}
	PPA_API void	APIENTRY AddRealObj(LPCSTR, double, BOOL, LONG) {}
	PPA_API void	APIENTRY AddRealVar(LPCSTR, double, BOOL) {}
	PPA_API void	APIENTRY AddStrObj(LPCSTR, LPCSTR, BOOL, int) {}
	PPA_API void	APIENTRY AddStrVar(LPCSTR, LPCSTR, int) {}
	PPA_API void	APIENTRY DeleteVar(LPCSTR) {}
	PPA_API void	APIENTRY Execute() {}
	PPA_API LPSTR	APIENTRY GetArgBStr(int) { return nullptr; }
	PPA_API int		APIENTRY GetArgInt(int) { return 0; }
	PPA_API DWORD	APIENTRY GetArgReal(int) { return 0; }
	PPA_API LPSTR	APIENTRY GetArgStr(int) { return nullptr; }
	PPA_API LPSTR	APIENTRY GetBStrVar(LPCSTR) { return nullptr; }
	PPA_API int		APIENTRY GetIntVar(LPCSTR) { return 0; }
	PPA_API int		APIENTRY GetPpaVersion() { return 124; }
	PPA_API double	APIENTRY GetRealVar(LPCSTR) { return 0; }
	PPA_API LPSTR	APIENTRY GetStrVar(LPCSTR) { return nullptr; }
	PPA_API BYTE	APIENTRY IsRunning() { return FALSE; }
	PPA_API void	APIENTRY SetDeclare(LPCSTR) {}
	PPA_API void	APIENTRY SetDefProc(LPCSTR) {}
	PPA_API void	APIENTRY SetDefine(LPCSTR) {}
	PPA_API void	APIENTRY SetIntObj(void*) {}
	PPA_API BOOL	APIENTRY SetIntVar(LPCSTR, int) { return FALSE; }
	PPA_API void	APIENTRY SetRealFunc(void*) {}
	PPA_API void	APIENTRY SetRealObj(void*) {}
	PPA_API BOOL	APIENTRY SetRealVar(LPCSTR, double) { return FALSE; }
	PPA_API void	APIENTRY SetSource(LPCSTR ss) {}
	PPA_API BOOL	APIENTRY SetStrVar(LPCSTR, LPCSTR) { return FALSE; }
	PPA_API void	APIENTRY ppaAbort() {}
}
