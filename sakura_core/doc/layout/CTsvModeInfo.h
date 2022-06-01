﻿/*!	@file
	@brief TSVモード管理
*/
/*
	Copyright (C) 2015, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CTSVMODEINFO_3AD2F099_5EBE_493E_9DD3_FEBAEDDE9513_H_
#define SAKURA_CTSVMODEINFO_3AD2F099_5EBE_493E_9DD3_FEBAEDDE9513_H_
#pragma once

#include <vector>
#include "basis/SakuraBasis.h"

// TSVモード
#define TSV_MODE_NONE	0	// TSVモードなし
#define TSV_MODE_TSV	1	// TSVモード
#define TSV_MODE_CSV	2	// CSVモード

class CDocLineMgr;

// TSVモード情報
class CTsvModeInfo {
	
public:
	void CalcTabLength(CDocLineMgr* cDocLineMgr);	// タブ位置を再計算する
	void CalcTabLength(LPCWSTR pLine);	// タブ位置を再計算する（一行）
	CLayoutInt GetActualTabLength(CLayoutInt pos, CLayoutInt px) const;	// 指定したレイアウト位置のタブ幅を取得（折り返しは考慮しない）

	int m_nTsvMode;
	CLayoutInt m_nMaxCharLayoutX;

private:
	std::vector<int> m_tabLength;
};
#endif /* SAKURA_CTSVMODEINFO_3AD2F099_5EBE_493E_9DD3_FEBAEDDE9513_H_ */
