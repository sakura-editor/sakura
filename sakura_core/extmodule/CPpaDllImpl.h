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

#include "extmodule/CDllHandler.h"

class CPpaDllImpl : public CDllImp {
private:
	void	(WINAPI *m_pfnExecute)();
	void	(WINAPI* m_pfnSetSource)(const char* ss);
	void	(WINAPI* m_pfnSetDeclare)(const char* ss);
	void	(WINAPI* m_pfnSetDefProc)(const char* ss);
	void	(WINAPI* m_pfnSetDefine)(const char* ss);
	void	(WINAPI* m_pfnAddIntVar)(const char*, int, int);
	void	(WINAPI* m_pfnAddStrVar)(const char*, const char*, int);
	void	(WINAPI *m_pfnSetIntFunc)(void* p);
	void	(WINAPI *m_pfnSetStrFunc)(void* p);
	void	(WINAPI *m_pfnSetProc)(void* p);
	void	(WINAPI *m_pfnSetErrProc)(void* p);
	void	(WINAPI *m_pfnAbort)();
	int		(WINAPI *m_pfnGetVersion)();
	void	(WINAPI *m_pfnDeleteVar)(const char*);
	int		(WINAPI *m_pfnGetArgInt)(int);
	char*	(WINAPI *m_pfnGetArgStr)(int);
	char*	(WINAPI *m_pfnGetArgBStr)(int);
	void	(WINAPI *m_pfnSetStrObj)(void* proc);
	void	(WINAPI *m_pfnSetIntObj)(void* proc);
	void	(WINAPI *m_pfnAddIntObj)(const char*, int, BOOL, int);
	void	(WINAPI *m_pfnAddStrObj)(const char*, const char*, BOOL, int);
	int		(WINAPI *m_pfnGetIntVar)(const char* ss);
	char*	(WINAPI *m_pfnGetStrVar)(const char* ss);
	char*	(WINAPI *m_pfnGetBStrVar)(const char* ss);
	BOOL	(WINAPI *m_pfnSetIntVar)(const char*, int);
	BOOL	(WINAPI *m_pfnSetStrVar)(const char*, const char*);
	void	(WINAPI *m_pfnAddRealVar)(const char*, double, BOOL);
	void	(WINAPI *m_pfnSetRealObj)(void* p);
	void	(WINAPI *m_pfnAddRealObj)(const char*, double, BOOL, LONG);
	double	(WINAPI *m_pfnGetRealVar)(const char*);
	BOOL	(WINAPI *m_pfnSetRealVar)(const char*, double);
	void	(WINAPI *m_pfnSetRealFunc)(void* p);
	DWORD	(WINAPI *m_pfnGetArgReal)(int);
	BYTE	(WINAPI *m_pfnIsRunning)();
	void	(WINAPI *m_pfnSetFinishProc)(void* p);	//	2003.06.01 Moca

protected:
	bool InitDllImp() override;

	void	Execute() const;
	void	SetSource(const char* ss) const;
	void	SetDeclare(const char* ss) const;
	void	SetDefProc(const char* ss) const;
	void	SetDefine(const char* ss) const;
	void	AddIntVar(const char* lpszDef, int nVal, int nCnst) const;
	void	AddStrVar(const char* lpszDef, const char* lpszVal, int nCnst) const;
	void	SetIntFunc(void* proc) const;
	void	SetStrFunc(void* proc) const;
	void	SetProc(void* proc) const;
	void	SetErrProc(void* proc) const;
	void	Abort() const;
	int		GetVersion() const;
	void	DeleteVar(const char* ss) const;
	int		GetArgInt(int index) const;
	char*	GetArgStr(int index) const;
	char*	GetArgBStr(int index) const;
	void	SetStrObj(void* proc) const;
	void	SetIntObj(void* proc) const;
	void	AddIntObj(const char* ss, int def, BOOL read, int index) const;
	void	AddStrObj(const char* ss, const char* def, BOOL read, int index) const;
	int		GetIntVar(const char* ss) const;
	char*	GetStrVar(const char* ss) const;
	char*	GetBStrVar(const char* ss) const;
	BOOL	SetIntVar(const char* ss, int val) const;
	BOOL	SetStrVar(const char* ss, const char* val) const;

	void	AddRealVar(const char* ss, double val, BOOL cnst) const;
	void	SetRealObj(void* proc) const;
	void	AddRealObj(const char* ss, double val, BOOL read, LONG index) const;
	double	GetRealVar(const char* ss) const;
	BOOL	SetRealVar(const char* ss, double val) const;
	void	SetRealFunc(void* proc) const;
	DWORD	GetArgReal(int index) const;

	BOOL	IsRunning() const;
	void	SetFinishProc(void* proc) const;

public:
	LPCWSTR	GetDllNameImp(int nIndex) override;
};
