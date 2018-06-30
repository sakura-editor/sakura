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
#ifndef SAKURA_CNATIVE_211120FF_E38A_4AFA_8270_8927A77F4DA79_H_
#define SAKURA_CNATIVE_211120FF_E38A_4AFA_8270_8927A77F4DA79_H_

#include "mem/CMemory.h"

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

#include "mem/CNativeA.h"
#include "mem/CNativeW.h"

#endif /* SAKURA_CNATIVE_211120FF_E38A_4AFA_8270_8927A77F4DA79_H_ */
/*[EOF]*/
