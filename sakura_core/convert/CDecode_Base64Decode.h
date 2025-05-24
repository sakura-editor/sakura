/*!	@file
	@brief BASE64 Decode

	@author 
*/

/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDECODE_BASE64DECODE_5AA66699_D761_436E_87F6_C8CA30BD775D_H_
#define SAKURA_CDECODE_BASE64DECODE_5AA66699_D761_436E_87F6_C8CA30BD775D_H_
#pragma once

#include "convert/CDecode.h"

class CDecode_Base64Decode final : public CDecode{
public:
	bool DoDecode(const CNativeW& cData, CMemory* pcDst) override;
};
#endif /* SAKURA_CDECODE_BASE64DECODE_5AA66699_D761_436E_87F6_C8CA30BD775D_H_ */
