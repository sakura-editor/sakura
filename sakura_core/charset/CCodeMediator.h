#pragma once

#include "charset/CESI.h"
class CEditDoc;

class CCodeMediator{
protected:
	// CESI.cpp の判定関数をここに移す
	static ECodeType DetectMBCode( CESI* );
	static ECodeType DetectUnicode( CESI* );

public:

	explicit CCodeMediator( CEditDoc &ref ) : m_pcEditDoc(&ref) { }

	static ECodeType DetectUnicodeBom( const char* pS, const int nLen );

	/* 日本語コードセット判別 */
	ECodeType CheckKanjiCode( const char*, int );
	/* ファイルの日本語コードセット判別 */
	ECodeType CheckKanjiCodeOfFile( const TCHAR* );

	static ECodeType CheckKanjiCode( CESI* );  // CESI 構造体（？）を外部で構築した場合に使用

private:
	CEditDoc* m_pcEditDoc;
};

#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
#include "CCesu8.h"
