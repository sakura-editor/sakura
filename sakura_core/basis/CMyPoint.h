#pragma once

#include <windows.h> //POINT

class CMyPoint : public POINT{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CMyPoint(){ x=0; y=0; }
	CMyPoint(int _x,int _y){ x=_x; y=_y; }
	CMyPoint(const POINT& rhs){ x=rhs.x; y=rhs.y; }

	//�Z�p���Z�q
	CMyPoint& operator += (const POINT& rhs){ x+=rhs.x; y+=rhs.y; return *this; }
	CMyPoint& operator -= (const POINT& rhs){ x-=rhs.x; y-=rhs.y; return *this; }
	CMyPoint& operator *= (int n){ x*=n; y*=n; return *this; }
	CMyPoint& operator /= (int n){ x/=n; y/=n; return *this; }

	//�Z�p���Z�q�Q
	CMyPoint operator + (const POINT& rhs) const{ CMyPoint tmp=*this; tmp+=rhs; return tmp; }
	CMyPoint operator - (const POINT& rhs) const{ CMyPoint tmp=*this; tmp-=rhs; return tmp; }
	CMyPoint operator * (int n) const{ CMyPoint tmp=*this; tmp*=n; return tmp; }
	CMyPoint operator / (int n) const{ CMyPoint tmp=*this; tmp/=n; return tmp; }

	//������Z�q
	CMyPoint& operator = (const POINT& rhs){ x=rhs.x; y=rhs.y; return *this; }

	//��r���Z�q
	bool operator == (const POINT& rhs) const{ return x==rhs.x && y==rhs.y; }
	bool operator != (const POINT& rhs) const{ return !this->operator==(rhs); }

	//�ݒ�
	void Set(int _x,int _y){ x=_x; y=_y; }
	void Set(const CMyPoint& pt){ x=pt.x; y=pt.y; }
	void SetX(int _x){ x=_x; }
	void SetY(int _y){ y=_y; }
	void Offset(int _x,int _y){ x+=_x; y+=_y; }
	void Offset(const CMyPoint& pt){ x+=pt.x; y+=pt.y; }

	//�擾
	int GetX() const{ return (int)x; }
	int GetY() const{ return (int)y; }
	CMyPoint Get() const{ return *this; }

	//! x,y �����ꂩ�� 0 ��菬�����ꍇ�� true ��Ԃ�
	bool HasNegative() const
	{
		return x<0 || y<0;
	}

	//! x,y �ǂ�������R���ł���� true
	bool BothNatural() const
	{
		return x>=0 && y>=0;
	}
};


/*!
	pt1 - pt2�̌��ʂ�Ԃ�
	Y��D�悵�Ĕ�r�BY������Ȃ�AX�Ŕ�r�B

	@return <0 pt1 <  pt2
	@return 0  pt1 == pt2
	@return >0 pt1 >  pt2
*/
template <class POINT_T>
inline int PointCompare(const POINT_T& pt1,const POINT_T& pt2)
{
	if(pt1.y!=pt2.y)return (Int)(pt1.y-pt2.y);
	return (Int)(pt1.x-pt2.x);
}


//! 2�_��Ίp�Ƃ����`�����߂�
template <class POINT_T>
inline void TwoPointToRect(
	RECT*	prcRect,
	POINT_T	pt1,
	POINT_T	pt2
)
{
	if( pt1.y < pt2.y ){
		prcRect->top	= (Int)pt1.y;
		prcRect->bottom	= (Int)pt2.y;
	}else{
		prcRect->top	= (Int)pt2.y;
		prcRect->bottom	= (Int)pt1.y;
	}
	if( pt1.x < pt2.x ){
		prcRect->left	= (Int)pt1.x;
		prcRect->right	= (Int)pt2.x;
	}else{
		prcRect->left	= (Int)pt2.x;
		prcRect->right	= (Int)pt1.x;
	}
}

