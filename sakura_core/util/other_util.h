/*
	2007.11.29 kobake

	未分類小物類
*/
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
#ifndef SAKURA_OTHER_UTIL_F0E0EBE0_94B6_4E28_8241_6D842C0E8B73_H_
#define SAKURA_OTHER_UTIL_F0E0EBE0_94B6_4E28_8241_6D842C0E8B73_H_

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

#endif /* SAKURA_OTHER_UTIL_F0E0EBE0_94B6_4E28_8241_6D842C0E8B73_H_ */
/*[EOF]*/
