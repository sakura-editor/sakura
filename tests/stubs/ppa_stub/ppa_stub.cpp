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

void WINAPI Execute()
{
}

void WINAPI SetSource(const char* ss)
{
}

void WINAPI SetDeclare(const char* ss)
{
}

void WINAPI SetDefProc(const char* ss)
{
}

void WINAPI SetDefine(const char* ss)
{
}

void WINAPI AddIntVar(const char* lpszDef, int nVal, int nCnst)
{
}

void WINAPI AddStrVar(const char* lpszDef, const char* lpszVal, int nCnst)
{
}

void WINAPI SetIntFunc(void* proc)
{
}

void WINAPI SetStrFunc(void* proc)
{
}

void WINAPI SetProc(void* proc)
{
}

void WINAPI SetErrProc(void* proc)
{
}

void WINAPI ppaAbort()
{
}

int WINAPI GetPpaVersion()
{
	return 0;
}

void WINAPI DeleteVar(const char* ss)
{
}

int WINAPI GetArgInt(int index)
{
	return 0;
}

char* WINAPI GetArgStr(int index)
{
	return nullptr;
}

char* WINAPI GetArgBStr(int index)
{
	return nullptr;
}

void WINAPI SetStrObj(void* proc)
{
}

void WINAPI SetIntObj(void* proc)
{
}

void WINAPI AddIntObj(const char* ss, int def, BOOL read, int index)
{
}

void WINAPI AddStrObj(const char* ss, const char* def, BOOL read, int index)
{
}

int WINAPI GetIntVar(const char* ss)
{
	return 0;
}

char* WINAPI GetStrVar(const char* ss)
{
	return nullptr;
}

char* WINAPI GetBStrVar(const char* ss)
{
	return nullptr;
}

BOOL WINAPI SetIntVar(const char* ss, int val)
{
	return FALSE;
}

BOOL WINAPI SetStrVar(const char* ss, const char* val)
{
	return FALSE;
}

void WINAPI AddRealVar(const char* ss, double val, BOOL cnst)
{
}

void WINAPI SetRealObj(void* proc)
{
}

void WINAPI AddRealObj(const char* ss, double val, BOOL read, LONG index)
{
}

double WINAPI GetRealVar(const char* ss)
{
	return 0;
}

BOOL WINAPI SetRealVar(const char* ss, double val)
{
	return FALSE;
}

void WINAPI SetRealFunc(void* proc)
{
}

DWORD WINAPI GetArgReal(int index)
{
	return NULL;
}

BYTE WINAPI IsRunning()
{
	return FALSE;
}

void WINAPI SetFinishProc(void* proc)
{
}
