#pragma once

#include "CMemory.h"

//※CMemoryをprotect継承することにより、あまり自由にCMemoryを使えないようにしておく
class CNative : protected CMemory{
public:
	//CMemory*ポインタを得る
	CMemory* _GetMemory(){ return static_cast<CMemory*>(this); }
	const CMemory* _GetMemory() const{ return static_cast<const CMemory*>(this); }

public:
	//汎用
	void Clear(); //!< 空っぽにする
};

#include "CNativeA.h"
#include "CNativeW.h"

