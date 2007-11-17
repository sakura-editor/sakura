#pragma once


//�P�ʂ������I�ɋ�ʂ��ꂽ�|�C���g�^�B��POINT�͌p�����Ȃ����Ƃɂ���
/*
template <int TYPE> class CStrictPoint : public CMyPoint{
public:
	CStrictPoint() : CMyPoint() { }
	CStrictPoint(int _x,int _y) : CMyPoint(_x,_y) { }
	CStrictPoint(const CStrictPoint& rhs) : CMyPoint(rhs) { }

	//��POINT����̕ϊ��́A�u�����I�Ɏw�肳�ꂽ�Ƃ��̂݁v������B
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
	//�R���X�g���N�^�E�f�X�g���N�^
	CStrictPoint(){ x=SuperIntType(0); y=SuperIntType(0); }
	CStrictPoint(int _x,int _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
#ifdef USE_STRICT_INT
	CStrictPoint(IntType _x,IntType _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
#endif
	CStrictPoint(const SUPER& rhs){ x=rhs.x; y=rhs.y; }

	//���̌^������A�u�����I�Ɏw�肷��΁v�ϊ����\
	template <class SRC>
	explicit CStrictPoint(const SRC& rhs){ x=(SuperIntType)rhs.x; y=(SuperIntType)rhs.y; }

	//�Z�p���Z�q
	CStrictPoint& operator += (const SUPER& rhs){ x+=rhs.x; y+=rhs.y; return *this; }
	CStrictPoint& operator -= (const SUPER& rhs){ x-=rhs.x; y-=rhs.y; return *this; }
	CStrictPoint& operator *= (int n){ x*=n; y*=n; return *this; }
	CStrictPoint& operator /= (int n){ x/=n; y/=n; return *this; }

	//�Z�p���Z�q�Q
	CStrictPoint operator + (const SUPER& rhs) const{ CStrictPoint tmp=*this; tmp+=rhs; return tmp; }
	CStrictPoint operator - (const SUPER& rhs) const{ CStrictPoint tmp=*this; tmp-=rhs; return tmp; }
	CStrictPoint operator * (int n) const{ CStrictPoint tmp=*this; tmp*=n; return tmp; }
	CStrictPoint operator / (int n) const{ CStrictPoint tmp=*this; tmp/=n; return tmp; }

	//������Z�q
	CStrictPoint& operator = (const SUPER& rhs){ x=rhs.x; y=rhs.y; return *this; }

	//��r���Z�q
	bool operator == (const SUPER& rhs) const{ return x==rhs.x && y==rhs.y; }
	bool operator != (const SUPER& rhs) const{ return !this->operator==(rhs); }

	//�ݒ�
	void Clear(){ x=SuperIntType(0); y=SuperIntType(0); }
	void Set(INT_TYPE _x, INT_TYPE _y){ x=SuperIntType(_x); y=SuperIntType(_y); }
	void Set(const SUPER& pt){ x=pt.x; y=pt.y; }
	void SetX(INT_TYPE _x){ x=SuperIntType(_x); }
	void SetY(INT_TYPE _y){ y=SuperIntType(_y); }
	void Offset(int _x,int _y){ x+=_x; y+=_y; }
	void Offset(const SUPER& pt){ x+=pt.x; y+=pt.y; }

	//�擾
	SuperIntType GetX() const{ return x; }
	SuperIntType GetY() const{ return y; }
	SUPER Get() const{ return *this; }
	INT_TYPE GetX2() const{ return INT_TYPE(x); }
	INT_TYPE GetY2() const{ return INT_TYPE(y); }

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

	//����
	POINT GetPOINT() const
	{
		POINT pt={(Int)x,(Int)y};
		return pt;
	}
};
