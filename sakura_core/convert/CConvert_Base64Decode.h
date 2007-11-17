#pragma once

#include "CConvert.h"

class CConvert_Base64Decode : public CConvert{
public:
	//CConvertインターフェース
	bool DoConvert(CNativeW* pcData)
	{
		//$$$ 未実装につき、falseを返す
		return false;
	}

public:
	//実装
	static void BASE64Decode(const CNativeW* pcSrc, CMemory* pcDst);	// Base64デコード
	template <class CHAR_TYPE>
	static long MemBASE64_Decode( const CHAR_TYPE* pszSrc, long nSrcLen, BYTE* pDst );	/* Base64デコード */
};
