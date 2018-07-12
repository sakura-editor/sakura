/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CSTRICTRECT_F3676246_1D72_4A04_9614_3881BBA67B16_H_
#define SAKURA_CSTRICTRECT_F3676246_1D72_4A04_9614_3881BBA67B16_H_

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

#endif /* SAKURA_CSTRICTRECT_F3676246_1D72_4A04_9614_3881BBA67B16_H_ */
/*[EOF]*/
