#pragma once

#include "charset/CESI.h"
class CEditDoc;

class CCodeMediator{
protected:
	// CESI.cpp �̔���֐��������Ɉڂ�
	static ECodeType DetectMBCode( CESI* );
	static ECodeType DetectUnicode( CESI* );

public:

	explicit CCodeMediator( CEditDoc &ref ) : m_pcEditDoc(&ref) { }

	static ECodeType DetectUnicodeBom( const char* pS, const int nLen );

	/* ���{��R�[�h�Z�b�g���� */
	ECodeType CheckKanjiCode( const char*, int );
	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
	ECodeType CheckKanjiCodeOfFile( const TCHAR* );

	static ECodeType CheckKanjiCode( CESI* );  // CESI �\���́i�H�j���O���ō\�z�����ꍇ�Ɏg�p

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
