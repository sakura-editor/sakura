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

void APIENTRY SetDefine(LPCSTR ss) {
}

void APIENTRY SetDefProc(LPCSTR ss) {
}

void APIENTRY SetDeclare(LPCSTR ss) {
}

void APIENTRY SetSource(LPCSTR ss) {
}

void APIENTRY Execute() {
}

void APIENTRY AddIntVar(LPCSTR lpszDef, int nVal, int nCnst) {
}

void APIENTRY AddStrVar(LPCSTR lpszDef, LPCSTR lpszVal, int nCnst) {
}

void APIENTRY ppaAbort() {
}

int APIENTRY GetPpaVersion() {
	return 124;
}

void APIENTRY DeleteVar(LPCSTR ss) {
}

int APIENTRY GetArgInt(int index) {
	return 0;
}

LPSTR APIENTRY GetArgStr(int index) {
	return nullptr;
}

LPSTR APIENTRY GetArgBStr(int index) {
	return nullptr;
}

void APIENTRY SetIntObj(void* proc) {
}

void APIENTRY AddIntObj(LPCSTR ss, int def, BOOL read, int index) {
}

void APIENTRY AddStrObj(LPCSTR ss, LPCSTR def, BOOL read, int index) {
}

int APIENTRY GetIntVar(LPCSTR ss) {
	return 0;
}

LPSTR APIENTRY GetStrVar(LPCSTR ss) {
	return nullptr;
}

LPSTR APIENTRY GetBStrVar(LPCSTR ss) {
	return nullptr;
}

BOOL APIENTRY SetIntVar(LPCSTR ss, int val) {
	return FALSE;
}

BOOL APIENTRY SetStrVar(LPCSTR ss, LPCSTR val) {
	return FALSE;
}

void APIENTRY AddRealVar(LPCSTR ss, double val, BOOL cnst) {
}

void APIENTRY SetRealObj(void* proc) {
}

void APIENTRY AddRealObj(LPCSTR ss, double val, BOOL read, LONG index) {
}

double APIENTRY GetRealVar(LPCSTR ss) {
	return 0;
}

BOOL APIENTRY SetRealVar(LPCSTR ss, double val) {
	return FALSE;
}

void APIENTRY SetRealFunc(void* proc) {
}

DWORD APIENTRY GetArgReal(int index) {
	return 0;
}

BYTE APIENTRY IsRunning() {
	return FALSE;
}
