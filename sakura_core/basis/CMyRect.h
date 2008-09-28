#pragma once

#include <windows.h> //RECT
#include "CMyPoint.h"

class CMyRect : public RECT{
public:
	//コンストラクタ・デストラクタ
	CMyRect()
	{
		SetLTRB(0,0,0,0);
	}
	CMyRect(int l,int t,int r,int b)
	{
		SetLTRB(l,t,r,b);
	}
	CMyRect(const RECT& rc)
	{
		RECT* p=this;
		*p=rc;
	}

	//演算子

	//代入
	void SetLTRB(int l,int t,int r,int b)
	{
		left  =l;
		top   =t;
		right =r;
		bottom=b;
	}
	void SetXYWH(int x,int y,int w,int h)
	{
		left   = x;
		top    = y;
		right  = x+w;
		bottom = y+h;
	}
	void SetPos(int x, int y)
	{
		int w = Width();
		int h = Height();
		left = x;
		top  = y;
		SetSize(w,h);
	}
	void SetSize(int w, int h)
	{
		right  = left + w;
		bottom = top  + h;
	}


	//計算
	int Width() const
	{
		return right-left;
	}
	int Height() const
	{
		return bottom-top;
	}
	//!左上座標 (TopLeft)
	CMyPoint UpperLeft() const
	{
		return CMyPoint(left,top);
	}
	//!右下座標 (BottomRight)
	CMyPoint LowerRight() const
	{
		return CMyPoint(right,bottom);
	}

};

//!CRect合成。rc1,rc2を含む最小の矩形を生成する。
CMyRect MergeRect(const CMyRect& rc1, const CMyRect& rc2);
