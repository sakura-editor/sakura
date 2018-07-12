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
#ifndef SAKURA_CMYRECT_25A0FB5F_E06F_4F51_B046_E6951B95B0059_H_
#define SAKURA_CMYRECT_25A0FB5F_E06F_4F51_B046_E6951B95B0059_H_

#include <Windows.h> //RECT
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

#endif /* SAKURA_CMYRECT_25A0FB5F_E06F_4F51_B046_E6951B95B0059_H_ */
/*[EOF]*/
