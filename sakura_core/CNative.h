#pragma once

#include "CMemory.h"

//��CMemory��protect�p�����邱�Ƃɂ��A���܂莩�R��CMemory���g���Ȃ��悤�ɂ��Ă���
class CNative : protected CMemory{
public:
	//CMemory*�|�C���^�𓾂�
	CMemory* _GetMemory(){ return static_cast<CMemory*>(this); }
	const CMemory* _GetMemory() const{ return static_cast<const CMemory*>(this); }

public:
	//�ėp
	void Clear(); //!< ����ۂɂ���
};

#include "CNativeA.h"
#include "CNativeW.h"

