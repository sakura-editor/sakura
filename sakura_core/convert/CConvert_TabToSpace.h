﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CCONVERT_TABTOSPACE_DDBB0EE5_D2FB_4624_9A68_4DD4B81C0744_H_
#define SAKURA_CCONVERT_TABTOSPACE_DDBB0EE5_D2FB_4624_9A68_4DD4B81C0744_H_
#pragma once

#include "CConvert.h"

class CConvert_TabToSpace final : public CConvert{
public:
	CConvert_TabToSpace(int nTabWidth, int nStartColumn, bool bExtEol)
	: m_nTabWidth(nTabWidth), m_nStartColumn(nStartColumn), m_bExtEol(bExtEol)
	{
	}

	bool DoConvert(CNativeW* pcData) override;

private:
	int m_nTabWidth;
	int m_nStartColumn;
	bool m_bExtEol;
};
#endif /* SAKURA_CCONVERT_TABTOSPACE_DDBB0EE5_D2FB_4624_9A68_4DD4B81C0744_H_ */
