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
#ifndef SAKURA_CMYPOINT_A1799013_DDEC_472A_850B_6FCA04FC21049_H_
#define SAKURA_CMYPOINT_A1799013_DDEC_472A_850B_6FCA04FC21049_H_

#include <Windows.h> //POINT

class CMyPoint : public POINT{
public:
	//コンストラクタ・デストラクタ
	CMyPoint(){ x=0; y=0; }
	CMyPoint(int _x,int _y){ x=_x; y=_y; }
	CMyPoint(const POINT& rhs){ x=rhs.x; y=rhs.y; }

	//算術演算子
	CMyPoint& operator += (const POINT& rhs){ x+=rhs.x; y+=rhs.y; return *this; }
	CMyPoint& operator -= (const POINT& rhs){ x-=rhs.x; y-=rhs.y; return *this; }
	CMyPoint& operator *= (int n){ x*=n; y*=n; return *this; }
	CMyPoint& operator /= (int n){ x/=n; y/=n; return *this; }

	//算術演算子２
	CMyPoint operator + (const POINT& rhs) const{ CMyPoint tmp=*this; tmp+=rhs; return tmp; }
	CMyPoint operator - (const POINT& rhs) const{ CMyPoint tmp=*this; tmp-=rhs; return tmp; }
	CMyPoint operator * (int n) const{ CMyPoint tmp=*this; tmp*=n; return tmp; }
	CMyPoint operator / (int n) const{ CMyPoint tmp=*this; tmp/=n; return tmp; }

	//代入演算子
	CMyPoint& operator = (const POINT& rhs){ x=rhs.x; y=rhs.y; return *this; }

	//比較演算子
	bool operator == (const POINT& rhs) const{ return x==rhs.x && y==rhs.y; }
	bool operator != (const POINT& rhs) const{ return !this->operator==(rhs); }

	//設定
	void Set(int _x,int _y){ x=_x; y=_y; }
	void Set(const CMyPoint& pt){ x=pt.x; y=pt.y; }
	void SetX(int _x){ x=_x; }
	void SetY(int _y){ y=_y; }
	void Offset(int _x,int _y){ x+=_x; y+=_y; }
	void Offset(const CMyPoint& pt){ x+=pt.x; y+=pt.y; }

	//取得
	int GetX() const{ return (int)x; }
	int GetY() const{ return (int)y; }
	CMyPoint Get() const{ return *this; }

	//! x,y いずれかが 0 より小さい場合に true を返す
	bool HasNegative() const
	{
		return x<0 || y<0;
	}

	//! x,y どちらも自然数であれば true
	bool BothNatural() const
	{
		return x>=0 && y>=0;
	}
};


/*!
	pt1 - pt2の結果を返す
	Yを優先して比較。Yが同一なら、Xで比較。

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


//! 2点を対角とする矩形を求める
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

#endif /* SAKURA_CMYPOINT_A1799013_DDEC_472A_850B_6FCA04FC21049_H_ */
/*[EOF]*/
