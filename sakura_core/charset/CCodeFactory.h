/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_
#define SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_
#pragma once

#include "charset/CUnicode.h"

class CCodeFactory{
public:
	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static CCodeBase* CreateCodeBase(
		ECodeType	eCodeType,		//!< 文字コード
		int			nFlag			//!< bit 0: MIME Encodeされたヘッダーをdecodeするかどうか
	);

	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static std::unique_ptr<CCodeBase> CreateCodeBase(
		ECodeType	eCodeType		//!< 文字コード
	)
	{
		return std::unique_ptr<CCodeBase>( CreateCodeBase( eCodeType, 0 ) );
	}

	static SLoadFromCodeResult	LoadFromCode(ECodeType eCodeType, std::string_view code);
	static SConvertToCodeResult	ConvertToCode(ECodeType eCodeType, std::wstring_view wide);
};

#endif /* SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_ */
