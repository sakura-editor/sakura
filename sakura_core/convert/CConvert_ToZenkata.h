#pragma once

#include "CConvert.h"


//!できる限り全角カタカナにする
class CConvert_ToZenkata : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};
