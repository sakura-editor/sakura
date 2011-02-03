#pragma once

template <class INT_TYPE, class POINT_TYPE> class CStrictRect{
private:
	typedef CStrictRect<INT_TYPE, POINT_TYPE> Me;
public:
	typedef INT_TYPE	IntType;
	typedef POINT_TYPE	PointType;
public:
	//�����o�ϐ��͌��J
	IntType left;
	IntType top;
	IntType right;
	IntType bottom;

public:
	//!������W (TopLeft)
	PointType UpperLeft() const
	{
		return PointType(left,top);
	}
	//!�E�����W (BottomRight)
	PointType LowerRight() const
	{
		return PointType(right,bottom);
	}

	//!�q�b�g�`�F�b�N
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
