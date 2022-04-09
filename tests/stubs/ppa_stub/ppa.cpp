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
#include "ppa.h"

void	APIENTRY AddIntObj(LPCSTR, int, BOOL, int) {}
void	APIENTRY AddIntVar(LPCSTR, int, int) {}
void	APIENTRY AddRealObj(LPCSTR, double, BOOL, LONG) {}
void	APIENTRY AddRealVar(LPCSTR, double, BOOL) {}
void	APIENTRY AddStrObj(LPCSTR, LPCSTR, BOOL, int) {}
void	APIENTRY AddStrVar(LPCSTR, LPCSTR, int) {}
void	APIENTRY DeleteVar(LPCSTR) {}
void	APIENTRY Execute() {}
LPSTR	APIENTRY GetArgBStr(int) { return nullptr; }
int		APIENTRY GetArgInt(int) { return 0; }
DWORD	APIENTRY GetArgReal(int) { return 0; }
LPSTR	APIENTRY GetArgStr(int) { return nullptr; }
LPSTR	APIENTRY GetBStrVar(LPCSTR) { return nullptr; }
int		APIENTRY GetIntVar(LPCSTR) { return 0; }
int		APIENTRY GetPpaVersion() { return 124; }
double	APIENTRY GetRealVar(LPCSTR) { return 0; }
LPSTR	APIENTRY GetStrVar(LPCSTR) { return nullptr; }
BYTE	APIENTRY IsRunning() { return FALSE; }
void	APIENTRY SetDeclare(LPCSTR) {}
void	APIENTRY SetDefProc(LPCSTR) {}
void	APIENTRY SetDefine(LPCSTR) {}
void	APIENTRY SetIntObj(void*) {}
BOOL	APIENTRY SetIntVar(LPCSTR, int) { return FALSE; }
void	APIENTRY SetRealFunc(void*) {}
void	APIENTRY SetRealObj(void*) {}
BOOL	APIENTRY SetRealVar(LPCSTR, double) { return FALSE; }
void	APIENTRY SetSource(LPCSTR ss) {}
BOOL	APIENTRY SetStrVar(LPCSTR, LPCSTR) { return FALSE; }
void	APIENTRY ppaAbort() {}
