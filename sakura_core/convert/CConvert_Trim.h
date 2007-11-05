#pragma once

#include "CConvert.h"


class CConvert_Trim : public CConvert{
public:
	CConvert_Trim(bool bLeft) : m_bLeft(bLeft) { }

public:
	bool DoConvert(CNativeW2* pcData);

private:
	bool m_bLeft;
};
