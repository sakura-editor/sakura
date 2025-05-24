/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCONVERT_TOHANKAKU_B22F6D13_783E_4D93_B078_8A42BE57FC7B_H_
#define SAKURA_CCONVERT_TOHANKAKU_B22F6D13_783E_4D93_B078_8A42BE57FC7B_H_
#pragma once

#include "CConvert.h"

//!半角にできるものは全部半角に変換
class CConvert_ToHankaku final : public CConvert{
public:
	bool DoConvert(CNativeW* pcData) override;
};

enum EToHankakuMode{
	TO_KATAKANA	= 0x01, //!< カタカナに影響アリ
	TO_HIRAGANA	= 0x02, //!< ひらがなに影響アリ
	TO_EISU		= 0x04, //!< 英数字に影響アリ
};
#endif /* SAKURA_CCONVERT_TOHANKAKU_B22F6D13_783E_4D93_B078_8A42BE57FC7B_H_ */
