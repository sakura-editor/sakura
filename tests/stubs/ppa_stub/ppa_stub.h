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

extern "C" {
	void	WINAPI Execute();
	void	WINAPI SetSource(const char* ss);
	void	WINAPI SetDeclare(const char* ss);
	void	WINAPI SetDefProc(const char* ss);
	void	WINAPI SetDefine(const char* ss);
	void	WINAPI AddIntVar(const char*, int, int);
	void	WINAPI AddStrVar(const char*, const char*, int);
	void	WINAPI SetIntFunc(void* p);
	void	WINAPI SetStrFunc(void* p);
	void	WINAPI SetProc(void* p);
	void	WINAPI SetErrProc(void* p);
	void	WINAPI ppaAbort();
	int		WINAPI GetPpaVersion();
	void	WINAPI DeleteVar(const char*);
	int		WINAPI GetArgInt(int);
	char*	WINAPI GetArgStr(int);
	char*	WINAPI GetArgBStr(int);
	void	WINAPI SetStrObj(void* proc);
	void	WINAPI SetIntObj(void* proc);
	void	WINAPI AddIntObj(const char*, int, BOOL, int);
	void	WINAPI AddStrObj(const char*, const char*, BOOL, int);
	int		WINAPI GetIntVar(const char* ss);
	char*	WINAPI GetStrVar(const char* ss);
	char*	WINAPI GetBStrVar(const char* ss);
	BOOL	WINAPI SetIntVar(const char*, int);
	BOOL	WINAPI SetStrVar(const char*, const char*);
	void	WINAPI AddRealVar(const char*, double, BOOL);
	void	WINAPI SetRealObj(void* p);
	void	WINAPI AddRealObj(const char*, double, BOOL, LONG);
	double	WINAPI GetRealVar(const char*);
	BOOL	WINAPI SetRealVar(const char*, double);
	void	WINAPI SetRealFunc(void* p);
	DWORD	WINAPI GetArgReal(int);
	BYTE	WINAPI IsRunning();
	void	WINAPI SetFinishProc(void* p);	//	2003.06.01 Moca
}
