/*!	@file
	@brief ���i���Ղ���J�X�^�}�C�Y�\�Ȑ����N���X
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

//int�ȊO�̐����^��int�ɃL���X�g���Ĉ���
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


//! �Öق̕ϊ��������Ȃ��A�����N���X
template <
	int STRICT_ID,			//!< �^�𕪂��邽�߂̐��l�B0 or 1�B
	bool ALLOW_CMP_INT,		//!< int�Ƃ̔�r���������ǂ���
	bool ALLOW_ADDSUB_INT,	//!< int�Ƃ̉����Z���������ǂ���
	bool ALLOW_CAST_INT,	//!< int�ւ̈Öق̕ϊ����������ǂ���
	bool ALLOW_ASSIGNOP_INT	//!< int�̑�����������ǂ���
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
	//!�S�~�N���X
	class CDummy{
	public:
		CDummy();
		CDummy(int);
	};
	template<bool t, bool=false> struct ChooseIntOrDummy {
		typedef int Type;
	};
	// �N���X���Ńe���v���[�g�̓��ꉻ�������G++�ɓ{����̂ŕ������ꉻ�ɂ���
	template<bool _> struct ChooseIntOrDummy<false, _> {
		typedef CDummy Type;
	};
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CStrictInteger(){ m_value=0; }
	CStrictInteger(const Me& rhs){ m_value=rhs.m_value; }

	//int����̕ϊ��́A�u�����I�Ɏw�肵���Ƃ��̂݁v�\
	explicit CStrictInteger(int value){ m_value=value; }

	//�Z�p���Z�q (���Z�A���Z�͓��N���X���m�ł��������Ȃ�)
	Me& operator += (const Me& rhs)	{ m_value += rhs.m_value; return *this; }
	Me& operator -= (const Me& rhs)	{ m_value -= rhs.m_value; return *this; }
	Me& operator *= (int n)			{ m_value *= n; return *this; }
	Me& operator /= (int n)			{ m_value /= n; return *this; }
	Me& operator %= (int n)			{ m_value %= n; return *this; }
	Me& operator %= (const Me& rhs)	{ m_value %= rhs.m_value; return *this; }

	//�Z�p���Z�q�Q (���Z�A���Z�͓��N���X���m�ł��������Ȃ�)
	Me operator + (const Me& rhs)	const{ Me ret=*this; return ret+=rhs; }
	Me operator - (const Me& rhs)	const{ Me ret=*this; return ret-=rhs; }
	Me operator * (int n)			const{ Me ret=*this; return ret*=n; }
	Me operator / (int n)			const{ Me ret=*this; return ret/=n; }
	Me operator % (int n)			const{ Me ret=*this; return ret%=n; }
	Me operator % (const Me& rhs)	const{ Me ret=*this; return ret%=rhs; }

	//�Z�p���Z�q�R
	int operator ++ ()   { return ++m_value; }	//++c;
	int operator ++ (int){ return m_value++; }	//c++;
	int operator -- ()   { return --m_value; }	//--c;
	int operator -- (int){ return m_value--; }	//c--;

	//�Z�p���Z�q�S
	Me operator - () const{ return Me(-m_value); }

	//������Z�q
	Me& operator = (const Me& rhs){ m_value=rhs.m_value; return *this; }

	//��r���Z�q
	bool operator <  (const Me& rhs) const{ return m_value <  rhs.m_value; }
	bool operator <= (const Me& rhs) const{ return m_value <= rhs.m_value; }
	bool operator >  (const Me& rhs) const{ return m_value >  rhs.m_value; }
	bool operator >= (const Me& rhs) const{ return m_value >= rhs.m_value; }
	bool operator == (const Me& rhs) const{ return m_value == rhs.m_value; }
	bool operator != (const Me& rhs) const{ return m_value != rhs.m_value; }

	//�֐�
	int GetValue() const{ return m_value; }
	void SetValue(int n){ m_value=n; }

	//Int(CLaxInt)�ւ̕ϊ��͏�ɋ���
	operator Int() const{ return Int(m_value); }

	//int�ȊO�̐����^��int�ɃL���X�g���Ĉ���
	STRICTINT_OTHER_TYPE_AS_INT(short)
	STRICTINT_OTHER_TYPE_AS_INT(size_t)
	STRICTINT_OTHER_TYPE_AS_INT(LONG)

	// -- -- -- -- �ʎ��CStrictInteger�Ƃ̉��Z�͐�΋�����(��肽����int�ł���Ă�������) -- -- -- -- //
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


	// -- -- -- -- ALLOW_ADDSUB_INT��true�̏ꍇ�́Aint�Ƃ̉����Z������ -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_ADDSUB_INT>::Type AddsubIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_ADDSUB_INT>::Type NotAddsubIntegerType;
public:
	Me& operator += (AddsubIntegerType rhs) { m_value += rhs; return *this; }
	Me& operator -= (AddsubIntegerType rhs) { m_value -= rhs; return *this; }
	Me  operator +  (AddsubIntegerType rhs) const{ Me ret=*this; return ret+=rhs; }
	Me  operator -  (AddsubIntegerType rhs) const{ Me ret=*this; return ret-=rhs; }
private:
	//��ALLOW_ADDSUB_INT��false�̏ꍇ�́Aprivate�����o��u�����Ƃɂ��A�u�����I�Ɂv�����Z���֎~����B
	Me& operator += (NotAddsubIntegerType rhs);
	Me& operator -= (NotAddsubIntegerType rhs);
	Me  operator +  (NotAddsubIntegerType rhs) const;
	Me  operator -  (NotAddsubIntegerType rhs) const;


	// -- -- -- -- ALLOW_CMP_INT��true�̏ꍇ�́Aint�Ƃ̔�r������ -- -- -- -- //
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
	//��ALLOW_CMP_INT��false�̏ꍇ�́Aprivate�����o��u�����Ƃɂ��A�u�����I�Ɂv��r���֎~����B
	bool operator <  (NotCmpIntegerType rhs) const;
	bool operator <= (NotCmpIntegerType rhs) const;
	bool operator >  (NotCmpIntegerType rhs) const;
	bool operator >= (NotCmpIntegerType rhs) const;
	bool operator == (NotCmpIntegerType rhs) const;
	bool operator != (NotCmpIntegerType rhs) const;


	// -- -- -- -- ALLOW_CAST_INT��true�̏ꍇ�́Aint�ւ̈Öق̕ϊ������� -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_CAST_INT>::Type CastIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_CAST_INT>::Type NotCastIntegerType;
public:
	operator CastIntegerType() const{ return m_value; }
private:
	//��ALLOW_CAST_INT��false�̏ꍇ�́Aprivate�����o��u�����Ƃɂ��A�u�����I�Ɂv�Öٕϊ����֎~����B
	operator NotCastIntegerType() const;


	// -- -- -- -- ALLOW_ASSIGNOP_INT��true�̏ꍇ�́Aint�̑�������� -- -- -- -- //
private:
	typedef typename ChooseIntOrDummy< ALLOW_ASSIGNOP_INT>::Type AssignIntegerType;
	typedef typename ChooseIntOrDummy<!ALLOW_ASSIGNOP_INT>::Type NotAssignIntegerType;
public:
	Me& operator = (const AssignIntegerType& rhs){ m_value = rhs; return *this; }
private:
	//��ALLOW_ASSIGNOP_INT��false�̏ꍇ�́Aprivate�����o��u�����Ƃɂ��A�u�����I�Ɂv������֎~����B
	Me& operator = (const NotAssignIntegerType&);


	// -- -- -- -- �����o�ϐ� -- -- -- -- //
private:
	int m_value;
};



//���ӂ�int���̏ꍇ�̉��Z�q
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
