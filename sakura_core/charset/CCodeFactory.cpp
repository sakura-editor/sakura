#include "stdafx.h"
#include "CCodeFactory.h"
#include "CCodeMediator.h"

//! eCodeTypeに適合する CCodeBaseインスタンス を生成
CCodeBase* CCodeFactory::CreateCodeBase(
	ECodeType	eCodeType,		//!< 文字コード
	int			nFlag			//!< bit 0: MIME Encodeされたヘッダをdecodeするかどうか
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
