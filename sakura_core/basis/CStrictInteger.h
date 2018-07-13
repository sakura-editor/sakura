/*!	@file
	@brief 厳格っぷりをカスタマイズ可能な整数クラス
	       Integer class, which is static-type-checked strict and flexible at compile.

	@author kobake
	@date 2007.10.16
*/
/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CSTRICTINTEGER_6F202774_0F82_4BB7_B107_37DE5443309E_H_
#define SAKURA_CSTRICTINTEGER_6F202774_0F82_4BB7_B107_37DE5443309E_H_

//int以外の整数型もintにキャストして扱う
#define STRICTINT_OTHER_TYPE_AS_INT(TYPE) \
	Me& operator += (TYPE rhs)			{ return operator += ((int)rhs); } \
	Me& operator -= (TYPE rhs)			{ return operator -= ((int)rhs); } \
	Me operator + (TYPE rhs)	const	{ return operator +  ((int)rhs); } \
	Me operator - (TYPE rhs)	const	{ return operator -  ((int)rhs); } \
	bool operator <  (TYPE rhs) const	{ return operator <  ((int)rhs); } \
	bool operator <= (TYPE rhs) const	{ return operator <= ((int)rhs); } \
	bool operator >  (TYPE rhs) const	{ return operator >  ((int)rhs); } \
	bool operator >= (TYPE rhs) const	{ return operator >= ((int)rhs); } \
	bool operator == (TYPE rhs) const	{ return operator == ((int)rhs); } \
	bool operator != (TYPE rhs) const	{ return operator != ((int)rhs); }


//! 暗黙の変換を許さない、整数クラス
template <
	int STRICT_ID,			//!< 型を分けるための数値。0 or 1。
	bool ALLOW_CMP_INT,		//!< intとの比較を許すかどうか
	bool ALLOW_ADDSUB_INT,	//!< intとの加減算を許すかどうか
	bool ALLOW_CAST_INT,	//!< intへの暗黙の変換を許すかどうか
	bool ALLOW_ASSIGNOP_INT	//!< intの代入を許すかどうか
>
class CStrictInteger{
private:
	typedef CStrictInteger<
		STRICT_ID,
		ALLOW_CMP_INT,
		ALLOW_ADDSUB_INT,
		ALLOW_CAST_INT,
		ALLOW_ASSIGNOP_INT
	> Me;
	static const int NOT_STRICT_ID = (1-STRICT_ID);

private:
	//!ゴミクラス
	class CDummy{
	public:
		CDummy();
		CDummy(int);
	};
	template<bool t, bool=false> struct ChooseIntOrDummy {
		typedef int Type;
	};
	// クラス内でテンプレートの特殊化をするとG++に怒られるので部分特殊化にする
	template<bool _> struct ChooseIntOrDummy<false, _> {
		typedef CDummy Type;
	};
public:
	//コンストラクタ・デストラクタ
	CStrictInteger(){ m_value=0; }
	CStrictInteger(const Me& rhs){ m_value=rhs.m_value; }

	//intからの変換は、「明示的に指定したときのみ」可能
	explicit CStrictInteger(int value){ m_value=value; }

	//算術演算子 (加算、減算は同クラス同士でしか許さない)
	Me& operator += (const Me& rhs)	{ m_value += rhs.m_value; return *this; }
	Me& operator -= (const Me& rhs)	{ m_value -= rhs.m_value; return *this; }
	Me& operator *= (int n)			{ m_value *= n; return *this; }
	Me& operator /= (int n)			{ m_value /= n; return *this; }
	Me& operator %= (int n)			{ m_value %= n; return *this; }
	Me& operator %= (const Me& rhs)	{ m_value %= rhs.m_value; return *this; }

	//算術演算子２ (加算、減算は同クラス同士でしか許さない)
	Me operator + (const Me& rhs)	const{ Me ret=*this; return ret+=rhs; }
	Me operator - (const Me& rhs)	const{ Me ret=*this; return ret-=rhs; }
	Me operator * (int n)			const{ Me ret=*this; return ret*=n; }
	Me operator / (int n)			const{ Me ret=*this; return ret/=n; }
	Me operator % (int n)			const{ Me ret=*this; return ret%=n; }
	Me operator % (const Me& rhs)	const{ Me ret=*this; return ret%=rhs; }

	//算術演算子３
	int operator ++ ()   { return ++m_value; }	//++c;
	int operator ++ (int){ return m_value++; }	//c++;
	int operator -- ()   { return --m_value; }	//--c;
	int operator -- (int){ return m_value--; }	//c--;

	//算術演算子４
	Me operator - () const{ return Me(-m_value); }

	//代入演算子
	Me& operator = (const Me& rhs){ m_value=rhs.m_value; return *this; }

	//比較演算子
	bool operator <  (const Me& rhs) const{ return m_value <  rhs.m_value; }
	bool operator <= (const Me& rhs) const{ return m_value <= rhs.m_value; }
	bool operator >  (const Me& rhs) const{ return m_value >  rhs.m_value; }
	bool operator >= (const Me& rhs) const{ return m_value >= rhs.m_value; }
	bool operator == (const Me& rhs) const{ return m_value == rhs.m_value; }
	bool operator != (const Me& rhs) const{ return m_value != rhs.m_value; }

	//関数
	int GetValue() const{ return m_value; }
	void SetValue(int n){ m_value=n; }

	//Int(CLaxInt)への変換は常に許す
	operator Int() const{ return Int(m_value); }

	//int以外の整数型もintにキャストして扱う
	STRICTINT_OTHER_TYPE_AS_INT(short)
	STRICTINT_OTHER_TYPE_AS_INT(size_t)
	STRICTINT_OTHER_TYPE_AS_INT(LONG)

	// -- -- -- -- 別種のCStrictIntegerとの演算は絶対許さん(やりたきゃintでも介してください) -- -- -- -- //
private:
	template <bool B0,bool B1,bool B2,bool B3> Me&  operator += (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&);
	template <bool B0,bool B1,bool B2,bool B3> Me&  operator -= (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&);
	template <bool B0,bool B1,bool B2,bool B3> Me   operator +  (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> Me   operator -  (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> Me&  operator =  (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&);
	template <bool B0,bool B1,bool B2,bool B3> bool operator <  (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> bool operator <= (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> bool operator >  (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> bool operator >= (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> bool operator == (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;
	template <bool B0,bool B1,bool B2,bool B3> bool operator != (const CStrictInteger<NOT_STRICT_ID,B0,B1,B2,B3>&) const;


	// -- -- -- -- ALLOW_ADDSUB_INTがtrueの場合は、intとの加減算を許す -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_ADDSUB_INT>::Type AddsubIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_ADDSUB_INT>::Type NotAddsubIntegerType;
public:
	Me& operator += (AddsubIntegerType rhs) { m_value += rhs; return *this; }
	Me& operator -= (AddsubIntegerType rhs) { m_value -= rhs; return *this; }
	Me  operator +  (AddsubIntegerType rhs) const{ Me ret=*this; return ret+=rhs; }
	Me  operator -  (AddsubIntegerType rhs) const{ Me ret=*this; return ret-=rhs; }
private:
	//※ALLOW_ADDSUB_INTがfalseの場合は、privateメンバを置くことにより、「明示的に」加減算を禁止する。
	Me& operator += (NotAddsubIntegerType rhs);
	Me& operator -= (NotAddsubIntegerType rhs);
	Me  operator +  (NotAddsubIntegerType rhs) const;
	Me  operator -  (NotAddsubIntegerType rhs) const;


	// -- -- -- -- ALLOW_CMP_INTがtrueの場合は、intとの比較を許す -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_CMP_INT>::Type CmpIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_CMP_INT>::Type NotCmpIntegerType;
public:
	bool operator <  (CmpIntegerType rhs) const{ return m_value <  rhs; }
	bool operator <= (CmpIntegerType rhs) const{ return m_value <= rhs; }
	bool operator >  (CmpIntegerType rhs) const{ return m_value >  rhs; }
	bool operator >= (CmpIntegerType rhs) const{ return m_value >= rhs; }
	bool operator == (CmpIntegerType rhs) const{ return m_value == rhs; }
	bool operator != (CmpIntegerType rhs) const{ return m_value != rhs; }
private:
	//※ALLOW_CMP_INTがfalseの場合は、privateメンバを置くことにより、「明示的に」比較を禁止する。
	bool operator <  (NotCmpIntegerType rhs) const;
	bool operator <= (NotCmpIntegerType rhs) const;
	bool operator >  (NotCmpIntegerType rhs) const;
	bool operator >= (NotCmpIntegerType rhs) const;
	bool operator == (NotCmpIntegerType rhs) const;
	bool operator != (NotCmpIntegerType rhs) const;


	// -- -- -- -- ALLOW_CAST_INTがtrueの場合は、intへの暗黙の変換を許す -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_CAST_INT>::Type CastIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_CAST_INT>::Type NotCastIntegerType;
public:
	operator CastIntegerType() const{ return m_value; }
private:
	//※ALLOW_CAST_INTがfalseの場合は、privateメンバを置くことにより、「明示的に」暗黙変換を禁止する。
	operator NotCastIntegerType() const;


	// -- -- -- -- ALLOW_ASSIGNOP_INTがtrueの場合は、intの代入を許す -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_ASSIGNOP_INT>::Type AssignIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_ASSIGNOP_INT>::Type NotAssignIntegerType;
public:
	Me& operator = (const AssignIntegerType& rhs){ m_value = rhs; return *this; }
private:
	//※ALLOW_ASSIGNOP_INTがfalseの場合は、privateメンバを置くことにより、「明示的に」代入を禁止する。
	Me& operator = (const NotAssignIntegerType&);


	// -- -- -- -- メンバ変数 -- -- -- -- //
private:
	int m_value;
};



//左辺がint等の場合の演算子
#define STRICTINT_LEFT_INT_CMP(TYPE) \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator <  (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs> lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator <= (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs>=lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator >  (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs< lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator >= (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs<=lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator == (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs==lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline bool operator != (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return rhs!=lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline CStrictInteger<N,B0,B1,B2,B3> operator + (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return  rhs+lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline CStrictInteger<N,B0,B1,B2,B3> operator - (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return -rhs+lhs; } \
	template <int N,bool B0,bool B1,bool B2,bool B3> inline CStrictInteger<N,B0,B1,B2,B3> operator * (TYPE lhs, const CStrictInteger<N,B0,B1,B2,B3>& rhs){ return  rhs*lhs; }
STRICTINT_LEFT_INT_CMP(int)
STRICTINT_LEFT_INT_CMP(short)
STRICTINT_LEFT_INT_CMP(size_t)
STRICTINT_LEFT_INT_CMP(LONG)

#endif /* SAKURA_CSTRICTINTEGER_6F202774_0F82_4BB7_B107_37DE5443309E_H_ */
/*[EOF]*/
