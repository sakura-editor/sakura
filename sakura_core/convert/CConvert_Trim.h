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
#ifndef SAKURA_CCONVERT_TRIM_09061551_FDED_4982_A974_FAC9AE9AF040_H_
#define SAKURA_CCONVERT_TRIM_09061551_FDED_4982_A974_FAC9AE9AF040_H_

#include "CConvert.h"


class CConvert_Trim : public CConvert{
public:
	CConvert_Trim(bool bLeft) : m_bLeft(bLeft) { }

public:
	bool DoConvert(CNativeW* pcData);

private:
	bool m_bLeft;
};

#endif /* SAKURA_CCONVERT_TRIM_09061551_FDED_4982_A974_FAC9AE9AF040_H_ */
/*[EOF]*/
