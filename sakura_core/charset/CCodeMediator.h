#pragma once

class CCodeMediator{
public:
	/* ファイルの日本語コードセット判別 */
	static ECodeType CheckKanjiCodeOfFile( const TCHAR* );
	/* 日本語コードセット判別 */
	static ECodeType CheckKanjiCode( const unsigned char*, int );
};

#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
