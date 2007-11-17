#pragma once

//!型チェックの緩い整数型
class CLaxInteger{
private:
	typedef CLaxInteger Me;

public:
	//コンストラクタ・デストラクタ
	CLaxInteger(){ m_value=0; }
	CLaxInteger(const Me& rhs){ m_value=rhs.m_value; }
	CLaxInteger(int value){ m_value=value; }

	//暗黙の変換
	operator const int&() const{ return m_value; }
	operator       int&()      { return m_value; }

private:
	int m_value;
};

