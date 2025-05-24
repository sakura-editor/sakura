/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_
#define SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_
#pragma once

#include <Windows.h> //SIZE

class CMySize : public SIZE{
public:
	//コンストラクタ・デストラクタ
	CMySize() : CMySize(0, 0) {}
	CMySize(int _cx,int _cy){ cx=_cx; cy=_cy; }
	CMySize(const SIZE& rhs){ cx=rhs.cx; cy=rhs.cy; }

	//関数
	void Set(int _cx,int _cy){ cx=_cx; cy=_cy; }

	//演算子
	bool operator == (const SIZE& rhs) const{ return cx==rhs.cx && cy==rhs.cy; }
	bool operator != (const SIZE& rhs) const{ return !operator==(rhs); }
};
#endif /* SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_ */
