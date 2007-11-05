#pragma once

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
};
