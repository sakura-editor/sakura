#pragma once

class CCodeMediator{
public:
	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
	static ECodeType CheckKanjiCodeOfFile( const TCHAR* );
	/* ���{��R�[�h�Z�b�g���� */
	static ECodeType CheckKanjiCode( const unsigned char*, int );
};

#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
