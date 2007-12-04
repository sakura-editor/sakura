/*
	2007.11.29 kobake

	�����ޏ�����
*/

#pragma once


/*!
	auto_ptr �� �z���

	2007.11.29 kobake �쐬
*/
template <class T> class auto_array_ptr{
private:
	typedef auto_array_ptr<T> Me;
public:
	//���
	auto_array_ptr (T*  rhs){ m_array = rhs;          }
	auto_array_ptr (Me& rhs){ m_array = rhs.detach(); }
	Me& operator = (T*  rhs){ reset(rhs         ); return *this; }
	Me& operator = (Me& rhs){ reset(rhs.detach()); return *this; }

	//�j��
	~auto_array_ptr(){ delete[] m_array; }

	//����� (����͂��Ȃ�)
	T* detach()
	{
		T* p = m_array;
		m_array = NULL;
		return p;
	}

	//�ێ��l��ύX����
	void reset(T* p)
	{
		if(m_array==p)return;
		delete[] m_array;
		m_array = p;
	}

	//�ێ��l�ɃA�N�Z�X
	T* get(){ return m_array; }

	//�z��v�f�ɃA�N�Z�X
	T& operator[](int i){ return m_array[i]; }

private:
	T* m_array;
};
