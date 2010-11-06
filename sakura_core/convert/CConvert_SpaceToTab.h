#pragma once

#include "CConvert.h"

class CConvert_SpaceToTab : public CConvert{
public:
	CConvert_SpaceToTab(int nTabWidth, int nStartColumn)
	: m_nTabWidth(nTabWidth), m_nStartColumn(nStartColumn)
	{
	}

	bool DoConvert(CNativeW* pcData);

private:
	int m_nTabWidth;
	int m_nStartColumn;
};
