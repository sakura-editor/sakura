/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_
#define SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_
#pragma once

#include "mem/CMemory.h"

//※CMemoryをprotect継承することにより、あまり自由にCMemoryを使えないようにしておく
class CNative : protected CMemory{
public:
	//CMemory*ポインタを得る
	CMemory* _GetMemory(){ return static_cast<CMemory*>(this); }
	const CMemory* _GetMemory() const{ return static_cast<const CMemory*>(this); }

public:
	//汎用
	void Clear(); //!< 空っぽにする
};

// 派生クラスでメンバー追加禁止
static_assert(sizeof(CNative) == sizeof(CMemory), "size check");

#endif /* SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_ */
