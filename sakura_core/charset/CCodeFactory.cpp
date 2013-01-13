#include "StdAfx.h"
#include "CCodeFactory.h"
#include "CCodeMediator.h"

// move start	from CCodeMediator.h	2012/12/02 Uchi
#include "CEuc.h"
#include "CJis.h"
#include "CShiftJis.h"
#include "CUnicode.h"
#include "CUnicodeBe.h"
#include "CUtf7.h"
#include "CUtf8.h"
#include "CCesu8.h"
// move end
#include "CLatin1.h"

//! eCodeType�ɓK������ CCodeBase�C���X�^���X �𐶐�
CCodeBase* CCodeFactory::CreateCodeBase(
	ECodeType	eCodeType,		//!< �����R�[�h
	int			nFlag			//!< bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
)
{
  	switch( eCodeType ){
	case CODE_SJIS:			return new CShiftJis();
	case CODE_EUC:			return new CEuc();
	case CODE_JIS:			return new CJis((nFlag&1)==1);
	case CODE_UNICODE:		return new CUnicode();
	case CODE_UTF8:			return new CUtf8();
	case CODE_UTF7:			return new CUtf7();
	case CODE_UNICODEBE:	return new CUnicodeBe();
	case CODE_CESU8:		return new CCesu8();
	case CODE_LATIN1:		return new CLatin1();	// 2010/3/20 Uchi
	default:
		assert(0);
	}
	return NULL;
}
