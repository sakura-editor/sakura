/*
	2007.11.29 kobake

	未分類小物類
*/

#pragma once


/*!
	auto_ptr の 配列版

	2007.11.29 kobake 作成
*/
template <class T> class auto_array_ptr{
private:
	typedef auto_array_ptr<T> Me;
public:
	//代入
	auto_array_ptr (T*  rhs){ m_array = rhs;          }
	auto_array_ptr (Me& rhs){ m_array = rhs.detach(); }
	Me& operator = (T*  rhs){ reset(rhs         ); return *this; }
	Me& operator = (Me& rhs){ reset(rhs.detach()); return *this; }

	//破棄
	~auto_array_ptr(){ delete[] m_array; }

	//手放す (解放はしない)
	T* detach()
	{
		T* p = m_array;
		m_array = NULL;
		return p;
	}

	//保持値を変更する
	void reset(T* p)
	{
		if(m_array==p)return;
		delete[] m_array;
		m_array = p;
	}

	//保持値にアクセス
	T* get(){ return m_array; }

	//配列要素にアクセス
	T& operator[](int i){ return m_array[i]; }

private:
	T* m_array;
};
