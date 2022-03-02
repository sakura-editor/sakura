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

#include "StdAfx.h"
#include "extmodule/CPpaDllImpl.h"

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval true 成功
	@retval false アドレス取得に失敗
*/
bool CPpaDllImpl::InitDllImp()
{
	const ImportTable table[] = 
	{
		{ &m_pfnExecute,		"Execute" },
		{ &m_pfnSetDeclare,		"SetDeclare" },
		{ &m_pfnSetSource,		"SetSource" },
		{ &m_pfnSetDefProc,		"SetDefProc" },
		{ &m_pfnSetDefine,		"SetDefine" },
		{ &m_pfnSetIntFunc,		"SetIntFunc" },
		{ &m_pfnSetStrFunc,		"SetStrFunc" },
		{ &m_pfnSetProc,		"SetProc" },
		{ &m_pfnSetErrProc,		"SetErrProc" },
		{ &m_pfnAbort,			"ppaAbort" },
		{ &m_pfnGetVersion,		"GetVersion" },
		{ &m_pfnDeleteVar,		"DeleteVar" },
		{ &m_pfnGetArgInt,		"GetArgInt" },
		{ &m_pfnGetArgStr,		"GetArgStr" },
		{ &m_pfnGetArgBStr,		"GetArgBStr" },
		{ &m_pfnGetIntVar,		"GetIntVar" },
		{ &m_pfnGetStrVar,		"GetStrVar" },
		{ &m_pfnGetBStrVar,		"GetBStrVar" },
		{ &m_pfnSetIntVar,		"SetIntVar" },
		{ &m_pfnSetStrVar,		"SetStrVar" },
		{ &m_pfnAddIntObj,		"AddIntObj" },
		{ &m_pfnAddStrObj,		"AddStrObj" },
		{ &m_pfnAddIntVar,		"AddIntVar" },
		{ &m_pfnAddStrVar,		"AddStrVar" },
		{ &m_pfnSetIntObj,		"SetIntObj" },
		{ &m_pfnSetStrObj,		"SetStrObj" },
		{ &m_pfnAddRealVar,		"AddRealVar" },
		{ &m_pfnSetRealObj,		"SetRealObj" },
		{ &m_pfnAddRealObj,		"AddRealObj" },
		{ &m_pfnGetRealVar,		"GetRealVar" },
		{ &m_pfnSetRealVar,		"SetRealVar" },
		{ &m_pfnSetRealFunc,	"SetRealFunc" },
		{ &m_pfnGetArgReal,		"GetArgReal" },
		{ &m_pfnIsRunning,		"IsRunning" },
		{ &m_pfnSetFinishProc,	"SetFinishProc"},
		{ NULL, 0 }
	};

	return RegisterEntries(table);
}

LPCWSTR CPpaDllImpl::GetDllNameImp(int nIndex)
{
	return L"PPA.DLL";
}

void CPpaDllImpl::Execute() const
{
	m_pfnExecute();
}

void CPpaDllImpl::SetSource(const char* ss) const
{
	m_pfnSetSource(ss);
}

void CPpaDllImpl::SetDeclare(const char* ss) const
{
	m_pfnSetDeclare(ss);
}

void CPpaDllImpl::SetDefProc(const char* ss) const
{
	m_pfnSetDefProc(ss);
}

void CPpaDllImpl::SetDefine(const char* ss) const
{
	m_pfnSetDefine(ss);
}

void CPpaDllImpl::AddIntVar(const char* lpszDef, int nVal, int nCnst) const
{
	m_pfnAddIntVar(lpszDef, nVal, nCnst);
}

void CPpaDllImpl::AddStrVar(const char* lpszDef, const char* lpszVal, int nCnst) const
{
	m_pfnAddStrVar(lpszDef, lpszVal, nCnst);
}

void CPpaDllImpl::SetIntFunc(void* proc) const
{
	m_pfnSetIntFunc(proc);
}

void CPpaDllImpl::SetStrFunc(void* proc) const
{
	m_pfnSetStrFunc(proc);
}

void CPpaDllImpl::SetProc(void* proc) const
{
	m_pfnSetProc(proc);
}

void CPpaDllImpl::SetErrProc(void* proc) const
{
	m_pfnSetErrProc(proc);
}

void CPpaDllImpl::Abort() const
{
	m_pfnAbort();
}

int CPpaDllImpl::GetVersion() const
{
	return m_pfnGetVersion();
}

void CPpaDllImpl::DeleteVar(const char* ss) const
{
	m_pfnDeleteVar(ss);
}

int CPpaDllImpl::GetArgInt(int index) const
{
	return m_pfnGetArgInt(index);
}

char* CPpaDllImpl::GetArgStr(int index) const
{
	return m_pfnGetArgStr(index);
}

char* CPpaDllImpl::GetArgBStr(int index) const
{
	return m_pfnGetArgBStr(index);
}

void CPpaDllImpl::SetStrObj(void* proc) const
{
	m_pfnSetStrObj(proc);
}

void CPpaDllImpl::SetIntObj(void* proc) const
{
	m_pfnSetIntObj(proc);
}

void CPpaDllImpl::AddIntObj(const char* ss, int def, BOOL read, int index) const
{
	m_pfnAddIntObj(ss, def, read, index);
}

void CPpaDllImpl::AddStrObj(const char* ss, const char* def, BOOL read, int index) const
{
	m_pfnAddStrObj(ss, def, read, index);
}

int CPpaDllImpl::GetIntVar(const char* ss) const
{
	return m_pfnGetIntVar(ss);
}

char* CPpaDllImpl::GetStrVar(const char* ss) const
{
	return m_pfnGetStrVar(ss);
}

char* CPpaDllImpl::GetBStrVar(const char* ss) const
{
	return m_pfnGetBStrVar(ss);
}

BOOL CPpaDllImpl::SetIntVar(const char* ss, int val) const
{
	return m_pfnSetIntVar(ss, val);
}

BOOL CPpaDllImpl::SetStrVar(const char* ss, const char* val) const
{
	return m_pfnSetStrVar(ss, val);
}

void CPpaDllImpl::AddRealVar(const char* ss, double val, BOOL cnst) const
{
	m_pfnAddRealVar(ss, val, cnst);
}

void CPpaDllImpl::SetRealObj(void* proc) const
{
	m_pfnSetRealObj(proc);
}
void CPpaDllImpl::AddRealObj(const char* ss, double val, BOOL read, LONG index) const
{
	m_pfnAddRealObj(ss, val, read, index);
}
double CPpaDllImpl::GetRealVar(const char* ss) const
{
	return m_pfnGetRealVar(ss);
}

BOOL CPpaDllImpl::SetRealVar(const char* ss, double val) const
{
	return m_pfnSetRealVar(ss, val);
}

void CPpaDllImpl::SetRealFunc(void* proc) const
{
	m_pfnSetRealFunc(proc);
}

DWORD CPpaDllImpl::GetArgReal(int index) const
{
	return m_pfnGetArgReal(index);
}

BOOL CPpaDllImpl::IsRunning() const
{
	return m_pfnIsRunning();
}

void CPpaDllImpl::SetFinishProc(void* proc) const
{
	m_pfnSetFinishProc(proc);
}
