﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_
#define SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_
#pragma once

#include "charset/charset.h"
#include <memory>
#include "charset/CCodeBase.h"

class CCodeFactory{
public:
	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static CCodeBase* CreateCodeBase(
		ECodeType	eCodeType,		//!< 文字コード
		int			nFlag			//!< bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);

	//! eCodeTypeに適合する CCodeBaseインスタンス を生成
	static std::unique_ptr<CCodeBase> CreateCodeBase(
		ECodeType	eCodeType		//!< 文字コード
	)
	{
		return std::unique_ptr<CCodeBase>( CreateCodeBase( eCodeType, 0 ) );
	}
};

#endif /* SAKURA_CCODEFACTORY_A5C6C204_F9BD_42BA_A5CD_1B086833CCA4_H_ */
