/*! @file */
#include "StdAfx.h"
#include "debug/Debug2.h"
#include <stdlib.h>
#include "githash.h"

//2007.08.30 kobake 追加

#ifdef _DEBUG
//!デバッグメッセージ出力
void debug_output(const char* str, ...)
{
	char buf[_MAX_PATH+150];
	va_list mark;
	va_start(mark,str);
	// FILE名, LINE 式 分必要
	tchar_vsnprintf_s(buf,_countof(buf),str,mark);
	va_end(mark);

	//デバッガに出力
	OutputDebugStringA(buf);
}

void debug_exit2(const char* file, int line, const char* exp)
{
	char szBuffer[1024];
	wsprintfA(szBuffer, "assert\n%s(%d):\n%s", file, line, exp);
	fprintf(stderr, "%s\n", szBuffer);
	OutputDebugStringA(szBuffer);
	throw AssertException();
}

void warning_point()
{
	int n;
	n=0; //※←ここにブレークポイントを設けておくと、任意ワーニングでブレークできる
}
#endif	// _DEBUG
