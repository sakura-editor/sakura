/*!	@file
	@brief TSVモード管理
*/
/*
	Copyright (C) 2015, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
