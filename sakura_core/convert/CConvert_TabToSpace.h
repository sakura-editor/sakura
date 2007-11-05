#pragma once

#include "CConvert.h"

class CConvert_TabToSpace : public CConvert{
public:
	CConvert_TabToSpace(int nTabWidth)
	: m_nTabWidth(nTabWidth)
	{
	}

	bool DoConvert(CNativeW2* pcData);

private:
	int m_nTabWidth;
};
