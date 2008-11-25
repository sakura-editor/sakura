#pragma once

#include "charset/CESI.h"

class CCodeMediator{
protected:
	// CESI.cpp �̔���֐��������Ɉڂ�
	static ECodeType DetectMBCode( CESI* );
	static ECodeType DetectUnicode( CESI* );

public:

	static ECodeType DetectUnicodeBom( const char* pS, const int nLen );

	/* ���{��R�[�h�Z�b�g���� */
	static ECodeType CheckKanjiCode( const char*, int );
	static ECodeType CheckKanjiCode( CESI* );  // CESI �\���́i�H�j���O���ō\�z�����ꍇ�Ɏg�p
	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
	static ECodeType CheckKanjiCodeOfFile( const TCHAR* );

};

#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
