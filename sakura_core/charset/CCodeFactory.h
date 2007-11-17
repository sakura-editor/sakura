#pragma once

class CCodeBase;

class CCodeFactory{
public:
	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static CCodeBase* CreateCodeBase(
		ECodeType	eCodeType,		//!< 文字コード
		int			nFlag			//!< bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);
};

