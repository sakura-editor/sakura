#include "stdafx.h"
#include "CCodeFactory.h"
#include "CCodeMediator.h"

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
	default:
		assert(0);
	}
	return NULL;
}
