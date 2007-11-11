#pragma once

#include "CConvert.h"

class CConvert_SpaceToTab : public CConvert{
public:
	CConvert_SpaceToTab(int nTabWidth)
	: m_nTabWidth(nTabWidth)
	{
	}

	bool DoConvert(CNativeW* pcData);

private:
	int m_nTabWidth;
};
