/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSTRICTRECT_B41F0F96_8886_4ED1_B6FE_1B4EA70F58D8_H_
#define SAKURA_CSTRICTRECT_B41F0F96_8886_4ED1_B6FE_1B4EA70F58D8_H_
#pragma once

#include "util/std_macro.h"

template <class INT_TYPE, class POINT_TYPE> class CStrictRect{
private:
	typedef CStrictRect<INT_TYPE, POINT_TYPE> Me;
public:
	typedef INT_TYPE	IntType;
	typedef POINT_TYPE	PointType;
public:
	//メンバ変数は公開
	IntType left;
	IntType top;
	IntType right;
	IntType bottom;

public:
	//!左上座標 (TopLeft)
	PointType UpperLeft() const
	{
		return PointType(left,top);
	}
	//!右下座標 (BottomRight)
	PointType LowerRight() const
	{
		return PointType(right,bottom);
	}

	//!ヒットチェック
	bool PtInRect(const PointType& pt) const
	{
		return pt.x>=left && pt.x<right && pt.y>=top && pt.y<bottom;
	}

	Me& UnionStrictRect(const Me& rc1, const Me& rc2)
	{
		this->left   = t_min(rc1.left  , rc2.left);
		this->top    = t_min(rc1.top   , rc2.top);
		this->right  = t_max(rc1.right , rc2.right);
		this->bottom = t_max(rc1.bottom, rc2.bottom);
		return *this;
	}
};
#endif /* SAKURA_CSTRICTRECT_B41F0F96_8886_4ED1_B6FE_1B4EA70F58D8_H_ */
