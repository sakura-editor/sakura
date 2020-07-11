/*! @file */
#include "StdAfx.h"
#include "debug/Debug2.h"

//2007.08.30 kobake 追加

#ifdef _DEBUG
bool g_IsDebugGUIMode = true;

//!MessageBox を表示するかどうかを変更する
void debug_setDebugGUIMode(bool isGUIMode)
{
	g_IsDebugGUIMode = isGUIMode;
}

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

//!強制終了
void debug_exit()
{
	if (g_IsDebugGUIMode)
	{
		MessageBox(NULL,L"assertとかに引っ掛かったぽいです",GSTR_APPNAME,MB_OK);
	}
	else
	{
		fprintf(stderr, "assertとかに引っ掛かったぽいです");
	}
	exit(1);
}

void debug_exit2(const char* file, int line, const char* exp)
{
	char szBuffer[1024];
	wsprintfA(szBuffer, "assert\n%s(%d):\n%s", file, line, exp);
	if (g_IsDebugGUIMode)
	{
		MessageBoxA(NULL, szBuffer , "sakura", MB_OK);
	}
	else
	{
		fprintf(stderr, "%s\n", szBuffer);
	}
	exit(1);
}

void warning_point()
{
	int n;
	n=0; //※←ここにブレークポイントを設けておくと、任意ワーニングでブレークできる
}
#endif	// _DEBUG
