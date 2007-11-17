#pragma once

//!�^�`�F�b�N�̊ɂ������^
class CLaxInteger{
private:
	typedef CLaxInteger Me;

public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CLaxInteger(){ m_value=0; }
	CLaxInteger(const Me& rhs){ m_value=rhs.m_value; }
	CLaxInteger(int value){ m_value=value; }

	//�Öق̕ϊ�
	operator const int&() const{ return m_value; }
	operator       int&()      { return m_value; }

private:
	int m_value;
};

