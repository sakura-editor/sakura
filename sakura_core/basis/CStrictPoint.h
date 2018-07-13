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
#ifndef SAKURA_CSTRICTPOINT_56029CB9_EAD4_489E_9300_5990D582337F_H_
#define SAKURA_CSTRICTPOINT_56029CB9_EAD4_489E_9300_5990D582337F_H_

//単位が明示的に区別されたポイント型。※POINTは継承しないことにした
/*
template <int TYPE> class CStrictPoint : public CMyPoint{
public:
	CStrictPoint() : CMyPoint() { }
	CStrictPoint(int _x,int _y) : CMyPoint(_x,_y) { }
	CStrictPoint(const CStrictPoint& rhs) : CMyPoint(rhs) { }

	//※POINTからの変換は、「明示的に指定されたときのみ」許可する。
	explicit CStrictPoint(const POINT& rhs) : CMyPoint(rhs) { }
};
*/
template <class SUPER, class INT_TYPE, class SUPER_INT_TYPE = INT_TYPE> class CStrictPoint : public SUPER{
private:
	typedef CStrictPoint<SUPER, INT_TYPE> Me;
public:
	typedef INT_TYPE       IntType;
	typedef SUPER_INT_TYPE SuperIntType;
public:
	using SUPER::x;
	using SUPER::y;
	//コンストラクタ・デストラクタ
	CStrictPoint(){ x=SuperIntType(0); y=SuperIntType(0); }
	CStrictPoint(int _x,int _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
#ifdef USE_STRICT_INT
	CStrictPoint(IntType _x,IntType _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
#endif
	CStrictPoint(const SUPER& rhs){ x=rhs.x; y=rhs.y; }

	//他の型からも、「明示的に指定すれば」変換が可能
	template <class SRC>
	explicit CStrictPoint(const SRC& rhs){ x=(SuperIntType)rhs.x; y=(SuperIntType)rhs.y; }

	//算術演算子
	CStrictPoint& operator += (const SUPER& rhs){ x+=rhs.x; y+=rhs.y; return *this; }
	CStrictPoint& operator -= (const SUPER& rhs){ x-=rhs.x; y-=rhs.y; return *this; }
	CStrictPoint& operator *= (int n){ x*=n; y*=n; return *this; }
	CStrictPoint& operator /= (int n){ x/=n; y/=n; return *this; }

	//算術演算子２
	CStrictPoint operator + (const SUPER& rhs) const{ CStrictPoint tmp=*this; tmp+=rhs; return tmp; }
	CStrictPoint operator - (const SUPER& rhs) const{ CStrictPoint tmp=*this; tmp-=rhs; return tmp; }
	CStrictPoint operator * (int n) const{ CStrictPoint tmp=*this; tmp*=n; return tmp; }
	CStrictPoint operator / (int n) const{ CStrictPoint tmp=*this; tmp/=n; return tmp; }

	//代入演算子
	CStrictPoint& operator = (const SUPER& rhs){ x=rhs.x; y=rhs.y; return *this; }

	//比較演算子
	bool operator == (const SUPER& rhs) const{ return x==rhs.x && y==rhs.y; }
	bool operator != (const SUPER& rhs) const{ return !this->operator==(rhs); }

	//設定
	void Clear(){ x=SuperIntType(0); y=SuperIntType(0); }
	void Set(INT_TYPE _x, INT_TYPE _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
	void Set(const SUPER& pt){ x=pt.x; y=pt.y; }
	void SetX(INT_TYPE _x){ x=SuperIntType(_x); }
	void SetY(INT_TYPE _y){ y=SuperIntType(_y); }
	void Offset(int _x,int _y){ x+=_x; y+=_y; }
	void Offset(const SUPER& pt){ x+=pt.x; y+=pt.y; }

	//取得
	SuperIntType GetX() const{ return x; }
	SuperIntType GetY() const{ return y; }
	SUPER Get() const{ return *this; }
	INT_TYPE GetX2() const{ return INT_TYPE(x); }
	INT_TYPE GetY2() const{ return INT_TYPE(y); }

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

	//特殊
	POINT GetPOINT() const
	{
		POINT pt={(Int)x,(Int)y};
		return pt;
	}
};

#endif /* SAKURA_CSTRICTPOINT_56029CB9_EAD4_489E_9300_5990D582337F_H_ */
/*[EOF]*/
