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

static void GetAssertFileName(CHAR* szAssertFileName, int nBuffSize)
{
	CHAR szExeFileName[MAX_PATH];
	const int cchExeFileName = ::GetModuleFileNameA(NULL, szExeFileName, MAX_PATH);

	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	
	_splitpath(szExeFileName, szDrive, szDir, NULL, NULL); 
	sprintf(szAssertFileName, "%s%sassert.txt", szDrive, szDir);
}

void debug_exit2(const char* file, int line, const char* exp)
{
	CHAR szLogName[MAX_PATH];
	GetAssertFileName(szLogName, MAX_PATH);

	char szBuffer[1024];
	wsprintfA(szBuffer, "assert\n%s(%d):\n%s", file, line, exp);
	fprintf(stderr, "%s\n", szBuffer);

	FILE * fp = NULL;
	fp = fopen(szLogName, "a");
	
	// 実行ファイルと同じディレクトリに書き込み権限がなかった時の保険
	if (fp == NULL)
	{
		fp = fopen("assert.txt", "a");
	}

	if (fp)
	{
#ifdef GIT_SHORT_COMMIT_HASH
		fprintf(fp, "%s: %s\n", GIT_SHORT_COMMIT_HASH, szBuffer);
#else
		fprintf(fp, "%s\n", szBuffer);
#endif
		fclose(fp);
	}
	throw AssertException();
}

void warning_point()
{
	int n;
	n=0; //※←ここにブレークポイントを設けておくと、任意ワーニングでブレークできる
}
#endif	// _DEBUG
