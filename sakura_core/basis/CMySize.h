#pragma once

#include <windows.h> //SIZE

class CMySize : public SIZE{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CMySize(){} //���������Ȃ�
	CMySize(int _cx,int _cy){ cx=_cx; cy=_cy; }
	CMySize(const SIZE& rhs){ cx=rhs.cx; cy=rhs.cy; }

	//�֐�
	void Set(int _cx,int _cy){ cx=_cx; cy=_cy; }

	//���Z�q
	bool operator == (const SIZE& rhs) const{ return cx==rhs.cx && cy==rhs.cy; }
	bool operator != (const SIZE& rhs) const{ return !operator==(rhs); }
};
